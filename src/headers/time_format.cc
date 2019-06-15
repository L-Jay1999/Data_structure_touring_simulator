#ifndef SRC_TIME_CC
#define SRC_TIME_CC

#include <iostream>
#include <string>

#include "time_format.h"

Time &Time::add_time(const Time &t) noexcept
{
	return add_time(t.hour_, t.day_);
}

int Time::hour_diff(const Time &t) const noexcept
{
	int temp_hour = hour_ - t.hour_, temp_day = day_ - t.day_;
	if (temp_hour < 0)
	{
		--temp_day;
		temp_hour += 24;
	}
	return temp_hour + temp_day * 24;
}

Time &Time::minus_time(const Time &t) noexcept
{
	return add_time(-t.hour_, -t.day_);
}

Time &Time::minus_time(const int hour, const int day) noexcept
{
	return add_time(-hour, -day);
}

std::string RouteShow(const Time &leave, const Time &arrive)
{
    std::string temp;
    temp += std::to_string(leave.get_hour());
    temp += ":00\t\t";
    temp += std::to_string(arrive.get_hour());
    temp += ":00";
    /*std::cout << leave.get_hour() << ":00" << "\t\t"
            << arrive.get_hour() << ":00";*/
    if (arrive.get_day())
    {
        temp += "+" + std::to_string(arrive.get_day()) + "\t\t";
        //std::cout << "+" << arrive.get_day() << "\t\t";
    }

	else
	{
		temp += "\t\t";
		//std::cout << "\t\t";
	}
	return temp;
}

Time &Time::add_time(const int hour, const int day /* = 0 */) noexcept
{
	day_ += day;
	hour_ += hour;
	int d = hour_ / 24;
	if (d)
	{
		hour_ -= 24 * d;
		day_ += d;
	}
	return *this;
}

Time Time::time_diff(const Time &t) const noexcept
{
	int temp_hour = hour_ - t.hour_, temp_day = day_ - t.day_;
	if (temp_hour < 0)
	{
		--temp_day;
		temp_hour += 24;
	}
	Time res(temp_day + 1, temp_hour);
	return res;
}
#endif //SRC_TIME_CC
