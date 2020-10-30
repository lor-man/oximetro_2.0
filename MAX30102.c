/*
 * MAX30102.C
 *
 *  Created on: 20/10/2020
 *      Author: omar
 */

#include <stdint.h>
#include <stdbool.h>
//#include "inc/tm4c123gh6pm.h"
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/i2c.h"
#include "MAX30102.h"

uint8_t rd_cont;//variable para contador de funcion de lectura


void wrMAX32(uint32_t i2c_base,uint8_t addrMax,uint8_t addrReg,uint8_t valReg){ //Escribe sobre un registro del MAX30102

    I2CMasterSlaveAddrSet(i2c_base,addrMax,false);
    I2CMasterDataPut(i2c_base,addrReg);
    I2CMasterControl(i2c_base,I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c_base)){}
    I2CMasterDataPut(i2c_base,valReg);
    I2CMasterControl(i2c_base,I2C_MASTER_CMD_BURST_SEND_FINISH);
    while(I2CMasterBusy(i2c_base)){}
}

void rdMAX32(uint32_t i2c_base,uint8_t addrMax,uint8_t addrReg,uint8_t num_bytes,uint8_t *buffer){
    I2CMasterSlaveAddrSet(i2c_base,addrMax,false);
    I2CMasterDataPut(i2c_base,addrReg);
    I2CMasterControl(i2c_base,I2C_MASTER_CMD_BURST_SEND_START);
    while(I2CMasterBusy(i2c_base)){}

       if(num_bytes>1){  /// si solo se desea leer una muestra del max30102 debido a que una lectura de datos devuelve como maximo 6 bytes de informacion

             for(rd_cont=0;rd_cont<num_bytes-1;rd_cont++){
               if(rd_cont==0){
                   I2CMasterSlaveAddrSet(i2c_base,addrMax,true);
                   I2CMasterControl(i2c_base,I2C_MASTER_CMD_BURST_RECEIVE_START);
                   while(I2CMasterBusy(i2c_base)){}
                   buffer[rd_cont]=I2CMasterDataGet(i2c_base);
               }
               else{
                   I2CMasterControl(i2c_base,I2C_MASTER_CMD_BURST_RECEIVE_CONT);
                   while(I2CMasterBusy(i2c_base)){}
                   buffer[rd_cont]=I2CMasterDataGet(i2c_base);
               }
           }
           I2CMasterControl(i2c_base,I2C_MASTER_CMD_BURST_RECEIVE_FINISH);
           while(I2CMasterBusy(i2c_base)){}
           buffer[5]=I2CMasterDataGet(i2c_base);
       }
       else
           if(num_bytes==1){//Si solo se desea leer un byte de informacion devolvera una solo byte
               I2CMasterSlaveAddrSet(i2c_base,addrMax,true);
               I2CMasterControl(i2c_base,I2C_MASTER_CMD_SINGLE_RECEIVE);
               while(I2CMasterBusy(i2c_base)){}
               buffer[0]=I2CMasterDataGet(i2c_base);
           }



}

