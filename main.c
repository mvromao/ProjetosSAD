
/* 
 * File:   main.c
 * Author: julito
 *
 * Created on April 15, 2025, 11:31 AM
*/

#include "device_functions.h"

int main(int argc, char** argv) {
	int value;
    char str[200];

    int freqSample = 1;
    int nSample = 5;
    int warningValue = 5;
	
    ANSA = 1;
    TRISA = 0;
    LATA = 0b00000000;
	
    setupTimer1(PR1);
    setupUART1();
    setupChannels();
    setupADC();
    
    TRISAbits.TRISA5 = 1;  // Set RA5 as input
   
    //setupTimer2();
    
    while(1) {
        //transmitPotentiometerData(value, &str);
        //recieveString(value,&str);
        //testChannels();
        //for (volatile long i = 0; i < 500000; i++);  // Adjust value as needed
        //readTemp();
        //monitorDigitalInputs(1);
		
		/*****************************
		INÍCIO
		*****************************/
		
		receiveInput(&str, freqSample, nSample, warningValue);
		
        for (volatile long i = 0; i < 300000; i++);
    }
    return (1);
}