/**
 * C/C++ Preprocessor Add Headers:
 * - FreeRTOS\Source\include
 * - FreeRTOS\Source\portable\IAR\ARM_CM4F
 * 
 * Assembler Preprocessor Add Headers:
 * - Copy to $PROJ_DIR$ a working FreeRTOSConfig.h and setup
 * - $PROJ_DIR$\.. (FreeRTOSConfig.h)
 *
 * Add source files:
 * - FreeRTOS\Source\*.c
 * - FreeRTOS\Source\portable\MemMang\heap_[1|2|3|4].c
 * - FreeRTOS\Source\portable\IAR\ARM_CM4F\*.c
 * - FreeRTOS\Source\portable\IAR\ARM_CM4F\*.s
 * 
 * Remove duplicate interrupt handlers definitions in stm32f4xx_it.c. Originals are in portasm.s
 *
 * Define tasks and start scheduler (below).
 */

/* Board includes */
#include "stm32f4_discovery.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_rcc.h"
/* Kernel includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "semphr.h"

void Task1(void*);
void Task2(void*);
void Task3(void*);
void initHW();

xQueueHandle pbq;

int main(void)
{
  
  initHW();
  
  /* Create IPC variables */
  pbq = xQueueCreate(10, sizeof(int));
  if (pbq == 0) {
    while(1); /* fatal error */
  }
  
  /* Create tasks */
  xTaskCreate(
      Task1,                            /* Function pointer */
      "Task1",                          /* Task name - for debugging only*/
      configMINIMAL_STACK_SIZE,         /* Stack depth in words */
      (void*) NULL,                     /* Pointer to tasks arguments (parameter) */
      tskIDLE_PRIORITY + 2UL,           /* Task priority*/
      NULL                              /* Task handle */
  );
  
  xTaskCreate(Task2, "Task2", configMINIMAL_STACK_SIZE, (void*) NULL, tskIDLE_PRIORITY + 2UL, NULL);
  xTaskCreate(Task3, "Task3", configMINIMAL_STACK_SIZE, (void*) NULL, tskIDLE_PRIORITY + 2UL, NULL);
  
  /* Start the Scheduler */
  vTaskStartScheduler();
  
  /* HALT */
  while(1); 
}

/**
 * TASK 1
 */
void Task1(void *pvParameters){
  
  while (1) {
    GPIO_ToggleBits(GPIOD, GPIO_Pin_12);
    
    /*
    Delay for a period of time. vTaskDelay() places the task into
    the Blocked state until the period has expired.
    The delay period is spacified in 'ticks'. We can convert
    yhis in milisecond with the constant portTICK_RATE_MS.
    */
    vTaskDelay(1500 / portTICK_RATE_MS);
  }
}

/**
 * TASK 2 PushButton - Queue
 */
void Task2(void *pvParameters){
  
  int sig = 1;
  
  while (1) {
    if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)>0) {
      while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)>0)
        vTaskDelay(100 / portTICK_RATE_MS);
      while(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0)==0)
        vTaskDelay(100 / portTICK_RATE_MS);
      
      GPIO_ToggleBits(GPIOD,GPIO_Pin_13);
      
      xQueueSendToBack(pbq, &sig, 0);
    }
  }
}

/**
 * TASK 3 LEDQ
 */
void Task3(void *pvParameters) {
  
  int sig;
  portBASE_TYPE status;
  
  while (1) {
    status = xQueueReceive(pbq, &sig, 0);
    if(status == pdTRUE) {
      GPIO_ToggleBits(GPIOD,GPIO_Pin_14);
    }
  }
}

/**
 * Init HW
 */
void initHW()
{
  GPIO_InitTypeDef GPIO_InitStructure;
  GPIO_InitTypeDef GPIO_InitStructure2;
  
  // Init LED
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOD, ENABLE);
  GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
  GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOD, &GPIO_InitStructure);
    
  // Init PushButton
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  GPIO_InitStructure2.GPIO_Mode = GPIO_Mode_IN;
  GPIO_InitStructure2.GPIO_Pin =  GPIO_Pin_0;
  GPIO_InitStructure2.GPIO_PuPd = GPIO_PuPd_NOPULL;
  GPIO_InitStructure2.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure2.GPIO_OType = GPIO_OType_PP;
  GPIO_Init(GPIOA, &GPIO_InitStructure2);
}