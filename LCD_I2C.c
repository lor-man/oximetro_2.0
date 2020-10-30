/*
 * LCD_I2C.cpp
 *
 *  Created on: 27/09/2020
 *      Author: omar
 */

#include <stdbool.h>
#include <stdint.h>
#include "driverlib/sysctl.h"
#include "driverlib/i2c.h"
#include "LCD_I2C.h"

uint32_t i2c;
uint8_t  lcd,aux_0,aux_1;
uint8_t lcdXY [2][16]={{0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F}, // mapeo de las posiciones visibles
                    {0x40,0x41,0x42,0x43,0x44,0x45,0x46,0x47,0x48,0x49,0x4A,0x4B,0x4C,0x4D,0x4E,0x4F}};  // el corrimiento se realiza a travez de la memoria DDRAM de la lcd

void delayMs(uint32_t ui32Ms) {

    uint32_t aux = 3*1000;
    SysCtlDelay(ui32Ms * (SysCtlClockGet()/aux));
}

void delayUs(uint32_t ui32Us) {
    uint32_t aux=3*1000000;
    SysCtlDelay(ui32Us * (SysCtlClockGet()/aux));
}

void LCD_CMD_I2C(uint8_t dato){// funcion que manda instrucciones
    aux_0=aux_1=0;
    aux_0=dato>>4;
    aux_0=aux_0<<4;
    aux_1=dato<<4;
    I2CMasterSlaveAddrSet(i2c,lcd,false);
    I2CMasterDataPut(i2c,aux_0|LCD_A);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
    I2CMasterDataPut(i2c,aux_0|LCD_A|LCD_E);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);//se manda los 4 bits mas significativos
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
    I2CMasterDataPut(i2c,aux_0|LCD_A);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
    I2CMasterDataPut(i2c,aux_1|LCD_A);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
    I2CMasterDataPut(i2c,aux_1|LCD_A|LCD_E);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);//se mandan los 4 bits menos significativos
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
    I2CMasterDataPut(i2c,aux_1|LCD_A);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
}

void lcd_conf(uint8_t dato){ // funcion auxiliar solo manda 4 bits de los 8 para configurar la lcd
    I2CMasterSlaveAddrSet(i2c,lcd,false);
    I2CMasterDataPut(i2c,dato|LCD_A);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
    I2CMasterDataPut(i2c,dato|LCD_A|LCD_E);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(i2c)){}
    delayUs(20);
    I2CMasterDataPut(i2c,dato|LCD_A);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(i2c)){}
    delayUs(40);
}

void lcd_init(uint32_t i2c_addr, uint8_t lcd_addr){
i2c=i2c_addr;
lcd=lcd_addr;
uint8_t contador;
    for(contador=0;contador<3;contador++){ //modo 8 bits
        lcd_conf(FUNCTION_SET|MODE_8_BIT);
    }
    for(contador=0;contador<2;contador++){ //modo de 4 bits
        lcd_conf(FUNCTION_SET|MODE_4_BIT);
    }
lcd_conf((MODE_2_LINE|MODE_5_8_DOTS)<<4);
LCD_CMD_I2C(DISPLAY_O_F_CONTROL|DISPLAY_ON|CURSOR_ON|BLINK_ON);
LCD_CMD_I2C(ENTRY_MODE_SET|SHIFT_RIGHT|SHIFT_DISPLAY_OFF);
LCD_CMD_I2C(CLEAR);
LCD_CMD_I2C(RETURN);
}
void LCD_PRINT_CHAR(uint8_t caracter){
    aux_0=aux_1=0;
    aux_0=caracter>>4;
    aux_0=aux_0<<4;
    aux_1=caracter<<4;
    I2CMasterSlaveAddrSet(i2c,lcd,false);
    I2CMasterDataPut(i2c,aux_0|LCD_A|LCD_RS);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c)){}
    delayUs(10);
    I2CMasterDataPut(i2c,aux_0|LCD_A|LCD_E|LCD_RS);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);//se manda los 4 bits mas significativos
    while(I2CMasterBusy(i2c)){}
    delayUs(10);
    I2CMasterDataPut(i2c,aux_0|LCD_A|LCD_RS);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(i2c)){}
    delayUs(10);
    I2CMasterDataPut(i2c,aux_1|LCD_A|LCD_RS);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);
    while(I2CMasterBusy(i2c)){}
    delayUs(10);
    I2CMasterDataPut(i2c,aux_1|LCD_A|LCD_E|LCD_RS);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_CONT);//se mandan los 4 bits menos significativos
    while(I2CMasterBusy(i2c)){}
    delayUs(10);
    I2CMasterDataPut(i2c,aux_1|LCD_A|LCD_RS);
    I2CMasterControl(i2c,I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(i2c)){}
}

void LCD_PRINT_STRING(const char *cadena){//imprime cadena de caracteres
    uint8_t cnt;
    for(cnt=0;cadena[cnt]!='\0';cnt++){
        LCD_PRINT_CHAR(cadena[cnt]);
    }
}
void LCD_SET_CURSOR(uint8_t fila, uint8_t columna){ // coloca el cursor en una posici�n especifica fila,columna
    LCD_CMD_I2C(SET_DDRAM|lcdXY[fila-1][columna-1]);
}

void LCD_SHIFT_RIGHT(){
    LCD_CMD_I2C(CURSOR_DISPLAY_SHIFT|DISPLAY_SHIFT_RIGHT);
}
void LCD_SHIFT_LEFT(){
    LCD_CMD_I2C(CURSOR_DISPLAY_SHIFT|DISPLAY_SHIFT_LEFT);
}




