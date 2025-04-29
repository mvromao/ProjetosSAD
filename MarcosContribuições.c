/* 
 * File:   main.c
 * Author: julito
 *
 * Created on April 15, 2025, 11:31 AM
 */

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>  // Required for MPLAB and PIC register names

#include <string.h>
#include <ctype.h>


#include <p24fj1024gb610.h>

#define max_samples 20
#define num_sensors 7
/*
 * 
 */
//funcoes trab final********************************************
unsigned char ledState = 0b00000001;
int virtualChannel = 0;         // Flag virtual channel
volatile int nSamples;
volatile int period;
volatile int timerIndex;
volatile int sensors[num_sensors] = [0,0,0,0,0,0,0];
volatile int sensorData[num_sensors][max_samples];
volatile int sampleIndex = 0;
int adc_channels = [4,5,1];
// 4 - Temperature Sensor
// 5 - Potentiometer
// 1 - LDR 


void setupChannels(){
    // Entradas analogicas ANSB(entrada) - TRISB (modo: input))
    ANSBbits.ANSB0 = 1; TRISBbits.TRISB0 = 1; //entrada analog AN0 - ANSB0, input
    ANSBbits.ANSB1 = 1; TRISBbits.TRISB1 = 1; //entrada analog AN1 - ANSB1, input
    ANSBbits.ANSB2 = 1; TRISBbits.TRISB2 = 1; //entrada analog AN2 - ANSB2, input

    // Entradas digitais TRISD (RD6, RD7)
    ANSDbits.ANSD6 = 0;  // Disable analog function on RD6
    ANSDbits.ANSD7 = 0;  // Disable analog function on RD7
    TRISDbits.TRISD6 = 1;
    TRISDbits.TRISD7 = 1;

    // Saidas digitais - TRISC (RC1, RC2, RC3)
    TRISCbits.TRISC1 = 0;   // Makes RC1 a digital output
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;

    // Canal digital configuravel TRISA (RA7) ? come a como entrada
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
    // Select Channel on Multiplexer,
    //      enable sampling on the ADC, 
    //      wait for 1000 clock cycles, 
    //      disable sampling and 
    //      get value from ADC (store on variable analog#).
    
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
    //ANSA = 0x0020;
    
    AD1CON1 = 0x0000;    // Manuel converter after sampling, manual sampling
    AD1CON2 = 0;
    AD1CON3 = 0x1F02;    // Sample time = 31 TAD, ADC clock = Fosc/2
    
    AD1CON1bits.ADON = 1;
    //AD1CON1bits.SAMP = 0;
}


void setupTimer1(int period){
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
    
    AD1CON1bits.SAMP = 1;   // Come ar o sampling
        
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

int readSensorADC(int c) {
    AD1CHS = c;  // Internal temperature sensor input
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

void monitorDigitalInputs(int flag) {
    char buffer[50];
    
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

void receiveInput(char * str, int freqSample, int nSample, int warningValue){
    //char receivedChar = readChar();

    // For demo, echo back the character
    //sprintf(str, "Recebido: %c\r\n", receivedChar);
    //writeString(str);

    // Optional: If you want to receive multiple characters (e.g., a command)
    
    int i = 0;
    while (1) {
        char c = readChar();
        if (c == '\n' || i >= 199) {
            str[i] = '\0';
            break;
        }
        str[i++] = c;
    }
    writeString(str); // DEPOIS TIRAR LINHA!!!!!!!!
    processMessages(str, freqSample, nSample, warningValue);
}

void processMessages(char * str, int freqSample, int nSample, int warningValue){
    auxStr[200];
    int i = 0, j = 0;

    while (str[i] != '\0') {
        while (str[i] != '}') {
            auxStr[i++] = str[j++];
        }
        auxStr[j] != '}';
        auxStr[j+1] != '\0';
        processInput(auxStr, freqSample, nSample, warningValue);
        j = 0;
    }
}

void processInput(char * str, int freqSample, int nSample, int warningValue){
	char buffer[50];
	
	if (str[0] == '{' && str[1] == '"') { // SÃ³ processar input que comece com {"
		switch (str[2]) { // O terceiro caracter do input recebido Ã© o que interessa para decidir o que se quer fazer.
			case 'b':
                // ALTERAR CANAL DIGITAL BIDIRECIONAL (RA7: 0 output, 1 input)
                changeBidirectionalChannel(str[5]);
				break;

			case 'v':
				// LIGAR/DESLIGAR CANAL VIRTUAL (RD6 e RD7)
                // Isto vale mesmo a pena????
                virtualChannel = ~
				break;

			case 'A':
				// DEFINIR ENTRADAS ANALÃGICAS E DIGITAIS (pode ser mais do que uma a cada vez!)
                defineSampleInputs(str);
				break;
				
			case 'D':
				if (str[3] >= 'a' && str[3] <= 'z') { // Se o quarto caracter for uma minÃºsucula, queremos alterar uma determinada saÃ­da digital.
					
				}
				else {
					// DEFINIR ENTRADAS ANALÃGICAS E DIGITAIS (pode ser mais do que uma a cada vez!)
                    defineSampleInputs(str);
				}
				break;	
			
			case 'p':
				// CONFIGURAR PERÃODO DE AMOSTRAGEM
                changeValue(freqSample, str[5]);
				break;
				
			case 'n':
				// CONFIGURAR NÃMERO DE AMOSTRAS POR MENSAGEM
                changeValue(nSample, str[5]);
				break;	
			
			case 'w':
                changeValue(warningValue, str[5]);
				break;	
						
			default:
				sprintf(buffer, "OpÃ§Ã£o incorreta, tente outra vez."); // 
				writeString(str);
				break;
		}
	}
	else {
		sprintf(buffer, "OpÃ§Ã£o incorreta, tente outra vez."); // 
		writeString(str);
	}
}

void changeValue (int value, char newValue) {
    value = (int)(newValue - 48);
}

void defineSampleInputs(const char *str) {
    const char *ptr = str;
    
    // Skip the opening '{'
    if (*ptr == '{') {
        ptr++;
    } else {
        printf("Invalid format: missing '{'\n");
        return;
    }

    while (*ptr != '}' && *ptr != '\0') {
        // Skip whitespace
        while (isspace((unsigned char)*ptr)) {
            ptr++;
        }

        // Check for end of string
        if (*ptr == '\0') {
            break;
        }

        // Expect a '"' at the start of the key
        if (*ptr != '"') {
            printf("Invalid format: expected '\"'\n");
            return;
        }
        ptr++;

        // Extract the key (should be 2 characters: type and identifier)
        char type = *ptr++;
        char identifier = *ptr++;

        // Expect closing '"'
        if (*ptr != '"') {
            printf("Invalid format: expected closing '\"'\n");
            return;
        }
        ptr++;

        // Expect ':'
        if (*ptr != ':') {
            printf("Invalid format: expected ':'\n");
            return;
        }
        ptr++;

        // Skip whitespace before value
        while (isspace((unsigned char)*ptr)) {
            ptr++;
        }

        // Get the value (0 or 1)
        int value = 0;
        if (*ptr == '0') {
            value = 0;
        } else if (*ptr == '1') {
            value = 1;
        } else {
            printf("Invalid value: expected 0 or 1\n");
            return;
        }
        ptr++;

        // Process the input based on type and identifier
        if (value == 1) {
            switch (type) {
                case 'A':
                    printf("Sampling analog input %c\n", identifier);
                    // Add your analog sampling 
                    break;
                case 'D':
                    printf("Sampling digital input %c\n", identifier);
                    // Add your digital sampling code here
                    break;
                default:
                    printf("Unknown input type: %c\n", type);
                    break;
            }
        }

        // Skip whitespace after value
        while (isspace((unsigned char)*ptr)) {
            ptr++;
        }

        // Check for comma or closing brace
        if (*ptr == ',') {
            ptr++;
        } else if (*ptr != '}') {
            printf("Invalid format: expected ',' or '}'\n");
            return;
        }
    }
}

void sampleInputs(){
    setupTimer1();    
}

void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
    if (timerIndex < period)
        timerIndex++;
    else {
        timerIndex = 0;
        if (sampleIndex < nSamples) {
            for (int i; i < num_sensors; i++) {
                if (i < 3) {
                    if (sensors[i]) {
                        AD1CHS = adcAddress[i];

                        AD1CON1bits.SAMP = 1;
                        for (volatile int i = 0; i < 1000; i++); // Short delay
                        AD1CON1bits.SAMP = 0;

                        while (!AD1CON1bits.DONE);
                        sensorData[i][sampleIndex] = ADC1BUF0;
                    }
                } else {
                    if(sensors[i]) {
                        // Check Digital Inputs
                        // talvez não vai poder ser iterativo, porque o acesso às portas digitais
                        // é feito por PORTDbits.RD6 e PORTDbits.RD7 e PORTDbits.RA7
                    }
                }
            }
            sampleIndex++;
        } else {
            sampleIndex = 0;
            T1CONbits.TON = 0;
        }
    }
}

void changeBidirectionalChannel (char c_value) {
    if (c_value == '0') {
        TRISAbits.TRISA7 = 0;
    }
    else if (c_value == '1') {
        TRISAbits.TRISA7 = 1;
    }
}


int main(int argc, char** argv) {
	int value;
    char str[200];

    int freqSample = 1;
    int nSample = 5;
    int warningValue = 5;
	
    ANSA = 1;
    TRISA = 0;
    LATA = 0b00000000;
	
    setupTimer1();
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
		INÃCIO
		*****************************/
		
		receiveInput(&str, freqSample, nSample, warningValue);
		
        for (volatile long i = 0; i < 300000; i++);
    }
    return (1);
}
