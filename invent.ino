#include <Arduino.h>
#include <LiquidCrystal.h> // Header for LCD library

const int MAX_SPRAYS = 10;      // Number of sprays when refilled

const byte RESET_INT_PIN = 2;   // Pin for INT.0 to get reset button
const byte DISPLAY_INT_PIN = 3; // Pin for INT.1 to get display button

const byte SPRAY_RELAY_PIN = 7; // Pin to control power relay to spray

const unsigned long PRE_SPRAY_TIME     = 1000UL * 60; // ms * seconds = 1 minute
const unsigned long LONG_DISPLAY_TIME  = 1000UL * 60; // ms * seconds = 1 minute
const unsigned long SHORT_DISPLAY_TIME = 1000UL * 10; // ms * seconds = 10s
const unsigned long SPRAY_TIME         = 1000UL * 30; // ms * seconds = 30s
//const unsigned long SPRAY_WAIT         = 1000UL * 14 * 24 * 60 * 60; // ms * days * hours * minutes * seconds = 2 weeks
const unsigned long SPRAY_WAIT         = 1000UL * 30;
const unsigned long NO_SPRAY_WAIT      = 1000UL * 30;

volatile int spraysRemaining;   // Number of sprays remaining - may be changed by interrupts so volatile

// Initialize the LCD library with the numbers of the interface pins
LiquidCrystal lcd(4, 6, 10, 11, 12, 13);

void setup() {
    // Assume the tank is full and set spray count
    spraysRemaining = MAX_SPRAYS;

    // Set our button pins to be input mode
    pinMode(RESET_INT_PIN, INPUT);
    pinMode(DISPLAY_INT_PIN, INPUT);

    // Attach our interrupt handlers to perform actions on button press
    // these temporarily interrupt loop() to perform the needed acctions
    // then return to loop() where it left off -
    // FALLING means when the button is released after being pressed
    attachInterrupt(digitalPinToInterrupt(RESET_INT_PIN), doReset, FALLING);
    attachInterrupt(digitalPinToInterrupt(DISPLAY_INT_PIN), doDisplay, FALLING);

    // Set our spray control pin to be output mode and turn the relay off
    pinMode(SPRAY_RELAY_PIN, OUTPUT);
    digitalWrite(SPRAY_RELAY_PIN, LOW);

    // Init LCD and turn off display
    lcd.begin(16,2);
    lcd.noDisplay();

    //DEBUG
    lcd.display();
    lcd.setCursor(0,0);
    lcd.print("Setup done");
    delay(10000);
}

void loop() {

    if (spraysRemaining > 0) {
        // Do spraying process
        delay(PRE_SPRAY_TIME);
        turnOnSpray();
        delay(SPRAY_TIME);
        turnOffSpray();

        spraysRemaining--; // Reduce spray count by one.

        // Show remaining sprays
        turnOnDisplay();
        delay(LONG_DISPLAY_TIME);
        turnOffDisplay();

        // Wait a long period before spraying again
        delay(SPRAY_WAIT);
    } else {
        // Wait a short period before checking the remaining sprays again
        delay(NO_SPRAY_WAIT);
    }

}

void turnOnSpray() {
    lcd.clear();
    lcd.display();
    lcd.print("SPRAYING SPRAYING");
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
    // Turn on the display, wait a short time and then turn it off
    turnOnDisplay();
    delay(SHORT_DISPLAY_TIME);
    turnOffDisplay();
}

void doReset() {
    // Someone has refilled the tank and pressed the reset button,
    // reset the number of sprays to the maximum for the tank.
    spraysRemaining = MAX_SPRAYS;
}