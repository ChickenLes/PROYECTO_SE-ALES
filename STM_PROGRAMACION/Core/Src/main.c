/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MAX_ORDER 20
#define RX_BUF_SIZE 256
#define MAX_CAPTURE 10000
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

DAC_HandleTypeDef hdac;

TIM_HandleTypeDef htim4;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */

//Coeficientes filtro IIR
float b_coef[MAX_ORDER + 1] = {1.0f};
float a_coef[MAX_ORDER + 1] = {1.0f};
int nb = 1;
int na = 1;

//Buffers ecuacion de diferencias
float x_buf[MAX_ORDER + 1] = {0};
float y_buf[MAX_ORDER + 1] = {0};

//UART
uint8_t rx_byte;
char rx_buffer[RX_BUF_SIZE];
int rx_index = 0;
volatile uint8_t linea_lista = 0;

//Buffer de captura (se llena a 8000Hz y luego se manda por UART)
uint16_t captura_buf[MAX_CAPTURE];

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
static void MX_DAC_Init(void);
static void MX_TIM4_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */
void enviar_uart(char *msg);
void limpiar_buffers_filtro(void);
float aplicar_filtro_iir(float x_nuevo);
uint16_t leer_adc(void);
void esperar_timer(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART2)
    {
        if (rx_byte == '\n' || rx_byte == '\r')
        {
            if (rx_index > 0)
            {
                rx_buffer[rx_index] = '\0';
                linea_lista = 1;
            }
        }
        else
        {
            if (rx_index < RX_BUF_SIZE - 1)
            {
                rx_buffer[rx_index] = rx_byte;
                rx_index++;
            }
        }
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

void enviar_uart(char *msg)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)msg, strlen(msg), 100);
}

void limpiar_buffers_filtro(void)
{
    for (int i = 0; i <= MAX_ORDER; i++)
    {
        x_buf[i] = 0.0f;
        y_buf[i] = 0.0f;
    }
}

//Espera al timer TIM4 para muestrear a 8000Hz exactos
//el timer hace overflow cada 1/8000 s, aca esperamos ese flag
void esperar_timer(void)
{
    while (__HAL_TIM_GET_FLAG(&htim4, TIM_FLAG_UPDATE) == RESET) {}
    __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);
}

float aplicar_filtro_iir(float x_nuevo)
{
    for (int i = MAX_ORDER; i > 0; i--)
    {
        x_buf[i] = x_buf[i - 1];
    }
    x_buf[0] = x_nuevo;

    float y_nuevo = 0.0f;

    for (int k = 0; k < nb; k++)
    {
        y_nuevo += b_coef[k] * x_buf[k];
    }

    for (int k = 1; k < na; k++)
    {
        y_nuevo -= a_coef[k] * y_buf[k];
    }

    for (int i = MAX_ORDER; i > 0; i--)
    {
        y_buf[i] = y_buf[i - 1];
    }
    y_buf[0] = y_nuevo;

    return y_nuevo;
}

uint16_t leer_adc(void)
{
    uint16_t val = 0;
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 5) == HAL_OK)
    {
        val = HAL_ADC_GetValue(&hadc1);
    }
    HAL_ADC_Stop(&hadc1);
    return val;
}

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
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_TIM4_Init();
  MX_USART1_UART_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  //arrancar DAC
  HAL_DAC_Start(&hdac, DAC_CHANNEL_1);

  //reconfigurar TIM4 a modo libre (sin slave) para usarlo como reloj de muestreo
  //Prescaler 84-1 y Period 124 dan 84MHz/84/125 = 8000Hz
  htim4.Instance->SMCR = 0;          //quitar modo slave
  htim4.Init.Prescaler = 84 - 1;
  htim4.Init.Period = 124;
  HAL_TIM_Base_Init(&htim4);
  HAL_TIM_Base_Start(&htim4);

  //arrancar UART con interrupcion
  HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(USART2_IRQn);
  HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  char tx_buf[32];

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

      //esperar al timer: garantiza muestreo a 8000Hz exactos
      esperar_timer();

      //Leer ADC
      uint16_t adc_val = leer_adc();
      float x_in = ((float)adc_val / 2048.0f) - 1.0f;

      //Aplicar filtro
      float y_out = aplicar_filtro_iir(x_in);

      float dac_float = (y_out + 1.0f) * 2048.0f;
      if (dac_float < 0.0f) { dac_float = 0.0f; }
      if (dac_float > 4095.0f) { dac_float = 4095.0f; }
      uint16_t dac_val = (uint16_t)dac_float;

      //Escribir al DAC
      HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_val);

      //Revisar el UART
      if (linea_lista)
      {
          //CAPTURE N
          if (strncmp(rx_buffer, "CAPTURE", 7) == 0)
          {
              int N = atoi(rx_buffer + 8);
              if (N <= 0 || N > MAX_CAPTURE) { N = 1000; }

              HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);


              __HAL_TIM_CLEAR_FLAG(&htim4, TIM_FLAG_UPDATE);

              //Buffer para los 8kHz
              for (int i = 0; i < N; i++)
              {
                  esperar_timer();
                  captura_buf[i] = leer_adc();
              }

              //Enviar por UART
              for (int i = 0; i < N; i++)
              {
                  sprintf(tx_buf, "%u\n", captura_buf[i]);
                  enviar_uart(tx_buf);
              }

              HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);
          }

          //FILTER_B N
          else if (strncmp(rx_buffer, "FILTER_B", 8) == 0)
          {
              int n = atoi(rx_buffer + 9);
              if (n > 0 && n <= MAX_ORDER + 1)
              {
                  nb = n;
                  for (int i = 0; i < nb; i++)
                  {
                      linea_lista = 0;
                      rx_index = 0;
                      while (!linea_lista) {}
                      b_coef[i] = (float)atof(rx_buffer);
                      linea_lista = 0;
                      rx_index = 0;
                  }
                  limpiar_buffers_filtro();
                  enviar_uart("B_OK\n");
              }
          }

          //FILTER_A N
          else if (strncmp(rx_buffer, "FILTER_A", 8) == 0)
          {
              int n = atoi(rx_buffer + 9);
              if (n > 0 && n <= MAX_ORDER + 1)
              {
                  na = n;
                  for (int i = 0; i < na; i++)
                  {
                      linea_lista = 0;
                      rx_index = 0;
                      while (!linea_lista) {}
                      a_coef[i] = (float)atof(rx_buffer);
                      linea_lista = 0;
                      rx_index = 0;
                  }
                  limpiar_buffers_filtro();
                  enviar_uart("A_OK\n");
              }
          }

          linea_lista = 0;
          rx_index = 0;
      }
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

  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 336;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

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
  * @brief ADC1 Initialization Function
  */
static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
  hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T4_CC4;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }

  //forzar software trigger (CubeMX lo deja con trigger de TIM4 y el ADC lee 0)
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  HAL_ADC_Init(&hadc1);
}

/**
  * @brief DAC Initialization Function
  */
static void MX_DAC_Init(void)
{
  DAC_ChannelConfTypeDef sConfig = {0};

  hdac.Instance = DAC;
  if (HAL_DAC_Init(&hdac) != HAL_OK)
  {
    Error_Handler();
  }

  sConfig.DAC_Trigger = DAC_TRIGGER_NONE;
  sConfig.DAC_OutputBuffer = DAC_OUTPUTBUFFER_ENABLE;
  if (HAL_DAC_ConfigChannel(&hdac, &sConfig, DAC_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM4 Initialization Function
  */
static void MX_TIM4_Init(void)
{
  TIM_SlaveConfigTypeDef sSlaveConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim4.Instance = TIM4;
  htim4.Init.Prescaler = 84-1;
  htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim4.Init.Period = 124;
  htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim4) != HAL_OK)
  {
    Error_Handler();
  }
  sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
  sSlaveConfig.InputTrigger = TIM_TS_ITR0;
  if (HAL_TIM_SlaveConfigSynchro(&htim4, &sSlaveConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim4, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief USART1 Initialization Function
  */
static void MX_USART1_UART_Init(void)
{
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
}

/**
  * @brief USART2 Initialization Function
  */
static void MX_USART2_UART_Init(void)
{
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
}

/**
  * @brief GPIO Initialization Function
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_RESET);

  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);
}

void Error_Handler(void)
{
  __disable_irq();
  while (1)
  {
  }
}

#ifdef  USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
}
#endif /* USE_FULL_ASSERT */
