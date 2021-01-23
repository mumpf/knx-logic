#include "TimerRestore.h"
#include "Arduino.h"
#include "Helper.h"

TimerRestore::TimerRestore()
{
    mNow.tm_year = 120;
    mNow.tm_mon = 0;
    mNow.tm_mday = 1;
    mNow.tm_wday = 3;
    mktime(&mNow);
    mTimeDelay = millis();
}

TimerRestore::~TimerRestore()
{
}

TimerRestore &TimerRestore::instance() {
    static TimerRestore sInstance;
    return sInstance;
}

void TimerRestore::setup(Timer &iTimer) {
    mNow = iTimer.mNow;
    doDayCalculations();
}

void TimerRestore::decreaseDay() {
    mNow.tm_mday -= 1;
    doDayCalculations();
}

void TimerRestore::doDayCalculations() {
    mktime(&mNow);
    calculateSummertime(); // initial summertime calculation if year changes
    calculateSunriseSunset();
    calculateHolidays();
    calculateEaster();
    calculateAdvent();
}
