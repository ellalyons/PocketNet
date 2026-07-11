#include <Arduino.h>

#include "pocketnet_input.h"
#include "hardware_config.h"

namespace
{
    struct ButtonState
    {
        uint8_t pin;
        bool previousState;
        bool pressedEvent;
        unsigned long lastChangeTime;
    };

    constexpr unsigned long DEBOUNCE_MS = 40;

    ButtonState leftButton = {
        PIN_LEFT,
        false,
        false,
        0
    };

    ButtonState rightButton = {
        PIN_RIGHT,
        false,
        false,
        0
    };

    ButtonState selectButton = {
        PIN_SELECT,
        false,
        false,
        0
    };

    ButtonState backButton = {
        PIN_ALT,
        false,
        false,
        0
    };

    void updateButton(ButtonState& button)
    {
        const bool currentState = digitalRead(button.pin);
        const unsigned long now = millis();

        if (
            currentState != button.previousState &&
            now - button.lastChangeTime >= DEBOUNCE_MS
        )
        {
            button.lastChangeTime = now;
            button.previousState = currentState;

            if (currentState)
            {
                button.pressedEvent = true;
            }
        }
    }

    ButtonState& getButtonState(Input::Button button)
    {
        switch (button)
        {
            case Input::Button::Left:
                return leftButton;

            case Input::Button::Right:
                return rightButton;

            case Input::Button::Select:
                return selectButton;

            case Input::Button::Back:
            default:
                return backButton;
        }
    }
}

namespace Input
{
    void begin()
    {
        pinMode(PIN_LEFT, INPUT);
        pinMode(PIN_RIGHT, INPUT);
        pinMode(PIN_SELECT, INPUT);
        pinMode(PIN_ALT, INPUT);

        leftButton.previousState = digitalRead(PIN_LEFT);
        rightButton.previousState = digitalRead(PIN_RIGHT);
        selectButton.previousState = digitalRead(PIN_SELECT);
        backButton.previousState = digitalRead(PIN_ALT);
    }

    void update()
    {
        updateButton(leftButton);
        updateButton(rightButton);
        updateButton(selectButton);
        updateButton(backButton);
    }

    bool wasPressed(Button button)
    {
        ButtonState& state = getButtonState(button);

        if (!state.pressedEvent)
        {
            return false;
        }

        state.pressedEvent = false;
        return true;
    }
}