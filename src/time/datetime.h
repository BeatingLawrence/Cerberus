#ifndef CERBERUS_TIME_DATETIME_H
#define CERBERUS_TIME_DATETIME_H

/*  This is the DateTime class
 *
 *  This class represent a point in time and has
 *  a resolution of 1 us.
 *
 *  The DateTime class stores the low presision part of
 *  the date&time in a tm struct, and keeps an offset expressed
 *  in microseconds as a TimeFrame object. This value is
 *  capped inside [0-1000000] interval.
 */
#include <time.h>

#include <cstdint>
#include <string>

#include "../Cerberus_global.h"
#include "timeframe.h"

namespace crb
{

    class CERBERUS_EXPORT DateTime
    {
       private:
        mutable tm m_time;
        mutable TimeFrame m_offset;
        char m_zone[26];

        time_t normalize() const;

       public:
        // Construct an invalid DateTime object
        DateTime();

        // Construct a DateTime object from POSIX timespec data
        DateTime(uint32_t seconds, uint32_t nanoseconds);

        // Check if the instance is valid
        bool isValid() const;

        // Manage DST activation
        bool usingDst() const;

        // Return the partial time component.
        uint32_t microseconds() const;
        uint32_t milliseconds() const;
        uint32_t seconds() const;
        uint32_t minutes() const;
        uint32_t hours() const;
        uint32_t days() const;
        uint32_t months() const;
        uint32_t years() const;

        /* Increment or decrement a component of the Time instance.
         * The object is normalized after the operation, e.g. if seconds
         * equal 50 before an increment of 20, they equal 10, and minutes
         * are incremented afterwards.
         * These methods return a reference to this instance so they can be
         * put in a chain
         */
        DateTime& addMicroseconds(int x);
        DateTime& addMilliseconds(int x);
        DateTime& addSeconds(int x);
        DateTime& addMinutes(int x);
        DateTime& addHours(int x);
        DateTime& addDays(int x);
        DateTime& addMonths(int x);
        DateTime& addYears(int x);

        /* Set a component of the DateTime instance.
         * The object is normalized after the operation.
         * These methods return a reference to this instance so they can be
         * put in a chain
         */
        DateTime& setMicroseconds(uint64_t x);
        DateTime& setMilliseconds(uint64_t x);
        DateTime& setSeconds(uint64_t x);
        DateTime& setMinutes(uint32_t x);
        DateTime& setHours(uint32_t x);
        DateTime& setDays(uint32_t x);
        DateTime& setMonths(uint32_t x);
        DateTime& setYears(uint32_t x);

        // add an amount of time
        DateTime& add(const TimeFrame& time);

        // subtract an amount of time
        DateTime& subtract(const TimeFrame& time);

        // An alias of add()
        DateTime& operator+(const TimeFrame& other);

        // An alias of subtract()
        DateTime& operator-(const TimeFrame& other);

        // Comparison
        bool operator<(const DateTime& other) const;
        bool operator<=(const DateTime& other) const;
        bool operator>(const DateTime& other) const;
        bool operator>=(const DateTime& other) const;

        bool isOlder(const DateTime& other) const;
        bool isNewer(const DateTime& other) const;

        // Return the time value as a string
        std::string toString() const;

        // Return the time value as a string for timestamping
        std::string toTimeStampString() const;

        // Return the epoch timestamp in milliseconds
        uint64_t toEpochMilliseconds() const;

        // Set the time according to the seconds and nanoseconds (timespec struct).
        // Please note that nanosecond precision is lost
        DateTime& fromTimespec(uint32_t seconds, uint32_t nanoseconds = 0);

        // Return the current system time
        static DateTime current();
    };
}  // namespace crb

#endif  // CERBERUS_TIME_DATETIME_H
