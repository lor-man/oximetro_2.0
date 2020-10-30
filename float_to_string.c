/*
 * float_to_string.c
 *
 *  Created on: 1/10/2020
 *      Author: omar
 */
#include<math.h>
void reverse(char* str, int len){ // invierte el orden en un vector de caracteres
    int i = 0, j=len-1,temp;
    while(i<j){
        temp=str[i];
        str[i]=str[j];
        str[j]=temp;
        i++;
        j--;
    }
}
int intToStr(int x,char str[],int d){// convierte un valor de entero a su equivalente en caracter
    int i=0;
    while(x){
        str[i++]=(x%10)+'0';
        x=x/10;
    }
    while(i<d)
        str[i++]='0';

    reverse(str,i);
    str[i]='\0';
    return i;
}
void ftoa(float n, char*res, int afterpoint){ // funcion principal para convertir el numero de punto flotante en una cadena de caracteres con la precision deseada
    int ipart = (int)n;
    float fpart = n - (float)ipart;
    int i = intToStr(ipart, res, 0);
    if(afterpoint !=0){
        res[i]='.';
        fpart = fpart *pow(10,afterpoint);
        intToStr((int)fpart,res+i+1,afterpoint);
    }
}

