# PomoTick

**Header-only Pomodoro timer library for Arduino**

*C++11 • Callback-driven • Pause & Resume • loop()-friendly*

## 🚀 Features

- ⏱️ **Full Pomodoro cycle**: 25 min work → 5 min short break → 15 min long break every 4th session
- 🔔 **Callback-driven**: Fires `void(PomoTickState)` on every state transition
- ⏸️ **Pause & Resume**: Freezes time accurately, no phantom ticks on resume
- 💾 **Zero Heap**: Single instance on the stack, no dynamic allocation
- 🔌 **Universal**: AVR (Uno), ESP32, SAMD, Teensy

## 📦 Installation

### Arduino IDE
```
Sketch → Include Library → Add .ZIP Library → PomoTick.zip
```

### PlatformIO
```ini
lib_deps =
    vishwamke-lab/PomoTick
```

## 💻 Quick Start
```cpp
#include <PomoTick.h>

void onStateChange(PomoTickState state) {
    switch (state) {
        case PomoTickState::WORK:        Serial.println(F("Work!"));        break;
        case PomoTickState::SHORT_BREAK: Serial.println(F("Short break!")); break;
        case PomoTickState::LONG_BREAK:  Serial.println(F("Long break!"));  break;
        case PomoTickState::PAUSED:      Serial.println(F("Paused."));      break;
        case PomoTickState::INACTIVE:    Serial.println(F("Stopped."));     break;
    }
}

PomoTick pomo(onStateChange);

void setup() {
    Serial.begin(9600);
    pomo.startSession();
}

void loop() {
    pomo.run(); // Must be called every iteration — this drives the countdown.
}
```

## 📚 API Reference

| Function | Description |
|---|---|
| `startSession()` | Start a work session (ignored if already in WORK) |
| `pauseSession()` | Freeze the timer, saving current state |
| `resumeSession()` | Restore state from before the pause |
| `stopSession()` | Reset everything back to INACTIVE |
| `run()` | Call every `loop()` — fires `tick()` once per second |
| `getCurrentState()` | Returns current `PomoTickState` |
| `getRemainingTime()` | Seconds remaining in the current phase |
| `getCompletedSessions()` | Number of fully completed work sessions |

## 🔄 State Machine
```
          startSession()
INACTIVE ─────────────────► WORK
                              │  ▲
           time expires       │  │ time expires
                  ┌───────────┘  └──────────────┐
                  ▼                              │
           SHORT_BREAK / LONG_BREAK ─────────────┘
                  │
       pauseSession() from any active state
                  ▼
               PAUSED
                  │
           resumeSession()
                  │
                  └──────────────► (previous state)
```

Long break triggers every 4th completed work session.

## ⏱️ Timing

| Phase | Duration | Trigger |
|---|---|---|
| Work | 25 min (1500 s) | `startSession()` or after any break |
| Short Break | 5 min (300 s) | After sessions 1, 2, 3 |
| Long Break | 15 min (900 s) | After every 4th session |

## ✅ Compatibility

| Platform | Status |
|---|---|
| Arduino Uno | Tested |
| ESP32 | Full |
| Teensy | Full |
| SAMD (MKR, Zero) | Full |

## 📁 Structure
```
PomoTick/
├── library.properties
├── library.json
├── keywords.txt
├── src/
│   └── PomoTick.h        ← Single header
├── README.md
└── examples/
    └── PomoTick/PomoTick.ino
```

## 📄 License

MIT © 2026 vishwamke-lab

A reliable, loop()-friendly Pomodoro timer with clean state callbacks!