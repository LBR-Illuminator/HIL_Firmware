/**
 * @file pwm_capture.h
 * @brief PWM Input Capture handling for Wiseled_LBR HIL
 */

#ifndef PWM_CAPTURE_H
#define PWM_CAPTURE_H

#include "main.h"

/**
 * @brief Initialize PWM input capture
 */
void PWM_Capture_Init(void);

/**
 * @brief Process PWM input capture events
 * This function is called from the TIM1 input capture interrupt handler
 * @param htim Pointer to the TIM_HandleTypeDef structure
 */
void PWM_Capture_ProcessEvent(TIM_HandleTypeDef *htim);

/**
 * @brief Start PWM input capture on all channels
 */
void PWM_Capture_Start(void);

/**
 * @brief Stop PWM input capture on all channels
 */
void PWM_Capture_Stop(void);

#endif /* PWM_CAPTURE_H */
