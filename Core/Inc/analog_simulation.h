/**
 * @file analog_simulation.h
 * @brief Analog Signal Simulation for Wiseled_LBR HIL
 *
 * This file contains functions for simulating analog signals
 * (current and temperature) for the Wiseled_LBR HIL system.
 */

#ifndef ANALOG_SIMULATION_H
#define ANALOG_SIMULATION_H

#include "main.h"
#include "tim.h"

/**
 * @brief Initialize analog simulation components
 */
void Analog_Simulation_Init(void);

/**
 * @brief Start PWM outputs for analog simulation
 */
void Analog_Simulation_Start(void);

/**
 * @brief Stop PWM outputs for analog simulation
 */
void Analog_Simulation_Stop(void);

/**
 * @brief Set simulated current value for a specific light
 * @param light_index Light index (0-2)
 * @param current_value Current value in milliamps (0-33000 mA)
 * @return 1 if successful, 0 otherwise
 */
uint8_t Analog_SetCurrentSimulation(uint8_t light_index, uint16_t current_value);

/**
 * @brief Get current PWM value for a specific light
 * @param light_index Light index (0-2)
 * @return Current PWM value (0-1023) or 0xFFFF if invalid index
 */
uint16_t Analog_GetCurrentPWM(uint8_t light_index);

/**
 * @brief Set simulated temperature value for a specific light
 * @param light_index Light index (0-2)
 * @param temperature_value Temperature value in tenths of a degree (0-3300 = 0-330.0°C)
 * @return 1 if successful, 0 otherwise
 */
uint8_t Analog_SetTemperatureSimulation(uint8_t light_index, uint16_t temperature_value);

/**
 * @brief Get temperature PWM value for a specific light
 * @param light_index Light index (0-2)
 * @return Temperature PWM value (0-1023) or 0xFFFF if invalid index
 */
uint16_t Analog_GetTemperaturePWM(uint8_t light_index);

#endif /* ANALOG_SIMULATION_H */
