#include "main.h"
#include "ADC.h"
#include "USART.h"
#include "DM.h"

/* Private variables and function prototypes -----------------------------------------------*/
UART_HandleTypeDef huart2;
void SystemClock_Config(void);

// Global Constants
#define ADC_ARR_LEN 2048
#define SAMPLING_FREQUENCY 2643

// ADC Reading & Calculation Variables
uint8_t ADC_flag = 0;
uint16_t ADC_value = 0;
uint16_t ADC_Arr[ADC_ARR_LEN];
int volatile counter = 25;

int main(void)
{
  /* Reset peripherals, & set system clock */
  HAL_Init();
  SystemClock_Config();

  // Set up data transfer protocol
  USART_init();

  // Set up UI
  GUI_init();

  // Set Up ADC
  ADC_init();
  ADC1->CR |= ADC_CR_ADSTART;	// Starts conversion process

  while (1)
  {
	  uint16_t samples_Taken = 0;	//counter for number of samples taken

	  while(samples_Taken < ADC_ARR_LEN){ // Sample Lens

		  if(ADC_flag){ //takes value every tenth read
			  int analogVal = ADC_Conversion(ADC_value);

			  //Convert Analog to Digital and stores it in Array
			  ADC_Arr[samples_Taken] = analogVal;
			  ADC_flag = 0;	//Reseting conversion flag

			  samples_Taken++;
		  }
	  }
	  HAL_Delay(500);

	  // Clear the AC side of the UI
	  clear_AC();

	  //Find min/max/average and store them
	  int Avg_Dig_Vals[3]; // These are saved as integers not doubles
	  ADC_Avg(ADC_Arr, ADC_ARR_LEN ,Avg_Dig_Vals);

	  //Print to Terminal
	  update_DC(Avg_Dig_Vals[0], Avg_Dig_Vals[1], Avg_Dig_Vals[2]);

	  ADC1->CR |= ADC_CR_ADSTART; //start recording again
  }
}


void ADC1_2_IRQHandler(){
	if(ADC1->ISR & ADC_ISR_EOC){
		ADC1->ISR &= ~(ADC_ISR_EOC);
		ADC_value = ADC1->DR;
		if(counter == 0){
			ADC_flag = 1;
			counter = 25;
		}
		counter--;
		ADC1->CR |= ADC_CR_ADSTART; //start recording again
	}
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
  if (HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();
  __HAL_RCC_LSEDRIVE_CONFIG(RCC_LSEDRIVE_LOW);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE|RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.MSICalibrationValue = 0;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_10;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_MSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

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
