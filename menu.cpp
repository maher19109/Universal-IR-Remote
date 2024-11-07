// menu.cpp
#include "menu.h"
#include "uart.h"

dogm_7036 LCD;

ScreenState currentScreen = START;
MenuOption currentMenuOption = RECORD;
Status currentStatus = READY;

const char charSet[] = "abcdefghijklmnopqrstuvwxyz0123456789";
int charSetLength = sizeof(charSet) - 1;
char irCommandName[8] = "";  // Max 8 characters + null terminator
int namePos = 0;             // Current character position in name
int charIndex = 0;           // Current index in charSet for character selection

unsigned long buttonPressTime = 0;
unsigned long lastButtonPress[4] = {0, 0, 0, 0}; // Track debounce for each button
const int debounceDelay = 300;                   // Increased debounce delay to 300 ms

const int longPressDuration = 1000; // 1 second for long press

bool isButtonPressed(uint8_t buttonPin);

void initDisplayMenu() { 
    LCD.initialize(10, 0, 0, 9, 4, 0, DOGM162);
    LCD.displ_onoff(true);
    LCD.cursor_onoff(false);  // Cursor off initially

    DDRD &= ~((1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5));
    PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4) | (1 << PD5);
    DDRD |= (1 << PD6) | (1 << PD7);
    DDRB |= (1 << PB0);

    initUART(9600);  // Initialize UART at 9600 baud rate
    updateDisplay();
}

void checkButtons() {
    unsigned long currentTime = millis();

    if (currentScreen == START) {
        if (isButtonPressed(PD2) || isButtonPressed(PD3) || isButtonPressed(PD4) || isButtonPressed(PD5)) {
            currentScreen = MODE_SELECTION;
            delay(100);  // Short delay to avoid accidental button presses after transition
            updateDisplay();
        }
    }
    else if (currentScreen == MODE_SELECTION) {
        if (isButtonPressed(PD2) && (currentTime - lastButtonPress[0] > debounceDelay)) {  // Toggle mode down
            currentMenuOption = static_cast<MenuOption>((currentMenuOption + 1) % 3);  // Cycle through RECORD, REPLAY, DELETE
            updateDisplay();
            lastButtonPress[0] = currentTime;
        } else if (isButtonPressed(PD3) && (currentTime - lastButtonPress[1] > debounceDelay)) {  // Toggle mode up
            currentMenuOption = static_cast<MenuOption>((currentMenuOption - 1 + 3) % 3);  // Cycle through RECORD, REPLAY, DELETE
            updateDisplay();
            lastButtonPress[1] = currentTime;
        } else if (isButtonPressed(PD4)) {  // Enter selected mode
            switch (currentMenuOption) {
                case RECORD:
                    record();
                    break;
                case REPLAY:
                    replay();
                    break;
                case DELETE:
                    deleteAction();
                    break;
            }
            delay(100);  // Delay to avoid accidental screen skipping
            updateDisplay();
        } else if (isButtonPressed(PD5)) {  // Go back to start screen
            currentScreen = START;
            delay(100);
            updateDisplay();
        }
    }
    else if (currentScreen == RECORD_CONFIRMATION) {
        LCD.cursor_onoff(false);  // Turn off cursor in confirmation screen
        if (isButtonPressed(PD4)) {  // Confirm saving
            currentScreen = NAME_ENTRY;
            startNameEntry();
        } else if (isButtonPressed(PD5)) {  // Cancel and return to start screen
            currentScreen = START;
            delay(100);
            updateDisplay();
        }
    }
    else if (currentScreen == REPLAY_MODE || currentScreen == DELETE_MODE) {
        if (isButtonPressed(PD2) || isButtonPressed(PD3) || isButtonPressed(PD4) || isButtonPressed(PD5)) {
            currentScreen = START;  // Return to start screen on any button press
            delay(100);
            updateDisplay();
        }
    }
    else if (currentScreen == NAME_ENTRY) {
        handleNameEntryButtons(currentTime);
    }
}

void handleNameEntryButtons(unsigned long currentTime) {
    if (isButtonPressed(PD2) && (currentTime - lastButtonPress[0] > debounceDelay)) {  // Increment character
        charIndex = (charIndex + 1) % charSetLength;
        handleNameEntry(PD2);
        lastButtonPress[0] = currentTime;
    }
    else if (isButtonPressed(PD3) && (currentTime - lastButtonPress[1] > debounceDelay)) {  // Decrement character
        charIndex = (charIndex - 1 + charSetLength) % charSetLength;
        handleNameEntry(PD3);
        lastButtonPress[1] = currentTime;
    }
    else if (isButtonPressed(PD4) && (currentTime - lastButtonPress[2] > debounceDelay)) {  // Move to next position
        // Move to the next character position, but prevent moving to the last character
        if (namePos < 6) {  // Allow up to the 7th position (index 6)
            namePos = (namePos + 1) % 7;  // Only cycle within 7 characters
            charIndex = strchr(charSet, irCommandName[namePos]) - charSet;
            handleNameEntry(PD4);
        }
        lastButtonPress[2] = currentTime;
    }
    else if (isButtonPressed(PD5)) {
        if (buttonPressTime == 0) buttonPressTime = currentTime;  // Start timing for long press

        // Check for long press duration
        if (currentTime - buttonPressTime > longPressDuration) {
            // Long press detected: cancel and return to start
            currentScreen = START;
            updateDisplay();
        }
    } else if (!isButtonPressed(PD5) && buttonPressTime != 0) {  // Button released after a short press
        if (currentTime - buttonPressTime < longPressDuration) {
            // Short press detected: move left to previous character
            namePos = (namePos - 1 + 7) % 7;  // Prevent moving to the 8th character
            charIndex = strchr(charSet, irCommandName[namePos]) - charSet;
            handleNameEntry(PD5);
        }
        buttonPressTime = 0;  // Reset timing
    }
}

void startNameEntry() {
    memset(irCommandName, 'a', sizeof(irCommandName) - 1);
    irCommandName[7] = '\0';  // Ensure the last character is always '\0'
    namePos = 0;
    charIndex = strchr(charSet, irCommandName[namePos]) - charSet;
    updateDisplay();
}

void handleNameEntry(uint8_t buttonPin) {
    irCommandName[namePos] = charSet[charIndex];
    updateDisplay();
}

void record() {
    // Display "Recording..." and delay for 2 seconds
    LCD.clear_display();
    LCD.position(1, 1);
    LCD.string("Recording...");
    delay(2000);

    // Display "Record done!" and delay for 2 seconds
    LCD.clear_display();
    LCD.position(1, 1);
    LCD.string("Record done!");
    delay(2000);

    // Transition to the save confirmation screen
    currentScreen = RECORD_CONFIRMATION;
    delay(100);  // Prevent accidental skips
    updateDisplay();
}

void replay() {
    // Display "Replaying..." and delay for 2 seconds
    LCD.clear_display();
    LCD.position(1, 1);
    LCD.string("Replaying...");
    delay(2000);

    // Display "Replay done!" and transition back to start
    LCD.clear_display();
    LCD.position(1, 1);
    LCD.string("Replay done!");
    delay(2000);
    currentScreen = START;
}

void deleteAction() {
    // Display "Deleting..." and delay for 2 seconds
    LCD.clear_display();
    LCD.position(1, 1);
    LCD.string("Deleting...");
    delay(2000);

    // Display "Delete done!" and transition back to start
    LCD.clear_display();
    LCD.position(1, 1);
    LCD.string("Delete done!");
    delay(2000);
    currentScreen = START;
}

void updateDisplay() {
    LCD.clear_display();

    switch (currentScreen) {
        case START:
            LCD.position(1, 1);
            LCD.string("   Universal");
            LCD.position(1, 2);
            LCD.string("   IR-Remote");
            break;

        case MODE_SELECTION:
            LCD.position(1, 1);
            switch (currentMenuOption) {
                case RECORD:
                    LCD.string("Mode: RECORD");
                    break;
                case REPLAY:
                    LCD.string("Mode: REPLAY");
                    break;
                case DELETE:
                    LCD.string("Mode: DELETE");
                    break;
            }
            break;

        case RECORD_CONFIRMATION:
            LCD.position(1, 1);
            LCD.string("Record done");
            LCD.position(1, 2);
            LCD.string("->Save <-Cancel");
            break;

        case REPLAY_MODE:
            LCD.position(1, 1);
            LCD.string("Replay done");
            break;

        case DELETE_MODE:
            LCD.position(1, 1);
            LCD.string("Delete done");
            break;

        case NAME_ENTRY:
            LCD.position(1, 1);
            LCD.string("Enter name:");
            LCD.position(1, 2);
            LCD.string(irCommandName);
            LCD.position(namePos + 1, 2);  // Cursor position
            LCD.cursor_onoff(true);
            break;
    }
}

bool isButtonPressed(uint8_t buttonPin) {
    return !(PIND & (1 << buttonPin));
}
