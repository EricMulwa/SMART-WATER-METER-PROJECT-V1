/*
 *                           SMART-WATER-METER PROJECT  (c) ENG. ERIC MULWA
***************************************************************************************************************
  * @file           : main.c                  426 Lines of COde
  * @brief          : Main program body
  **************************************************************************************************************
  * Copyright (c) ENG. ERIC MULWA
  * All rights reserved.
  *
  *This Project is the Work of ERIC MULWA done at CDED Center, DeKUT During his Final Year External Attachment.
  *
  *The Project is a Smart-Water-Meter that can read the volume of water consumed per day in m^3, Display the volume
  *in the 7-segment display (HT1621) in real time and Send the data wirelessly through an SMS to the User
  * & to the Water Provider servers for billing.
  *
  *************************************************************************************************************
  */
/* USER CODE END Header */
/* Includes --------------------------------------------------------------------------------------------------*/
#include "main.h"
/* Private includes ------------------------------------------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include <string.h>
#include "HT1621.h"
/* USER CODE END Includes */

/* Private typedef -------------------------------------------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define --------------------------------------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro ---------------------------------------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables -----------------------------------------------------------------------------------------*/
SPI_HandleTypeDef hspi2;

UART_HandleTypeDef huart1;

/* USER CODE BEGIN PV */
const uint8_t displ_size = 4;

//Motor Control with GSM section 1
#define PREF_SMS_STORAGE "\"SM\""
char ATcommand[80];
uint8_t ATisOK = 0;
uint8_t slot = 0;
uint8_t rx_buffer[100] = {0};
uint8_t rx_index = 0;
uint8_t rx_data;
/* USER CODE END PV */

/* Private function prototypes ------------------------------------------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI2_Init(void);
static void MX_USART1_UART_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ----------------------------------------------------------------------------------------*/
/* USER CODE BEGIN 0 */
volatile uint32_t pulseCount = 0; //Read interrupt pulses from the Pin

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration----------------------------------------------------------------------------------------*/

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
  MX_SPI2_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */
  HT1621_Init(displ_size);
  HT1621_Clear();
  HT1621_String("HELO", 9);
  HAL_Delay(1000);

  //Motor control with GSM section 2
  // Wait until getting response OK to AT
  while(!ATisOK)
  {
    sprintf(ATcommand,"AT\r\n");
    HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
    HAL_UART_Receive (&huart1, rx_buffer, 100, 100);
    HAL_Delay(1000);
    if(strstr((char *)rx_buffer,"OK"))
    {
      ATisOK = 1;
    }
    HAL_Delay(1000);
    memset(rx_buffer,0,sizeof(rx_buffer));
  }

  // Send AT+CMGF=1
  sprintf(ATcommand,"AT+CMGF=1\r\n");
  HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
  HAL_UART_Receive (&huart1, rx_buffer, 100, 100);
  HAL_Delay(1000);
  memset(rx_buffer,0,sizeof(rx_buffer));
  // Send AT+CNMI=2,1 to enable notification when SMS arrives
  sprintf(ATcommand,"AT+CNMI=2,1\r\n");
  HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
  // Enabling interrupt receive
  HAL_UART_Receive_IT(&huart1,&rx_data,1);// receive data (one character only)
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
//Display the water volume in M^3 consumed by the user
	  float Volume = pulseCount * 0.003165;
	  HT1621_Digit(Volume, 0, false, 6);
	  HAL_Delay(500);
//Sending Volume of water Consumed SMS
	  char mobileNumber1[] = "+254796456877";  // User end mobile number.
	  char mobileNumber2[] = "+254796456877";  // Office end mobile number.
	  char ATcommand[80];
	  uint8_t buffer[30] = {0};
	  uint8_t ATisOK = 0; //unsigned 8-bit integer variable named ATisOK, initialized to 0.
	  //loop that will keep running until ATisOK becomes true.
	  while(!ATisOK){
	  		sprintf(ATcommand,"AT\r\n");
	  		HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
	  		HAL_UART_Receive (&huart1, buffer, 30, 100);
	  		HAL_Delay(1000); //Gives GSM module enough time to respond to the AT command.
	  		if(strstr((char *)buffer,"OK")){
	  			ATisOK = 1;
	  		}
	  		HAL_Delay(1000);
	  		memset(buffer,0,sizeof(buffer)); // clears the buffer variable
	  }
	  sprintf(ATcommand,"AT+CMGF=1\r\n"); //sets the GSM module to text mode.
	  HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000); //sends the ATcommand to the GSM
	  HAL_UART_Receive (&huart1, buffer, 30, 100); //receives response from GSM module and store in the buffer variable
	  HAL_Delay(1000);
	  memset(buffer,0,sizeof(buffer));
	  //send data to user end
	  sprintf(ATcommand,"AT+CMGS=\"%s\"\r\n",mobileNumber1); //sets phone number to send SMS
	  HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
	  HAL_Delay(100);
      sprintf(ATcommand,"Volume of Water consumed is: %.2f%c, Liters", Volume, 0x1a); // format the message with the value of Volume
      HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
      HAL_UART_Receive (&huart1, buffer, 30, 100);
      memset(buffer,0,sizeof(buffer));
      //send data to office end
	  sprintf(ATcommand,"AT+CMGS=\"%s\"\r\n",mobileNumber2); //sets phone number to send SMS
	  HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
	  HAL_Delay(100);
      sprintf(ATcommand,"Volume of Water consumed is: %.2f%c, Liters", Volume, 0x1a); // format the message with the value of Volume
      HAL_UART_Transmit(&huart1,(uint8_t *)ATcommand,strlen(ATcommand),1000);
      HAL_UART_Receive (&huart1, buffer, 30, 100);
      memset(buffer,0,sizeof(buffer));
      HAL_Delay(86400000); //delay for 24 hours

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
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 12;
  RCC_OscInitStruct.PLL.PLLN = 96;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
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

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief SPI2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI2_Init(void)
{

  /* USER CODE BEGIN SPI2_Init 0 */

  /* USER CODE END SPI2_Init 0 */

  /* USER CODE BEGIN SPI2_Init 1 */

  /* USER CODE END SPI2_Init 1 */
  /* SPI2 parameter configuration*/
  hspi2.Instance = SPI2;
  hspi2.Init.Mode = SPI_MODE_MASTER;
  hspi2.Init.Direction = SPI_DIRECTION_2LINES;
  hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi2.Init.CLKPolarity = SPI_POLARITY_HIGH;
  hspi2.Init.CLKPhase = SPI_PHASE_2EDGE;
  hspi2.Init.NSS = SPI_NSS_SOFT;
  hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
  hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi2.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi2.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi2.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI2_Init 2 */

  /* USER CODE END SPI2_Init 2 */

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 115200;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(HT1621_CS_GPIO_Port, HT1621_CS_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : HT1621_CS_Pin */
  GPIO_InitStruct.Pin = HT1621_CS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(HT1621_CS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : Pulses_Pin */
  GPIO_InitStruct.Pin = Pulses_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Pulses_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if(huart->Instance==USART1)
  {
    // if the character received is other than 'enter' ascii13, save the data in buffer
    if(rx_data!=13)
    {
      rx_buffer[rx_index++]=rx_data;
    }
    else
    {
      // if new message arrived, read the message
      if( sscanf((char*)rx_buffer, "\n+CMTI: " PREF_SMS_STORAGE ",%hhd", &slot)==1)
      {
        sprintf(ATcommand,"AT+CMGR=%d\r\n",slot);
        HAL_UART_Transmit_IT(&huart1,(uint8_t *)ATcommand,strlen(ATcommand));
      }
      // if message read contains "valve-on", Open the ball valve for water to flow
      else if (strstr((char *)rx_buffer,"Valve-on"))
      {
    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    	  HAL_Delay(2800);
    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
    	  HAL_Delay(2000);
      }
      // if message read contains "valve-off", close the ball valve to cut water supply
      else if (strstr((char *)rx_buffer,"valve-off"))
      {
    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
    	  HAL_Delay(2800);
    	  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
    	  HAL_Delay(2000);
      }
      // This will delete all messages in the SIM card.
      if (strstr((char *)rx_buffer,"READ"))
      {
          sprintf(ATcommand,"AT+CMGD=,4\r\n");
          HAL_UART_Transmit_IT(&huart1,(uint8_t *)ATcommand,strlen(ATcommand));
      }
      rx_index=0;
      memset(rx_buffer,0,sizeof(rx_buffer));
    }
    // Enabling interrupt receive again
    HAL_UART_Receive_IT(&huart1,&rx_data,1); // receive data (one character only)
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
