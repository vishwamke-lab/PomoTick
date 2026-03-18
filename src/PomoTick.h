#ifndef POMOTICK_H
#define POMOTICK_H

#include <functional> // Allows us to store a callback via std::function — called whenever the state changes.
#include <Arduino.h>  // Arduino-specific types (uint8_t, uint16_t) and millis().

// Every state the timer can be in at any given moment.
// uint8_t as the underlying type keeps this to one byte instead of four.
enum class PomoTickState : uint8_t {
    INACTIVE,     // Timer hasn't been started, or has been fully stopped.
    WORK,         // A focus session is running.
    SHORT_BREAK,  // A short break is running (after a regular work session).
    LONG_BREAK,   // A long break is running (after every 4th work session).
    PAUSED        // Timer is paused mid-session.
};

// Implements a Pomodoro timer that tracks work sessions and breaks.
// Designed to live inside Arduino's loop() — call run() every iteration to keep time accurate.
class PomoTick {

private:
    // Duration in seconds for each phase. constexpr means they're resolved at compile time, saving RAM.
    static constexpr uint16_t WORK_TIME            = 1500; // 25 min
    static constexpr uint16_t SHORT_BREAK_TIME     = 300;  // 5 min
    static constexpr uint16_t LONG_BREAK_TIME      = 900;  // 15 min
    static constexpr uint8_t  SESSIONS_BEFORE_LONG = 4;    // Long break trigger interval.

    PomoTickState state_;             // Current timer state.
    PomoTickState prepauseState_;     // State saved on pause so resumeSession() knows where to return.
    uint8_t       completedSessions_; // Number of fully completed work sessions.
    uint16_t      remainingTime_;     // Seconds left in the current phase.
    std::function<void(PomoTickState)> stateCb_; // User-supplied callback, fired on every state change.
    unsigned long lastMillis_;        // Timestamp of the last one-second tick, used to measure elapsed time.

    // Counts down one second and handles phase transitions when time runs out.
    // Private helper — run() decides when to call this.
    void tick() {
        if (remainingTime_ > 0) {
            remainingTime_--;
        } else {
            // Time's up — advance to the next phase.
            if (state_ == PomoTickState::WORK) {
                completedSessions_++;
                // Every SESSIONS_BEFORE_LONG completions, give a long break instead of a short one.
                if (completedSessions_ % SESSIONS_BEFORE_LONG == 0) {
                    state_ = PomoTickState::LONG_BREAK;
                    remainingTime_ = LONG_BREAK_TIME;
                } else {
                    state_ = PomoTickState::SHORT_BREAK;
                    remainingTime_ = SHORT_BREAK_TIME;
                }
            } else {
                // Break ended — start the next work session automatically.
                state_ = PomoTickState::WORK;
                remainingTime_ = WORK_TIME;
            }
            if (stateCb_) stateCb_(state_);
        }
    }

    // Resets all fields to defaults. Called by stopSession().
    void reset() {
        state_             = PomoTickState::INACTIVE;
        prepauseState_     = PomoTickState::INACTIVE;
        completedSessions_ = 0;
        remainingTime_     = 0;
        lastMillis_        = 0;
    }

public:
    // Constructor — takes the callback to fire on state changes.
    // explicit prevents accidental implicit conversions from a bare function pointer.
    explicit PomoTick(const std::function<void(PomoTickState)>& stateCb) :
        state_            {PomoTickState::INACTIVE},
        prepauseState_    {PomoTickState::INACTIVE},
        completedSessions_{0},
        remainingTime_    {0},
        lastMillis_       {0},
        stateCb_          {stateCb}
    {}

    // Starts a new work session. Ignored if one is already active.
    void startSession() {
        if (state_ == PomoTickState::WORK) return;
        
        state_         = PomoTickState::WORK;
        remainingTime_ = WORK_TIME;
        lastMillis_    = millis();
        
        if (stateCb_) stateCb_(state_);
    }

    // Call this inside loop(). Fires tick() once per second; does nothing while INACTIVE or PAUSED.
    void run() {
        if (state_ == PomoTickState::INACTIVE || state_ == PomoTickState::PAUSED) return;
        
        unsigned long now = millis();
        
        if (now - lastMillis_ < 1000) return;
        lastMillis_ = now;
        
        tick();
    }

    // Pauses the timer and saves the current state so we can restore it on resume.
    void pauseSession() {
        prepauseState_ = state_;
        state_         = PomoTickState::PAUSED;
        
        if (stateCb_) stateCb_(state_);
    }

    // Resumes from pause. lastMillis_ is reset to now to avoid a skipped tick on resume.
    void resumeSession() {
        if (state_ != PomoTickState::PAUSED) return;
        
        state_      = prepauseState_;
        lastMillis_ = millis();
        
        if (stateCb_) stateCb_(state_);
    }

    // Fully stops the timer and clears all session data.
    void stopSession() {
        reset();
        
        if (stateCb_) stateCb_(state_);
    }

    uint8_t       getCompletedSessions() { return completedSessions_; }
    PomoTickState getCurrentState()      { return state_; }
    uint16_t      getRemainingTime()     { return remainingTime_; }
};

#endif // POMOTICK_H