/* 
 * File:   main.c
 * Author: Renedito
 *
 * Created on April 1, 2025, 10:57 AM
 */

#include <stdio.h>
#include <stdlib.h>

#include <p24fj1024gb610.h>
/*
 * 
 */



int main(int argc, char** argv) {
    //  Setup       Activia Low
    ANSA = 1;   //  Analogico
    ANSD = 1;
    
    TRISA = 0;  //  
    
    LATA = 0b00000000;
    
    uint32_t i = 0;
    TRISDbits.TRISD6 = 1; //input

    //  Loop
    while(1){
        if(!PORTDbits.RD6) {
            LATA = 0b00001111;
        } else {
            LATA = 0b11110000;
        }
    }
    return (1);
}