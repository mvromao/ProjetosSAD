/* 
 * File:   main.c
 * Author: Renedito
 *
 * Created on May 6, 2025, 12:08 PM
 */

#include <stdio.h>
#include <stdlib.h>

/*
 * 
 */
int main(int argc, char** argv) {
    I2CxCONbits.SEN = 1;
    I2CxSTATbits.TBF;       // Buffer full
    PEN = 1;
            
    return (EXIT_SUCCESS);
}

