#ifndef SRC_CG_H
#define SRC_CG_H

#pragma execution_character_set("utf-8")

#include <vector>

#include "time_format.h"
#include "user_type.h"

const int kCityNum = 31;

struct Route
{
  Trans_id transport_type;
  Train_id train_seat_type;
  Time start_time;
  Time end_time;
  int price;
};

class CityGraph // 城市图
{
public:
  //构造函数
  CityGraph();
  int get_city_num() const { return kCityNum; }

  //打印ijk的存储数据
  void show(City_id former_city, City_id current_city, int k) const;

  //获得存储在ijk的Route
  const Route &get_route(City_id former_city, City_id current_city, int k) const { return city_[former_city][current_city].at(k); }

  //获得city[i][j]的大小
  int get_size(City_id i, City_id j) const noexcept { return city_[i][j].size(); }

#ifdef TEST_CG

  void print()
  {
    for (int i = 0; i < 5; i++)
      for (int j = 0; j < 5; j++)
      {
        if (i != j)
        {
          for (int k = 0; k < city_[i][j].size(); k++)
            Show(i, j, k);
        }
      }
  }
#endif // TEST_CG

private:
  bool LoadCityGraph(const std::string &name, int type); // 加载文件
  std::vector<Route> city_[kCityNum][kCityNum];
};

#endif //SRC_CG_H
