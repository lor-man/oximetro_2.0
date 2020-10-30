/*
 * LCD_I2C.h
 *
 *  Created on: 27/09/2020
 *      Author: omar
 RS------>P0
 RW------>P1
 E------->P2
 A------->P3
 D4/D0------>P4
 D5/D1------>P5
 D6/D2------>P6
 D7/D3------>P7
 DATOS DE I2C
 D7-D6-D5-D4-A-E-RW-RS
 MSB---------------LSB
*/
#ifndef LCD_I2C_H_
#define LCD_I2C_H_

#define CLEAR   0x01 //limpia la pantalla
#define RETURN  0x02//Retorna al inicio
#define ENTRY_MODE_SET 0x04
#define DISPLAY_O_F_CONTROL 0x08
#define CURSOR_DISPLAY_SHIFT 0x10
#define FUNCTION_SET 0x20
#define SET_CGRAM   0x40
#define SET_DDRAM   0x80

#define LCD_RS 0x01
#define LCD_RW 0X02
#define LCD_E  0x04
#define LCD_A  0x08

//Entry mode set DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
               //0   0   0   0   0   1   I/D S
#define SHIFT_RIGHT 0x02
#define SHIFT_LEFT  0x00
#define SHIFT_DISPLAY_ON 0x01
#define SHIFT_DISPLAY_OFF 0x00

//Display on/off control DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
                       //0   0   0   0   1   D   C   B
#define DISPLAY_ON 0x04
#define DISPLAY_OFF 0x00
#define CURSOR_ON  0x02
#define CURSOR_OFF 0x00
#define BLINK_ON    0x01
#define BLINK_OFF   0x00

//Cursor of display shift DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
                        //0   0   0   1   S/C R/L 0   0
#define CURSOR_SHIFT_LEFT 0x00 //Corrimiento de cursor a la izquierda
#define CURSOR_SHIFT_RIGHT 0x04 //Corrimiento de cursor a la derecha
#define DISPLAY_SHIFT_LEFT 0x08 //Corrimiento de pantalla a la izquierda
#define DISPLAY_SHIFT_RIGHT 0x0C//Corrimiento de pantalla a la derecha
//Function set DB7 DB6 DB5 DB4 DB3 DB2 DB1 DB0
             //0   0   1   DL  N   F   0   0
#define MODE_8_BIT 0x10
#define MODE_4_BIT 0x00
#define MODE_1_LINE 0x00
#define MODE_2_LINE 0x08
#define MODE_5_8_DOTS 0x00
#define MODE_5_10_DOTS 0x04

void lcd_init(uint32_t i2c_addr, uint8_t lcd_addr); // inicializa la pantalla necesita como parametros la direccion de la pantalla y la direccion del i2c que se este utilizando
void delayMs(uint32_t ui32Ms); //Retardo aproximado en ms por medio de la funcion SysCtlDelay
void delayUs(uint32_t ui32Ms);//Retardo aproximado en us por medio de la funcion SysCtlDelay
void lcd_conf(uint8_t dato);//funcion auxiliar utilizada por lcd_init()
void LCD_CMD_I2C(uint8_t dato);//funcion para enviar instrucciones a la lcd
void LCD_PRINT_CHAR(uint8_t caracter);// funcion que imprime el caracter deseado
void LCD_PRINT_STRING(const char *cadena);//imprime una cadena de caracteres utiliza la funcion LCD_PRINT_CHAR
void LCD_CURSOR_OFF();// apaga el cursor
void LCD_SET_CURSOR(uint8_t fila, uint8_t columna); //mueve el cursor a la posicion indicada
void LCD_SHIFT_RIGHT();// realiza un corrimiento de pantalla hacia la izquierda
void LCD_SHIFT_LEFT();// realiza un corrimiento de pantalla hacia la derecha

#endif /* LCD_I2C_H_ */
