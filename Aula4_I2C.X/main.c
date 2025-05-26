
/* 
 * File:   main.c
 * Author: julito
 *
 * Created on April 15, 2025, 11:31 AM
*/

#include "device_functions.h"

int main(int argc, char** argv) { 
    int values[5];
    
    I2C2_Init();
    printf("Started on printf!\n");
    writeString("\nStarted!\n");
    
    while(1) {
        I2C2_SendToArduino(0xAC);
        I2C2_ReadFromArduino(values, 5);
        writeString(values);
        for (volatile long i = 0; i < 300000; i++);
        
    }
    return (1);
}