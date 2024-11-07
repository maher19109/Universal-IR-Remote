// menu.h
#ifndef MENU_H
#define MENU_H

#include <Arduino.h>
#include "dogm_7036.h"

// Enum for screen states to manage different display screens and modes
enum ScreenState {
    START,
    MODE_SELECTION,
    RECORD_CONFIRMATION,
    REPLAY_MODE,
    DELETE_MODE,
    NAME_ENTRY
};


// Enum for menu options
enum MenuOption {
    RECORD,
    REPLAY,
    DELETE
};

// Enum for status (for future extensions, if needed)
enum Status {
    READY,
    RECORDING_DONE,
    REPLAY_DONE,
    ENTER_NAME
};



// Function declarations
void initDisplayMenu();              // Initializes the display and buttons
void checkButtons();                 // Checks button states and updates screen accordingly
void handleNameEntryButtons(unsigned long currentTime); // Handles button presses in NAME_ENTRY state
void startNameEntry();               // Initializes name entry
void handleNameEntry(uint8_t buttonPin); // Updates the current character in the name based on button input
void updateDisplay();                // Updates the display based on the current screen state
bool isButtonPressed(uint8_t buttonPin); // Checks if a specific button is pressed

// New function declarations for IR-Remote actions
void record();                       // Placeholder for the Record function
void replay();                       // Placeholder for the Replay function
void deleteAction();                 // Placeholder for the Delete function

#endif
