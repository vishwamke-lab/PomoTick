#include <Arduino.h>
#include <PomoTick.h>

// Called automatically whenever the timer changes state.
// Use this to update a display, toggle an LED, buzz a buzzer, etc.
void onStateChange(PomoTickState newState) {
    Serial.print(F("State changed → "));
    switch (newState) {
        case PomoTickState::WORK:        Serial.println(F("WORK"));        break;
        case PomoTickState::SHORT_BREAK: Serial.println(F("SHORT_BREAK")); break;
        case PomoTickState::LONG_BREAK:  Serial.println(F("LONG_BREAK"));  break;
        case PomoTickState::PAUSED:      Serial.println(F("PAUSED"));      break;
        case PomoTickState::INACTIVE:    Serial.println(F("INACTIVE"));    break;
    }
}

// Create a PomoTick instance and hand it our callback.
PomoTick pomo(onStateChange);

// Tracks the last time we printed the remaining time, so we can log once per second.
unsigned long lastPrint = 0;

void setup() {
    Serial.begin(9600);
    Serial.println(F("PomoTick example — commands via Serial:"));
    Serial.println(F("  s = start    p = pause"));
    Serial.println(F("  r = resume   x = stop"));
    Serial.println();

    // Kick off the first work session straight away.
    pomo.startSession();
}

void loop() {
    // Must be called every loop iteration — this is what drives the countdown.
    pomo.run();

    // Print remaining time once per second so we can watch it tick down.
    if (millis() - lastPrint >= 1000) {
        lastPrint = millis();
        Serial.print(F("Remaining: "));
        Serial.print(pomo.getRemainingTime());
        Serial.print(F("s  |  Sessions completed: "));
        Serial.println(pomo.getCompletedSessions());
    }

    // Simple Serial command interface so you can poke the timer over USB.
    if (Serial.available()) {
        char cmd = Serial.read();
        switch (cmd) {
            case 's':
                Serial.println(F("→ Starting session..."));
                pomo.startSession();
                break;
            case 'p':
                Serial.println(F("→ Pausing..."));
                pomo.pauseSession();
                break;
            case 'r':
                Serial.println(F("→ Resuming..."));
                pomo.resumeSession();
                break;
            case 'x':
                Serial.println(F("→ Stopping and resetting..."));
                pomo.stopSession();
                break;
            default:
                break; // Ignore anything else (newlines, spaces, etc.)
        }
    }
}