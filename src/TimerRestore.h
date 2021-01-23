#pragma once

/***********************************
 * 
 * This timer is just use for timer restore function during startup
 * 
 * *********************************/

#include "Timer.h"

class TimerRestore : public Timer
{
  private:
    TimerRestore();
    ~TimerRestore();
    TimerRestore(const TimerRestore&);    // make copy constructor private
    TimerRestore &operator=(const TimerRestore&); // prevent copy
    void doDayCalculations();

  public:
    // singleton!
    static TimerRestore &instance();
    void setup(Timer &iTimer);
    void decreaseDay();
};

