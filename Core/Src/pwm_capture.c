/**
 * @file pwm_capture.c
 * @brief PWM Input Capture handling for Wiseled_LBR HIL
 */

#include "pwm_capture.h"
#include "tim.h"
#include "hil_comm_protocol.h"

// Global array to store capture data for each channel (defined in main.c before move)
PWMCaptureData pwm_capture[3] = {0};

// Private variables
static uint32_t rising_edge[3] = {0, 0, 0};
static uint32_t falling_edge[3] = {0, 0, 0};
static uint8_t capture_state[3] = {0, 0, 0}; // 0: waiting for rising, 1: waiting for falling

#define DUTY_CYCLE_SCALER 100

/**
 * @brief Initialize PWM input capture
 */
void PWM_Capture_Init(void) {
    // Reset all capture data
    for(int i = 0; i < 3; i++) {
        pwm_capture[i].current_capture = 0;
        pwm_capture[i].last_capture = 0;
        pwm_capture[i].pulse_width = 0;
        pwm_capture[i].period = 0;
        pwm_capture[i].duty_cycle = 0;
        pwm_capture[i].capture_complete = 0;

        rising_edge[i] = 0;
        falling_edge[i] = 0;
        capture_state[i] = 0;
    }
}

/**
 * @brief Start PWM input capture on all channels
 */
void PWM_Capture_Start(void) {
    // Start PWM input capture interrupt for TIM1
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
    HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
}

/**
 * @brief Stop PWM input capture on all channels
 */
void PWM_Capture_Stop(void) {
    // Stop PWM input capture interrupt for TIM1
    HAL_TIM_IC_Stop_IT(&htim1, TIM_CHANNEL_1);
    HAL_TIM_IC_Stop_IT(&htim1, TIM_CHANNEL_2);
    HAL_TIM_IC_Stop_IT(&htim1, TIM_CHANNEL_3);
}

/**
 * @brief Process PWM input capture events
 * This function is the implementation of what was HAL_TIM_IC_CaptureCallback in main.c
 * @param htim Pointer to the TIM_HandleTypeDef structure
 */
void PWM_Capture_ProcessEvent(TIM_HandleTypeDef *htim) {
    // Ensure this is TIM1 input capture
    if (htim->Instance == TIM1) {
        // Process channel 1
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (capture_state[0] == 0) { // Rising edge
                rising_edge[0] = capture_value;
                capture_state[0] = 1;

                // Configure for falling edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_FALLING);
            } else { // Falling edge
                falling_edge[0] = capture_value;
                capture_state[0] = 0;

                // Calculate pulse width
                if (falling_edge[0] >= rising_edge[0]) {
                    pwm_capture[0].pulse_width = falling_edge[0] - rising_edge[0];
                } else {
                    // Handle timer overflow
                    pwm_capture[0].pulse_width = ((htim->Init.Period + 1) - rising_edge[0]) + falling_edge[0];
                }

                // Calculate period (will be updated on next rising edge)
                if (pwm_capture[0].last_capture <= rising_edge[0]) {
                    pwm_capture[0].period = rising_edge[0] - pwm_capture[0].last_capture;
                } else {
                    // Handle timer overflow
                    pwm_capture[0].period = ((htim->Init.Period + 1) - pwm_capture[0].last_capture) + rising_edge[0];
                }

                // Store current rising edge for next period calculation
                pwm_capture[0].last_capture = rising_edge[0];

                // Calculate duty cycle (0-1000 range)
                if (pwm_capture[0].period > 0) {
                    pwm_capture[0].duty_cycle = (pwm_capture[0].pulse_width * DUTY_CYCLE_SCALER) / pwm_capture[0].period;
                }

                // Mark capture as complete
                pwm_capture[0].capture_complete = 1;

                // Configure for next rising edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_1, TIM_INPUTCHANNELPOLARITY_RISING);
            }
        }

        // Process channel 2 (similar logic)
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
            uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

            if (capture_state[1] == 0) { // Rising edge
                rising_edge[1] = capture_value;
                capture_state[1] = 1;

                // Configure for falling edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_FALLING);
            } else { // Falling edge
                falling_edge[1] = capture_value;
                capture_state[1] = 0;

                // Calculate pulse width
                if (falling_edge[1] >= rising_edge[1]) {
                    pwm_capture[1].pulse_width = falling_edge[1] - rising_edge[1];
                } else {
                    // Handle timer overflow
                    pwm_capture[1].pulse_width = ((htim->Init.Period + 1) - rising_edge[1]) + falling_edge[1];
                }

                // Calculate period (will be updated on next rising edge)
                if (pwm_capture[1].last_capture <= rising_edge[1]) {
                    pwm_capture[1].period = rising_edge[1] - pwm_capture[1].last_capture;
                } else {
                    // Handle timer overflow
                    pwm_capture[1].period = ((htim->Init.Period + 1) - pwm_capture[1].last_capture) + rising_edge[1];
                }

                // Store current rising edge for next period calculation
                pwm_capture[1].last_capture = rising_edge[1];

                // Calculate duty cycle (0-1000 range)
                if (pwm_capture[1].period > 0) {
                    pwm_capture[1].duty_cycle = (pwm_capture[1].pulse_width * DUTY_CYCLE_SCALER) / pwm_capture[1].period;
                }

                // Mark capture as complete
                pwm_capture[1].capture_complete = 1;

                // Configure for next rising edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_2, TIM_INPUTCHANNELPOLARITY_RISING);
            }
        }

        // Process channel 3 (similar logic)
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
            uint32_t capture_value = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

            if (capture_state[2] == 0) { // Rising edge
                rising_edge[2] = capture_value;
                capture_state[2] = 1;

                // Configure for falling edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_FALLING);
            } else { // Falling edge
                falling_edge[2] = capture_value;
                capture_state[2] = 0;

                // Calculate pulse width
                if (falling_edge[2] >= rising_edge[2]) {
                    pwm_capture[2].pulse_width = falling_edge[2] - rising_edge[2];
                } else {
                    // Handle timer overflow
                    pwm_capture[2].pulse_width = ((htim->Init.Period + 1) - rising_edge[2]) + falling_edge[2];
                }

                // Calculate period (will be updated on next rising edge)
                if (pwm_capture[2].last_capture <= rising_edge[2]) {
                    pwm_capture[2].period = rising_edge[2] - pwm_capture[2].last_capture;
                } else {
                    // Handle timer overflow
                    pwm_capture[2].period = ((htim->Init.Period + 1) - pwm_capture[2].last_capture) + rising_edge[2];
                }

                // Store current rising edge for next period calculation
                pwm_capture[2].last_capture = rising_edge[2];

                // Calculate duty cycle (0-1000 range)
                if (pwm_capture[2].period > 0) {
                    pwm_capture[2].duty_cycle = (pwm_capture[2].pulse_width * DUTY_CYCLE_SCALER) / pwm_capture[2].period;
                }

                // Mark capture as complete
                pwm_capture[2].capture_complete = 1;

                // Configure for next rising edge
                __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_3, TIM_INPUTCHANNELPOLARITY_RISING);
            }
        }
    }
}
