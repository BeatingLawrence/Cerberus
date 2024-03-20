#ifndef CERBERUS_TIME_TIMEFRAME_H
#define CERBERUS_TIME_TIMEFRAME_H

/*  This is the TimeFrame class.
 *
 *  This class represents a time duration
 *  or a time offset and has a resolution of 1us
 *
 *  The overlap is at circa 584942,4 years
 *
 */

#include <cstdint>

#include "../Cerberus_global.h"

namespace cerberus
{

    struct CERBERUS_EXPORT SplittedTime
    {
        uint64_t seconds;
        uint32_t nanoseconds;
    };

    class CERBERUS_EXPORT TimeFrame
    {
       private:
        uint64_t m_us;
        bool m_negative;

        void add(uint64_t val);
        void subtract(uint64_t val);

       public:
        enum Unit
        {
            U_MicroSecond,
            U_MilliSecond,
            U_Second,
            U_Minute,
            U_Hour,
            U_Day,
            U_Month,
            U_Year,
        };

        // Construct an invalid TimeFrame object
        TimeFrame();

        // Construct a TimeFrame object
        TimeFrame(uint64_t count, Unit unit = U_MilliSecond);

        // Return true if the instance is valid
        bool isValid() const;

        // Return true if the time value is negative
        bool isNegative() const;

        void setNegative(bool neg = true);

        /*  Return the ABS partial integer time.
         *  e.g. Calling seconds() on an instance
         *  of 2 hours will return 0 because the partial
         *  seconds value is actually zero
         */
        uint64_t microseconds() const;
        uint64_t milliseconds() const;
        uint64_t seconds() const;
        uint64_t minutes() const;
        uint64_t hours() const;
        uint64_t days() const;
        uint64_t months() const;
        uint64_t years() const;

        /*  Return the ABS converted time value.
         *  e.g. Calling seconds() on an instance
         *  of 2 hours will return 7200 because there are
         *  3600 seconds in an hour so a conversion is performed
         */
        uint64_t toMicroseconds() const;
        uint64_t toMilliseconds() const;
        uint64_t toSeconds() const;
        uint64_t toMinutes() const;
        uint64_t toHours() const;
        uint64_t toDays() const;
        uint64_t toMonths() const;

        // Return the ABS time as a SplittedTime object which contains
        // the seconds part and the REMAINING time as nanoseconds
        SplittedTime splittedTime() const;

        // Sum a time value
        TimeFrame& addMicroseconds(int64_t time);

        // Set the time value
        TimeFrame& setMicroseconds(uint64_t time);
        // continue ...

        // Add an amount of time to this instance of TimeFrame
        // If the other instance is negative, calling
        // this method is the same as subtract()
        TimeFrame& add(const TimeFrame& other);

        // Subtract an amount of time from this instance of TimeFrame
        // If the other instance is negative, calling
        // this method is the same as add()
        TimeFrame& subtract(const TimeFrame& other);

        // Aliases of add()
        TimeFrame operator+(const TimeFrame& other);
        void operator+=(const TimeFrame& other);

        // Aliases of subtract()
        TimeFrame operator-(const TimeFrame& other);
        void operator-=(const TimeFrame& other);

        // Comparison
        bool operator<(const TimeFrame& other) const;
        bool operator>(const TimeFrame& other) const;
        bool operator<=(const TimeFrame& other) const;
        bool operator>=(const TimeFrame& other) const;
        bool operator==(const TimeFrame& other) const;
        bool operator!=(const TimeFrame& other) const;
    };
}  // namespace cerberus

#endif  // CERBERUS_TIME_TIMEFRAME_H
