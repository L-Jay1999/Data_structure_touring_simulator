#ifndef SRC_TIME_H
#define SRC_TIME_H

#include <string>
/*  class Time
 *  用于表示时间与处理时间
*/
class Time
{
public:
  // 接受一个形如dhh00的整数,将d作为日期,hh作为小时储存
	explicit Time(int time = 10000) : Time(time / 10000, (time - time / 10000 * 10000) / 100) {}
	Time(int day, int hour) : day_(day - 1), hour_(hour) {}
	Time(const Time &other) : day_(other.day_), hour_(other.hour_) {};
	Time &operator=(const Time &rhs)
	{
		day_ = rhs.day_;
		hour_ = rhs.hour_;
		return *this;
	}

	// 获取当前时间与给定时间的差值,返回值为差值时间的拷贝(this - t)
    Time time_diff(const Time &t) const noexcept;

	// 将当前时间与给定时间相加,返回相加后时间的引用(改变了使用此方法的对象)
    Time &add_time(const Time &t) noexcept;
    Time &add_time(const int hour, const int day = 0) noexcept;

	// 将当前时间与给定时间相减,返回相减后时间的引用(改变了使用此方法的对象)
    Time &minus_time(const Time &t) noexcept;
    Time &minus_time(const int hour, const int day = 0) noexcept;

	// 获取当前时间与给定时间的差值,返回值为两者相差的小时数(this - t)
    int hour_diff(const Time &) const noexcept;

	// 返回当前时间对应的小时数
    int to_hour() const noexcept { return day_ * 24 + hour_; }

    int get_day() const noexcept { return day_; }
    int get_hour() const noexcept { return hour_; }
    void reset() noexcept { hour_ = 0, day_ = 0; }               //重置
    int get_length() const noexcept { return hour_ + day_ * 24; } //获得总时长
    void set_hour(int hour) noexcept { hour_ = hour; }

#ifdef TEST_TIME
#include <iostream>
	std::ostream &print(std::ostream &os = std::cout) const
	{
        os << "日期是: " << get_day()
            << "\t时间是: " << get_hour();
		return os;
}
#endif // TEST_TIME

private:
	int day_ = 0; // day_为0表示当天
	int hour_ = 0;
};

std::string RouteShow(const Time &leave, const Time &arrive);

#endif // SRC_TIME_H
