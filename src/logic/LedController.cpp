#include "LedController.h"

LedController::LedController(StateManager &stateManager) {
    pinMode(RED_LED_PIN, OUTPUT);
    pinMode(GREEN_LED_PIN, OUTPUT);
    this->stateManager = &stateManager;
    registerBlinkingRoutine();
}

void LedController::blinkHelper(Color color, int cnt, int duration) {
    while (this->blinking) {
        Tasker::yield();
    }
    this->blinking = true;
    int red = LOW, green = LOW;
    switch (color) {
        case RED:
            red = HIGH;
            break;
        case GREEN:
            green = HIGH;
            break;
        case ORANGE:
            red = HIGH;
            green = HIGH;
            break;
        default:
            return;
    }
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    digitalWrite(RED_LED_PIN, red);
    digitalWrite(GREEN_LED_PIN, green);
    Tasker::sleep(duration / 2);
    digitalWrite(RED_LED_PIN, LOW);
    digitalWrite(GREEN_LED_PIN, LOW);
    Tasker::sleep(duration / 2);
    this->blinking = false;
}

void LedController::registerBlinkingRoutine() {
    DefaultTasker.loopEvery("blink", 250, [this] {
        if (stateManager->isError()) {
            blinkHelper(RED, 1, 500);
            return;
        }
        if (stateManager->isReconnecting()) {
            blinkHelper(ORANGE, 1, 500);
        }
        if (stateManager->isGpsReconnecting()) {
            blinkHelper(ORANGE, 1, 1500);
        }
    });
}

void LedController::blink(Color color, int cnt) {
    int red = LOW, green = LOW;
    switch (color) {
        case RED:
            red = HIGH;
            break;
        case GREEN:
            green = HIGH;
            break;
        case ORANGE:
            red = HIGH;
            green = HIGH;
            break;
        default:
            return;
    }

    DefaultTasker.once("blink", [red, green, cnt] {
        for (int i = 0; i < cnt; i++) {
            digitalWrite(RED_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, LOW);
            digitalWrite(RED_LED_PIN, red);
            digitalWrite(GREEN_LED_PIN, green);
            Tasker::sleep(250);
            digitalWrite(RED_LED_PIN, LOW);
            digitalWrite(GREEN_LED_PIN, LOW);
            Tasker::sleep(250);
        }
    });
}
