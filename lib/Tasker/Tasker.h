//Inspired by: https://github.com/jendakol/bomb-game/blob/0c25175c45a22eec3f1b15a869de26f3343957da/src/Tasker.h
#ifndef TRACKER_TASKER_H
#define TRACKER_TASKER_H

#include <functional>
#include <vector>

class Tasker {
public:
    explicit Tasker(int core);

    void once(const char *name, std::function<void(void)> op) const;

    void loop(const char *name, std::function<void(void)> op) const;

    void loopEvery(const char *name, int millis, std::function<void(void)> op) const;

    static void sleep(int millis);
    static void yield();

private:
    int core;
};

extern Tasker DefaultTasker;
extern Tasker NetworkTasker;

#endif //TRACKER_TASKER_H
