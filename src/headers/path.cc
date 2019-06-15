#ifndef SRC_PATH_CC
#define SRC_PATH_CC

#pragma execution_character_set("utf-8")

#include <cassert>

#include <iostream>
#include <deque>
#include <algorithm>

#include "path.h"
#include "time_format.h"
#include "city_graph.h"

void Path::remove(const CityGraph &graph)
{
    if (len_ == 1)
    {
        cities_.pop_back();
        start_city_ = 0;
        end_city_ = 0;
        len_ = 0;
        total_price_ = 0;
        total_timecost_.reset();
    }

    if (!len_)
        return;
    else
    {
        len_--;

        end_city_ = cities_.at(len_ - 1).current_city;
        int i = cities_.at(len_).former_city;
        int j = cities_.at(len_).current_city;
        int k = cities_.at(len_).kth_way;

        total_price_ -= graph.get_route(i, j, k).price;
        int fi = cities_.at(len_ - 1).former_city;
        int fj = cities_.at(len_ - 1).current_city;
        int fk = cities_.at(len_ - 1).kth_way;

        Time temp = graph.get_route(i, j, k).end_time.time_diff(graph.get_route(fi, fj, fk).end_time);
        total_timecost_.minus_time(temp);
        cities_.pop_back();
    }
}

void Path::append(const CityGraph &graph, City_id i, City_id j, int k, int back)
{
    PathNode temp = {i, j, k};
    if (!(len_++))
        end_city_ = j;
    start_city_ = i;

    if (back)
        cities_.push_back(temp);
    else
        cities_.push_front(temp);

    total_price_ += graph.get_route(i, j, k).price;
    // total_timecost_.add_time(graph.get_route(i, j, k).end_time.time_diff(graph.get_route(i, j, k).start_time));  // 只计算路途上的时间,不计等候时间
}

void Path::append(const CityGraph &graph, City_id i, City_id j, int k, Time total_time)
{
    PathNode temp = {i, j, k};
    if (!(len_++))
        end_city_ = j;
    start_city_ = i;

    cities_.push_front(temp);
    total_price_ += graph.get_route(i, j, k).price;
}

Path &Path::append(const Path &path)
{
    len_ += path.len_;
    end_city_ = path.end_city_;
    total_price_ += path.total_price_;
    for (const auto &node : path.cities_)
        cities_.push_back(node);
    return *this;
}

void Path::fixTotalTime(const CityGraph &graph, const Time &start_time)
{
    Time now = start_time;
    total_timecost_ = Time();
    for (const auto &node : cities_)
    {
        Time temp_now(1, now.get_hour());
        Route r = graph.get_route(node.former_city, node.current_city, node.kth_way);
        int wait_hour = r.start_time.hour_diff(temp_now);
        int route_hour = r.end_time.hour_diff(r.start_time);
        wait_hour += ((wait_hour < 0) ? 24 : 0);
        total_timecost_.add_time(wait_hour + route_hour);
        now.add_time(wait_hour + route_hour);
    }
}

void Path::show() const
{
    for (auto path_node : cities_)
        std::cout << path_node.former_city << '\t'
                  << path_node.current_city << '\t'
                  << path_node.kth_way << std::endl;
}

#ifdef TEST_PATH

bool Path::ValidatePath() const // 验证path是否合法
{
  assert(cities_.size() == len_ && len_ > 0);
  City_id temp = cities_.at(0).current_city;
  for (int i = 1; i < len_; ++i)
    if (cities_.at(i).former_city != temp)
      return false;
  return true;
}
#endif // TEST_PATH

#endif // SRC_PATH_CC
