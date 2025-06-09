// device_functions.h
// Header file for separating main.c logic

#ifndef DEVICE_FUNCTIONS_H
#define DEVICE_FUNCTIONS_H

#include <xc.h>

#define max_samples 20
#define num_sensors 2

#define I2C_BAUDRATE 100000 // 100kHz standard
#define FCY 8000000UL      // PIC FCY
#define SLAVE_ADDR 0x48     // Example Arduino slave address (must match Arduino code)

// External variables
extern volatile int nSamples;
extern volatile int period;
extern volatile int timerIndex;
extern volatile int sensors[num_sensors];
extern volatile int sensorData[num_sensors][max_samples];
extern volatile int sampleIndex;
extern volatile int sample;

// ---------- Setup functions ----------
void setupUART1();
void setupTimer1();
void I2C2_Init();

// ---------- UART I/O ----------
void writeChar(char a);
void writeString(char *str);
char readChar();

// ---------- Command receiving and processing ----------
void receiveInput(char *str);
void processMessages(char *str);
void processInput(char *str);
void defineSampleInputs(const char *str);

// ---------- Settings ----------
void changeValue(int value, char newValue);
void sampleInputs();
void sendResultStringJSON();

// ---------- I2C communication ----------
void I2C2_Start(void);
void I2C2_Stop(void);
void I2C2_Write(unsigned char data);
unsigned int I2C2_read(unsigned char ack);
void I2C2_ReadFromArduino(int* buffer, int len);
void I2C2_SendToArduino(const char data);
void parseValues(int* values, int *lux, int *hall);

#endif // DEVICE_FUNCTIONS_H