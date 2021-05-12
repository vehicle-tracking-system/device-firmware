#ifndef TRACKER_LEDCONTROLLER_H
#define TRACKER_LEDCONTROLLER_H

#include "Constants.h"
#include "../../lib/Tasker/Tasker.h"
#include "StateManager.h"

class LedController {
public:
    explicit LedController(StateManager &stateManager);

    enum Color {
        RED,
        GREEN,
        ORANGE
    };

    /**
     * Blink (asynchronously) with specific color.
     * @param color blink color
     * @param cnt number of blinks
     */
    static void blink(Color color, int cnt = 1);

private:
    void registerBlinkingRoutine();

    void blinkHelper(LedController::Color color, int cnt, int duration);

    StateManager *stateManager;
    bool blinking = false;
};


#endif //TRACKER_LEDCONTROLLER_H
