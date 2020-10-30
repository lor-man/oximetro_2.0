/*
 * MAX30102.h
 *
 *  Created on: 20/10/2020
 *      Author: omar
 */

#ifndef MAX30102_H_
#define MAX30102_H_
//REGISTROS
#define REG_INTERRUPT_STATUS_1 0x00  //Estado de interrupcion
#define REG_INTERRUPT_STATUS_2 0x01  //Estado de interrupcion
#define REG_INTERRUPT_ENABLE_1 0x02  //Habilitacion de interrupcion
#define REG_INTERRUPT_ENABLE_2 0x03  //Habilitacion de interrupcion
#define REG_FIFO_WR_PTR 0x04  //Puntero de escritura
#define REG_OVERFLOW_CNT 0x05 //Conteo de muestras perdidas
#define REG_FIFO_RD_PTR 0x06  //Puntero de lectura
#define REG_FIFO_DATA_RGR 0x07  //Datos de fifo
#define REG_FIFO_CONF 0x08  //Configuracion de fifo
#define REG_MODE_CONF 0x09  //Configuracion de modo
#define REG_SPO2_CONF 0x0A  //Configuracion de modo SPO2
#define REG_LED_PULSE_AMP_1 0x0C  //Configuracion de amplitud de led Rojo
#define REG_LED_PULSE_AMP_2 0x0D  //Configuracion de amplitud de led IR
#define REG_MULTI_LED_1 0x11  // Configuracion de modo de led
#define REG_MULTI_LED_2 0x12  // Configuracion de modod de led
#define REG_TEMP_INT 0x1F // Parte entera de temperatura
#define REG_TEMP_FRAC 0x20  //Parte decimal de temperatura
#define REG_TEMP_CONF 0X21  //Habilitacion de 1 muestra de temperatura
#define REG_REV_ID 0xFE  //Revision
#define REG_PART_ID 0XFF  //Numero de parte

//INTERRUPCIONES--BANDERAS
#define A_FULL_F 0x80 //Fifo casi llena
#define PPG_RDY_F 0x40  //Dato nuevo en la fifo
#define ALC_OVF_F 0x20  //Cancelacion de luz ambiental excedida
#define PWR_RDY 0x01  //Modulo listo
#define DIE_TEMP_RDY 0x02  //Muestra de temperatura lista

//HABILITACION DE INTERRUPCIONES
#define A_FULL_EN  0x80
#define PPG_RDY_EN 0x40
#define ALC_OVF_EN 0x20
#define DIE_TEMP_RDY_EN 0x02

//CONFIGURACION DEL FIFO--
#define SMP_AVE_NONE 0x00  // Sin muestras promedio
#define SMP_AVE_2 0x20  //2 muestras
#define SMP_AVE_4 0x40  //4 muestras
#define SMP_AVE_8 0x60  //8 muestras
#define SMP_AVE_16 0x80  //16 muestras
#define SMP_AVE_32 0xA0  // 32 muestras -- 110 111
#define FIFO_ROLLOVER_EN 0x10
// FIFO_A_FULL Determina cuantas muestras quedan en la FIFO cuando se emite la interrupcion valores desde
// 0x0 hasta 0xF  por ejemplo si FIFO_A_FULL= 0x0 entonces quedan 0 datos vacias en la fifo cuando se da la interrupcion
// y 32 datos sin leer. 0x0--> 0 vacios 32 sin leer, 0xF--> 15 vacias y 17 sin leer

//CONFIGURACION DE MODO DE OPERACION
#define SHDN 0x80   //Apaga el sensor
#define RESET 0x40 //Resetea el sensor
#define HEART_RATE_MODE_EN 0x02 //Habilita el modo de pulso
#define SPO2_MODE_EN 0x03  //Habilita el modo de SPO2
#define MULTI_LED 0x07 //HABILITA EL MODO MULTILED

//CONFIGURACION SPO2
#define SPO2_ADC_RGE_2048 0x00 //Escala de 2048nA
#define SPO2_ADC_RGE_4096 0x20  //Escala de 4096nA
#define SPO2_ADC_RGE_8192 0x40 //Escala de 8192nA
#define SPO2_ADC_RGE_16384 0x60 //Escala de 16384nA

#define SPO2_SPS_50  0x00 // Muestras por segundo 50
#define SPO2_SPS_100 0x04 // Muestras por segundo 100
#define SPO2_SPS_200 0x08// Muestras por segundo 200
#define SPO2_SPS_400 0x0C// Muestras por segundo 400
#define SPO2_SPS_800 0x10// Muestras por segundo 800
#define SPO2_SPS_1000 0x14 // Muestras por segundo 1000
#define SPO2_SPS_1600 0x18// Muestras por segundo 1600
#define SPO2_SPS_3200 0x1C// Muestras por segundo 3200

#define SPO2_PW_LED_69 0x00 //Ancho de pulso de 69us con resolucion de 15bits
#define SPO2_PW_LED_118 0x01 //Ancho de pulso de 118us con resolucion de 16bits
#define SPO2_PW_LED_215 0x02 //Ancho de pulso de 215us con resolucion de 17bits
#define SPO2_PW_LED_411 0x03 //Ancho de pulso de 411us con resolucion de 18bits

//Corriente de led
#define LED_CURRENT_0_0 0x00  //Corriente de LED D1-->Rojo o D2-->IR 0mA
#define LED_CURRENT_0_2 0x01 //Corriente de LED D1-->Rojo o D2-->IR 0.2mA
#define LED_CURRENT_0_4 0x02 //Corriente de LED D1-->Rojo o D2-->IR 0.4mA
#define LED_CURRENT_3_0 0x0F //Corriente de LED D1-->Rojo o D2-->IR 3.0mA
#define LED_CURRENT_6_2 0x1F //Corriente de LED D1-->Rojo o D2-->IR 6.2mA
#define LED_CURRENT_12_6 0x3F //Corriente de LED D1-->Rojo o D2-->IR 12.6mA
#define LED_CURRENT_25_4 0x7F //Corriente de LED D1-->Rojo o D2-->IR 25.4mA
#define LED_CURRENT_51_0 0xFF //Corriente de LED D1-->Rojo o D2-->IR 51.0mA

void wrMAX32(uint32_t i2c_base,uint8_t addrMax,uint8_t addrReg,uint8_t valReg);
void rdMAX32(uint32_t i2c_base,uint8_t addrMax,uint8_t addrReg,uint8_t num_bytes,uint8_t *buffer);


#endif /* MAX30102_H_ */
