#include <Arduino.h>
#include "dogm_7036.h"
#include "menu.h"

// Initialize the main function
int main() {
    // Initialize the Arduino environment
    init();

    // Initialize display and menu system
    initDisplayMenu();

    // Main program loop
    while (true) {
        checkButtons();
        delay(50);  // Add a small delay to prevent excessive CPU usage
    }

    return 0;
}

