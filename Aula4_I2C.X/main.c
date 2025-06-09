#include "device_functions.h"

int main(int argc, char** argv) {
    char buf[128];
    
    setupUART1();
    I2C2_Init();
    setupTimer1();
    
    writeString("PIC Started...\r\n");

    while (1) {
        receiveInput(&buf);
        
        for(volatile long i = 0; i <10000; i++);
    }

    return 0;
}