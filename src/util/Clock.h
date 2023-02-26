//
// Copyright (c) 2021 Julian Hinxlage. All rights reserved.
//

#pragma once
#include <stdint.h>

class Clock {
public:
    Clock();
    void reset();
    double elapsed();
    double round();
    static double now();
    static void sleep(double seconds);
private:
    uint64_t startTime;
};
