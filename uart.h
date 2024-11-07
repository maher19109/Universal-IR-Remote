// uart.h
#ifndef UART_H
#define UART_H

#include <Arduino.h>


void initUART(unsigned long baudRate);
void uartSendString(const char *str);
void uartSendChar(char c);
void uartSendNewLine();

#endif
