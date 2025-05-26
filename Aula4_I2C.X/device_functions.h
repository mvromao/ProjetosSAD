// device_functions.h
// Header file for separating main.c logic

#ifndef DEVICE_FUNCTIONS_H
#define DEVICE_FUNCTIONS_H

#include <xc.h>

#define max_samples 20
#define num_sensors 7

// External variables
extern unsigned char ledState;
extern int virtualChannel;
extern int bidirectional;
extern int warningValue;
extern volatile int nSamples;
extern volatile int period;
extern volatile int timerIndex;
extern volatile int sensors[num_sensors];
extern volatile int sensorData[num_sensors][max_samples];
extern volatile int sampleIndex;
extern int adc_channels[];

// ---------- Setup functions ----------
void setupChannels();
void setupUART1();
void setupADC();
void setupTimer1();
void I2C2_Init();

// ---------- UART I/O ----------
void writeChar(char a);
void writeString(char *str);
char readChar();

// ---------- ADC and temperature ----------
int getPotentiometerValue();
void transmitPotentiometerData(int value, char *str);
int readTemperatureSensorADC();
int readSensorADC(int c);
float readCalibratedTemp(int adcValue);
void readTemp();

// ---------- Digital monitoring ----------
void monitorDigitalInputs(int flag);
void checkButtonS5(); // Logic to handle button S5 press

// ---------- Channel testing ----------
void testChannels();

// ---------- Command receiving and processing ----------
void receiveInput(char *str);
void processMessages(char *str);
void processInput(char *str);
void defineSampleInputs(const char *str);

// ---------- Settings ----------
void changeValue(int value, char newValue);
void changeBidirectionalChannel(char c_value);
void sampleInputs();
void checkWarningValue();
void sendResultStringJSON();

// ---------- LED controls ----------
void turnOnLED(char led); // Takes 'x', 'y', 'z' and activates corresponding LED (RC1/RC2/RC3)

// ---------- I2C communication ----------
void I2C2_Start(void);
void I2C2_Stop(void);
void I2C2_Write(unsigned char data);
void I2C2_SendToArduino(const char data);

#endif // DEVICE_FUNCTIONS_H
