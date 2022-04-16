#ifndef CERBERUS_TIME_TIME_H
#define CERBERUS_TIME_TIME_H

/*  This class represent a time duration.
 *
 *  Duration can be expressed in microseconds,
 *  milliseconds, seconds, minutes, hours
 *
 *  Overflow is at 584942,4 years
 */

#include <memory>
#include "Cerberus_global.h"

namespace cerberus
{
    namespace time
    {
        typedef std::shared_ptr<class Time> cerberus_time;

        class CERBERUS_EXPORT Time
        {
            private:
                uint64_t m_period_uS;

            public:
                enum Unit
                {
                    U_MicroSecond,
                    U_MilliSecond,
                    U_Second,
                    U_Minute,
                    U_Hour,
                };

                static cerberus_time create(uint64_t count, Unit unit = U_MilliSecond);

                //Constructs an invalid Time object
                Time();

                //Constructs a Time object
                Time(uint64_t count, Unit unit = U_MilliSecond);

                //Returns true if time is != 0
                bool isValid() const;

                //Returns the integer microseconds value
                uint64_t getMicroseconds() const;

                //Returns the integer milliseonds value
                uint64_t getMilliseconds() const;

                //Returns the integer seconds value
                uint64_t getSeconds() const;

                //Returns the integer minutes value
                uint64_t getMinutes() const;

                //Returns the integer hours value
                uint64_t getHours() const;
        };
    }
}

#endif // CERBERUS_TIME_TIME_H
