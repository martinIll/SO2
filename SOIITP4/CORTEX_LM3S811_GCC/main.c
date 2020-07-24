/*
 * FreeRTOS Kernel V10.3.1
 * Copyright (C) 2020 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://www.FreeRTOS.org
 * http://aws.amazon.com/freertos
 *
 * 1 tab == 4 spaces!
 */


/*
 * This project contains an application demonstrating the use of the
 * FreeRTOS.org mini real time scheduler on the Luminary Micro LM3S811 Eval
 * board.  See http://www.FreeRTOS.org for more information.
 *
 * main() simply sets up the hardware, creates all the demo application tasks,
 * then starts the scheduler.  http://www.freertos.org/a00102.html provides
 * more information on the standard demo tasks.
 *
 * In addition to a subset of the standard demo application tasks, main.c also
 * defines the following tasks:
 *
 * + A 'Print' task.  The print task is the only task permitted to access the
 * LCD - thus ensuring mutual exclusion and consistent access to the resource.
 * Other tasks do not access the LCD directly, but instead send the text they
 * wish to display to the print task.  The print task spends most of its time
 * blocked - only waking when a message is queued for display.
 *
 * + A 'Button handler' task.  The eval board contains a user push button that
 * is configured to generate interrupts.  The interrupt handler uses a
 * semaphore to wake the button handler task - demonstrating how the priority
 * mechanism can be used to defer interrupt processing to the task level.  The
 * button handler task sends a message both to the LCD (via the print task) and
 * the UART where it can be viewed using a dumb terminal (via the UART to USB
 * converter on the eval board).  NOTES:  The dumb terminal must be closed in
 * order to reflash the microcontroller.  A very basic interrupt driven UART
 * driver is used that does not use the FIFO.  19200 baud is used.
 *
 * + A 'check' task.  The check task only executes every five seconds but has a
 * high priority so is guaranteed to get processor time.  Its function is to
 * check that all the other tasks are still operational and that no errors have
 * been detected at any time.  If no errors have every been detected 'PASS' is
 * written to the display (via the print task) - if an error has ever been
 * detected the message is changed to 'FAIL'.  The position of the message is
 * changed for each write.
 */



/* Environment includes. */
#include "DriverLib.h"

/* Scheduler includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

/* Demo app includes. */
#include "integer.h"
#include "PollQ.h"
#include "semtest.h"
#include "BlockQ.h"
#include "string.h"
#include "printf-stdarg.h"

/* Delay between cycles of the 'check' task. */
#define mainCHECK_DELAY						( ( TickType_t ) 5000 / portTICK_PERIOD_MS )

/* UART configuration - note this does not use the FIFO so is not very
efficient. */
#define mainBAUD_RATE				( 19200 )
#define mainFIFO_SET				( 0x10 )

/* Demo task priorities. */
#define mainQUEUE_POLL_PRIORITY		( tskIDLE_PRIORITY + 2 )
#define mainCHECK_TASK_PRIORITY		( tskIDLE_PRIORITY + 3 )
#define mainSEM_TEST_PRIORITY		( tskIDLE_PRIORITY + 1 )
#define mainBLOCK_Q_PRIORITY		( tskIDLE_PRIORITY + 2 )

/* Demo board specifics. */
#define mainPUSH_BUTTON             GPIO_PIN_4

/* Misc. */
#define mainQUEUE_SIZE				( 8 )
#define mainDEBOUNCE_DELAY			( ( TickType_t ) 150 / portTICK_PERIOD_MS )
#define mainNO_DELAY				( ( TickType_t ) 0 )


#define SMALL 40
#define TINY 10
#define TOP_TASK 1

/*
 * Configure the processor and peripherals for this demo.
 */
static void prvSetupHardware( void );

/*task productor*/
static void vProductorTask(void *pvParameters);
/*task consumidor*/
static void vConsumidorTask(void *pvParameters );
/*task top*/
static void vTopTask(void *pvParameters );


/*funcion auxiliar para impresion por uart*/
static void uartPrint(char * msg,uint8_t len);

/*funcion auxiliar para imprimir el watermark de un task*/
static void printWaterMark( );

/*funcion auxiliar para imprimir cantidad de espacios disponibles y usados en la cola*/
static void printUsedFree();

/* String that is transmitted on the UART. */
static char *cMessage = "Task woken by button interrupt! --- ";
static volatile char *pcNextChar;

/* The semaphore used to wake the button handler task from within the GPIO
interrupt handler. */
SemaphoreHandle_t xButtonSemaphore;

/* The queue used to send strings to the print task for display on the LCD. */
QueueHandle_t xPrintQueue;

/*-----------------------------------------------------------*/
static const char *pcTextForTask0 = "0\r\n";
static const char *pcTextForTask1 = "1\r\n";
static const char *pcTextForTask2 = "2\r\n";
static const char *pcTextForTask3 = "3\r\n";

/**
 * @brief funcion main se hacen las configuraciones del hardware, se inicializa la cola y se inician las task

 *@returns int

*/
int main( void )
{
	/* Configure the clocks, UART and GPIO. */
	prvSetupHardware();

	/* Create the semaphore used to wake the button handler task from the GPIO
	ISR. */
	// vSemaphoreCreateBinary( xButtonSemaphore );
	// xSemaphoreTake( xButtonSemaphore, 0 );

	 /* Create the queue used to pass message to vPrintTask. */
	 xPrintQueue = xQueueCreate( mainQUEUE_SIZE, sizeof( char * ) );
	
	if(TOP_TASK){
		xTaskCreate( vTopTask, "Top", configMINIMAL_STACK_SIZE+(SMALL*2), NULL, 3, NULL );
	}
	
	xTaskCreate( vProductorTask, "Prod0", configMINIMAL_STACK_SIZE, (void*)pcTextForTask0, 1, NULL );
	xTaskCreate( vProductorTask, "Prod1", configMINIMAL_STACK_SIZE, (void*)pcTextForTask1, 1, NULL );
	xTaskCreate( vProductorTask, "Prod2", configMINIMAL_STACK_SIZE, (void*)pcTextForTask2, 1, NULL );
	xTaskCreate( vProductorTask, "Prod3", configMINIMAL_STACK_SIZE, (void*)pcTextForTask3, 1, NULL );
	xTaskCreate( vConsumidorTask, "Cons1", configMINIMAL_STACK_SIZE, NULL, 2, NULL );



	/* Start the scheduler. */
	vTaskStartScheduler();

	/* Will only get here if there was insufficient heap to start the
	scheduler. */

	return 0;
}
/*-----------------------------------------------------------*/



static void prvSetupHardware( void )
{
	/* Setup the PLL. */
	SysCtlClockSet( SYSCTL_SYSDIV_10 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_6MHZ );

	
	/* Enable the UART.  */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

	/* Set GPIO A0 and A1 as peripheral function.  They are used to output the
	UART signals. */
	GPIODirModeSet( GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1, GPIO_DIR_MODE_HW );

	/* Configure the UART for 8-N-1 operation. */
	UARTConfigSet( UART0_BASE, mainBAUD_RATE, UART_CONFIG_WLEN_8 | UART_CONFIG_PAR_NONE | UART_CONFIG_STOP_ONE );

	/* We don't want to use the fifo.  This is for test purposes to generate
	as many interrupts as possible. */
	HWREG( UART0_BASE + UART_O_LCR_H ) &= ~mainFIFO_SET;

	/* Enable Tx interrupts. */
	HWREG( UART0_BASE + UART_O_IM ) |= UART_INT_TX;
	IntPrioritySet( INT_UART0, configKERNEL_INTERRUPT_PRIORITY );
	IntEnable( INT_UART0 );


	/* Initialise the LCD> */
    OSRAMInit( false );
    OSRAMStringDraw("www.FreeRTOS.org", 0, 0);
	OSRAMStringDraw("LM3S811 demo", 16, 1);
}
/*-----------------------------------------------------------*/

void vUART_ISR(void)
{
unsigned long ulStatus;

	/* What caused the interrupt. */
	ulStatus = UARTIntStatus( UART0_BASE, pdTRUE );

	/* Clear the interrupt. */
	UARTIntClear( UART0_BASE, ulStatus );

	/* Was a Tx interrupt pending? */
	if( ulStatus & UART_INT_TX )
	{
		/* Send the next character in the string.  We are not using the FIFO. */
		if( *pcNextChar != 0 )
		{
			if( !( HWREG( UART0_BASE + UART_O_FR ) & UART_FR_TXFF ) )
			{
				HWREG( UART0_BASE + UART_O_DR ) = *pcNextChar;
			}
			pcNextChar++;
		}
	}
}
/*-----------------------------------------------------------*/

void vGPIO_ISR( void )
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;

	/* Clear the interrupt. */
	GPIOPinIntClear(GPIO_PORTC_BASE, mainPUSH_BUTTON);

	/* Wake the button handler task. */
	xSemaphoreGiveFromISR( xButtonSemaphore, &xHigherPriorityTaskWoken );

	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
}
/*-----------------------------------------------------------*/

/**
 * @brief task productor imprime su nombre y agrega un string con su numero a la cola junto con su watermark y los lugares disponibles y en uso de la cola
 * @param pvParameters numero del productor
 *@returns void

*/

static void vProductorTask(void *pvParameters )
{
	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{
		char* pcTaskName;
		char printString[SMALL];
		pcTaskName = ( char * ) pvParameters;
		//imprime el nombre del productor	
		sprintf(printString,"productor %s",pcTaskName);
		uartPrint( printString , strlen(printString) );
		printWaterMark();
		printUsedFree();
		xQueueSend( xPrintQueue, &pcTaskName, portMAX_DELAY );
		//delay para la proxima ejecucion	
		vTaskDelay( pdMS_TO_TICKS(2500) );
	}
}

/**
 * @brief task consumidor toma elementos de la cola y e imprime de quien lo consumio junto con su watermark y los lugares disponibles y en uso de la cola
 * @param pvParameters  vacio pero esta por la especificacion de FreeRTOS
 * *@returns void

*/
static void vConsumidorTask(void *pvParameters )
{
	

	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{	
		char* pcMessage=pvPortMalloc(sizeof(char)*SMALL);
		char pcTaskName[SMALL];
		xQueueReceive( xPrintQueue, &pcMessage, portMAX_DELAY );
		//imprimo de que productor esta consumiendo
		sprintf(pcTaskName,"consumo de productor  %s",pcMessage);
		uartPrint( pcTaskName , strlen(pcTaskName) );
		//se  imprime el watermark y las estadisticas de espacio
		printWaterMark();
		vPortFree(pcMessage);
		printUsedFree();
		//delay para la proxima ejecucion	
		vTaskDelay( pdMS_TO_TICKS(1000) );
	}
	

	
}

/**
 * @brief task TOP imprime informacion sobre las tareas en uso 
 * @param pvParameters  vacio pero esta por la especificacion de FreeRTOS
 * *@returns void

*/
static void vTopTask(void *pvParameters )
{
	//inicializo los strings estaticamente
	char pcMessage[SMALL*7];
	char separator[46];
	char header[46];
	/* As per most tasks, this task is implemented in an infinite loop. */
	for( ;; )
	{	
		//guardo el separador y el encabezado en sus respectivas variubles despues imprimo la salida de vtaskList con formato
		sprintf(separator,"**********************************************");
		sprintf(header,"Task            estado   Prio  watermark  Num");
		uartPrint( separator , strlen(separator) );
		uartPrint( header , strlen(header) );
		uartPrint( separator , strlen(separator) );
		vTaskList(pcMessage);
		uartPrint( pcMessage , strlen(pcMessage) );
		uartPrint( separator , strlen(separator) );
		uartPrint( separator , strlen(separator) );
		printWaterMark();
		//delay para la proxima ejecucion
		vTaskDelay( pdMS_TO_TICKS(10000) );
		
	}

}


/**
 * @brief transmite el mensaje indicado caracter por caracter por el puerto serie 
 * @param char*  mensaje a transmitirse por el puerto serie
 * @param uint8_t largo del mensaje
 * *@returns void

*/
static void uartPrint(char * msg,uint8_t len)
{
	//recoro el msg caracter por caracter y lo transmito
	for(uint8_t i=0;i<len;i++){
		UARTCharPut(UART0_BASE,(unsigned)msg[i]);
	}
	//imprimo un return carriage al final
	UARTCharPut(UART0_BASE,'\n');
}

/**
 * @brief imprime el watermark de la tarea que lo llama 
 * *@returns void

*/
static void printWaterMark( )
{
	UBaseType_t uxHighWaterMark;
	char  completeWatermark[SMALL];
	//recupero el watermark
	uxHighWaterMark = uxTaskGetStackHighWaterMark( NULL );
	//lo imprimo
	sprintf(completeWatermark, "watermark-> %d\n", uxHighWaterMark);
	uartPrint(completeWatermark,strlen(completeWatermark));
}

/**
 * @brief imprime la cantidad de espacios en uso y disponibles de la cola en el momento del llamado 
 * *@returns void

*/
static void printUsedFree(){
	UBaseType_t available,used;
	char string[SMALL];
	//recupero espacios en uso y disponibles	
	available=uxQueueSpacesAvailable(xPrintQueue);
	used=uxQueueMessagesWaiting(xPrintQueue);
	//los imprimo
	sprintf(string,"Espacios usados:%d\nEspacios disponibles:%d\n",used,available);
	uartPrint(string,strlen(string));
}
