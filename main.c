#include "main.h"
#include "ADC.h"
#include "USART.h"
#include "DistanceSensor.h"

/*
READ!!!!!! Long Story Short, Once you read the Data Sheet and understand the we request for the system to take a distance measurement by
a 10 microsecond wave which it then waits on a response that will be in the form of a square pulse with a period to Some equation of the
distance(hands down a terrible explanation of how it works). Basically I'm at the point where I can see that the distance_Timing(the
variable that I know the distance is) to decrease when I bring my hand closer and increase when I move my had away. I still need to turn
that value into Cm (Just Math). There is also an Issue where it will get stuck in an infinite loop randomly and I don't know why. It's completely
random; sometimes right when I run it, and sometimes not until way later. Ran the debugger and I'm 90% sure that its the ADC that stopped reading
therefore never setting the ADC_flag high. Could I have deleted something when changing the ADC IRQ handler?

--SJR
*/
/* Private variables and function prototypes -----------------------------------------------*/
UART_HandleTypeDef huart2;
void SystemClock_Config(void);

// Global Constants
#define ADC_ARR_LEN 2048
#define SAMPLING_FREQUENCY 2643

// ADC Reading & Calculation Variables
uint8_t ADC_flag = 0;
int ADC_value = 0;
uint16_t ADC_Arr[ADC_ARR_LEN];
int volatile counter = 25;

//P3 Globals
uint8_t read_Distance = 0; //after the distance sensor has sent the 10 microsecond pulse and will be receiving a read soon
uint8_t read_State = 0; //Flag to indicate while wait for timing echo wave
int distance_Timing = 0;//some of these don't need to global


int main(void)
{
  /* Reset peripherals, & set system clock */
  HAL_Init();
  SystemClock_Config();

  // Set up data transfer protocol
  USART_init();

  // Set Up ADC
  ADC_init();
  ADC1->CR |= ADC_CR_ADSTART;	// Starts conversion process


  //ADDed Code for P3
  DistanceSensor_init();
  int new_read = 1;
  //Current Infinite While Loop for P3
  while(1){


	  if(new_read){
		  //This basically starts TIM2
		  requestDistance();
		  //I used a tone of flags in this, will clean up tomorrow
		  new_read = 0;

	  }
	  uint16_t samples_Taken = 0;
	  int previous_Val = 0;
	  uint16_t edges[2];
	  uint16_t edge_Counter = 0;

	  ADC1->CR |= ADC_CR_ADSTART; //start recording again

	  while(read_Distance){
		  //While loop to wait on the echo response from the distance sensor
		  //Stored values to get the distance between the two edges
		  if(ADC_flag){
			  ADC_flag = 0;
			  if(previous_Val < (ADC_value - 2000) || previous_Val > (ADC_value + 2000)){
				  //CASE: If the value changes by more than about 1.5V(catching both edges of the waveform to get period)
				  edges[edge_Counter] = samples_Taken;
				  edge_Counter ++;
				  if(edge_Counter == 2){
					  read_Distance = 0;
					  //Difference between the two is the period in ADC clock cycle reads
					  distance_Timing = edges[1] - edges[0];
					  USART_print_num(distance_Timing);
					  USART_ESC_Code("[H");
					  new_read = 1;

				  }
			  }
			  //Setting the previous value to the ADC to find the point in which ADC read the value goes high
			  //Technically I believe the fastest way is using input capture mode(could be second option if this doesn't work too well
			  previous_Val = ADC_value;
			  samples_Taken ++;
			  ADC1->CR |= ADC_CR_ADSTART; //start recording again
		  }
	  }
  }

//  while (1)
//  {
//	  uint16_t samples_Taken = 0;	//counter for number of samples taken
//
//	  while(samples_Taken < ADC_ARR_LEN){ // Sample Lens
//
//		  if(ADC_flag){ //takes value every tenth read
//			  int analogVal = ADC_Conversion(ADC_value);
//
//			  //Convert Analog to Digital and stores it in Array
//			  ADC_Arr[samples_Taken] = analogVal;
//			  ADC_flag = 0;	//Reseting conversion flag
//
//			  samples_Taken++;
//		  }
//	  }
//	  HAL_Delay(500);
//
//	  // Clear the AC side of the UI
//	  clear_AC();
//
//	  //Find min/max/average and store them
//	  int Avg_Dig_Vals[3]; // These are saved as integers not doubles
//	  ADC_Avg(ADC_Arr, ADC_ARR_LEN ,Avg_Dig_Vals);
//
//	  //Print to Terminal
//	  update_DC(Avg_Dig_Vals[0], Avg_Dig_Vals[1], Avg_Dig_Vals[2]);
//
//	  ADC1->CR |= ADC_CR_ADSTART; //start recording again
//  }
}


void ADC1_2_IRQHandler(){
	if(ADC1->ISR & ADC_ISR_EOC){
		ADC1->ISR &= ~(ADC_ISR_EOC);
		ADC_value = ADC1->DR;
		ADC_flag = 1;
	}
	ADC1->CR |= ADC_CR_ADSTART; //start recording again
}

void TIM2_IRQHandler(void){
	if(TIM2->SR & TIM_SR_UIF){
		TIM2->SR &= ~(TIM_SR_UIF);
		//setting global to get read for a read
		GPIOA->ODR &= ~(GPIO_PIN_5);
		read_Distance = 1;
		//Stops timer until another request is made
		TIM2->CR1 &= ~(TIM_CR1_CEN);
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
