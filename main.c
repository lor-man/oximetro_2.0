
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/adc.h"
#include "driverlib/interrupt.h"
#include "driverlib/timer.h"
#include "LCD_I2C.h"
#include "float_to_string.h"
#include "driverlib/i2c.h"
#include "driverlib/uart.h"
#include "MAX30102.h"


#define LCD_I2C 0x27  //Direccion de la pantalla lcd
#define MAX30102 0x57 //Direccion del MAX30102

uint8_t max30102[6]; //Buffer de datos para la  lectura del max30102
unsigned long intensidad_rojo; // variable don de se almacenan los valores de la lectura del max30102
unsigned long intensidad_ir;   // variable don de se almacenan los valores de la lectura del max30102
float prev_w_red=0; // variable del filtro
float prev_w_ir=0; // variable del filtro
float alpha = 0.99; //Constante para el filtro
float int_rojo; // valor sin componente DC del led Rojo
float int_ir;// valor sin componente DC del led infrarrojo
uint32_t muestras=0; //contador de muestras de Spo2
float acrms_rojo_1,acrms_ir_1,acrms_rojo,acrms_ir,R,spo2; // acrms_rojo_1 y acrms_ir_1 variables temporales para encontrar el valor rms de las se;ales del led rojo e infrarrojo
                                                          // se;al acrms_rojo y acrms_infrarrojo se;ales con valor promedio, R valor para el calculo del SPO2
uint32_t contador_muestras=0; //Contador para el numero de muestras del adc
volatile float temp_promedio=0; //Variable de temperatura promedio
volatile float temp_final=0; //Variable de temperatura final
volatile float Temp_Tiva; // variable traduccion de medida de digital del adc a temperatura en grados centigrados
uint32_t sample[4]; // vectpr de almacenamiento de muestras del ADC

char s_temperatura[7]; // vector para almacenar el dato de temperatura
char s_spo2[7];
char s_envio[50];


struct filtroDC { // Filtro de DC para las se;ales del led rojo e infrarrojo del max30102
float w;
float y;
};

struct filtroDC removedorDC(float x, float prev_w,float alpha){ //Funcion del filtro
    struct filtroDC result;
    result.w = x +alpha*prev_w;
    result.y=result.w - prev_w;
    return result;
};

void configUART(); //Configura el UART
void UARTsprintf(uint32_t ui32Base, const char* ucData_in);// funcion para transmitir una cadena de caracteres a travez de UART
void printLCD(); //Se manda a llamar para imprimir los encabezados en la pantalla
void configI2C0(); //Configura el I2c
void configI2C2(); //Configura el I2c
void configADC(); //Configura el ADC
void configInt(); //Configura las interrupciones
void adc_int_handler(); // funcion encargada de manejar la interrupcion debido al ADC


int main(void)
{
    SysCtlClockSet(SYSCTL_USE_PLL|SYSCTL_OSC_MAIN|SYSCTL_XTAL_16MHZ|SYSCTL_SYSDIV_2_5);//Configura el reloj a 80MHz
    configI2C0();
    configI2C2();
    lcd_init(I2C0_BASE,LCD_I2C);//Inicializa la pantalla lcd
    LCD_CMD_I2C(DISPLAY_O_F_CONTROL|DISPLAY_ON|CURSOR_OFF|BLINK_OFF);//Apaga el cursor y el parpadeo de la pantalla lcd
    printLCD();//Imprime en pantalla encabezados de pantalla

    /////////////////////Configuracion del max en modo SPO2\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\

        wrMAX32(I2C2_BASE,MAX30102,REG_INTERRUPT_ENABLE_1,PPG_RDY_EN);//Habilita la interrupcion por nueva muestra PPG_RDY_EN
        wrMAX32(I2C2_BASE,MAX30102,REG_FIFO_CONF, FIFO_ROLLOVER_EN);//Cuando la memoria fifo esta llena se regresa a la posicion inicial y se llena con nuevos datos
        wrMAX32(I2C2_BASE,MAX30102,REG_MODE_CONF,SPO2_MODE_EN); // Habilita el modo de medicion de spo2
        wrMAX32(I2C2_BASE,MAX30102,REG_SPO2_CONF,SPO2_ADC_RGE_16384|SPO2_SPS_400|SPO2_PW_LED_411); // configura el modo spo2 con 400 muestras por segundo , un ancho de pulso de led de 411us
        wrMAX32(I2C2_BASE,MAX30102,REG_LED_PULSE_AMP_1,LED_CURRENT_12_6); //Configura la corriente del led rojo
        wrMAX32(I2C2_BASE,MAX30102,REG_LED_PULSE_AMP_2,LED_CURRENT_12_6); // configura la corriente del led infrarrojo
        /////////////////////////////////////////////////////////////////////////////////////////////

    configUART();
    configADC();
    configInt();
    acrms_rojo_1=acrms_ir_1=acrms_rojo=acrms_ir=0;// inicializa las variable con un valor
    memset(max30102,0x00,sizeof(max30102));//inicializa el buffer del max30102, del spo2 y de la temperatura
    memset(s_spo2,'\0',sizeof(s_spo2));
    memset(s_temperatura,'\0',sizeof(s_temperatura));

        while(1){

             rdMAX32(I2C2_BASE,MAX30102,REG_INTERRUPT_STATUS_1,1,max30102); // se lee el registro de interrupcion
                        if(PPG_RDY_F & max30102[0]){ // si se dio la interrupcion por nueva muestra se procede a leer la fifo y obtener los 6 bytes de informacion
                        rdMAX32(I2C2_BASE,MAX30102,REG_FIFO_DATA_RGR,6,max30102);// Se lee la fifo
                                      intensidad_rojo=max30102[0]&0b00000011; // se realiza los corrimientos necesarios debido a que cada muestra cuenta con 3 bytes por lo que se deben de
                                      intensidad_rojo=max30102[1]|intensidad_rojo<<8; // realizar corrimientos para obtener el valor tanto en el valor del led rojo e infrarrojo
                                      intensidad_rojo=max30102[2]|intensidad_rojo<<8;
                                      intensidad_ir=max30102[3]&0b00000011;
                                      intensidad_ir=max30102[4]|intensidad_ir<<8;
                                      intensidad_ir=max30102[5]|intensidad_ir<<8;
                                      intensidad_rojo=(float)intensidad_rojo; // se hace una conversion a tipo flotante
                                      intensidad_ir=(float)intensidad_ir;

                                      struct filtroDC red;  // se hace uso de la estructura par el filtrar tanto la se;la del led rojo e ifrarrojo
                                      red = removedorDC(intensidad_rojo,prev_w_red,alpha); // dejando pasar las frecuencias mayores a 0.5Hz
                                      prev_w_red = red.w;
                                      struct filtroDC ir;
                                      ir = removedorDC(intensidad_ir, prev_w_ir,alpha);
                                      prev_w_ir= ir.w;

                                      int_ir = -1.0*((ir.y*16384.0)/262143.0);   // se obtiene el valor sin la componente de DC de la se;al tanto del led rojo como del infrarrojo
                                      int_rojo = -1.0*((red.y*16384.0)/262143.0);

                                      acrms_rojo_1 = int_rojo*int_rojo;  // se obtiene el cuadrado del valor de la se;la AC
                                      acrms_rojo = acrms_rojo+acrms_rojo_1;//del led rojo e infrarrojo

                                      acrms_ir_1 = int_ir*int_ir;
                                      acrms_ir = acrms_ir+acrms_ir_1;

                                      muestras ++; // se incrementa el contador de las muestras
                      }
                       if( muestras == 100){ // cuando se obtienen las 100 muestras
                           acrms_ir = sqrt(acrms_ir/100); // se obtiene el valor de RMS de la se;al tanto del led rojo e infrarrojo
                           acrms_rojo = sqrt(acrms_rojo/100);
                           R = log(acrms_rojo)/log(acrms_ir);// se obtiene el coeficiente R que se utiliza para calcular la cantidad de saturacion del oxigeno en la sangre
                           spo2 = -45.06*((R*R)/10000)+30.354*(R/100)+94.845; // se calcula la cantidad de saturaci[on
                           ftoa(spo2,s_spo2,2); //se convierte el valor de spo2 a un string
                           LCD_SET_CURSOR(1,6);//se coloca el cursor en la posicion fila 2 columna 11
                           LCD_PRINT_STRING(s_spo2); // se imprime la cadena de caracters que contiene la temperatura
                           muestras = 0; // se recetea el contador de muestras
                       }

    }
}

void configUART(){
   // SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB); //Habilita el puerto B donde se encuentra el UART1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1); //Habilita el periferico UART1
    GPIOPinTypeUART(GPIO_PORTB_BASE,GPIO_PIN_0|GPIO_PIN_1); //Define la funcion de UART en los pines 0 y 1 del puerto B a travez del mux interno
    GPIOPinConfigure(GPIO_PB0_U1RX); //pin PB0 como receptor
    GPIOPinConfigure(GPIO_PB1_U1TX); //pin PB1 como transmisor
    UARTConfigSetExpClk(UART1_BASE,SysCtlClockGet(),9600,(UART_CONFIG_WLEN_8|UART_CONFIG_STOP_ONE|UART_CONFIG_PAR_NONE));// Configura la tasa de baudios de transmision/recepcion del uart a 9600 baudios, un ancho de palabra de 8 bits 1 byte, 1 bit de parada y sin paridad
}

void UARTsprintf(uint32_t ui32Base, const char* ucData_in){ // recibe 2 parametros la direccion del modulo uart, y la direccion del vector que contiene el contenido
    uint8_t count_str = 0;  // variable para contar cuando se a llegado al final de la cadena
    while(count_str <= strlen(ucData_in)){  //mientras el largo de cadena sea menor a la variable count_str se mandara el caracter correspondiente a la posicion count_str
        UARTCharPut(ui32Base, ucData_in[count_str]);
        count_str += 1;
    }
}

void printLCD(){
    LCD_CMD_I2C(CLEAR);  // limpia la pantalla lcd
    LCD_CMD_I2C(RETURN); // coloca el cursor al inicio de la pantalla
    LCD_PRINT_STRING("SpO2="); // escribe la palabra SpO2=
    LCD_SET_CURSOR(2,1); // mueve el cursor a la posicion fila 2 columna 1
    LCD_PRINT_STRING("BPM="); // escribe BPM=
    LCD_SET_CURSOR(1,11); //coloca el cursor en  fila 1 columna 9
    LCD_PRINT_STRING("%");// imprime %
    LCD_SET_CURSOR(2,9); //coloca el cursor en la fila 2 columna 9
    LCD_PRINT_STRING("T="); // imprime T=
}

void configI2C0(){
   // SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);//Habilita el periferico puerto A donde se encuentra el I2C1
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);//Habilita el periferico puerto B donde se encuentra el I2C0
    //SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C1);// configura el periferico I2C1
    //SysCtlPeripheralReset(SYSCTL_PERIPH_I2C1); // resetea el periferico
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);// configura el periferico I2C0
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C0); // resetea el periferico
    //GPIOPinConfigure(GPIO_PA6_I2C1SCL);//configura el pin pa6 como el scl
    //GPIOPinConfigure(GPIO_PA7_I2C1SDA);//configura el pin pa7 como el sda
    GPIOPinConfigure(GPIO_PB2_I2C0SCL);//configura el pin pb2 como el scl
    GPIOPinConfigure(GPIO_PB3_I2C0SDA);//configura el pin pb3 como el sda
    //GPIOPinTypeI2CSCL(GPIO_PORTA_BASE,GPIO_PIN_6);// configura a travez del mux interno la funcion de i2c al pin 2 del puerto A
    //GPIOPinTypeI2C(GPIO_PORTA_BASE,GPIO_PIN_7);//configura a travez del mux intenermo la funcion de i2c al pin 3 del puerto A
    GPIOPinTypeI2CSCL(GPIO_PORTB_BASE,GPIO_PIN_2);// configura a travez del mux interno la funcion de i2c al pin 2 del puerto B
    GPIOPinTypeI2C(GPIO_PORTB_BASE,GPIO_PIN_3);//configura a travez del mux intenermo la funcion de i2c al pin 3 del puerto B
    I2CMasterInitExpClk(I2C0_BASE,SysCtlClockGet(),false); // habilita el puerto con una velocidad de 100Kpbs
    //I2CMasterInitExpClk(I2C1_BASE,SysCtlClockGet(),false); // habilita el puerto con una velocidad de 100Kpbs
}
void configI2C2(){

    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);//Habilita el periferico puerto B donde se encuentra el I2C2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);// configura el periferico I2C2
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2); // resetea el periferico
    GPIOPinConfigure(GPIO_PE4_I2C2SCL);//configura el pin pe4 como el scl
    GPIOPinConfigure(GPIO_PE5_I2C2SDA);//configura el pin pe5 como el sda
    GPIOPinTypeI2CSCL(GPIO_PORTE_BASE,GPIO_PIN_4);// configura a travez del mux interno la funcion de i2c al pin 4 del puerto E
    GPIOPinTypeI2C(GPIO_PORTE_BASE,GPIO_PIN_5);//configura a travez del mux intenermo la funcion de i2c al pin 5 del puerto E
    I2CMasterInitExpClk(I2C2_BASE,SysCtlClockGet(),false); // habilita el puerto con una velocidad de 100Kpbs

}

void configADC(){
    GPIOPinTypeADC(GPIO_PORTB_BASE,GPIO_PIN_5); //Habilita la funcion de ADC en el pin 5 del puerto B a travez del mux interno
    SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0); //Habilita el ADC0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);//Habilita el Timer0
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC); //Configura el timer0 como periodico
    TimerLoadSet(TIMER0_BASE,TIMER_A,19999999);//Carga un valor de 80,000 al timer 0    79999999   39999999
    TimerControlTrigger(TIMER0_BASE,TIMER_A,true);//Coloca al timer 0 como disparador del proceso de muestreo del ADC0
    TimerEnable(TIMER0_BASE,TIMER_A);//Habilita el Timer0 para conteo
    ADCSequenceDisable(ADC0_BASE,1); //Deshabilita el secuenciador 1
    ADCSequenceConfigure(ADC0_BASE,1,ADC_TRIGGER_TIMER,0);//Configura que el timer0 inicie el proceso de muestreo del timer 0
    ADCSequenceStepConfigure(ADC0_BASE,1,0,ADC_CTL_CH11); //la secuencia 0 toma el valor del canal analogico 11
    ADCSequenceStepConfigure(ADC0_BASE,1,1,ADC_CTL_CH11);//la secuencia 1 toma el valor del canal analogico 11
    ADCSequenceStepConfigure(ADC0_BASE,1,2,ADC_CTL_CH11);//la secuencia 2 toma el valor del canal analogico 11
    ADCSequenceStepConfigure(ADC0_BASE,1,3,ADC_CTL_CH11|ADC_CTL_END|ADC_CTL_IE);//la secuencia 4 toma el valor del canal analogico 11 y esta es la secuencia final
    ADCSequenceEnable(ADC0_BASE,1);//Habilita el secuenciador 1
}//El secuenciador 1 puede obtener hasta 4 muestras del mismo o diferente canal, por lo tanto tiene una profundidad de 4 bytes de datos

void configInt(){
    ADCIntEnable(ADC0_BASE,1);//Habilita la interrupcion del adc0 por parte del adc
    ADCIntRegister(ADC0_BASE,1,adc_int_handler); // registra la funcion de servicio de la interrupcion
    IntEnable(INT_ADC0SS1);//habilita la interrupcion del secuenciador 1 por parte de la NVIC
    IntMasterEnable(); // Habilita la interrupcion por parte de la NVIC
}

void adc_int_handler(){ // funcion de
    ADCIntClear(ADC0_BASE,1); //limpia la interrupcion producido por el ADC0
    ADCSequenceDataGet(ADC0_BASE,1,sample);// obtiene los datos almacenados en la memoria fifo del secuenciador 1


    if(contador_muestras<=10){ // Se obtienen 1000 muestras de la temperatura para eliminar el error que pueda tener y obtener una lectura estable
        Temp_Tiva=((float)(3.3*(sample[0]+sample[1]+sample[2]+sample[3])/4)/4096)*(100);// se hace la conversion del valor binario del adc a temperatura en grados centigrados
        temp_promedio=temp_promedio+Temp_Tiva;// se suman las 1000 muestras para obtener un promedio
        contador_muestras++; // aumenta el contador de las muestras cada vez que se genera la interrupcion
    }
    else{
        temp_final=temp_promedio/10; // cuando se llega a las 1000 muestras se obtiene la temperatura dividiendo temp_promedio entre 1000
        contador_muestras=0; // se resetea el contador de muestras
        temp_promedio=0; // se resetea el valor de la temperatura promedio
        ftoa(temp_final,s_temperatura,2); //se convierte de un numero flotante a una cadena de caracteres
        LCD_SET_CURSOR(2,11);//se coloca el cursor en la posicion fila 2 columna 11
        LCD_PRINT_STRING(s_temperatura); // se imprime la cadena de caracters que contiene la temperatura
        //s_temperatura[5]='T';

        UARTsprintf(UART1_BASE,s_spo2); // se envia la temperatura ya en formato de caracter por medio del Uart
       memset(s_spo2,'\0',sizeof(s_spo2)); // se resetea el valor del vector que contiene la temperatura para elminar cualquier dato restante en la siguiente impresion.
        UARTsprintf(UART1_BASE,s_temperatura); // se envia la temperatura ya en formato de caracter por medio del Uart
       memset(s_temperatura,'\0',sizeof(s_temperatura)); // se resetea el valor del vector que contiene la temperatura para elminar cualquier dato restante en la siguiente impresion.
    }
}




