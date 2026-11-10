/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "lwip/api.h"
#include <string.h>
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

COM_InitTypeDef BspCOMInit;
ADC_HandleTypeDef hadc3;

/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
static void MPU_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC3_Init(void);
void StartDefaultTask(void *argument);

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
#include <stdio.h>
#include <string.h>

/* Adicione as duas linhas abaixo para o compilador reconhecer as funções nativas */
#include "FreeRTOS.h"
#include "task.h"

/* FUNÇÃO DE DELAY ULTRA-PRECISO USANDO O CONTADOR DE CICLOS DA CPU (DWT) */
void DWT_Delay_ms(uint32_t ms) {
    uint32_t startTicks = DWT->CYCCNT;
    uint32_t delayTicks = ms * (SystemCoreClock / 1000);
    while ((DWT->CYCCNT - startTicks) < delayTicks);
}

/* FUNÇÃO DE LEITURA DA TEMPERATURA DA MCU VIA ADC3 */
float Read_MCU_Temperature(void)
{
    uint32_t raw_val = 0;

    HAL_ADC_Start(&hadc3);

    if (HAL_ADC_PollForConversion(&hadc3, 10) == HAL_OK)
    {
        raw_val = HAL_ADC_GetValue(&hadc3);
    }

    HAL_ADC_Stop(&hadc3);

    // Conversão de 16-bit resolution do STM32H7 para Celsius
    float temperature = (((float)raw_val * (3.3f / 65535.0f)) - 0.76f) / 0.025f + 25.0f;

    return temperature;
}

void BlinkTask(void *argument) {
  while(1) {
    BSP_LED_Toggle(LED_YELLOW);
    osDelay(500);
  }
}

static void http_server_serve(struct netconn *conn) {
  struct netbuf *inbuf;
  char *buf;
  u16_t buflen;
  err_t err;

  err = netconn_recv(conn, &inbuf);
  if (err == ERR_OK) {
    netbuf_data(inbuf, (void**)&buf, &buflen);

    /* 1. SE O NAVEGADOR PEDIR A PÁGINA PRINCIPAL (GET / ) */
    if (buflen >= 6 && strncmp(buf, "GET / ", 6) == 0) {

    	char html_page[6000];
    	      snprintf(html_page, 6000,
    	          "HTTP/1.1 200 OK\r\n"
    	          "Content-Type: text/html; charset=utf-8\r\n"
    	          "Connection: close\r\n\r\n"
    	          "<!DOCTYPE HTML>"
    	          "<html>"
    	          "<head>"
    	          "  <title>Dashboard STM32H7</title>"
    	          "  <meta charset='utf-8'>"
    	          "  <meta name='viewport' content='width=device-width, initial-scale=1.0'>" // <--- ADICIONADO AQUI
    	          "  <style>"
    	          "    body { font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; background-color: #0f172a; color: #f8fafc; margin: 0; padding: 10px; }"
    	          "    .container { width: 100%%; max-width: 950px; margin: auto; box-sizing: border-box; }" // <--- AJUSTADO AQUI
    	          "    .header { background: linear-gradient(135deg, #3b82f6, #1d4ed8); padding: 15px; border-radius: 12px; text-align: center; box-shadow: 0 4px 6px rgba(0,0,0,0.3); margin-bottom: 15px; }"
    	          "    .header h2 { margin: 0; color: #ffffff; font-size: 20px; }"
    	          "    .header p { margin: 5px 0 0 0; color: #cbd5e1; font-size: 13px; }"
    	          "    .grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(240px, 1fr)); gap: 15px; margin-bottom: 15px; }"
    	          "    .card { background: #1e293b; padding: 15px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.2); border-left: 5px solid #3b82f6; }"
    	          "    .card.temp { border-left-color: #ef4444; }"
    	          "    .card.ram { border-left-color: #10b981; }"
    	          "    .card.uptime { border-left-color: #f59e0b; }"
    	          "    .card h3 { margin-top: 0; font-size: 12px; color: #94a3b8; text-transform: uppercase; letter-spacing: 1px; }"
    	          "    .card .value { font-size: 22px; font-weight: bold; color: #f8fafc; margin-top: 5px; }"
    	          "    .tasks-card { background: #1e293b; padding: 15px; border-radius: 12px; box-shadow: 0 4px 6px rgba(0,0,0,0.2); margin-top: 15px; overflow-x: auto; }"
    	          "    .tasks-card h3 { margin-top: 0; color: #38bdf8; border-bottom: 1px solid #334155; padding-bottom: 8px; font-size: 15px; }"
    	          "    pre { background: #0f172a; color: #38bdf8; padding: 10px; border-radius: 8px; overflow-x: auto; font-family: 'Courier New', Courier, monospace; font-size: 12px; border: 1px solid #334155; margin: 0; }"
    	          "    button { touch-action: manipulation; }" // Melhora o toque em telas mobile
    	          "  </style>"
    	          // ... (o restante do script e do corpo HTML continua exatamente igual)
          "  <script>"
          "    let tempoSelecionado = 0;"
          "    "
          "    function ajustarTempo(valor) {"
          "      tempoSelecionado += valor;"
          "      if (tempoSelecionado < 0) tempoSelecionado = 0;"
          "      if (tempoSelecionado > 5000) tempoSelecionado = 5000;"
          "      document.getElementById('tempoPulsoVal').innerText = tempoSelecionado + ' ms';"
          "    }"
          "    "
          "    function dispararPulso() {"
          "      fetch('/disparo?tempo=' + tempoSelecionado)"
          "        .then(response => response.text())"
          "        .then(res => {"
          "           alert('Pulso de ' + tempoSelecionado + ' ms enviado com sucesso!');"
          "        })"
          "        .catch(error => console.log('Erro no disparo'));"
          "    }"
          "    "
          "    function atualizarPainel() {"
          "      fetch('/metrics')"
          "        .then(res => res.text())"
          "        .then(html => { document.getElementById('metrics').innerHTML = html; });"
          "      "
          "      fetch('/tasks')"
          "        .then(res => res.text())"
          "        .then(html => { document.getElementById('tasks').innerHTML = html; });"
          "    }"
          "    "
          "    atualizarPainel();"
          "    setInterval(atualizarPainel, 2000);"
          "  </script>"
          "</head>"
          "<body>"
          "  <div class='container'>"
          "    <div class='header'>"
          "      <h2>Painel de Controle STM32H753ZI</h2>"
          "      <p>Desenvolvido por Junior Lopes</p>"
          "    </div>"
          "    "
          "    <div id='metrics'>"
          "      <p style='text-align:center; color:#94a3b8;'>Carregando métricas...</p>"
          "    </div>"
          "    "
          "    <div class='card' style='border-left-color: #8b5cf6; margin-top: 20px; margin-bottom: 20px;'>"
          "      <h3>Gerador de Pulso Digital</h3>"
          "      <div class='value' id='tempoPulsoVal' style='margin-bottom: 15px;'>0 ms</div>"
          "      <button onclick='ajustarTempo(-10)' style='padding: 8px 15px; background: #334155; color: white; border: none; border-radius: 6px; cursor: pointer; font-weight: bold; margin-right: 5px;'>- 10 ms</button>"
          "      <button onclick='ajustarTempo(10)' style='padding: 8px 15px; background: #334155; color: white; border: none; border-radius: 6px; cursor: pointer; font-weight: bold; margin-right: 15px;'>+ 10 ms</button>"
          "      <button onclick='dispararPulso()' style='padding: 8px 20px; background: #8b5cf6; color: white; border: none; border-radius: 6px; cursor: pointer; font-weight: bold;'>Disparar Pulso</button>"
          "    </div>"
          "    "
          "    <div id='tasks'>"
          "      <p style='text-align:center; color:#94a3b8;'>Carregando tarefas...</p>"
          "    </div>"
          "    "
          "  </div>"
          "</body>"
          "</html>"
      );
      netconn_write(conn, html_page, strlen(html_page), NETCONN_COPY);
    }

    /* 2. ROTA PARA OS CARDS DE TOPO (GET /metrics ) */
    else if (strstr(buf, "GET /metrics") != NULL) {
      char metrics_buffer[512];
      uint32_t uptime_s = xTaskGetTickCount() / 1000;
      uint32_t free_heap = xPortGetFreeHeapSize();
      float mcu_temp = Read_MCU_Temperature();

      int temp_int = (int)mcu_temp;
      int temp_dec = (int)((mcu_temp - temp_int) * 100);
      if (temp_dec < 0) temp_dec = -temp_dec;

      snprintf(metrics_buffer, sizeof(metrics_buffer),
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/html; charset=utf-8\r\n"
          "Connection: close\r\n\r\n"
          "<div class='grid'>"
          "  <div class='card uptime'>"
          "    <h3>Tempo Ligado (Uptime)</h3>"
          "    <div class='value'>%lu s</div>"
          "  </div>"
          "  <div class='card ram'>"
          "    <h3>Memória RAM Livre</h3>"
          "    <div class='value'>%lu bytes</div>"
          "  </div>"
          "  <div class='card temp'>"
          "    <h3>Temperatura da MCU</h3>"
          "    <div class='value'>%d.%02d &deg;C</div>"
          "  </div>"
          "</div>",
          uptime_s, free_heap, temp_int, temp_dec
      );
      netconn_write(conn, metrics_buffer, strlen(metrics_buffer), NETCONN_COPY);
    }

    /* 3. ROTA PARA A TABELA DE TAREFAS (GET /tasks ) */
    else if (strstr(buf, "GET /tasks") != NULL) {
      char tasks_buffer[1024];
      char task_list_buffer[512];
      vTaskList(task_list_buffer);

      snprintf(tasks_buffer, sizeof(tasks_buffer),
          "HTTP/1.1 200 OK\r\n"
          "Content-Type: text/html; charset=utf-8\r\n"
          "Connection: close\r\n\r\n"
          "<div class='tasks-card'>"
          "  <h3>Status das Tarefas (FreeRTOS)</h3>"
          "  <p style='color:#94a3b8; font-size:12px;'><i>(Nome, Estado, Prioridade, Pilha Sobrando, ID)</i></p>"
          "  <pre>%s</pre>"
          "</div>",
          task_list_buffer
      );
      netconn_write(conn, tasks_buffer, strlen(tasks_buffer), NETCONN_COPY);
    }

    /* 4. ROTA DE DISPARO DE PULSO (GET /disparo ) */
        else if (strstr(buf, "GET /disparo") != NULL) {
          int tempo_ms = 500;

          char *p = strstr(buf, "tempo=");
          if (p != NULL) {
              tempo_ms = atoi(p + 6);
          }

          if (tempo_ms < 0) tempo_ms = 0;
          if (tempo_ms > 5000) tempo_ms = 5000;

          // --- PROTEÇÃO PARA O TEMPO 0 (NÃO DISPARA NADA) ---
          if (tempo_ms == 0) {
              char resp_buffer[64];
              snprintf(resp_buffer, sizeof(resp_buffer), "HTTP/1.1 200 OK\r\n\r\nPulso ignorado (0 ms)");
              netconn_write(conn, resp_buffer, strlen(resp_buffer), NETCONN_COPY);
          }
          else {
              // --- ACIONAMENTO DUPLO (LED Vermelho + Pino PB1 para o Osciloscópio) ---
              BSP_LED_On(LED_RED);                        // Liga o LED Vermelho físico
              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_SET); // <--- Coloca PB1 em Nível Alto (3.3V)

              // Multiplica por 19 para compensar o clock real do hardware do H7
              TIM2->ARR = tempo_ms * 19;
              TIM2->EGR = TIM_EGR_UG;   // Atualiza o evento do timer
              TIM2->CR1 |= TIM_CR1_CEN; // Dispara a contagem por hardware

              // Aguarda o hardware do TIM2 concluir o tempo exato de forma autônoma
              while ((TIM2->CR1 & TIM_CR1_CEN) != 0);

              // Desliga ambos ao mesmo tempo
              BSP_LED_Off(LED_RED);                       // Apaga o LED Vermelho
              HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET); // <--- Retorna PB1 para 0V
              // --------------------------------------------------------------------

              char resp_buffer[64];
              snprintf(resp_buffer, sizeof(resp_buffer), "HTTP/1.1 200 OK\r\n\r\nPulso de %d ms OK", tempo_ms);
              netconn_write(conn, resp_buffer, strlen(resp_buffer), NETCONN_COPY);
          }
        }

    netbuf_delete(inbuf);
  }
  netconn_close(conn);
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

  /* MPU Configuration--------------------------------------------------------*/
  MPU_Config();

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_ADC3_Init();



  /* USER CODE BEGIN 2 */
      // 1. Configuração manual do pino PB1 como Saída Digital (para o osciloscópio)
      GPIO_InitTypeDef GPIO_InitStruct = {0};
      __HAL_RCC_GPIOB_CLK_ENABLE();

      GPIO_InitStruct.Pin = GPIO_PIN_1;
      GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
      GPIO_InitStruct.Pull = GPIO_NOPULL;
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

      // 2. Configuração manual do TIM2 com frequência fixa real de 200 MHz do H7
      __HAL_RCC_TIM2_CLK_ENABLE();

      // O barramento APB1 do timer opera a 200 MHz baseado no seu SystemClock_Config
      // Dividimos por 200.000 para que cada incremento do timer valia exatamente 1 milissegundo (1 kHz)
      TIM2->PSC = 200000 - 1;
      TIM2->CR1 |= TIM_CR1_OPM; // Ativa o Modo de Pulso Único (para sozinho após o tempo)
    /* USER CODE END 2 */



  /* Init scheduler */
  osKernelInitialize();

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
    const osThreadAttr_t blinkTask_attributes = {
      .name = "blinkTask",
      .stack_size = 128 * 4,
      .priority = (osPriority_t) osPriorityNormal,
    };
    osThreadNew(BlinkTask, NULL, &blinkTask_attributes);
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Initialize leds */
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_YELLOW);
  BSP_LED_Init(LED_RED);

  /* Initialize USER push-button, will be used to trigger an interrupt each time it's pressed.*/
  BSP_PB_Init(BUTTON_USER, BUTTON_MODE_EXTI);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */

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

  /** Supply configuration update enable
  */
  HAL_PWREx_ConfigSupply(PWR_LDO_SUPPLY);

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  while(!__HAL_PWR_GET_FLAG(PWR_FLAG_VOSRDY)) {}

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSIState = RCC_HSI_DIV1;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 5;
  RCC_OscInitStruct.PLL.PLLN = 160;
  RCC_OscInitStruct.PLL.PLLP = 2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
  RCC_OscInitStruct.PLL.PLLRGE = RCC_PLL1VCIRANGE_2;
  RCC_OscInitStruct.PLL.PLLVCOSEL = RCC_PLL1VCOWIDE;
  RCC_OscInitStruct.PLL.PLLFRACN = 0;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2
                              |RCC_CLOCKTYPE_D3PCLK1|RCC_CLOCKTYPE_D1PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.SYSCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB3CLKDivider = RCC_APB3_DIV2;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_APB1_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_APB2_DIV2;
  RCC_ClkInitStruct.APB4CLKDivider = RCC_APB4_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_CKPER;
  PeriphClkInitStruct.CkperClockSelection = RCC_CLKPSOURCE_HSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC3_Init(void)
{

  /* USER CODE BEGIN ADC3_Init 0 */

  /* USER CODE END ADC3_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC3_Init 1 */

  /* USER CODE END ADC3_Init 1 */

  /** Common config
  */
  hadc3.Instance = ADC3;
  hadc3.Init.ClockPrescaler = ADC_CLOCK_ASYNC_DIV16;
  hadc3.Init.ScanConvMode = ADC_SCAN_DISABLE;
  hadc3.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  hadc3.Init.LowPowerAutoWait = DISABLE;
  hadc3.Init.ContinuousConvMode = DISABLE;
  hadc3.Init.NbrOfConversion = 1;
  hadc3.Init.DiscontinuousConvMode = DISABLE;
  hadc3.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc3.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc3.Init.ConversionDataManagement = ADC_CONVERSIONDATA_DR;
  hadc3.Init.Overrun = ADC_OVR_DATA_PRESERVED;
  hadc3.Init.LeftBitShift = ADC_LEFTBITSHIFT_NONE;
  hadc3.Init.OversamplingMode = DISABLE;
  hadc3.Init.Oversampling.Ratio = 1;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }
  hadc3.Init.Resolution = ADC_RESOLUTION_16B;
  if (HAL_ADC_Init(&hadc3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure Regular Channel
  */
  sConfig.Channel = ADC_CHANNEL_TEMPSENSOR;
  sConfig.Rank = ADC_REGULAR_RANK_1;
  sConfig.SamplingTime = ADC_SAMPLETIME_810CYCLES_5;
  sConfig.SingleDiff = ADC_SINGLE_ENDED;
  sConfig.OffsetNumber = ADC_OFFSET_NONE;
  sConfig.Offset = 0;
  sConfig.OffsetSignedSaturation = DISABLE;
  if (HAL_ADC_ConfigChannel(&hadc3, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC3_Init 2 */

  /* USER CODE END ADC3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */

  /* USER CODE BEGIN 5 */
void StartDefaultTask(void *argument)
{

	/* Espera 3 segundos para a Auto-Negociação física do cabo de rede terminar */
    osDelay(7000);

	/* init code for LWIP */

	MX_LWIP_Init();

  /* USER CODE BEGIN 5 */
    struct netconn *conn, *newconn;
    err_t err;

    /* Cria uma nova conexão TCP */
    conn = netconn_new(NETCONN_TCP);
    if (conn != NULL) {
      /* Liga a porta 80 (HTTP) */
      err = netconn_bind(conn, NULL, 80);
      if (err == ERR_OK) {
        /* Começa a ouvir a porta */
        netconn_listen(conn);

        while (1) {
          /* Fica aguardando alguém acessar pelo navegador */
          err = netconn_accept(conn, &newconn);
          if (err == ERR_OK) {
            http_server_serve(newconn);
            netconn_delete(newconn);
          }
        }
      }
    }

    /* Loop de segurança caso a alocação falhe */
    for(;;)
    {
      osDelay(1000);
    }
}
  /* USER CODE END 5 */

 /* MPU Configuration */

void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct = {0};

  /* Disables the MPU */
  HAL_MPU_Disable();

  /** Initializes and configures the Region and the memory to be protected
  */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.BaseAddress = 0x0;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);
  /* Enables the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);

}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM6 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM6)
  {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  __disable_irq();
  while (1)
  {
    BSP_LED_Toggle(LED_RED);
    HAL_Delay(100); /* Pisca muito rápido */
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
