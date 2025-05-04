// device_functions.h
// Header file for separating main.c logic

#ifndef DEVICE_FUNCTIONS_H
#define DEVICE_FUNCTIONS_H

#include <xc.h>

#define max_samples 20
#define num_sensors 7

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

// Setup functions
void setupChannels();
void setupUART1();
void setupADC();
void setupTimer1();
void setupTimer2();

// UART I/O
void writeChar(char a);
void writeString(char *str);
char readChar();

// ADC and temperature
int getPotentiometerValue();
void transmitPotentiometerData(int value, char *str);
int readTemperatureSensorADC();
int readSensorADC(int c);
float readCalibratedTemp(int adcValue);
void readTemp();

// Digital monitoring
void monitorDigitalInputs(int flag);

// Channel testing
void testChannels();

// Command receiving and processing
void receiveInput(char *str);
void processMessages(char *str);
void processInput(char *str);
void defineSampleInputs(const char *str);

// Settings
void changeValue(int value, char newValue);
void changeBidirectionalChannel(char c_value);
void sampleInputs();
void checkWarningValue();
void sendResultStringJSON();
#endif // DEVICE_FUNCTIONS_H
