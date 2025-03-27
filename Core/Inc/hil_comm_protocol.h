/**
 * @file hil_comm_protocol.h
 * @brief Communication Protocol for Wiseled_LBR Hardware-in-the-Loop (HIL) Testing
 */

#ifndef WISELED_HIL_COMM_PROTOCOL_H
#define WISELED_HIL_COMM_PROTOCOL_H

#include <stdint.h>

// Protocol Constants
#define HIL_START_MARKER    0xAA
#define HIL_END_MARKER      0x55

// Light/Channel Identifiers
typedef enum {
    LIGHT_1_WHITE = '1',
    LIGHT_2_GREEN = '2',
    LIGHT_3_RED   = '3'
} HILLightChannel;

// Command Types
typedef enum {
    CMD_GET = 'G',
    CMD_SET = 'S',
    CMD_PING = 'P'
} HILCommandType;

// Function/Signal Types
typedef enum {
    SIGNAL_PWM_INPUT   = 'P',
    SIGNAL_CURRENT     = 'C',
    SIGNAL_TEMPERATURE = 'T',
    SIGNAL_SYSTEM      = 'S'
} HILSignalType;

// Response Status
typedef enum {
    RESPONSE_OK    = 'O',
    RESPONSE_ERROR = 'N'
} HILResponseStatus;

// HIL Message Structure
typedef struct {
    uint8_t     start;      // Start marker (0xAA)
    char        cmd;        // Command type ('G', 'S', 'P')
    char        light;      // Light channel ('1', '2', '3') or system
    char        function;   // Signal type
    uint16_t    value;      // 16-bit value for signal generation/reading
    uint8_t     checksum;   // Simple XOR checksum
    uint8_t     end;        // End marker (0x55)
} HILMessage;


typedef struct {
    uint16_t current_capture;     // Most recent capture value
    uint16_t last_capture;        // Previous capture value
    uint16_t pulse_width;         // Pulse width in timer ticks
    uint16_t period;              // Total period in timer ticks
    uint16_t duty_cycle;          // Duty cycle (0-1000 range)
    uint8_t  capture_complete;    // Flag indicating a complete capture cycle
} PWMCaptureData;

// Declare the global array as an extern
extern PWMCaptureData pwm_capture[3];
extern uint8_t rx_byte;

// Function Prototypes for Message Processing
uint8_t HIL_CalculateChecksum(const HILMessage* msg);
uint8_t HIL_ValidateChecksum(const HILMessage* msg);
void HIL_ProcessGetCommand(const HILMessage* msg);
void HIL_ProcessSetCommand(const HILMessage* msg);
void HIL_ProcessPingCommand(const HILMessage* msg);
void HIL_SendResponse(HILResponseStatus status, const HILMessage* original_msg);

// New Interrupt-Driven UART Functions
/**
 * Start UART reception in interrupt mode
 */
void HIL_StartUARTReception(void);

/**
 * Stop UART reception interrupt
 */
void HIL_StopUARTReception(void);

/**
 * Process messages from the reception buffer
 * Call this in the main loop or a low-priority task
 */
void HIL_ProcessReceivedMessages(void);

#endif // WISELED_HIL_COMM_PROTOCOL_H
