#include <Arduino.h>
#include <Ticker.h>

// Pinovi i parametri
#define BTN1_PIN 12
#define BTN2_PIN 14
#define LED_PIN  2
#define SENSOR_PIN  33
#define SERIAL_BAUD_RATE 115200
#define SERIAL_RX_PIN 16

// Volatile varijable za praćenje stanja prekida
volatile bool btn1_pressed = false;
volatile bool btn2_pressed = false;
volatile bool sensor_triggered = false;
volatile bool timer_triggered = false;
volatile bool serial_received = false;
char received_char;

unsigned long lastBtn1Press = 0;
unsigned long lastBtn2Press = 0;
const unsigned long debounceDelay = 200;

Ticker timer;  // Timer za intervale

// Prioriteti prekida
#define BTN1_INTR_PRIORITY 10
#define BTN2_INTR_PRIORITY 20
#define SENSOR_INTR_PRIORITY 30
#define TIMER_INTR_PRIORITY 40

// ISR funkcija za tipku 1
void IRAM_ATTR btn1ISR() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBtn1Press > debounceDelay) {
        noInterrupts();
        btn1_pressed = true;
        lastBtn1Press = currentMillis;
        interrupts();
    }
}

// ISR funkcija za tipku 2
void IRAM_ATTR btn2ISR() {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBtn2Press > debounceDelay) {
        noInterrupts();
        btn2_pressed = true;
        lastBtn2Press = currentMillis;
        interrupts();
    }
}

// ISR funkcija za PIR senzor
void IRAM_ATTR sensorISR() {
    noInterrupts();
    sensor_triggered = true;
    interrupts();
}

// ISR funkcija za timer
void IRAM_ATTR onTimer() {
    noInterrupts();
    timer_triggered = true;
    digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Prebaci LED
    interrupts();
}

// ISR funkcija za serijski port
void IRAM_ATTR serialISR() {
    if (Serial.available()) {
        received_char = Serial.read();
        noInterrupts();
        serial_received = true;
        interrupts();
    }
}

void setup() {
    Serial.begin(SERIAL_BAUD_RATE);  // Pokreni serijsku komunikaciju
    pinMode(BTN1_PIN, INPUT_PULLUP);  // Tipka 1 kao ulaz s pull-up otpornikom
    pinMode(BTN2_PIN, INPUT_PULLUP);  // Tipka 2 kao ulaz s pull-up otpornikom
    pinMode(LED_PIN, OUTPUT);         // LED kao izlaz
    pinMode(SENSOR_PIN, INPUT);       // PIR senzor kao ulaz

    // Postavi prekide za ESP32
    attachInterrupt(BTN1_PIN, btn1ISR, RISING);
    attachInterrupt(BTN2_PIN, btn2ISR, RISING);
    attachInterrupt(SENSOR_PIN, sensorISR, CHANGE);  // Detekcija promjena na PIR senzoru
    attachInterrupt(SERIAL_RX_PIN, serialISR, FALLING);

    // Timer aktivacija svakih 1 sekundu
    timer.attach(1.0, onTimer);
}

void loop() {
    // Obrada tipki
    if (btn1_pressed) {
        btn1_pressed = false;
        // Tipka 1: togglaj LED
        digitalWrite(LED_PIN, !digitalRead(LED_PIN));  // Toglaj LED
        Serial.println("Button 1 Pressed: LED toggled");
    }

    if (btn2_pressed) {
        btn2_pressed = false;
        // Tipka 2: uključi LED
        digitalWrite(LED_PIN, HIGH);  // LED on
        Serial.println("Button 2 Pressed: LED ON");
    }

    // Obrada PIR senzora
    if (sensor_triggered) {
        sensor_triggered = false;
        // Detekcija pokreta
        Serial.println("Motion Detected by PIR Sensor");
    }

    // Obrada timera
    if (timer_triggered) {
        timer_triggered = false;
        // Timer: svaki put kad timer istekne
        Serial.println("Timer Interrupt Triggered");
    }

    // Obrada serijskog porta
    if (serial_received) {
        serial_received = false;
        // Primi podatke sa serijskog porta
        Serial.print("Received from Serial: ");
        Serial.println(received_char);
    }
}
