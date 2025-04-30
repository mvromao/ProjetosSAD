#include "device_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

unsigned char ledState = 0b00000001;
int virtualChannel = 0;
volatile int nSamples;
volatile int period;
volatile int timerIndex;
volatile int sensors[num_sensors] = {0};
volatile int sensorData[num_sensors][max_samples];
volatile int sampleIndex = 0;
int adc_channels[] = {4, 5, 1};

void setupChannels() {
    ANSBbits.ANSB0 = 1; TRISBbits.TRISB0 = 1;
    ANSBbits.ANSB1 = 1; TRISBbits.TRISB1 = 1;
    ANSBbits.ANSB2 = 1; TRISBbits.TRISB2 = 1;

    ANSDbits.ANSD6 = 0;
    ANSDbits.ANSD7 = 0;
    TRISDbits.TRISD6 = 1;
    TRISDbits.TRISD7 = 1;

    TRISCbits.TRISC1 = 0;
    TRISCbits.TRISC2 = 0;
    TRISCbits.TRISC3 = 0;

    TRISAbits.TRISA7 = 1;
}

void testChannels() {
    char buffer[50];
    int analog0, analog1, analog2;

    AD1CHS = 0; AD1CON1bits.SAMP = 1; for(int i = 0; i < 1000; i++); AD1CON1bits.SAMP = 0; while(!AD1CON1bits.DONE); analog0 = ADC1BUF0;
    AD1CHS = 1; AD1CON1bits.SAMP = 1; for(int i = 0; i < 1000; i++); AD1CON1bits.SAMP = 0; while(!AD1CON1bits.DONE); analog1 = ADC1BUF0;
    AD1CHS = 2; AD1CON1bits.SAMP = 1; for(int i = 0; i < 1000; i++); AD1CON1bits.SAMP = 0; while(!AD1CON1bits.DONE); analog2 = ADC1BUF0;

    sprintf(buffer, "AN0=%d, AN1=%d, AN2=%d\r\n", analog0, analog1, analog2);
    writeString(buffer);

    int rd6 = PORTDbits.RD6;
    int rd7 = PORTDbits.RD7;
    sprintf(buffer, "RD6=%d, RD7=%d\r\n", rd6, rd7);
    writeString(buffer);

    LATCbits.LATC1 = !LATCbits.LATC1;
    LATCbits.LATC2 = !LATCbits.LATC2;
    LATCbits.LATC3 = !LATCbits.LATC3;
    writeString("Toggled RC1, RC2, RC3\r\n");

    int ra7 = PORTAbits.RA7;
    sprintf(buffer, "RA7 (as input) = %d\r\n", ra7);
    writeString(buffer);

    TRISAbits.TRISA7 = 0;
    LATAbits.LATA7 = 1;
    writeString("RA7 set as output and set HIGH\r\n");
    for (volatile long j = 0; j < 500000; j++);
    LATAbits.LATA7 = 0;
    writeString("RA7 set LOW\r\n");
    TRISAbits.TRISA7 = 1;
}

void setupUART1() {
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

void setupADC() {
    AD1CON1 = 0x0000;
    AD1CON2 = 0;
    AD1CON3 = 0x1F02;
    AD1CON1bits.ADON = 1;
}

void setupTimer1(int period) {
    TMR1 = 0;
    T1CON = 0x801110;
    PR1 = period;
    IPC0bits.T1IP = 0x01;
    IFS0bits.T1IF = 0;
    IEC0bits.T1IE = 1;
}

void setupTimer2() {
    // Empty if unused
}

void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0;
    LATA = ledState;
    ledState <<= 1;
    if (ledState == 0) ledState = 0b00000001;
}

void writeChar(char a) {
    while(U1STAbits.UTXBF);
    U1TXREG = a;
}

void writeString(char *str) {
    while (*str) {
        writeChar(*str);
        str++;
    }
}

char readChar() {
    while (!U1STAbits.URXDA);
    return U1RXREG;
}

int getPotentiometerValue() {
    AD1CHS = 5;
    AD1CON1bits.SAMP = 1;
    for (int i = 0; i < 100; i++);
    AD1CON1bits.SAMP = 0;
    while(!AD1CON1bits.DONE);
    return ADC1BUF0;
}

void transmitPotentiometerData(int value, char *str) {
    value = getPotentiometerValue();
    sprintf(str, "Valor do potentiometro = %d\r\n", value);
    writeString(str);
}

int readTemperatureSensorADC() {
    AD1CHS = 0x1F;
    AD1CON1bits.SAMP = 1;
    for (volatile int i = 0; i < 1000; i++);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}

int readSensorADC(int c) {
    AD1CHS = c;
    AD1CON1bits.SAMP = 1;
    for (volatile int i = 0; i < 1000; i++);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    return ADC1BUF0;
}

float readCalibratedTemp(int adcValue) {
    return 0.1 * adcValue - 64.3;
}

void readTemp() {
    int adc = readTemperatureSensorADC();
    float temp = readCalibratedTemp(adc);
    char buffer[50];
    sprintf(buffer, "Temp = %.2f C\r\n", temp);
    writeString(buffer);
}

void monitorDigitalInputs(int flag) {
    char buffer[50];
    int s3 = !PORTDbits.RD6;
    int s6 = !PORTDbits.RD7;

    if (!flag) {
        sprintf(buffer, "S3 = %d, S6 = %d\r\n", s3, s6);
    } else {
        int buttonState = (s3 << 1) | s6;
        sprintf(buffer, "State = %d\r\n", buttonState);
    }
    writeString(buffer);
}

void changeBidirectionalChannel(char c_value) {
    TRISAbits.TRISA7 = (c_value == '1') ? 1 : 0;
}

void changeValue(int value, char newValue) {
    value = (int)(newValue - '0');
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
    char auxStr[200];
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
	
	if (str[0] == '{' && str[1] == '"') { // Só processar input que comece com {"
		switch (str[2]) { // O terceiro caracter do input recebido é o que interessa para decidir o que se quer fazer.
			case 'b':
                // ALTERAR CANAL DIGITAL BIDIRECIONAL (RA7: 0 output, 1 input)
                changeBidirectionalChannel(str[5]);
				break;

			case 'v':
				// LIGAR/DESLIGAR CANAL VIRTUAL (RD6 e RD7)
                // Isto vale mesmo a pena????
				break;

			case 'A':
				// DEFINIR ENTRADAS ANALÓGICAS E DIGITAIS (pode ser mais do que uma a cada vez!)
                defineSampleInputs(str);
				break;
				
			case 'D':
				if (str[3] >= 'a' && str[3] <= 'z') { // Se o quarto caracter for uma minúsucula, queremos alterar uma determinada saída digital.
					
				}
				else {
					// DEFINIR ENTRADAS ANALÓGICAS E DIGITAIS (pode ser mais do que uma a cada vez!)
                    defineSampleInputs(str);
				}
				break;	
			
			case 'p':
				// CONFIGURAR PERÍODO DE AMOSTRAGEM
                changeValue(freqSample, str[5]);
				break;
				
			case 'n':
				// CONFIGURAR NÚMERO DE AMOSTRAS POR MENSAGEM
                changeValue(nSample, str[5]);
				break;	
			
			case 'w':
                changeValue(warningValue, str[5]);
				break;	
						
			default:
				sprintf(buffer, "Opção incorreta, tente outra vez."); // 
				writeString(str);
				break;
		}
	}
	else {
		sprintf(buffer, "Opção incorreta, tente outra vez."); // 
		writeString(str);
	}
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
        while (isspace((unsigned char)*ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
            // Check for end of string
            if (*ptr == '\0') {
                break;
            }
            ptr++;
        }

        // Extract the key (should be 2 characters: type and identifier)
        char type = *ptr++;
        char identifier = *ptr++;

        while (isspace((unsigned char)*ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
            // Check for end of string
            if (*ptr == '\0') {
                break;
            }
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
                    // Add your analog sampling code here
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
    }
}

void sampleInputs() {
    setupTimer1(100);
}
