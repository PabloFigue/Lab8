/*
 *Laboratorio 8
 *Pablo Andres Figueroa Gámez
*/


/////////PARTE 1 ////////////
#include <stdint.h>
#include <stdbool.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "driverlib/sysctl.h"
#include "driverlib/interrupt.h"
#include "driverlib/gpio.h"
#include "driverlib/timer.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"

//Se definen los pines de los leds
#define TOGGLE_PIN   GPIO_PIN_6
#define RED_LED      GPIO_PIN_1
#define GREEN_LED    GPIO_PIN_3
#define BLUE_LED     GPIO_PIN_2

// Variables volátiles globales para el estado de los LEDs
volatile bool toggle_red = false;
volatile bool toggle_green = false;
volatile bool toggle_blue = false;
volatile uint32_t ui32Loop;

//delay para los LEDS
void delay(void) {
    for(ui32Loop = 0; ui32Loop < 3000000; ui32Loop++) {} // Simple delay
}

//////////// Prototipos de funciones/////////////////////
void setupTimer0(void);
void setupUART0(void);
void UARTIntHandler(void);
void Timer0IntHandler(void);

int main(void)
{
    SysCtlClockSet(SYSCTL_SYSDIV_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ); //Se configura el reloj interno a 40Mhz

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);  // Habilita puerto F
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF));
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED | GREEN_LED | BLUE_LED);  // LEDs como salida

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);  // Habilita puerto A
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA));
    GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, TOGGLE_PIN);  // pin de toggle como salida

    setupTimer0();  // Configura el Timer0
    setupUART0();  // Configura el UART0
    IntMasterEnable();  // interrupciones globales

    while(1) //Loop
    {
        ////////////////CODIGO GENERAL ///////////////////
    }
}

/////////////////FUNCIONES DE CONFIGURACIÓN//////////////////////////////////////////

void setupTimer0(void) //configuracion del TMR0
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);  // Habilita el Timer0
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_TIMER0));
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);  // Configura el Timer0 como periódico de 32 bits

    TimerLoadSet(TIMER0_BASE, TIMER_BOTH, SysCtlClockGet() / 4);  // Establece el valor de carga del Timer0
    IntEnable(INT_TIMER0A);  // Habilita la interrupción del Timer0A
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  // Habilita la interrupción por timeout en el Timer0A
    TimerEnable(TIMER0_BASE, TIMER_A);  // Habilita el Timer0A
}

void setupUART0(void) //configuracion del UART
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);  // Habilita el reloj para el periférico UART0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    GPIOPinConfigure(GPIO_PA0_U0RX);  // Configura los pines PA0 y PA1 para ser utilizados como pines de UART
    GPIOPinConfigure(GPIO_PA1_U0TX);  // Configura los pines PA0 y PA1 para ser utilizados como pines de UART
    GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);  // Configura los pines PA0 y PA1 para ser utilizados como pines de UART
    UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(), 115200, (UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE | UART_CONFIG_PAR_NONE));  // Configura el UART0 con una tasa de baudios de 115200, 8 bits de datos, 1 bit de parada y sin paridad

    IntEnable(INT_UART0); //Habilitar interrupciones para el UART0

    UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);  // Habilita las interrupciones de recepción y transmisión del UART0

    UARTEnable(UART0_BASE); //Iniciar UART0
}


//////////////////////////FUNCIONES DE SERVICIO DE INTERRUPCION////////////////////////////////

void Timer0IntHandler(void)
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);  // Limpia la interrupción del Timer0A

    if (toggle_red)
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED, GPIOPinRead(GPIO_PORTF_BASE, RED_LED) ^ RED_LED);  // Toggle red LED
    if (toggle_green)
        GPIOPinWrite(GPIO_PORTF_BASE, GREEN_LED, GPIOPinRead(GPIO_PORTF_BASE, GREEN_LED) ^ GREEN_LED);  // Toggle green LED
    if (toggle_blue)
        GPIOPinWrite(GPIO_PORTF_BASE, BLUE_LED, GPIOPinRead(GPIO_PORTF_BASE, BLUE_LED) ^ BLUE_LED);  // Toggle blue LED


    // Toggling TOGGLE_PIN en el puerto A
    GPIOPinWrite(GPIO_PORTA_BASE, TOGGLE_PIN, GPIOPinRead(GPIO_PORTA_BASE, TOGGLE_PIN) ^ TOGGLE_PIN);  // Toggle TOGGLE_PIN
}


void UARTIntHandler(void)
{
    char cReceived = 0;
    UARTIntClear(UART0_BASE, UART_INT_RX | UART_INT_RT);  // Limpiar bandera de interrupción para recepción y transmisión


    if(UARTCharsAvail(UART0_BASE)) // verfica si hay un caracter disponible
    {
        cReceived = UARTCharGetNonBlocking(UART0_BASE);
        UARTCharPutNonBlocking(UART0_BASE, cReceived);
    }

    if (cReceived == 'r')  // Si se recibió una 'r'
    {

        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED | BLUE_LED | GREEN_LED, 0);//apaga

        toggle_red = !toggle_red;

    }
    else if (cReceived == 'g')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED | BLUE_LED | GREEN_LED, 0);//apaga

        toggle_green = !toggle_green;

    }
    else if (cReceived == 'b')
    {
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED | BLUE_LED | GREEN_LED, 0);//apaga

        toggle_blue = !toggle_blue;

    }
}
