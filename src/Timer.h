#pragma once

/***********************************
 * 
 * Do all time relevant processing in logicmodule
 * 
 * *********************************/

#include <stdint.h>
#include <math.h>
#include <ctime>

#define SUN_SUNRISE 0x00
#define SUN_SUNSET 0x01

#define REMOVED 0
#define EASTER -1
#define ADVENT -2

struct sTime
{
    uint8_t minute;
    uint8_t hour;
};

struct sDay
{
    int8_t day;
    int8_t month;
};

enum eTimeValid
{
    tmInvalid,
    tmMinutesValid,
    tmDateValid,
    tmValid
};

class Timer
{
  private:
    sDay cHolidays[29] = {{1, 1}, {6, 1}, {-52, EASTER}, {-48, EASTER}, {-47, EASTER}, {-46, EASTER}, {-3, EASTER}, {-2, EASTER}, {0, EASTER}, {1, EASTER}, {1, 5}, {39, EASTER}, {49, EASTER}, {50, EASTER}, {60, EASTER}, {8, 8}, {15, 8}, {3, 10}, {31, 10}, {1, 11}, {-32, ADVENT}, {-21, ADVENT}, {-14, ADVENT}, {-7, ADVENT}, {0, ADVENT}, {24, 12}, {25, 12}, {26, 12}, {31, 12}};
    struct tm mDateTime;
    struct tm mTimeHelper;
    eTimeValid mTimeValid = tmInvalid;
    uint32_t mTimeDelay = 0;
    bool mMinuteChanged = false;
    bool mIsHolidayToday = false;
    bool mIsHolidayTomorrow = false;
    sTime mSunrise;
    sTime mSunset;
    sDay mEaster = {0, 0}; // easter sunday
    sDay mAdvent = {0, 0}; // fourth advent
    int8_t mMinuteTick = -1;  // timer evaluation is called each time the minute changes
    int8_t mDayTick = -1;     // sunrise/sunset calculation happens each time the day changes
    int16_t mEasterTick = -1; // easter calculation happens each time year changes

    void calculateEaster();
    void calculateAdvent();
    void calculateHolidays();
    void calculateSunriseSunset();
    bool isEqualDate(sDay &iDate1, sDay &iDate2);
    sDay getDayByOffset(int8_t iOffset, sDay &iDate);

    Timer();
    ~Timer();
    Timer(const Timer&);    // make copy constructor private
    Timer &operator=(const Timer&); // prevent copy

  public:
    // singleton!
    static Timer &instance();

    void setup(uint32_t iHolidayBitmask);
    void loop();

    
    uint8_t getDay();
    uint8_t getMonth();
    uint16_t getYear();
    uint8_t getHour();
    uint8_t getMinute();
    uint8_t getSecond();
    uint8_t getWeekday();
    sTime *getSunInfo(uint8_t iSunInfo);
    sDay *getEaster();
    char *getTimeAsc();
    bool minuteChanged(); // true every minute
    void clearMinuteChanged(); //has to be cleared externally
    void setTimeFromBus(tm *iTime);
    void setDateFromBus(tm *iDate);
};

/* A macro to compute the number of days elapsed since 2000 Jan 0.0 */
/* (which is equal to 1999 Dec 31, 0h UT)                           */

#define days_since_2000_Jan_0(y, m, d) \
    (367L * (y) - ((7 * ((y) + (((m) + 9) / 12))) / 4) + ((275 * (m)) / 9) + (d)-730530L)

/* Some conversion factors between radians and degrees */

#ifndef PI
#define PI 3.1415926535897932384
#endif

#define RADEG (180.0 / PI)
#define DEGRAD (PI / 180.0)

/* The trigonometric functions in degrees */

#define sind(x) sin((x)*DEGRAD)
#define cosd(x) cos((x)*DEGRAD)
#define tand(x) tan((x)*DEGRAD)

#define atand(x) (RADEG * atan(x))
#define asind(x) (RADEG * asin(x))
#define acosd(x) (RADEG * acos(x))
#define atan2d(y, x) (RADEG * atan2(y, x))

/* Following are some macros around the "workhorse" function __daylen__ */
/* They mainly fill in the desired values for the reference altitude    */
/* below the horizon, and also selects whether this altitude should     */
/* refer to the Sun's center or its upper limb.                         */

/* This macro computes the length of the day, from sunrise to sunset. */
/* Sunrise/set is considered to occur when the Sun's upper limb is    */
/* 35 arc minutes below the horizon (this accounts for the refraction */
/* of the Earth's atmosphere).                                        */
#define day_length(year, month, day, lon, lat) \
    __daylen__(year, month, day, lon, lat, -35.0 / 60.0, 1)

/* This macro computes the length of the day, including civil twilight. */
/* Civil twilight starts/ends when the Sun's center is 6 degrees below  */
/* the horizon.                                                         */
#define day_civil_twilight_length(year, month, day, lon, lat) \
    __daylen__(year, month, day, lon, lat, -6.0, 0)

/* This macro computes the length of the day, incl. nautical twilight.  */
/* Nautical twilight starts/ends when the Sun's center is 12 degrees    */
/* below the horizon.                                                   */
#define day_nautical_twilight_length(year, month, day, lon, lat) \
    __daylen__(year, month, day, lon, lat, -12.0, 0)

/* This macro computes the length of the day, incl. astronomical twilight. */
/* Astronomical twilight starts/ends when the Sun's center is 18 degrees   */
/* below the horizon.                                                      */
#define day_astronomical_twilight_length(year, month, day, lon, lat) \
    __daylen__(year, month, day, lon, lat, -18.0, 0)

/* This macro computes times for sunrise/sunset.                      */
/* Sunrise/set is considered to occur when the Sun's upper limb is    */
/* 35 arc minutes below the horizon (this accounts for the refraction */
/* of the Earth's atmosphere).                                        */
#define sun_rise_set(year, month, day, lon, lat, rise, set) \
    __sunriset__(year, month, day, lon, lat, -35.0 / 60.0, 1, rise, set)

/* This macro computes the start and end times of civil twilight.       */
/* Civil twilight starts/ends when the Sun's center is 6 degrees below  */
/* the horizon.                                                         */
#define civil_twilight(year, month, day, lon, lat, start, end) \
    __sunriset__(year, month, day, lon, lat, -6.0, 0, start, end)

/* This macro computes the start and end times of nautical twilight.    */
/* Nautical twilight starts/ends when the Sun's center is 12 degrees    */
/* below the horizon.                                                   */
#define nautical_twilight(year, month, day, lon, lat, start, end) \
    __sunriset__(year, month, day, lon, lat, -12.0, 0, start, end)

/* This macro computes the start and end times of astronomical twilight.   */
/* Astronomical twilight starts/ends when the Sun's center is 18 degrees   */
/* below the horizon.                                                      */
#define astronomical_twilight(year, month, day, lon, lat, start, end) \
    __sunriset__(year, month, day, lon, lat, -18.0, 0, start, end)

/* Function prototypes */

double __daylen__(int year, int month, int day, double lon, double lat,
                  double altit, int upper_limb);

int __sunriset__(int year, int month, int day, double lon, double lat,
                 double altit, int upper_limb, double *rise, double *set);

void sunpos(double d, double *lon, double *r);

void sun_RA_dec(double d, double *RA, double *dec, double *r);

double revolution(double x);

double rev180(double x);

double GMST0(double d);
