
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
 
    setupUART1();
    setupChannels();
    setupADC();

    writeString("=============================================================\n       WILKOMMEN AUS GRUN HOREN - DEUTSCHE BURGENWERK   \n  ENNTWICKELT VON  ROMAO, JULIO LOPES E RODRIGO SIMOES\n==============================================================\n"    );
    while(1) {
        //transmitPotentiometerData(value, &str);
        //recieveString(value,&str);
        //testChannels();
        //for (volatile long i = 0; i < 500000; i++);  // Adjust value as needed
        //readTemp();
        //monitorDigitalInputs(1);
		
		/*****************************
		IN?CIO
		*****************************/
         

		receiveInput(&str);
		checkWarningValue();
        //for (volatile long i = 0; i < 300000; i++);
        for (volatile long i = 0; i < 10000; i++);

    }
    return (1);
}