/**
 * @file hil_comm_protocol.c
 * @brief Implementation of HIL Communication Protocol
 */

#include "hil_comm_protocol.h"
#include "main.h"
#include "usart.h"
#include <string.h>

// Global UART handle (defined in main.c)
extern UART_HandleTypeDef huart3;

// Firmware version
#define FIRMWARE_VERSION 0x0100  // Version 1.00

// Buffer for UART reception
#define UART_RX_BUFFER_SIZE 16
typedef struct {
    HILMessage buffer[UART_RX_BUFFER_SIZE];
    volatile uint8_t head;
    volatile uint8_t tail;
    volatile uint8_t count;
} UARTRingBuffer;

// UART Ring Buffer
static UARTRingBuffer uart_rx_buffer = {0};

// UART message reception state machine
typedef enum {
    WAIT_START_MARKER,
    RECEIVING_MESSAGE,
    MESSAGE_COMPLETE
} UARTRxState;

static struct {
    UARTRxState state;
    HILMessage current_message;
    uint8_t bytes_received;
    uint8_t* rx_ptr;
} uart_rx_context = {
    .state = WAIT_START_MARKER,
    .bytes_received = 0
};

void HIL_ProcessSetCommand(const HILMessage* msg) {
    HILMessage response = {0};

    // Set response start and end markers
    response.start = HIL_START_MARKER;
    response.end = HIL_END_MARKER;

    // Validate message
    if (!HIL_ValidateChecksum(msg)) {
        response.cmd = RESPONSE_ERROR;
        HIL_SendResponse(RESPONSE_ERROR, msg);
        return;
    }

    // Process SET command based on light and function
    uint8_t light_index = msg->light - '1'; // Convert char to 0-based index

    if (light_index < 3) {
        switch (msg->function) {
            case SIGNAL_PWM_INPUT:
                // Reject SET command for PWM input (capture-only signal)
                response.cmd = RESPONSE_ERROR;
                break;

            case SIGNAL_CURRENT:
                // Set current simulation value
                if (Analog_SetCurrentSimulation(light_index, msg->value)) {
                    response.cmd = RESPONSE_OK;
                } else {
                    response.cmd = RESPONSE_ERROR;
                }
                break;

            case SIGNAL_TEMPERATURE:
                // Set temperature simulation value
                if (Analog_SetTemperatureSimulation(light_index, msg->value)) {
                    response.cmd = RESPONSE_OK;
                } else {
                    response.cmd = RESPONSE_ERROR;
                }
                break;

            default:
                response.cmd = RESPONSE_ERROR;
                break;
        }
    } else {
        response.cmd = RESPONSE_ERROR;
    }

    // Calculate checksum
    response.checksum = HIL_CalculateChecksum(&response);

    // Send response
    HIL_SendResponse(response.cmd, &response);
}

void HIL_ProcessGetCommand(const HILMessage* msg) {
    HILMessage response = {0};

    // Set response start and end markers
    response.start = HIL_START_MARKER;
    response.end = HIL_END_MARKER;

    // Validate message
    if (!HIL_ValidateChecksum(msg)) {
        response.cmd = RESPONSE_ERROR;
        HIL_SendResponse(RESPONSE_ERROR, msg);
        return;
    }

    // Process GET command based on light and function
    uint8_t light_index = msg->light - '1';  // Convert char to 0-based index

    if (light_index < 3) {
        switch (msg->function) {
            case SIGNAL_PWM_INPUT:
                // Retrieve PWM capture data for specific light
                if (pwm_capture[light_index].capture_complete) {
                    response.cmd = msg->light;
                    response.function = SIGNAL_PWM_INPUT;

                    // Return duty cycle (0-100 range)
                    response.value = pwm_capture[light_index].duty_cycle;

                    // Clear capture complete flag
                    pwm_capture[light_index].capture_complete = 0;
                } else {
                    response.cmd = RESPONSE_ERROR;
                }
                break;

            case SIGNAL_CURRENT:
                // Return current PWM value
                {
                    uint16_t pwm_value = Analog_GetCurrentPWM(light_index);
                    if (pwm_value != 0xFFFF) {
                        response.cmd = msg->light;
                        response.function = SIGNAL_CURRENT;
                        response.value = pwm_value;
                    } else {
                        response.cmd = RESPONSE_ERROR;
                    }
                }
                break;

            case SIGNAL_TEMPERATURE:
                // Return temperature PWM value
                {
                    uint16_t pwm_value = Analog_GetTemperaturePWM(light_index);
                    if (pwm_value != 0xFFFF) {
                        response.cmd = msg->light;
                        response.function = SIGNAL_TEMPERATURE;
                        response.value = pwm_value;
                    } else {
                        response.cmd = RESPONSE_ERROR;
                    }
                }
                break;

            default:
                response.cmd = RESPONSE_ERROR;
                break;
        }
    } else {
        response.cmd = RESPONSE_ERROR;
    }

    // Calculate checksum
    response.checksum = HIL_CalculateChecksum(&response);

    // Send response
    HIL_SendResponse(RESPONSE_OK, &response);
}

/**
 * Calculate checksum using XOR of all data bytes
 * @param msg Pointer to HIL message
 * @return Calculated checksum
 */
uint8_t HIL_CalculateChecksum(const HILMessage* msg) {
    uint8_t checksum = 0;

    // XOR all bytes of the message except start, checksum, and end markers
    checksum ^= msg->cmd;
    checksum ^= msg->light;
    checksum ^= msg->function;

    // XOR each byte of the 16-bit value
    checksum ^= (msg->value & 0xFF);
    checksum ^= ((msg->value >> 8) & 0xFF);

    return checksum;
}

/**
 * Validate message checksum
 * @param msg Pointer to HIL message
 * @return 1 if checksum is valid, 0 otherwise
 */
uint8_t HIL_ValidateChecksum(const HILMessage* msg) {
    return (msg->checksum == HIL_CalculateChecksum(msg));
}

/**
 * Add message to ring buffer
 * @param msg Pointer to HIL message
 * @return 1 if successful, 0 if buffer full
 */
static uint8_t add_to_buffer(const HILMessage* msg) {
    if (uart_rx_buffer.count >= UART_RX_BUFFER_SIZE) {
        return 0;  // Buffer full
    }

    uint8_t tail = uart_rx_buffer.tail;
    uart_rx_buffer.buffer[tail] = *msg;
    uart_rx_buffer.tail = (tail + 1) % UART_RX_BUFFER_SIZE;
    uart_rx_buffer.count++;

    return 1;
}

/**
 * Process received complete message
 */
static void process_received_message() {
    // Validate start and end markers
    if (uart_rx_context.current_message.start != HIL_START_MARKER ||
        uart_rx_context.current_message.end != HIL_END_MARKER) {
        // Send error response
        HIL_SendResponse(RESPONSE_ERROR, NULL);
        return;
    }

    // Add to processing buffer
    if (!add_to_buffer(&uart_rx_context.current_message)) {
        // Buffer full, send error
        HIL_SendResponse(RESPONSE_ERROR, NULL);
        return;
    }

    // Reset reception state
    uart_rx_context.state = WAIT_START_MARKER;
    uart_rx_context.bytes_received = 0;
}

/**
 * Send HIL response message
 * @param status Response status
 * @param original_msg Original received message (for correlation)
 */
void HIL_SendResponse(HILResponseStatus status, const HILMessage* original_msg) {
    HILMessage response = {0};

    // Always set start and end markers
    response.start = HIL_START_MARKER;
    response.end = HIL_END_MARKER;

    // If a complete message is provided, use it
    if (original_msg) {
        response = *original_msg;

        // Ensure markers are always set
        response.start = HIL_START_MARKER;
        response.end = HIL_END_MARKER;
    }

    response.cmd = status;

    // Recalculate checksum to ensure it's correct
    response.checksum = HIL_CalculateChecksum(&response);

    // Transmit response via UART
    HAL_UART_Transmit(&huart3, (uint8_t*)&response, sizeof(HILMessage), 100);
}

/**
 * Process messages from the reception buffer
 * Call this in the main loop or a low-priority task
 */
void HIL_ProcessReceivedMessages(void) {
    // Process all messages in the buffer
    while (uart_rx_buffer.count > 0) {
    	// Additional error checking
    	if (uart_rx_buffer.count > UART_RX_BUFFER_SIZE) {
    		// Emergency reset of buffer
    		uint8_t error_msg[] = "UART Buffer Overflow\r\n";
    		HAL_UART_Transmit(&huart3, error_msg, sizeof(error_msg), 100);

    		// Reset buffer
    		uart_rx_buffer.count = 0;
    		uart_rx_buffer.head = 0;
    		uart_rx_buffer.tail = 0;
    		break;
    	}
        // Get message from buffer
        HILMessage msg = uart_rx_buffer.buffer[uart_rx_buffer.head];

        // Remove from buffer
        uart_rx_buffer.head = (uart_rx_buffer.head + 1) % UART_RX_BUFFER_SIZE;
        uart_rx_buffer.count--;

        // Process message based on command type
        switch (msg.cmd) {
            case CMD_GET:
                HIL_ProcessGetCommand(&msg);
                break;

            case CMD_SET:
                HIL_ProcessSetCommand(&msg);
                break;

            case CMD_PING:
                HIL_ProcessPingCommand(&msg);
                break;

            default:
                HIL_SendResponse(RESPONSE_ERROR, &msg);
                break;
        }
    }
}

/**
 * UART Receive Interrupt Handler
 * @param huart Pointer to UART handle
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart == &huart3) {
        rx_byte = huart->Instance->DR;  // Read directly from data register

        switch (uart_rx_context.state) {
            case WAIT_START_MARKER:
                if (rx_byte == HIL_START_MARKER) {
                    uart_rx_context.current_message.start = rx_byte;
                    uart_rx_context.state = RECEIVING_MESSAGE;
                    uart_rx_context.rx_ptr = (uint8_t*)&uart_rx_context.current_message + 1;
                    uart_rx_context.bytes_received = 1;
                }
                break;

            case RECEIVING_MESSAGE:
                *uart_rx_context.rx_ptr++ = rx_byte;
                uart_rx_context.bytes_received++;

                if (uart_rx_context.bytes_received == sizeof(HILMessage)) {
                    uart_rx_context.state = MESSAGE_COMPLETE;
                    process_received_message();
                }
                break;

            case MESSAGE_COMPLETE:
                uart_rx_context.state = WAIT_START_MARKER;
                uart_rx_context.bytes_received = 0;
                break;
        }

        HAL_UART_Receive_IT(&huart3, &rx_byte, 1);

    }
}

/**
 * Process ping command
 * @param msg Pointer to HIL message
 */
void HIL_ProcessPingCommand(const HILMessage* msg) {
    HILMessage response = {0};

    // Set response start and end markers
    response.start = HIL_START_MARKER;
    response.end = HIL_END_MARKER;

    // Validate message
    if (!HIL_ValidateChecksum(msg)) {
        response.cmd = RESPONSE_ERROR;
        HIL_SendResponse(RESPONSE_ERROR, msg);
        return;
    }

    // Prepare ping response
    // Explicitly set firmware version
    response.cmd = RESPONSE_OK;
    response.light = 'S';  // System-level response
    response.function = SIGNAL_SYSTEM;

    // Explicitly set firmware version (1.00)
    response.value = 0x0100;  // Major version 1, Minor version 0

    response.checksum = HIL_CalculateChecksum(&response);

    // Send response
    HIL_SendResponse(RESPONSE_OK, &response);
}

/**
 * Start UART reception in interrupt mode
 */
void HIL_StartUARTReception(void) {
    // Enable UART receive interrupt
	HAL_UART_Receive_IT(&huart3, &rx_byte, 1);
}

/**
 * Stop UART reception interrupt
 */
void HIL_StopUARTReception(void) {
    // Disable UART receive interrupt
    __HAL_UART_DISABLE_IT(&huart3, UART_IT_RXNE);
}
