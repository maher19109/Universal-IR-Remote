// uart.cpp
#include "uart.h"

// Initialize UART with the given baud rate
void initUART(unsigned long baudRate) {
    Serial.begin(baudRate);
}

// Send a null-terminated string over UART
void uartSendString(const char *str) {
    while (*str) {
        uartSendChar(*str++);
    }
}

// Send a single character over UART
void uartSendChar(char c) {
    Serial.write(c);
}

// Send a newline character over UART
void uartSendNewLine() {
    Serial.write('\n');
}
