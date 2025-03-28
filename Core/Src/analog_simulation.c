/**
 * @file analog_simulation.c
 * @brief Analog Signal Simulation Implementation for Wiseled_LBR HIL
 */

#include "analog_simulation.h"

// Private constants
#define CURRENT_MAX_VALUE      33000    // 33A in mA
#define TEMPERATURE_MAX_VALUE  3300     // 330.0°C in tenths of a degree
#define PWM_MAX_VALUE          1023     // 10-bit PWM resolution (0-1023)

// Store current PWM values
static uint16_t current_pwm_values[3] = {0, 0, 0};
static uint16_t temperature_pwm_values[3] = {0, 0, 0};

/**
 * @brief Initialize analog simulation components
 */
void Analog_Simulation_Init(void) {
    // Reset all simulation values
    for (int i = 0; i < 3; i++) {
        current_pwm_values[i] = 0;
        temperature_pwm_values[i] = 0;
    }

    // Set initial PWM values to 0
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, 0); // Light 1 Current
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, 0); // Light 2 Current
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, 0); // Light 3 Current

    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, 0); // Light 1 Temperature
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, 0); // Light 2 Temperature
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, 0); // Light 3 Temperature
}

/**
 * @brief Start PWM outputs for analog simulation
 */
void Analog_Simulation_Start(void) {
    // Start PWM generation for current simulation
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1); // Light 1 Current
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3); // Light 2 Current
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4); // Light 3 Current

    // Start PWM generation for temperature simulation
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1); // Light 1 Temperature
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2); // Light 2 Temperature
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3); // Light 3 Temperature
}

/**
 * @brief Stop PWM outputs for analog simulation
 */
void Analog_Simulation_Stop(void) {
    // Stop PWM generation for current simulation
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_1); // Light 1 Current
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_3); // Light 2 Current
    HAL_TIM_PWM_Stop(&htim2, TIM_CHANNEL_4); // Light 3 Current

    // Stop PWM generation for temperature simulation
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1); // Light 1 Temperature
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_2); // Light 2 Temperature
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_3); // Light 3 Temperature
}

/**
 * @brief Set simulated current value for a specific light
 * @param light_index Light index (0-2)
 * @param current_value Current value in milliamps (0-33000 mA)
 * @return 1 if successful, 0 otherwise
 */
uint8_t Analog_SetCurrentSimulation(uint8_t light_index, uint16_t current_value) {
    // Validate input parameters
    if (light_index > 2 || current_value > CURRENT_MAX_VALUE) {
        return 0;
    }

    // Scale current value to PWM value (0-1023)
    // 0mA = 0, 33000mA = 1023
    uint32_t pwm_value = (current_value * PWM_MAX_VALUE) / CURRENT_MAX_VALUE;

    // Store PWM value
    current_pwm_values[light_index] = pwm_value;

    // Set PWM value for the corresponding timer channel
    switch (light_index) {
        case 0: // Light 1
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_1, pwm_value);
            break;

        case 1: // Light 2
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm_value);
            break;

        case 2: // Light 3
            __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_4, pwm_value);
            break;

        default:
            return 0;
    }

    return 1;
}

/**
 * @brief Get current PWM value for a specific light
 * @param light_index Light index (0-2)
 * @return Current PWM value (0-1023) or 0xFFFF if invalid index
 */
uint16_t Analog_GetCurrentPWM(uint8_t light_index) {
    if (light_index > 2) {
        return 0xFFFF; // Invalid value
    }

    return current_pwm_values[light_index];
}

/**
 * @brief Set simulated temperature value for a specific light
 * @param light_index Light index (0-2)
 * @param temperature_value Temperature value in tenths of a degree (0-3300 = 0-330.0°C)
 * @return 1 if successful, 0 otherwise
 */
uint8_t Analog_SetTemperatureSimulation(uint8_t light_index, uint16_t temperature_value) {
    // Validate input parameters
    if (light_index > 2 || temperature_value > TEMPERATURE_MAX_VALUE) {
        return 0;
    }

    // Scale temperature value to PWM value (0-1023)
    // 0°C = 0, 330.0°C = 1023
    uint32_t pwm_value = (temperature_value * PWM_MAX_VALUE) / TEMPERATURE_MAX_VALUE;

    // Store PWM value
    temperature_pwm_values[light_index] = pwm_value;

    // Set PWM value for the corresponding timer channel
    switch (light_index) {
        case 0: // Light 1
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_1, pwm_value);
            break;

        case 1: // Light 2
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pwm_value);
            break;

        case 2: // Light 3
            __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_3, pwm_value);
            break;

        default:
            return 0;
    }

    return 1;
}

/**
 * @brief Get temperature PWM value for a specific light
 * @param light_index Light index (0-2)
 * @return Temperature PWM value (0-1023) or 0xFFFF if invalid index
 */
uint16_t Analog_GetTemperaturePWM(uint8_t light_index) {
    if (light_index > 2) {
        return 0xFFFF; // Invalid value
    }

    return temperature_pwm_values[light_index];
}
