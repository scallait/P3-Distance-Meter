#include "main.h"
#include "ADC.h"
#include "USART.h"
#include "DistanceSensor.h"
#include "DM.h"
#include "keypad.h"


/* Private variables and function prototypes -----------------------------------------------*/
UART_HandleTypeDef huart2;
void SystemClock_Config(void);

// Global Constants
#define ADC_ARR_LEN 100
#define NO_KEY_PRESS -1
#define EDGE_THRESHOLD 1000
#define CONFIRM_KEY 10
#define ONE_INPUT_MODE 1
#define TWO_INPUT_MODE 2
#define DEBOUNCE_DELAY 5
#define CM_TO_INCHES 0.393701

// ADC Reading & Calculation Globals
uint8_t ADC_flag = 0;			// Flag for if ADC value is ready to be read
int ADC_value = 0;				// Variable to hold value
uint16_t ADC_Arr[ADC_ARR_LEN];	// Array to hold ADC values
int cnt2D = 0;					// Measurement count for 2 input mode

//Digital Meter Specific Globals
uint8_t read_Distance = 0; 		// after the distance sensor has sent the 10 microsecond pulse and will be receiving a read soon
uint8_t read_State = 0; 		// Flag to indicate while wait for timing echo wave
int distance_Timing = 0;		// some of these don't need to global


int main(void)
{
  /* Reset peripherals, & set system clock */
  HAL_Init();
  SystemClock_Config();

  // Set up data transfer protocol
  USART_init();
  GUI_init();

  // Set up the Keypad
  keypad_init();
  int key_press = NO_KEY_PRESS;

  // Set Up ADC
  ADC_init();
  ADC1->CR |= ADC_CR_ADSTART;	// Starts conversion process

  // Distance Sensor set up and flags
  DistanceSensor_init();
  int new_read = 1;
  int MEASURE_RDY = 0;

  // Variables to save ADC Values
  uint16_t edge_Counter = 0;
  uint16_t array2D[2] = {0};


  // Input Mode Variables and flags
  int cnt2D = 0;
  int DIM_FLAG = ONE_INPUT_MODE;


  while(1){

	  key_press = keypad_read();			// Read the keypad

	  // Check for key presses
	  if(key_press == ONE_INPUT_MODE){ // 1 input measurement mode
		  // set up for 1 input measurement mode
		  GUI_init();
		  DIM_FLAG = ONE_INPUT_MODE;
		  MEASURE_RDY = 0;
	  }
	  else if(key_press == TWO_INPUT_MODE){	// 2 input measurement mode
		  // Set up for 2 input measurement mode
		  DIM_FLAG = TWO_INPUT_MODE;
		  MEASURE_RDY = 0;
		  array2D[0] = 0;
		  array2D[1] = 0;
		  GUI_2D_init();
	  }
	  else if(key_press == CONFIRM_KEY){	// confirm current measurement key
		  MEASURE_RDY = 1;
	  }

	  // Sends out pulse for a new measurement reading
	  if(new_read){
		  requestDistance();
		  new_read = 0;
		  while(!(ADC1->ISR & ADC_ISR_EOC));	// Wait for return pulse
	  }

	  // ADC Variables
	  uint16_t samples_Taken = 0;
	  int previous_Val = 0;
	  uint16_t edges[2 * ADC_ARR_LEN];

	  ADC1->CR |= ADC_CR_ADSTART; // Start ADC again

	  // While loop to wait on ADC processing
	  while(read_Distance){

		  //Stored values to get the distance between the two edges
		  if(ADC_flag){

			  ADC_flag = 0;
			  if(previous_Val < (ADC_value - EDGE_THRESHOLD) || previous_Val > (ADC_value + EDGE_THRESHOLD)){
				  //CASE: If the value changes by more than about 1.5V(catching both edges of the waveform to get period)
				  edges[edge_Counter] = samples_Taken;
				  edge_Counter++;

				  // Read a full pulse
				  if((edge_Counter % 2) == 0){
					  read_Distance = 0;
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

	  // Check if required number of measurements are taken
	  if(edge_Counter >= (2 * ADC_ARR_LEN)){
		  read_Distance = 0;

		  // Calculate Distance & Timing
		  distance_Timing = calcAverage(2 * ADC_ARR_LEN, edges);
		  distance_Timing = calcDistance(distance_Timing);

		  // Check input mode and display accordingly
		  if(DIM_FLAG == TWO_INPUT_MODE){
			  // Check if current measurement is confirmed in 2 input mode
			  if(MEASURE_RDY){
				  array2D[cnt2D] = distance_Timing;

				  // Check if 2nd measurement
				  if(cnt2D >= 1){
					  print2Distance(array2D);
					  cnt2D = -1;
					  while((keypad_read() == NO_KEY_PRESS) || (keypad_read() == CONFIRM_KEY)){};
				  }
				  HAL_Delay(DEBOUNCE_DELAY);
				  MEASURE_RDY = 0;
				  cnt2D++;
			  }
			  // 1 input measurment mode
			  else{
				  array2D[cnt2D] = distance_Timing;
				  print2Distance(array2D);
			  }
		  }
		  else{
			  printDistance(distance_Timing, distance_Timing * CM_TO_INCHES);
		  }

		  //Difference between the two is the period in ADC clock cycle reads
		  new_read = 1;
		  edge_Counter = 0;
	  }


  }
}

// Handler for ADC interrupts
void ADC1_2_IRQHandler(){
	// Check if ADC conversion is done
	if(ADC1->ISR & ADC_ISR_EOC){
		ADC1->ISR &= ~(ADC_ISR_EOC);
		ADC_value = ADC1->DR;
		ADC_flag = 1;		// Set ADC ready to read flag
	}
	ADC1->CR |= ADC_CR_ADSTART; //start recording again
}

// This is the timer
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
