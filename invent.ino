#include <Arduino.h>

const int MAX_SPRAYS = 10;      // Number of sprays when refilled

const byte RESET_INT_PIN = 2;   // Pin for INT.0 to get reset button
const byte DISPLAY_INT_PIN = 3; // Pin for INT.1 to get display button

const int LONG_DISPLAY_TIME  = 1000 * 60; // ms * seconds = 1 minute
const int SHORT_DISPLAY_TIME = 1000 * 10; // ms * seconds = 10s
const int SPRAY_TIME         = 1000 * 30; // ms * seconds = 30s
const int SPRAY_WAIT         = 1000 * 14 * 24 * 60 * 60; // ms * days * hours * minutes * seconds = 2 weeks

volatile int spraysRemaining;   // Number of sprays remaining - may be changed by interrupts so volatile

void setup() {
    // Assume the tank is full and set spray count
    spraysRemaining = MAX_SPRAYS;

    // Set our button pins to be input mode
    pinMode(RESET_INT_PIN, INPUT_PULLUP);
    pinMode(DISPLAY_INT_PIN, INPUT_PULLUP);

    // Attach our interrupt handlers to perform actions on button press
    // these temporarily interrupt loop() to perform the needed acctions
    // then return to loop() where it left off -
    // FALLING means when the button is released after being pressed
    attachInterrupt(digitalPintToInterrupt(RESET_INT_PIN), doReset, FALLING);
    attachInterrupt(digitalPintToInterrupt(DISPLAY_INT_PIN), doDisplay, FALLING);
}

void loop() {

}

void doDisplay() {

}

void doReset() {
    // Someone has refilled the tank and pressed the reset button,
    // reset the number of sprays to the maximum for the tank.
    spraysRemaining = MAX_SPRAYS;
}