#include "device_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

unsigned char ledState = 0b00000001;
int virtualChannel = 0;
int bidirectional = 1;
int warningValue = 0;
volatile int nSamples = 0;
volatile int period = 0;
volatile int timerIndex = 0;
volatile int sensors[num_sensors] = {0, 0, 0, 0, 0, 0, 0};
volatile int sensorData[num_sensors][max_samples];
volatile int sampleIndex = 0;
int adc_channels[] = {4, 5, 1};
//  4 - Temperature Sensor
//  5 - Potentiometer
//  1 - LDR

void setupChannels() {
    ANSBbits.ANSB0 = 1; TRISBbits.TRISB0 = 1;
    ANSBbits.ANSB1 = 1; TRISBbits.TRISB1 = 1;
    ANSBbits.ANSB2 = 1; TRISBbits.TRISB2 = 1;

    ANSDbits.ANSD6 = 0; TRISDbits.TRISD6 = 1;
    ANSDbits.ANSD7 = 0; TRISDbits.TRISD7 = 1;

    TRISAbits.TRISA4 = 0;
    TRISAbits.TRISA5 = 0;
    TRISAbits.TRISA6 = 0;

    ANSAbits.ANSA7 = 0; TRISAbits.TRISA7 = 1;
    
    LATAbits.LATA7 = 0;
    LATAbits.LATA6 = 0;
    LATAbits.LATA5 = 0;
    LATAbits.LATA4 = 0;
    
    
    
}
/*
void testChannels() {
    char buffer[50];
    int analog0, analog1, analog2;

    AD1CHS = 0;
    AD1CON1bits.SAMP = 1;
    for (int i = 0; i < 1000; i++);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    analog0 = ADC1BUF0;
    AD1CHS = 1;
    AD1CON1bits.SAMP = 1;
    for (int i = 0; i < 1000; i++);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    analog1 = ADC1BUF0;
    AD1CHS = 2;
    AD1CON1bits.SAMP = 1;
    for (int i = 0; i < 1000; i++);
    AD1CON1bits.SAMP = 0;
    while (!AD1CON1bits.DONE);
    analog2 = ADC1BUF0;

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
*/

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

void setupTimer1() {
    TMR1 = 0;
    T1CON = 0x801110;
    PR1 = (8000000 / 2) / (256);
    IPC0bits.T1IP = 0x01;

    IFS0bits.T1IF = 0; //Clear the Timer1 interrupt status flag
    IEC0bits.T1IE = 1; //Enable Timer1 interrupts
    T1CONbits.TON = 0x01;
}

void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void) {
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
    char buff[100];

    if (timerIndex < period)
        timerIndex++;
    else {

        timerIndex = 0;
        if (sampleIndex < nSamples) {
            for (int i = 0; i < num_sensors; i++) {
                if (i < 3) {
                    if (sensors[i] == 1) {
                        AD1CHS = adc_channels[i];

                        AD1CON1bits.SAMP = 1;
                        for (volatile int i = 0; i < 1000; i++); // Short delay
                        AD1CON1bits.SAMP = 0;

                        while (!AD1CON1bits.DONE);
                        sensorData[i][sampleIndex] = ADC1BUF0;
                    }
                } else {
                    switch (i) {
                        case 3: sensorData[i][sampleIndex] = !PORTDbits.RD6;
                            break;
                        case 4: sensorData[i][sampleIndex] = !PORTDbits.RD7;
                            break;
                        case 5:
                            if(bidirectional)
                                sensorData[i][sampleIndex] = !PORTAbits.RA7;
                            else 
                                sensorData[i][sampleIndex] = -1;
                            break;
                        case 6: if (virtualChannel)
                                sensorData[i][sampleIndex] = ((!PORTDbits.RD6 << 1) | !PORTDbits.RD7);
                            else
                                sensorData[i][sampleIndex] = 0;
                            break;
                    }
                }
            }
            sampleIndex++;
        } else {
            // Preparar string de envio do JSON tipo {?Ax?:[512,514,516,516,510], ?DB?: [0,0,1,1,1], ?DV?: [0,1,2,3,3]} 
            sendResultStringJSON();
            // Limpar vetores de dados 
            sampleIndex = 0;
            T1CONbits.TON = 0;
        }
    }
}

void sendResultStringJSON() {
    char ResultString[250];
    const char *Sensor_names[] = {"'Ax': ", "'Ay': ", "'Az': ", "'D6': ", "'D7': ", "'DB': ", "'DV': "};
    char temp[20];

    strcpy(ResultString, "{");
    writeString("Empezo el envio de mensajes por JSON\n");
    int first = 1;
    for (int i = 0; i < num_sensors - 1; i++) {
        if (sensors[i]) {
            if (!first) {
                strcat(ResultString, ", ");
            }
            first = 0;

            strcat(ResultString, Sensor_names[i]);
            strcat(ResultString, "[");

            for (int j = 0; j < nSamples; j++) {
                sprintf(temp, "%d", sensorData[i][j]);
                strcat(ResultString, temp);
                if (j < nSamples - 1) {
                    strcat(ResultString, ",");
                }
            }
            strcat(ResultString, "]");
        }
    }
    if (virtualChannel) {
        strcat(ResultString, ", 'DV': [");
        for (int j = 0; j < nSamples; j++) {
            //printf("Sensor 4 data: %d, Sensor 5 data: %d, Sensor Virtual data: %d\n",sensorData[4][j],sensorData[5][j],(sensorData[4][j] << 1 | sensorData[5][j]));
            sprintf(temp, "%d", (sensorData[3][j] << 1 | sensorData[4][j]));
            strcat(ResultString, temp);
            if (j < nSamples - 1) {
                strcat(ResultString, ",");
            }
        }
        strcat(ResultString, "]");
    }

    strcat(ResultString, "}");
    strcat(ResultString, "\r\n");
    writeString(ResultString);
}

void writeChar(char a) {
    while (U1STAbits.UTXBF);
    U1TXREG = a;
}

void turnOnLED(const char * str) {
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
        while (isspace((unsigned char) *ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
            // Check for end of string
            if (*ptr == '\0') {
                break;
            }
            ptr++;
        }

        // Extract the key (should be 2 characters: type and identifier)
        char type = *ptr++;
        char identifier = *ptr++;

        while (isspace((unsigned char) *ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
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

        printf("\tD%c = %d\n", identifier, value);
        switch (identifier) {
            case('x'): LATAbits.LATA6 = value;
                break;
            case('y'): LATAbits.LATA5 = value;
                break;
            case('z'): LATAbits.LATA4 = value;
                break;
        }
        break;
        printf("LED %c alterado para %d.\n", identifier, value);
    }  
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

float readCalibratedTemp(int adcValue) {
    return 0.1 * adcValue - 64.3;
}

void changeBidirectionalChannel() {
    if(!bidirectional){
        TRISAbits.TRISA7 = 0;
        LATAbits.LATA7 = 1;
    }else{
        LATAbits.LATA7 = 0;
        TRISAbits.TRISA7 = 1;
    }

    char message[50];
    sprintf(message, "Canal bidirectional esta %d\r\n", bidirectional);
    writeString(message);
    sprintf(message, "TRISA7=%d, RA7=%d, LATA7=%d\r\n", TRISAbits.TRISA7, PORTAbits.RA7, LATAbits.LATA7);
    writeString(message);

}

void receiveInput(char * str) {
    //char receivedChar = readChar();

    // For demo, echo back the character
    //sprintf(str, "Recebido: %c\r\n", receivedChar);
    //writeString(str);

    // Optional: If you want to receive multiple characters (e.g., a command)

    int i = 0;

    char c;
    while (1) {
        c = readChar();
        if (c == '\n' || i >= 199) {
            str[i] = '\0';
            break;
        }
        str[i++] = c;
    }

    //while (U1STAbits.URXDA) {
    //    volatile char dump = U1RXREG;
    //}

    //writeString(str); // DEPOIS TIRAR LINHA!!!!!!!!
    //writeString("\n");
    processMessages(str);
}

void processMessages(char * str) {
    char auxStr[200];
    int i = 0, j = 0;

    while (str[i] != '\0') {
        while (str[i] != '}') {
            auxStr[i++] = str[j++];
        }
        auxStr[j] != '}';
        auxStr[j + 1] != '\0';
        processInput(auxStr);
        i++; // TESTE
        j = 0;
    }
}

void processInput(char * str) {
    char buffer[70];
    int flag;
    if (str[0] == '{' && str[1] == '"') { // Só processar input que comece com {"
        switch (str[2]) { // O terceiro caracter do input recebido é o que interessa para decidir o que se quer fazer.
            case 'b':
                // ALTERAR CANAL DIGITAL BIDIRECIONAL (RA7: 0 output, 1 input)
                writeString("Chegou no change direction\n");
                bidirectional = (int)(str[5] - '0');
                changeBidirectionalChannel();
                break;

            case 'v':
                // LIGAR/DESLIGAR CANAL VIRTUAL (RD6 e RD7)
                // Isto vale mesmo a pena????
                // sim
                // eles pedem
                virtualChannel = (int) (str[5] - '0');
                sprintf(buffer, "Canal virtual de comunicacion cambiado para %d\n", virtualChannel);
                writeString(buffer);
                break;

            case 'A':
                printf("Current sensor map:\n");
                // DEFINIR ENTRADAS ANALÓGICAS E DIGITAIS (pode ser mais do que uma a cada vez!)
                defineSampleInputs(str);
                break;

            case 'D':
                if (str[3] >= 'a' && str[3] <= 'z') { // Se o quarto caracter for uma minúsucula, queremos alterar uma determinada saída digital.
                    turnOnLED(str);
                } else {
                    // DEFINIR ENTRADAS ANALÓGICAS E DIGITAIS (pode ser mais do que uma a cada vez!)
                    defineSampleInputs(str);
                }
                break;

            case 'p':
                // CONFIGURAR PERÍODO DE AMOSTRAGEM
                period = (int) (str[5] - '0');
                sprintf(buffer, "Periodo cambiado para %d\n", period);
                writeString(buffer);
                break;

            case 'n':
                // CONFIGURAR NÚMERO DE AMOSTRAS POR MENSAGEM
                nSamples = (int) (str[5] - '0');
                sprintf(buffer, "Numero de amuestras cambiado para %d\n", nSamples);
                writeString(buffer);
                break;
            case 'w':
                warningValue = (int) (str[5] - '0');
                sprintf(buffer, "Codigo de aviso cambiado para %d\n", warningValue);
                writeString(buffer);
                break;
            case 's':
                sampleInputs();
                break;
            default:
                sprintf(buffer, "Opção incorreta, tente outra vez."); // 
                writeString(str);
                break;
        }
    } else {
        sprintf(buffer, "Opção incorreta, tente outra vez."); // 
        writeString(str);
    }
    //memset(str, 0, sizeof(str)); 
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
        while (isspace((unsigned char) *ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
            // Check for end of string
            if (*ptr == '\0') {
                break;
            }
            ptr++;
        }

        // Extract the key (should be 2 characters: type and identifier)
        char type = *ptr++;
        char identifier = *ptr++;

        while (isspace((unsigned char) *ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
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
        //if (value == 1) {

        switch (type) {
            case 'A':
                printf("\tA%c = %d\n", identifier, value);
                switch (identifier) {
                    case('x'): sensors[0] = value;
                        break;
                    case('y'): sensors[1] = value;
                        break;
                    case('z'): sensors[2] = value;
                        break;
                }
                break;
            case 'D':
                printf("\tD%c = %d\n", identifier, value);
                switch (identifier) {
                    case('6'): sensors[3] = value;
                        break;
                    case('7'): sensors[4] = value;
                        break;
                    case('B'): sensors[5] = value;
                        break;
                    case('V'): sensors[6] = value;
                        break;
                }
                break;
            default:
                printf("Unknown input type: %c\n", type);
                break;
        }
        //}
    }
}

void checkWarningValue() {
    if (virtualChannel)
        if (warningValue == ((!PORTDbits.RD6 << 1) | !PORTDbits.RD7))
            writeString("Warning value is the same as bidirectional channel\n");
}

void sampleInputs() {
    setupTimer1();
}
