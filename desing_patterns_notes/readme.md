# Design Patterns

## Behaviour Pattern
### State Pattern

> The State Pattern allows an object to alter its behavior
when its internal state changes. The object will appear to
change its class.

When to use the State Pattern (simple rule)
👉 Use the State Pattern when:
* Your object’s behavior is controlled by a state variable
* You see large conditional blocks (if / else / switch)
* The same condition structure appears in multiple methods
* Each state has its own distinct behavior

What the State Pattern changes (conceptually)
Instead of:
* One object with many condition checks

You move to:
* One object
* Multiple state classes
* Each state class contains only the behavior for that state

So:
* No big conditionals
* Each state’s logic is isolated
* Adding a new state = add a new class, not modify existing code

```cpp
#include <iostream>
#include <memory> // For std::unique_ptr

// Forward declaration
class MusicPlayer;

// --- 1. Abstract State ---
class State {
public:
    virtual ~State() = default;
    virtual void play(MusicPlayer* player) = 0;
    virtual void stop(MusicPlayer* player) = 0;
};

// --- 3. Context ---
class MusicPlayer {
private:
    std::unique_ptr<State> currentState;

public:
    MusicPlayer();

    // Setter for the state
    void setState(std::unique_ptr<State> newState) {
        currentState = std::move(newState);
    }

    // Delegate behavior to the current state object
    void pressPlay() {
        currentState->play(this);
    }

    void pressStop() {
        currentState->stop(this);
    }
};

// --- 2. Concrete States (Defined after MusicPlayer due to cyclic dependency) ---

// State when the player is currently playing music
class PlayingState : public State {
public:
    void play(MusicPlayer* player) override {
        std::cout << "Player: Already playing. Enjoy the music!" << std::endl;
    }

    void stop(MusicPlayer* player) override; // Defined below
};

// State when the player is currently stopped
class StoppedState : public State {
public:
    void play(MusicPlayer* player) override; // Defined below

    void stop(MusicPlayer* player) override {
        std::cout << "Player: Already stopped." << std::endl;
    }
};

// Implementations of methods that trigger a state transition
void PlayingState::stop(MusicPlayer* player) {
    std::cout << "Player: Stopping playback." << std::endl;
    // Transition to StoppedState
    player->setState(std::make_unique<StoppedState>());
}

void StoppedState::play(MusicPlayer* player) {
    std::cout << "Player: Starting playback." << std::endl;
    // Transition to PlayingState
    player->setState(std::make_unique<PlayingState>());
}

// Initial state of the MusicPlayer (Stopped)
MusicPlayer::MusicPlayer() {
    currentState = std::make_unique<StoppedState>();
    std::cout << "MusicPlayer initialized (State: Stopped)" << std::endl;
}

// --- Client Code ---
int main() {
    MusicPlayer player; // Starts in StoppedState
    
    std::cout << "\n--- Action 1: Press Play ---" << std::endl;
    player.pressPlay(); // Calls play() on StoppedState -> Transition to PlayingState

    std::cout << "\n--- Action 2: Press Play (again) ---" << std::endl;
    player.pressPlay(); // Calls play() on PlayingState -> No transition

    std::cout << "\n--- Action 3: Press Stop ---" << std::endl;
    player.pressStop(); // Calls stop() on PlayingState -> Transition to StoppedState
    
    std::cout << "\n--- Action 4: Press Stop (again) ---" << std::endl;
    player.pressStop(); // Calls stop() on StoppedState -> No transition

    return 0;
}
```

C Implementation

```c
#include <stdio.h>
#include <stdlib.h>

// --- Forward Declarations ---
typedef struct MusicPlayer MusicPlayer;
typedef struct StateVTable StateVTable;

// --- 1. State V-Table (Interface) ---
// This is equivalent to the abstract C++ State class
struct StateVTable {
    void (*play)(MusicPlayer* player);
    void (*stop)(MusicPlayer* player);
};

// --- 2. Context Structure ---
struct MusicPlayer {
    // The current state is represented by a pointer to the V-Table
    const StateVTable* currentState; 
};

// --- 3. Concrete State Implementations (Functions) ---

// Transition to the new state
void MusicPlayer_transitionTo(MusicPlayer* player, const StateVTable* newState) {
    player->currentState = newState;
}

// --- Playing State Functions ---
void PlayingState_play(MusicPlayer* player) {
    printf("Player: Already playing. Enjoy the music!\n");
}

void PlayingState_stop(MusicPlayer* player); // Defined later

// --- Stopped State Functions ---
void StoppedState_play(MusicPlayer* player); // Defined later

void StoppedState_stop(MusicPlayer* player) {
    printf("Player: Already stopped.\n");
}

// --- 4. Global Concrete State V-Tables (The State Objects) ---

// Declare the state V-tables (which are defined later)
extern const StateVTable g_PlayingStateVTable;
extern const StateVTable g_StoppedStateVTable;

// Implementations of methods that trigger a state transition
void PlayingState_stop(MusicPlayer* player) {
    printf("Player: Stopping playback.\n");
    // Transition to StoppedState
    MusicPlayer_transitionTo(player, &g_StoppedStateVTable);
}

void StoppedState_play(MusicPlayer* player) {
    printf("Player: Starting playback.\n");
    // Transition to PlayingState
    MusicPlayer_transitionTo(player, &g_PlayingStateVTable);
}

// Global V-Table Definitions - The actual state implementations
const StateVTable g_PlayingStateVTable = {
    .play = PlayingState_play,
    .stop = PlayingState_stop
};

const StateVTable g_StoppedStateVTable = {
    .play = StoppedState_play,
    .stop = StoppedState_stop
};


// --- 5. Context Methods (Delegation) ---

void MusicPlayer_init(MusicPlayer* player) {
    // Set the initial state (StoppedState)
    player->currentState = &g_StoppedStateVTable;
    printf("MusicPlayer initialized (State: Stopped)\n");
}

// Delegate behavior to the current state's function pointer
void MusicPlayer_pressPlay(MusicPlayer* player) {
    player->currentState->play(player);
}

void MusicPlayer_pressStop(MusicPlayer* player) {
    player->currentState->stop(player);
}

// --- Client Code ---
int main() {
    // Create and initialize the context object
    MusicPlayer player; 
    MusicPlayer_init(&player); // Starts in StoppedState
    
    printf("\n--- Action 1: Press Play ---\n");
    MusicPlayer_pressPlay(&player); // Calls StoppedState_play -> Transition to PlayingState

    printf("\n--- Action 2: Press Play (again) ---\n");
    MusicPlayer_pressPlay(&player); // Calls PlayingState_play -> No transition

    printf("\n--- Action 3: Press Stop ---\n");
    MusicPlayer_pressStop(&player); // Calls PlayingState_stop -> Transition to StoppedState
    
    printf("\n--- Action 4: Press Stop (again) ---\n");
    MusicPlayer_pressStop(&player); // Calls StoppedState_stop -> No transition

    return 0;
}
```