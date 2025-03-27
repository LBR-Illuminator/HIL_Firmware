/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "usb_otg.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint8_t rx_byte;
// Global array to store capture data for each channel
PWMCaptureData pwm_capture[3] = {0};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_USART3_UART_Init();
  MX_USB_OTG_FS_PCD_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */

  HAL_UART_Transmit(&huart3, (uint8_t*)"Wiseled_LBR HIL System Initialized\r\n", 36, 100);

  // Start PWM generation for current and temperature simulation
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_4);

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_3);

  // Start PWM input capture for TIM1
  HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_1);
  HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_2);
  HAL_TIM_IC_Start(&htim1, TIM_CHANNEL_3);
  __HAL_TIM_ENABLE_IT(&htim1, TIM_IT_CC1);

  // Start PWM input capture interrupt for TIM1
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_1);
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_2);
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);

  // Start UART reception in interrupt mode
  HIL_StartUARTReception();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim) {
    // Ensure this is TIM1 input capture
    if (htim->Instance == TIM1) {
        // Static variables to track first capture
        static uint8_t is_first_capture[3] = {1, 1, 1};

        // Capture data based on active channel
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1) {
            // Read current capture value
            uint16_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);

            if (!is_first_capture[0]) {
                // Calculate pulse width
                pwm_capture[0].pulse_width = current_capture > pwm_capture[0].last_capture
                    ? current_capture - pwm_capture[0].last_capture
                    : (0xFFFF - pwm_capture[0].last_capture) + current_capture;

                // Calculate period
                pwm_capture[0].period = pwm_capture[0].pulse_width;

                // Calculate duty cycle (0-1000 range)
                if (pwm_capture[0].period > 0) {
                    pwm_capture[0].duty_cycle =
                        (pwm_capture[0].pulse_width * 1000) / pwm_capture[0].period;
                }

                // Mark capture as complete
                pwm_capture[0].capture_complete = 1;
            }

            // Store current capture for next iteration
            pwm_capture[0].last_capture = current_capture;
            is_first_capture[0] = 0;
        }

        // Similar logic for Channel 2
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_2) {
            uint16_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_2);

            if (!is_first_capture[1]) {
                pwm_capture[1].pulse_width = current_capture > pwm_capture[1].last_capture
                    ? current_capture - pwm_capture[1].last_capture
                    : (0xFFFF - pwm_capture[1].last_capture) + current_capture;

                pwm_capture[1].period = pwm_capture[1].pulse_width;

                if (pwm_capture[1].period > 0) {
                    pwm_capture[1].duty_cycle =
                        (pwm_capture[1].pulse_width * 1000) / pwm_capture[1].period;
                }

                pwm_capture[1].capture_complete = 1;
            }

            pwm_capture[1].last_capture = current_capture;
            is_first_capture[1] = 0;
        }

        // Similar logic for Channel 3
        if (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_3) {
            uint16_t current_capture = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_3);

            if (!is_first_capture[2]) {
                pwm_capture[2].pulse_width = current_capture > pwm_capture[2].last_capture
                    ? current_capture - pwm_capture[2].last_capture
                    : (0xFFFF - pwm_capture[2].last_capture) + current_capture;

                pwm_capture[2].period = pwm_capture[2].pulse_width;

                if (pwm_capture[2].period > 0) {
                    pwm_capture[2].duty_cycle =
                        (pwm_capture[2].pulse_width * 1000) / pwm_capture[2].period;
                }

                pwm_capture[2].capture_complete = 1;
            }

            pwm_capture[2].last_capture = current_capture;
            is_first_capture[2] = 0;
        }
    }
}

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
