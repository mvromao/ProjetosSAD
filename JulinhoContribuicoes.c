/* 
 * File:   main.c
 * Author: julito
 *
 * Created on April 15, 2025, 11:31 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>  // Required for MPLAB and PIC register names


#include <p24fj1024gb610.h>


/*
 * 
 */
//funcoes trab final********************************************
unsigned char ledState = 0b00000001;


void setupChannels(){
    // Entradas analogicas ANSB(entrada) - TRISB (modo: input))
    ANSBbits.ANSB0 = 1; TRISBbits.TRISB0 = 1; //entrada analog AN0 - ANSB0 , input
    ANSBbits.ANSB1 = 1; TRISBbits.TRISB1 = 1; //entrada analog AN1 - ANSB1, input
    ANSBbits.ANSB2 = 1; TRISBbits.TRISB2 = 1; //entrada analog AN2 - ANSB2, input

    // Entradas digitais TRISD (RD6, RD7)
    ANSDbits.ANSD6 = 0;  // Disable analog function on RD6
    ANSDbits.ANSD7 = 0;  // Disable analog function on RD7
    TRISDbits.TRISD6 = 1;
    TRISDbits.TRISD7 = 1;

    // Saídas digitais - TRISC (RC1, RC2, RC3)
    TRISCbits.TRISC1 = 0;   // Makes RC1 a digital output
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;

    // Canal digital configurável TRISA (RA7) ? começa como entrada
    TRISAbits.TRISA7 = 1; 
}

/*OUTPUT ARDUINO IDE 
12:30:35.620 -> AN0=969, AN1=962, AN2=947
12:30:35.657 -> RD6=0, RD7=0
12:30:35.657 -> Toggled RC1, RC2, RC3
12:30:35.657 -> RA7 (as input) = 0
12:30:35.693 -> RA7 set as output and set HIGH
12:30:37.422 -> RA7 set LOW
 */
void testChannels() {
    char buffer[50];

    // --- Test Analog Channels ---
    int analog0, analog1, analog2;

    AD1CHS = 0; AD1CON1bits.SAMP = 1; for(int i=0;i<1000;i++); AD1CON1bits.SAMP = 0; while(!AD1CON1bits.DONE); analog0 = ADC1BUF0;
    AD1CHS = 1; AD1CON1bits.SAMP = 1; for(int i=0;i<1000;i++); AD1CON1bits.SAMP = 0; while(!AD1CON1bits.DONE); analog1 = ADC1BUF0;
    AD1CHS = 2; AD1CON1bits.SAMP = 1; for(int i=0;i<1000;i++); AD1CON1bits.SAMP = 0; while(!AD1CON1bits.DONE); analog2 = ADC1BUF0;

    sprintf(buffer, "AN0=%d, AN1=%d, AN2=%d\r\n", analog0, analog1, analog2);
    writeString(buffer);

    // --- Test Digital Inputs (RD6, RD7) ---
    int rd6 = PORTDbits.RD6;
    int rd7 = PORTDbits.RD7;

    sprintf(buffer, "RD6=%d, RD7=%d\r\n", rd6, rd7);
    writeString(buffer);

    // --- Toggle Digital Outputs (RC1, RC2, RC3) ---
    LATCbits.LATC1 = !LATCbits.LATC1;
    LATCbits.LATC2 = !LATCbits.LATC2;
    LATCbits.LATC3 = !LATCbits.LATC3;

    writeString("Toggled RC1, RC2, RC3\r\n");

    // --- RA7 as input test ---
    int ra7 = PORTAbits.RA7;
    sprintf(buffer, "RA7 (as input) = %d\r\n", ra7);
    writeString(buffer);

    // Optional: RA7 as output test
    TRISAbits.TRISA7 = 0;         // Set as output
    LATAbits.LATA7 = 1;           // Set RA7 high
    writeString("RA7 set as output and set HIGH\r\n");

    // ? Replaced __delay_ms(100) with loop
    for (volatile long j = 0; j < 500000; j++);  // Delay loop

    LATAbits.LATA7 = 0;
    writeString("RA7 set LOW\r\n");

    TRISAbits.TRISA7 = 1;         // Restore as input
}

void setupUART1(){
    __builtin_write_OSCCONL(OSCCON & 0xbf);
        
    RPOR8bits.RP17R = 0x0003;
    RPINR18bits.U1RXR = 0x000A;
        
    U1STA = 0;
    U1MODE = 0x8000;
    U1BRG = 12;
    U1STAbits.UTXEN = 1;
    U1STAbits.URXEN = 1;

    U1MODEbits.UARTEN = 1;
    
    IEC0bits.U1TXIE = 0;
    IEC0bits.U1RXIE = 0;
    
}  

//***************************************************************
/*void setupUART1() {
    __builtin_write_OSCCONL(OSCCON & 0xbf);
    
    IPC3bits.U1TXIP2= 1;
    IPC3bits.U1TXIP1= 0;
    IPC3bits.U1TXIP0= 0;
    IPC2bits.U1RXIP2= 1;

    IPC2bits.U1RXIP1= 0;
    IPC2bits.U1RXIP0= 0;
    
    U1STA = 0;
    U1MODE = 0x8000;
    U1BRG = 25; // (4000000/ (16 * 9600) - 1)
    
    U1STAbits.UTXEN = 1;
    IEC0bits.U1RXIE = 1;
    IEC0bits.U1TXIE = 1;
}*/
void setupTimer2(){
    #pragma config FNOSC = PRI
    #pragma config POSCMOD = HS
    #pragma config JTAGEN = OFF
    #pragma config FWDTEN = OFF
}

void setupADC() {
    ANSA = 0x0020;
    AD1CON1 = 0x0000;    // Manuel converter after sampling, manual sampling
    AD1CON2 = 0;
    AD1CON3 = 0x1F02;    // Sample time = 31 TAD, ADC clock = Fosc/2
    
    AD1CON1bits.ADON = 1;
    //AD1CON1bits.SAMP = 0;

}
void __attribute__((__interrupt__, __shadow__)) _T2Interrupt(void) {
    /* Interrupt Service Routine code goes here */
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
    
}

void writeChar(char a){
    while(U1STAbits.UTXBF);
        U1TXREG = a;
    return;
}

void writeString(char * str){
    while (*str){
        writeChar(*str);
        *str++;
    }
}
int getPotentiometerValue() {
    AD1CHS = 5;          // AN5 as input
    
    AD1CON1bits.SAMP = 1;   // Começar o sampling
        
    for (int i = 0; i < 100; i++);  
    AD1CON1bits.SAMP = 0;
        
    while(!AD1CON1bits.DONE);
       
    return ADC1BUF0;
}

void transmitPotentiometerData(int value, char * str) {
       
    value =  getPotentiometerValue();
    
    sprintf(str, "Valor do potentiometro = %d\r\n", value);
        
    //sprintf(str, "Hello World!\n");
    writeString(str);
}

char readChar() {
    while (!U1STAbits.URXDA);  // Wait until data is available
    return U1RXREG;            // Return received character
}

void RecieveData(int value, char * str){
    //char receivedChar = readChar();

    // For demo, echo back the character
    //sprintf(str, "Recebido: %c\r\n", receivedChar);
    //writeString(str);

    // Optional: If you want to receive multiple characters (e.g., a command)
    
    int i = 0;
    while (1) {
        char c = readChar();
        if (c == '\n' || i >= 49) {
            str[i] = '\n';
            str[i+1] = '\0';
            break;
        }
        str[i++] = c;
    }
    writeString(str);
}

void setupTimer1(){
    TMR1 = 0;
    T1CON = 0x801110;
    PR1 = (8000000/2) / (8);
    IPC0bits.T1IP = 0x01;
    
    IFS0bits.T1IF = 0; //Clear the Timer1 interrupt status flag
    IEC0bits.T1IE = 1; //Enable Timer1 interrupts
    //T1CONbits.TON = 0x01;
}
void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void) {
    /* Interrupt Service Routine code goes here */
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
    LATA = ledState;
    
    ledState <<= 1;
    if(ledState == 0)
        ledState = 0b00000001;
}

int readTemperatureSensorADC() {
    AD1CHS = 0x1F;  // Internal temperature sensor input
    AD1CON1bits.SAMP = 1;
    for (volatile int i = 0; i < 1000; i++);  // Short delay
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}
float readCalibratedTemp(int adcValue) {
    return 0.1 * adcValue - 64.3;
}

void readTemp(){
    int adc = readTemperatureSensorADC();
    float temp = readCalibratedTemp(adc);
    char buffer[50];
    sprintf(buffer, "Temp = %.2f C\r\n", temp);
    writeString(buffer);

}

void monitorDigitalInputs() {
    char buffer[50];
    int flag = 1;  // Initialize flag (you might want to make this static or global if needed elsewhere)
    
    // Invert logic: 1 when pressed, 0 when released
    int s3 = !PORTDbits.RD6;  // S3 ? RD6
    int s6 = !PORTDbits.RD7;  // S6 ? RD7
   
    
    if(!flag) {
        sprintf(buffer, "S3 = %d, S6 = %d\r\n", s3, s6);
    } else {
        // Combine button states into a single integer (0-3)
        int buttonState = (s3 << 1) | s6;
        /* Explanation:
         * - s3 << 1 shifts s3's value to the left (becomes bit 1)
         * - | s6 ORs with s6 (bit 0)
         * Resulting combinations:
         * (0,0) ? 0b00 ? 0
         * (0,1) ? 0b01 ? 1
         * (1,0) ? 0b10 ? 2
         * (1,1) ? 0b11 ? 3
         */
        sprintf(buffer, "State = %d\r\n", buttonState);
    }
    
    writeString(buffer);
}



int main(int argc, char** argv) {
    
    
    ANSA = 1;
    TRISA = 0;
    LATA = 0b00000000;
    setupTimer1();
    setupUART1();
    setupChannels();
    setupADC();
    
    TRISAbits.TRISA5 = 1;  // Set RA5 as input
    
    int value;
    char str[50];
   
    //setupTimer2();
    
    while(1) {
        //transmitPotentiometerData(value, &str);
        //RecieveData(value,&str);
        //testChannels();
        //for (volatile long i = 0; i < 500000; i++);  // Adjust value as needed
        //readTemp();
        monitorDigitalInputs();
        for (volatile long i = 0; i < 300000; i++);
    }
    return (1);
}

