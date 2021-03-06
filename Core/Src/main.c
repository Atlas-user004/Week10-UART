/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h> //sprintf
#include <string.h> //strlen
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
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char TxDataBuffer[32] = { 0 };
char RxDataBuffer[32] = { 0 };

uint32_t TimeStamp = 0; //for blinking LED

uint8_t TURN = 0;

//text for operation
char LED_Freq[32] = {0}; //Text that tell freq of LED
char LED_Status[32] = {0}; //Text that tell you LED is On/Off
char LED_Switch[5] = {0}; //memory On/Off
uint16_t Freq = 1; // freq of LED

char Button_Status[32] = {0}; //Text that tell status of button
char Status[10] = {0}; //memory Press/Unpress
uint8_t Status_state[2] = {1}; //memeory button state

uint8_t State = 0;
enum STATEMACHINE
{
	StateMenu = 0,
	StateMenu_WaitInput = 10,
	StateLED = 20,
	StateLED_WaitInput = 30,
	StateButton = 40,
	StateButton_WaitInput = 50
};

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void UARTRecieveAndResponsePolling();
int16_t UARTRecieveIT();
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
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */
  {
	  char temp[]="\r\n\r\n\r\n\r\n\r\nWelcome\r\nPlease type something to start UART.\r\n";
	  HAL_UART_Transmit(&huart2, (uint8_t*)temp, strlen(temp),10);
  }
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  sprintf(Status, "Unpress");
  sprintf(LED_Switch, "Off.");
	while (1)
	{
		/*Method 1 Polling Mode*/

		//UARTRecieveAndResponsePolling();

		/*Method 2 Interrupt Mode*/
		HAL_UART_Receive_IT(&huart2,  (uint8_t*)RxDataBuffer, 32);

		/*Method 2 W/ 1 Char Received*/
		int16_t inputchar = UARTRecieveIT();
		if(inputchar!=-1)
		{
			sprintf(TxDataBuffer, "ReceivedChar:[%c]\r\n\r\n", inputchar);
			HAL_UART_Transmit(&huart2, (uint8_t*)TxDataBuffer, strlen(TxDataBuffer), 1000);
		}
		//State machine
		switch (State)
		{
			case StateMenu:
				{
					char MENU[] = "----------MENU----------\r\n-->Press '0' for LED control.\r\n-->Press '1' for Button status.\r\n------------------------\r\n";
					HAL_UART_Transmit(&huart2, (uint8_t*)MENU, strlen(MENU), 100);
				}
				State = StateMenu_WaitInput;
				break;
			case StateMenu_WaitInput:
				switch (inputchar)
				{
					case 0: //No input. Wait for input
						break;
					case -1: //No input. Wait for input
						break;
					case '0':
						{
							char GotoLED[] = " \r\nGo to LED control.\r\nPlease wait....\r\n \r\n";
							HAL_UART_Transmit(&huart2, (uint8_t*)GotoLED, strlen(GotoLED), 100);
						}
						State = StateLED;
						break;
					case '1':
						{
							char GotoButton[] = " \r\nGo to Button status.\r\nPlease wait....\r\n";
							HAL_UART_Transmit(&huart2, (uint8_t*)GotoButton, strlen(GotoButton), 100);
						}
						State = StateButton;
						break;
					default:
						{
							char ERROR[] = " \r\n!!!ERROR!!!\r\nPlease retype.\r\n \r\n";
							HAL_UART_Transmit(&huart2, (uint8_t*)ERROR, strlen(ERROR), 100);
						}
						State = StateMenu;
						break;
				}
				break;
			case StateLED:
				{
					char LED[] = "------------------------\r\nLED control\r\n-->Press 'a' for speed LED up (+1 Hz).\r\n-->Press 's' for speed LED down (-1 Hz).\r\n-->Press 'd' for On/Off LED.\r\n-->Press 'x' for back to menu.\r\n------------------------\r\n";
					HAL_UART_Transmit(&huart2, (uint8_t*)LED, strlen(LED), 100);
				}
				{
					sprintf(LED_Status, "Status of LED:[%s]\r\n", LED_Switch);
					HAL_UART_Transmit(&huart2, (uint8_t*)LED_Status, strlen(LED_Status), 100);
					sprintf(LED_Freq, "Frequency of LED:[%d]\r\n", Freq);
					HAL_UART_Transmit(&huart2, (uint8_t*)LED_Freq, strlen(LED_Freq), 100);
				}
				State = StateLED_WaitInput;
				break;
			case StateLED_WaitInput:
				switch (inputchar)
				{
					case 0: //No input. Wait for input
						break;
					case -1: //No input. Wait for input
						break;
					case 'a':
						Freq += 1;
						State = StateLED;
						break;
					case 's':
						if(Freq > 1)
						{
							Freq -= 1;
						}
						else
						{
							Freq -= 0;
						}
						State = StateLED;
						break;
					case 'd':
						if(TURN == 0)
						{
							TURN = 1;
							sprintf(LED_Switch, "On.");
						}
						else if(TURN == 1)
						{
							TURN = 0;
							sprintf(LED_Switch, "Off.");
						}
						State = StateLED;
						break;
					case 'x':
						{
							char GoBack[] = " \r\nGo to Menu.\r\nPlease wait....\r\n \r\n";
							HAL_UART_Transmit(&huart2, (uint8_t*)GoBack, strlen(GoBack), 100);
						}
						State = StateMenu;
						break;

					default:
						{
							char ERROR[] = " \r\n!!!ERROR!!!\r\nPlease retype.\r\n \r\n";
							HAL_UART_Transmit(&huart2, (uint8_t*)ERROR, strlen(ERROR), 100);
						}
						State = StateLED;
						break;
				}
				break;
			case StateButton:
				{
					char Button[] = "\r\n------------------------\r\nButton status\r\n-->Press 'x' for back to menu.\r\n------------------------\r\n";
					HAL_UART_Transmit(&huart2, (uint8_t*)Button, strlen(Button), 100);
					sprintf(Button_Status,"Status of Button:[%s]\r\n", Status);
					HAL_UART_Transmit(&huart2, (uint8_t*)Button_Status, strlen(Button_Status), 100);
				}
				State = StateButton_WaitInput;
				break;
			case StateButton_WaitInput:
				Status_state[1] = HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13);
				switch (inputchar)
				{
					case 0: //No input. Wait for input
						if(Status_state[1] != Status_state[0])
						{
							if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 0)
							{
								sprintf(Status, "Press");
							}
							else
							{
								sprintf(Status, "Unpress");
							}
							State = StateButton;
						}
						break;
					case -1: //No input. Wait for input
						if(Status_state[1] != Status_state[0])
						{
							if(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == 0)
							{
								sprintf(Status, "Press");
							}
								else
							{
								sprintf(Status, "Unpress");
							}
							State = StateButton;
						}
						break;
					case 'x':
						{
							char GoBack[] = " \r\nGo to Menu.\r\nPlease wait....\r\n \r\n";
							HAL_UART_Transmit(&huart2, (uint8_t*)GoBack, strlen(GoBack), 10);
						}
						State = StateMenu;
						break;
					default:
						{
							char ERROR[] = " \r\n!!!ERROR!!!\r\nPlease retype.\r\n \r\n";
							HAL_UART_Transmit(&huart2, (uint8_t*)ERROR, strlen(ERROR), 10);
						}
						State = StateButton;
						break;
				}
				Status_state[0] = Status_state[1];
				break;
			default:
				{
					char ERROR[] = " \r\n!!!ERROR!!!\r\nPlease retype.\r\n \r\n";
					HAL_UART_Transmit(&huart2, (uint8_t*)ERROR, strlen(ERROR), 10);
				}
				State = StateMenu;
				break;
		}
		if(TURN == 1)
		{
			if((HAL_GetTick() - TimeStamp) >= (500/Freq))
			{
				TimeStamp = HAL_GetTick();
				HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
			}
		}
		else
		{
			HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
		}
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void UARTRecieveAndResponsePolling()
{
	char Recieve[32]={0};

	HAL_UART_Receive(&huart2, (uint8_t*)Recieve, 32, 1000);

	sprintf(TxDataBuffer, "Received:[%s]\r\n", Recieve);
	HAL_UART_Transmit(&huart2, (uint8_t*)TxDataBuffer, strlen(TxDataBuffer), 1000);

}


int16_t UARTRecieveIT()
{
	static uint32_t dataPos =0;
	int16_t data=-1;
	if(huart2.RxXferSize - huart2.RxXferCount!=dataPos)
	{
		data=RxDataBuffer[dataPos];
		dataPos= (dataPos+1)%huart2.RxXferSize;
	}
	return data;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	sprintf(TxDataBuffer, "Received:[%s]\r\n\r\n", RxDataBuffer);
	HAL_UART_Transmit(&huart2, (uint8_t*)TxDataBuffer, strlen(TxDataBuffer), 1000);
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
