# State Pattern

The State Pattern allows an object to alter its behavior
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
//State.h

#ifndef STATE_H
#define STATE_H

class State {
public:
    virtual ~State() = default;

    virtual void insertQuarter() = 0;
    virtual void ejectQuarter() = 0;
    virtual void turnCrank() = 0;
    virtual void dispense() = 0;
};

#endif


// GumballMachine.h
#ifndef GUMBALL_MACHINE_H
#define GUMBALL_MACHINE_H

#include "State.h"

// Forward declarations
class SoldOutState;
class NoQuarterState;
class HasQuarterState;
class SoldState;

class GumballMachine {
private:
    State* soldOutState;
    State* noQuarterState;
    State* hasQuarterState;
    State* soldState;

    State* state;
    int count;

public:
    explicit GumballMachine(int numberGumballs);

    // User actions
    void insertQuarter();
    void ejectQuarter();
    void turnCrank();

    // Internal helpers
    void setState(State* state);
    void releaseBall();

    int getCount() const;

    // State getters
    State* getSoldOutState();
    State* getNoQuarterState();
    State* getHasQuarterState();
    State* getSoldState();
};

#endif

// GumballMachine.cpp

#include <iostream>
#include "GumballMachine.h"
#include "SoldOutState.h"
#include "NoQuarterState.h"
#include "HasQuarterState.h"
#include "SoldState.h"

GumballMachine::GumballMachine(int numberGumballs)
    : count(numberGumballs)
{
    soldOutState    = new SoldOutState(this);
    noQuarterState  = new NoQuarterState(this);
    hasQuarterState = new HasQuarterState(this);
    soldState       = new SoldState(this);

    state = (count > 0) ? noQuarterState : soldOutState;
}

void GumballMachine::insertQuarter() {
    state->insertQuarter();
}

void GumballMachine::ejectQuarter() {
    state->ejectQuarter();
}

void GumballMachine::turnCrank() {
    state->turnCrank();
    state->dispense();
}

void GumballMachine::setState(State* s) {
    state = s;
}

void GumballMachine::releaseBall() {
    if (count > 0) {
        std::cout << "A gumball comes rolling out the slot...\n";
        count--;
    }
}

int GumballMachine::getCount() const {
    return count;
}

State* GumballMachine::getSoldOutState() {
    return soldOutState;
}

State* GumballMachine::getNoQuarterState() {
    return noQuarterState;
}

State* GumballMachine::getHasQuarterState() {
    return hasQuarterState;
}

State* GumballMachine::getSoldState() {
    return soldState;
}

// SoldOutState.h
#ifndef SOLD_OUT_STATE_H
#define SOLD_OUT_STATE_H

#include "State.h"

class GumballMachine;

class SoldOutState : public State {
private:
    GumballMachine* machine;

public:
    explicit SoldOutState(GumballMachine* m);

    void insertQuarter() override;
    void ejectQuarter() override;
    void turnCrank() override;
    void dispense() override;
};

#endif

// SoldOutState.cpp
#include <iostream>
#include "SoldOutState.h"
#include "GumballMachine.h"

SoldOutState::SoldOutState(GumballMachine* m) : machine(m) {}

void SoldOutState::insertQuarter() {
    std::cout << "Machine is sold out.\n";
}

void SoldOutState::ejectQuarter() {
    std::cout << "No quarter to eject.\n";
}

void SoldOutState::turnCrank() {
    std::cout << "You turned, but there are no gumballs.\n";
}

void SoldOutState::dispense() {
    std::cout << "No gumball dispensed.\n";
}

// NoQuarterState.h
#ifndef NO_QUARTER_STATE_H
#define NO_QUARTER_STATE_H

#include "State.h"

class GumballMachine;

class NoQuarterState : public State {
private:
    GumballMachine* machine;

public:
    explicit NoQuarterState(GumballMachine* m);

    void insertQuarter() override;
    void ejectQuarter() override;
    void turnCrank() override;
    void dispense() override;
};

#endif

// NoQuarterState.cpp
#include <iostream>
#include "NoQuarterState.h"
#include "GumballMachine.h"

NoQuarterState::NoQuarterState(GumballMachine* m) : machine(m) {}

void NoQuarterState::insertQuarter() {
    std::cout << "You inserted a quarter.\n";
    machine->setState(machine->getHasQuarterState());
}

void NoQuarterState::ejectQuarter() {
    std::cout << "You haven't inserted a quarter.\n";
}

void NoQuarterState::turnCrank() {
    std::cout << "You turned, but there's no quarter.\n";
}

void NoQuarterState::dispense() {
    std::cout << "You need to pay first.\n";
}

// main.cpp
#include "GumballMachine.h"

int main() {
    GumballMachine machine(2);

    machine.insertQuarter();
    machine.turnCrank();

    machine.insertQuarter();
    machine.ejectQuarter();
    machine.turnCrank();

    machine.insertQuarter();
    machine.turnCrank();

    return 0;
}

```