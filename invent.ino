#include <Arduino.h>
#include <LiquidCrystal.h> // Header for LCD library

const int MAX_SPRAYS = 10;      // Number of sprays when refilled

const byte RESET_PIN = 2;   // Pin for reset button
const byte DISPLAY_PIN = 3; // Pin for display button

const byte SPRAY_RELAY_PIN = 7; // Pin to control power relay to spray

const unsigned long DEBOUNCE_DELAY = 50UL; // Value to filture button bounces

const unsigned long PRE_SPRAY_TIME     = 1000UL * 60; // ms * seconds = 1 minute
const unsigned long POST_SPRAY_TIME    = 1000UL * 60; // ms * seconds = 1 minute
const unsigned long SPRAY_TIME         = 1000UL * 30; // ms * seconds = 30s
const unsigned long SHORT_DISPLAY_TIME = 1000UL * 10; // ms * seconds = 10s
//const unsigned long SPRAY_TIME         = 1000UL * 14 * 24 * 60 * 60; // ms * days * hours * minutes * seconds = 2 weeks
const unsigned long SPRAY_WAIT_TIME    = 1000UL * 30;
const unsigned long NO_SPRAY_WAIT_TIME = 1000UL * 30;

// Loop state definitions and current loop state variable
const int PRE_SPRAY_STATE = 0;
const int SPRAYING_STATE = 1;
const int POST_SPRAY_STATE = 2;
const int WAITING_STATE = 3;

int currentState;

// Variables for keeping track of button presses and debouncing them
int lastResetButtonState = LOW;
int lastDisplayButtonState = LOW;
unsigned long lastResetButtonDebounceTime = 0UL;
unsigned long lastDisplayButtonDebounceTime = 0UL;

// Time variables for tracking passage of time without using delay()
unsigned long lastLoopMillis;
long remainingStateMillis;

int spraysRemaining;   // Number of sprays remaining

// Variable for keeping track of display state
bool displayOn = false;
long remainingDisplayTime = 0;

// Initialize the LCD library with the numbers of the interface pins
LiquidCrystal lcd(4, 6, 10, 11, 12, 13);

void setup() {
    Serial.begin(115200);

    // Assume the tank is full and set spray count
    spraysRemaining = MAX_SPRAYS;

    // We'll always start in the pre-spray state
    currentState = PRE_SPRAY_STATE;

    // Set our button pins to be input mode
    pinMode(RESET_PIN, INPUT);
    pinMode(DISPLAY_PIN, INPUT);

    // Set our spray control pin to be output mode
    // and turn the relay off by setting it to low
    pinMode(SPRAY_RELAY_PIN, OUTPUT);
    digitalWrite(SPRAY_RELAY_PIN, LOW);

    // Init LCD and turn off display
    lcd.begin(16,2);
    lcd.noDisplay();

    //DEBUG
    lcd.display();
    lcd.setCursor(0,0);
    lcd.print("Setup done");

    // Init time varaiables
    remainingStateMillis = PRE_SPRAY_TIME;
    lastLoopMillis = millis();
}

void loop() {
    //debug("loop");

    if (displayOn) {
        remainingDisplayTime -= (millis() - lastLoopMillis);
        if (remainingDisplayTime <= 0) {
            turnOffDisplay();
            displayOn = false;
        }
    }

    switch (currentState) {
        case PRE_SPRAY_STATE:
            preSprayState();
            break;
        case SPRAYING_STATE:
            sprayingState();
            break;
        case POST_SPRAY_STATE:
            postSprayState();
            break;
        case WAITING_STATE:
            waitingState();
            break;
        default:
            Serial.print("Unexpected state");
    }

    doCheckButtons();

    lastLoopMillis = millis();
}

void preSprayState() {
    debug("prespray");
    // Reduce remaining time by the delta used by the last loop
    remainingStateMillis -= (millis() - lastLoopMillis);
    if (remainingStateMillis <= 0) {
        // Transition to next state if this state's time has expired
        currentState = SPRAYING_STATE;
        remainingStateMillis = SPRAY_TIME;
    }
}

void sprayingState() {
    debug("spraying");
    // See if this is the first exeuction
    if (remainingStateMillis == SPRAY_TIME) {
        turnOnSpray();
    }
    remainingStateMillis -= (millis() - lastLoopMillis);
    if (remainingStateMillis <= 0) {
        // Transition to next state as this state's time has expired
        turnOffSpray();
        currentState = POST_SPRAY_STATE;
        remainingStateMillis = POST_SPRAY_TIME;
    }
}

void postSprayState() {
    debug("postspray");
    //See if this is the first execution
    if (remainingStateMillis == POST_SPRAY_TIME) {
        spraysRemaining--;
        turnOnDisplay();
    }
    remainingStateMillis -= (millis() - lastLoopMillis);
    if (remainingStateMillis <= 0) {
        // Transition to next state as this state's time has expired
        turnOffDisplay();
        currentState = WAITING_STATE;
        remainingStateMillis = (spraysRemaining > 0 ? SPRAY_WAIT_TIME : NO_SPRAY_WAIT_TIME);
    }
}

void waitingState() {
    debug("waiting");
    remainingStateMillis -= (millis() - lastLoopMillis);
    if (remainingStateMillis <= 0) {
        // Transition to next state if there are sprays left or stay in this one if there are not
        if (spraysRemaining > 0) {
            currentState = PRE_SPRAY_STATE;
            remainingStateMillis = PRE_SPRAY_TIME;
        } else {
            remainingStateMillis = NO_SPRAY_WAIT_TIME;
        }
    }
}

void turnOnSpray() {
    lcd.clear();
    lcd.display();
    lcd.print("SPRAYING NOW");
    digitalWrite(SPRAY_RELAY_PIN, HIGH); // Turn on the pin to allow power to flow
    // through the relay to the spray pump
}

void turnOffSpray() {
    digitalWrite(SPRAY_RELAY_PIN, LOW); // Turn off the pin to stop power flow
    // through the relay to the spray pump
    lcd.noDisplay();
}

void turnOnDisplay() {
    char buffer[17];
    lcd.display(); // Turn on the display
    lcd.clear();
    lcd.setCursor(0,0); // Line one, column one
    if (spraysRemaining > 0) {
        // Show number of sprays left
        snprintf(buffer, 16, "Sprays left: %d", spraysRemaining);
        lcd.print(buffer);
    } else {
        // Tell them to refill
        lcd.print("No sprays remaining.");
        lcd.setCursor(0,1); // Line two, column one
        lcd.print("Refill now.");
    }
}

void turnOffDisplay() {
    lcd.noDisplay(); // Power down the LCD controller
}

void doDisplay() {
    // Turn on the display, and setup the display state to wait
    turnOnDisplay();
    remainingDisplayTime = SHORT_DISPLAY_TIME;
    displayOn = true;
}

void doReset() {
    // Someone has refilled the tank and pressed the reset button,
    // reset the number of sprays to the maximum for the tank.
    spraysRemaining = MAX_SPRAYS;
}

void doCheckButtons() {
    // Check the reset button first
    int resetButtonRead = digitalRead(RESET_PIN);

    // If it changed, reset the debounce time
    if (resetButtonRead != lastResetButtonState) {
        lastResetButtonDebounceTime = millis();
    }

    if ((millis() - lastResetButtonDebounceTime) > DEBOUNCE_DELAY) {
        // This state has held long enough to be debounced so chage the current state if not
        // already the same:

        if (resetButtonRead != lastResetButtonState) {
            lastResetButtonState = resetButtonRead;
            // If it changed to being pressed, do the reset.
            if(resetButtonRead = HIGH) {
                doReset();
            }
        }
    }

    // Now check the display button since that still has a delay in it.. :(
    int displayButtonRead = digitalRead(DISPLAY_PIN);

    // If it changed, reset the debounce time
    if (displayButtonRead != lastDisplayButtonState) {
        lastDisplayButtonDebounceTime = millis();
    }

    if ((millis() - lastDisplayButtonDebounceTime) > DEBOUNCE_DELAY) {
        // This state has held long enough to be debounced so chage the current state if not
        // already the same:

        if (displayButtonRead != lastDisplayButtonState) {
            lastDisplayButtonState = displayButtonRead;
            // If it changed to being pressed, do the display.
            if(displayButtonRead = HIGH) {
                doDisplay();
            }
        }
    }

}

void debug(char * buffer) {
    lcd.display();
    lcd.setCursor(0,1);
    lcd.print("                ");
    lcd.setCursor(0,1);
    lcd.print(buffer);
}
