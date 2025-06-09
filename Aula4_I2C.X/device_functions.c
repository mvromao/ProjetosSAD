#include "device_functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <xc.h>

volatile int nSamples = 0;
volatile int period = 0;
volatile int timerIndex = 0;
volatile int sensors[num_sensors] = {1, 1};
volatile int sensorData[num_sensors][max_samples];
volatile int sampleIndex = 0;
volatile int sample = 0;

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

void setupTimer1() {
    TMR1 = 0;
    T1CON = 0x801110;
    PR1 = (8000000 / 2) / (100000); //PR1 = 40 - 100khz;
    IPC0bits.T1IP = 0x01;

    IFS0bits.T1IF = 0; //Clear the Timer1 interrupt status flag
    IEC0bits.T1IE = 1; //Enable Timer1 interrupts
    T1CONbits.TON = 0x01;
}

void __attribute__((__interrupt__, __shadow__)) _T1Interrupt(void) {
    sample = 1;
    IFS0bits.T1IF = 0; //Reset Timer1 interrupt flag and Return from ISR
}

void sendResultStringJSON() {
    char ResultString[250];
    const char *Sensor_names[] = {"\"Lux\": ", "\"Hall\": "};
    char temp[20];

    strcpy(ResultString, "{");
    int first = 1;
    for (int i = 0; i <= num_sensors - 1; i++) {
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

    strcat(ResultString, "}");
    strcat(ResultString, "\r\n");
    writeString(ResultString);
}

void writeChar(char a) {
    while (U1STAbits.UTXBF);
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

void changeValue(int value, char newValue) {
    value = (int) (newValue - '0');
}

void receiveInput(char * str) {
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

    processMessages(str);
}

void processMessages(char *str) {
    char auxStr[200];
    int i = 0, j = 0;

    while (str[i] != '\0') {
        // Skip any leading commas or whitespace
        while (str[i] == ',' || str[i] == ' ' || str[i] == '\n' || str[i] == '\r') {
            i++;
        }
        j = 0;
        // Copy until '}'
        while (str[i] != '}' && str[i] != '\0' && j < sizeof(auxStr) - 2) {
            auxStr[j++] = str[i++];
        }
        if (str[i] == '}') {
            auxStr[j++] = '}';
            i++;
        }
        auxStr[j] = '\0';

        // Only process if auxStr is a valid JSON object
        if (j > 0 && auxStr[0] == '{' && auxStr[j-1] == '}') {
            processInput(auxStr);
        }
    }
}

void processInput(char * str) {
    char buffer[70];
    // Optionally comment out the next two lines if you don't want to echo input
    // writeString(str);
    // writeString("\n");
    if (str[0] == '{' && str[1] == '"') {
        switch (str[2]) {
            case 'A':
                defineSampleInputs(str);
                break;
            case 'p':
                if (sscanf(str, "{\"p\":%d}", &period) == 1) {
                    sprintf(buffer, "Periodo cambiado para %d\n", period);
                    writeString(buffer);
                }
                break;
            case 'n':
                if (sscanf(str, "{\"n\":%d}", &nSamples) == 1) {
                    sprintf(buffer, "Numero de amuestras cambiado para %d\n", nSamples);
                    writeString(buffer);
                }
                break;
            case 's':
                sampleInputs();
                break;
            default:
                writeString("Opcao incorreta, tente outra vez.\n");
                break;
        }
    } else {
        writeString("Opcao incorreta, tente outra vez.\n");
    }
}

void defineSampleInputs(const char *str) {
    const char *ptr = str;

    if (*ptr == '{') {
        ptr++;
    } else {
        printf("Invalid format: missing '{'\n");
        return;
    }

    while (*ptr != '}' && *ptr != '\0') {
        while (isspace((unsigned char) *ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
            if (*ptr == '\0') {
                break;
            }
            ptr++;
        }

        char type = *ptr++;
        char identifier = *ptr++;

        while (isspace((unsigned char) *ptr) || *ptr == ',' || *ptr == '"' || *ptr == ':') {
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


        //printf("\tA%c = %d\n", identifier, value);
        switch (identifier) {
            case('0'): sensors[0] = value;
                break;
            case('1'): sensors[1] = value;
                break;
        }
        break;
        //}
    }
}

void sampleInputs() {
    int lux, hall;
    int values[5];

    setupTimer1();

    timerIndex = 0;
    while (sampleIndex < nSamples) {
        while (sample = 0);
        sample = 0;
        
        if (timerIndex < period)
            timerIndex++;
        else {
            I2C2_SendToArduino(0xAC);

            I2C2_ReadFromArduino(values, 5);

            parseValues(values, &lux, &hall);
            sensorData[0][sampleIndex] = lux;
            sensorData[1][sampleIndex] = hall;
            sampleIndex++;
        }
    }
    sendResultStringJSON();
    sampleIndex = 0;
    T1CONbits.TON = 0;
}

//FUNCOES 2 TRABALHO*****************************************************************************************************+

// I2C Initialization

void I2C2_Init(void) {
    ANSA = 0;

    // Enable I2C2 module
    I2C2CONL = 0; // Clear control bits
    I2C2CONLbits.I2CEN = 0; // Disable I2C before config

    // Set Baud Rate Generator for 100kHz
    //I2C2BRG = ((FCY / I2C_BAUDRATE) - FCY / 1111111) - 1; //39 caso n?o d?
    I2C2BRG = 39;

    I2C2CONLbits.I2CEN = 1; // Enable I2C
}

// Generate Start Condition

void I2C2_Start(void) {
    I2C2CONLbits.SEN = 1;
    while (I2C2CONLbits.SEN); // Wait for start to finish
}

// Send Stop Condition

void I2C2_Stop(void) {
    I2C2CONLbits.PEN = 1;
    while (I2C2CONLbits.PEN); // Wait for stop
}

void I2C2_SendToArduino(const char data) {
    I2C2_Start();

    // Send address with write bit (R/W = 0)
    I2C2TRN = SLAVE_ADDR << 1;
    while (I2C2STATbits.TRSTAT); // Wait until transmission is complete
    if (I2C2STATbits.ACKSTAT) {
        // Slave didn't acknowledge the address
        I2C2_Stop();
        return;
    }

    // Send data byte
    I2C2TRN = data;
    while (I2C2STATbits.TRSTAT); // Wait until transmission is complete
    if (I2C2STATbits.ACKSTAT) {
        // Slave didn't acknowledge the data
        I2C2_Stop();
        return;
    }

    I2C2_Stop();
}

unsigned int I2C2_Read(unsigned char ack) {
    I2C2CONLbits.RCEN = 1;
    while (!I2C2STATbits.RBF);
    unsigned int data = I2C2RCV;

    I2C2CONLbits.ACKDT = ack;
    I2C2CONLbits.ACKEN = 1;
    while (I2C2CONLbits.ACKEN);

    return data;
}

void I2C2_ReadFromArduino(int *buffer, int len) {
    I2C2_Start();
    I2C2TRN = (SLAVE_ADDR << 1) | 1; // Send slave address + Write bit

    while (I2C2STATbits.TBF);
    while (I2C2STATbits.TRSTAT);
    for (int i = 0; i < len; i++) {
        buffer[i] = I2C2_Read(i == (len - 1));
    }

    I2C2_Stop();
}

void parseValues(int* values, int *lux, int *hall) {
    *lux = (values[1] << 8) | values[2];
    *hall = (values[3] << 8) | values[4];
}