#ifndef SRC_TRAVELLER
#define SRC_TRAVELLER

#include <vector>
#include <iostream>
#include <string>
#include <algorithm>
#include <cstdint>
#include <random>
#include <fstream>
#include <sstream>
#include <memory>
#include <initializer_list>

#include "user_type.h"
#include "path.h"
//#include "io.h"
#include "time_format.h"
#include "city_graph.h"

const int kMaxInt = INT32_MAX; // 0x7fffffff
const std::string save_path = "../data/traveller_data.txt";
const std::string name_path = "../data/namelist.txt";
const int SaveLines = 8;

#ifdef TEST_GET_PATH
extern int call_counter_time;
extern int call_counter_money;
extern int depth_counter;
#endif // TEST_GET_PATH

struct DFSLeastMoneyParWarp
{
  int temp_price;
  int path_price;
  bool *isMeet;
  int current;
  int depth;
  int min_price;
};

struct DFSLeastTimeParWarp
{
  Time t;
  City_id current;
  bool *isMeet;
  int depth;
  std::vector<City_id> temp;
};

class Traveller // 旅行者
{
public:
  Traveller() = default;
  Traveller(std::string id) : id_(id){};

  // 显示旅客id
  void PrintID() const { std::cout << id_ << std::endl; }
  const std::string &get_ID() const { return id_; }
  void set_id(std::string name) { id_ = name; }

  // 打印旅客路径
  void ShowPath() const { touring_path_.Show(); }
  const Path &get_path() const { return touring_path_; }

  // 为旅客计划一条路径
  Path SchedulePath(const CityGraph &graph, const std::vector<City_id> &plan, Strategy s, Time t = Time(), Time limit = Time());
  Path SchedulePath(const CityGraph &graph, Strategy s, Time t = Time(), Time limit = Time())
  {
    return SchedulePath(graph, travelling_plan_, s, t, limit);
  }

  // 设置旅行路径
  void set_path(Path &path)
  {
    touring_path_ = path;
    position_pathnode_ = -1;
  }

  // 打印旅客计划
  void PrintPlan() const
  {
    std::for_each(travelling_plan_.begin(), travelling_plan_.end(), [](City_id city) { std::cout << city << " "; });
    std::cout << std::endl;
  }
  const std::vector<City_id> &get_plan() const { return travelling_plan_; }

  void set_plan(const std::vector<City_id> &plan) { travelling_plan_ = plan; }
  void set_plan(std::initializer_list<City_id> il) { travelling_plan_ = std::vector<City_id>(il); }

  // 保存当前旅客信息
  bool SaveData() const;

  bool LoadData(int cnt, const CityGraph &graph);

  void append_plan(City_id city) { travelling_plan_.push_back(city); }

  int get_position() const { return position_pathnode_; }

  int get_left_hour() const { return next_city_hour_left_; }

  void InitState(const CityGraph &graph);

  void UpdateState(const CityGraph &graph, Time now);
  TravellerState get_state() const { return state_; }

  void set_strategy(Strategy strategy) { strategy_ = strategy; }
  bool set_strategy(int strategy)
  {
    switch (strategy)
    {
    case 0:
      strategy_ = LEAST_MONEY;
      break;
    case 1:
      strategy_ = LEAST_TIME;
      break;
    case 2:
      strategy_ = LIMIT_TIME;
      break;
    default:
      return false;
    }
    return true;
  }

  void set_init_time(const Time &t) { init_time_ = t; }
  const Time &get_init_time() const { return init_time_; }

private:
  std::string id_ = "";                  // 旅客id
  TravellerState state_ = STAY;          // 旅客当前状态
  Strategy strategy_ = LEAST_MONEY;      // 旅行策略
  std::vector<City_id> travelling_plan_; // 旅行计划 <起点>, <中继点>... , <终点>
  Path touring_path_;                    // 旅行路径
  int next_city_hour_left_ = 0;          // 到下一个城市的剩余多少小时
  int position_pathnode_ = -2;           // 当前在第k个pathnode上, -2代表没有出行计划，-1代表有出行计划但没到出发时间
  //std::vector<PathNode>::iterator next_city_; // 路径中的下一个城市
  Time init_time_;                       // 最开始时的时间
  Path GetPathLeastMoney(const CityGraph &graph, const std::vector<City_id> &plan);
  Path GetPathLeastTime(const CityGraph &graph, const std::vector<City_id> &plan, Time now);
  Path GetPathLTM(const CityGraph &graph, const std::vector<City_id> &plan, Time now, Time limit);
  void DFSLeastTime(const CityGraph &graph, const std::vector<City_id> &plan, Path &path, Path &temp_path, DFSLeastTimeParWarp &par_warp);
  void DFSLeastMoney(const std::vector<std::vector<int>> &price_matrix, std::vector<int> &path, std::vector<int> &temp_path, DFSLeastMoneyParWarp &par_warp);
  void DFSLTM(const CityGraph &graph, const std::vector<City_id> &plan, Path &path, Path temp, int layer, int &least_money, const int limit);
};

void Traveller::DFSLeastMoney(const std::vector<std::vector<int>> &price_matrix, std::vector<int> &path, std::vector<int> &temp_path, DFSLeastMoneyParWarp &par_warp)
{
#ifdef TEST_GET_PATH
  call_counter_money++;
#endif                                             // TEST_GET_PATH
  if (par_warp.current == price_matrix.size() - 1) // 到达终点时判断递归深度(路径长度)是否符合要求
  {
    if (par_warp.depth == price_matrix.size() - 1) // 路径长度是否符合要求
    {
      path = temp_path;
      par_warp.path_price = par_warp.temp_price;
#ifdef TEST_GET_PATH
      depth_counter++;
      std::cout << par_warp.path_price << std::endl;
#endif // TEST_GET_PATH
    }
    else
      return;
  }

  std::vector<int> path_save;
  int current_save = par_warp.current;

  for (int i = 0; i != price_matrix.size(); ++i)
  {
    if (i == par_warp.current || par_warp.isMeet[i])
      continue;
    else
    {
      par_warp.isMeet[i] = true;
      path_save = temp_path;
      par_warp.temp_price += price_matrix[par_warp.current][i];
      if (par_warp.temp_price <= par_warp.path_price && (price_matrix.size() - 2 - par_warp.depth) * par_warp.min_price <= par_warp.path_price - par_warp.temp_price)
      {
        temp_path.push_back(i);
        par_warp.depth++, par_warp.current = i; // 进入更深层的递归时保存当前的状态
        DFSLeastMoney(price_matrix, path, temp_path, par_warp);
        par_warp.depth--, par_warp.current = current_save; // 还原状态
      }
      par_warp.temp_price -= price_matrix[par_warp.current][i]; // 还原状态
      par_warp.isMeet[i] = false;
      temp_path = path_save;
    }
  }
}

void Traveller::DFSLeastTime(const CityGraph &graph, const std::vector<City_id> &plan, Path &path, Path &temp_path, DFSLeastTimeParWarp &par_warp)
{
#ifdef TEST_GET_PATH
  call_counter_time++;
#endif // TEST_GET_PATH
  if (par_warp.current == plan.size() - 1)
  {
    if (par_warp.depth == plan.size() - 1)
    {
      path = temp_path;
#ifdef TEST_GET_PATH
      depth_counter++;
      std::cout << path.GetTotalTime().to_hour() << std::endl;
#endif // TEST_GET_PATH
    }
    else
      return;
  }

  int temp_current = par_warp.current;
  Time temp_time;
  Time time_save = par_warp.t;
  Path path_save;

  for (int i = 0; i < plan.size(); ++i)
  {
    if (i == par_warp.current || par_warp.isMeet[i])
      continue;
    else
    {
      par_warp.temp[0] = plan[par_warp.current], par_warp.temp[1] = plan[i];
      temp_time = init_time_;
      path_save = temp_path;
      par_warp.isMeet[i] = true;
      temp_path.Append(GetPathLeastTime(graph, par_warp.temp, par_warp.t));
      temp_path.FixTotalTime(graph, init_time_);
      if (temp_path.GetTotalTime().to_hour() < path.GetTotalTime().to_hour() && plan.size() - 1 - par_warp.depth < path.GetTotalTime().to_hour() - temp_path.GetTotalTime().to_hour())
      {
        par_warp.current = i, par_warp.depth++, par_warp.t = temp_time.add_time(temp_path.GetTotalTime());
        DFSLeastTime(graph, plan, path, temp_path, par_warp);
        par_warp.current = temp_current, par_warp.depth--, par_warp.t = time_save;
      }
      par_warp.isMeet[i] = false;
      temp_path = path_save;
    }
  }
}

Path Traveller::SchedulePath(const CityGraph &graph, const std::vector<City_id> &plan, Strategy s, Time start_time, Time limit)
{
  init_time_ = start_time;
  strategy_ = s;
  travelling_plan_ = plan;

  if (plan.size() < 2)
    throw plan.size();
  if (s == LEAST_MONEY)
  {
    std::vector<std::vector<Path>> path_matrix;
    std::vector<std::vector<int>> price_matrix;
    std::vector<int> order_of_path, temp_path;
    int min_price = kMaxInt;
    Path res;
    int path_price = GetPathLeastMoney(graph, plan).GetTotalPrice();

    std::vector<City_id> temp_plan_shuffle = plan;
    int temp_path_price;
    std::random_device rd;
    std::mt19937 g(rd());
    for (int i = 0; i < 100; i++)
    {
      shuffle(temp_plan_shuffle.begin() + 1, temp_plan_shuffle.end() - 1, g);
      temp_path_price = GetPathLeastMoney(graph, temp_plan_shuffle).GetTotalPrice();
      if (temp_path_price < path_price)
        path_price = temp_path_price;
    }

    size_t sz = plan.size();
    bool *isMeet = new bool[sz]();
    std::vector<City_id> temp_plan{0, 1};

    for (int i = 0; i != sz; ++i)
    {
      isMeet[i] = false;
      path_matrix.emplace_back();

      for (int j = 0; j != sz; ++j)
      {
        if (i != j && i != sz - 1 && j)
        {
          temp_plan[0] = plan[i], temp_plan[1] = plan[j];
          path_matrix[i].push_back(GetPathLeastMoney(graph, temp_plan));
        }
        else
        {
          path_matrix[i].emplace_back();
        }
      }
    }

    for (int i = 0; i != path_matrix.size(); ++i)
    {
      price_matrix.emplace_back();
      for (int j = 0; j != path_matrix[i].size(); ++j)
      {
        price_matrix[i].push_back(path_matrix[i][j].GetTotalPrice());
        if (price_matrix[i].back() && min_price > price_matrix[i].back())
          min_price = price_matrix[i].back();
      }
    }

    isMeet[0] = true;
    DFSLeastMoneyParWarp par_warp = {0, path_price, isMeet, 0, 0, min_price};
    order_of_path.push_back(0);
    temp_path.push_back(0);
    DFSLeastMoney(price_matrix, order_of_path, temp_path, par_warp);

    for (int i = 1; i != order_of_path.size(); ++i)
      res.Append(path_matrix[order_of_path[i - 1]][order_of_path[i]]);
    res.FixTotalTime(graph, start_time);

    delete isMeet;
    return res;
    // return GetPathLeastMoney(graph, plan);
  }
  else if (s == LEAST_TIME)
  {
    Path res = GetPathLeastTime(graph, plan, start_time);
    res.FixTotalTime(graph, start_time);
    int i = 0;
    auto sz = plan.size();

    if (plan.size() == 2)
      return res;
    else
    {
      bool *isMeet = new bool[sz]();
      Path temp_path;
      std::vector<City_id> buf;
      for (i = 1; i != sz; ++i)
        isMeet[i] = false;
      std::vector<City_id> temp{0, 1};
      DFSLeastTimeParWarp par_warp = {start_time, 0, isMeet, 0, temp};
      DFSLeastTime(graph, plan, res, temp_path, par_warp);
      delete isMeet;
    }
    return res;
  }
  else if (s == LIMIT_TIME)
  {
    Path a = GetPathLTM(graph, plan, start_time, limit);
    if (a.GetLen() == 0)
    {
      std::cout << "未找到符合要求路线" << std::endl;
    }
    return a;
  }
}

Path Traveller::GetPathLeastMoney(const CityGraph &graph, const std::vector<City_id> &plan)
{
  City_id destination; // 终点
  City_id origin;      // 起点
  Path path;
  std::vector<int> find_min_cost;
  // std::cout << plan[0] << plan[1] << std::endl;
  for (int cnt = plan.size() - 1; cnt > 0; cnt--)
  {
    destination = plan[cnt];
    origin = plan[cnt - 1];
    int cost[kCityNum];      // 记录最小花费
    int preway[kCityNum][2]; // preway[cityA][] = {CityB, transport_index_from_CityB_to_CityA}
    bool is_count[kCityNum] = {false};

    for (int j = 0; j < kCityNum; j++) //对数据进行初始化
    {
      if (j == origin)
        continue;
      for (int k = 0; k < graph.Getsize(origin, j); k++) // 将所有从origin到j的价格push到find_min_cost中
        find_min_cost.push_back(graph.GetRoute(origin, j, k).price);

      if (!find_min_cost.empty()) // 如果可以从origin到j
      {
        std::vector<int>::iterator min = min_element(find_min_cost.begin(), find_min_cost.end());
        cost[j] = *min;
        preway[j][0] = origin;
        preway[j][1] = distance(find_min_cost.begin(), min);
        find_min_cost.clear();
      }
      else // 不可达
      {
        cost[j] = kMaxInt;
        preway[j][0] = -1;
        preway[j][1] = -1;
      }
    }

    cost[origin] = kMaxInt; // 去除origin
    preway[origin][0] = -1;
    preway[origin][1] = -1;
    is_count[origin] = true;

    while (!is_count[destination])
    {
      int temp = kMaxInt;
      int city_temp = origin;

      for (int i = 0; i < kCityNum; i++) //找到最小值
      {
        if (!is_count[i] && cost[i] < temp)
        {
          temp = cost[i];
          city_temp = i;
        }
      }

      is_count[city_temp] = true;
      if (city_temp == destination)
        break;

      for (int j = 0; j < kCityNum; j++) //更新
      {
        if (is_count[j])
          continue;
        for (int k = 0; k < graph.Getsize(city_temp, j); k++)
          find_min_cost.push_back(graph.GetRoute(city_temp, j, k).price);
        if (!find_min_cost.empty())
        {
          std::vector<int>::iterator min = min_element(find_min_cost.begin(), find_min_cost.end());
          int newcost = *min + cost[city_temp];
          if (newcost < cost[j])
          {
            cost[j] = newcost;
            preway[j][0] = city_temp;
            preway[j][1] = distance(find_min_cost.begin(), min);
          }
          find_min_cost.clear();
        }
      }
    }
    for (int traceback = destination; traceback != origin; traceback = preway[traceback][0])
    {
      path.Append(graph, preway[traceback][0], traceback, preway[traceback][1]);
    }
  }
  //path.Reverse();
  //path.Show();
  return path;
}

Path Traveller::GetPathLeastTime(const CityGraph &graph, const std::vector<City_id> &plan, Time now)
{
  City_id destination; // 终点
  City_id origin;      // 起点
  Path path;
  std::vector<int> find_min_cost;
  Time temp_now;
  std::vector<int>::iterator min;
  for (int cnt = plan.size() - 1; cnt > 0; cnt--)
  {
    destination = plan[cnt];
    origin = plan[cnt - 1];
    int cost[kCityNum];      // 记录最小花费
    int preway[kCityNum][2]; // preway[cityA][] = {CityB, transport_index_from_CityB_to_CityA}
    // Time pretime[kCityNum];
    bool is_count[kCityNum] = {false};

    for (int j = 0; j < kCityNum; j++) //对数据进行初始化
    {
      if (j == origin)
        continue;
      for (int k = 0; k < graph.Getsize(origin, j); k++) // 将所有从origin到j的价格push到find_min_cost中
      {                                                  //*****和LM的区别*****
        const Route &route = graph.GetRoute(origin, j, k);
        temp_now.set_hour(now.GetHour());
        int wait_hour = route.start_time.hour_diff(temp_now); // 计算从当前时间开始需要等待的时间
        if (wait_hour < 0)                                    // 如果发车时间在now之前,想要搭乘这辆车就必须等候到其发车
          wait_hour += 24;
        int route_hour = route.end_time.hour_diff(route.start_time); // 路途上的时间
        find_min_cost.push_back(wait_hour + route_hour);
      }

      if (!find_min_cost.empty()) // 如果可以从origin到j
      {
        min = min_element(find_min_cost.begin(), find_min_cost.end());
        cost[j] = *min;
        preway[j][0] = origin;
        preway[j][1] = distance(find_min_cost.begin(), min);
        // pretime[j] = now;
        find_min_cost.clear();
      }
      else // 不可达
      {
        cost[j] = kMaxInt;
        preway[j][0] = -1;
        preway[j][1] = -1;
      }
    }

    cost[origin] = kMaxInt; // 去除origin
    preway[origin][0] = -1;
    preway[origin][1] = -1;
    is_count[origin] = true;

    while (!is_count[destination])
    {
      int temp = kMaxInt;
      int city_temp = origin;

      for (int i = 0; i < kCityNum; i++) //找到最小值
      {
        if (!is_count[i] && cost[i] < temp)
        {
          temp = cost[i];
          city_temp = i;
        }
      }

      is_count[city_temp] = true;
      if (city_temp == destination)
        break;
      //*****和LM的区别*****
      // Route temp_route = graph.GetRoute(preway[city_temp][0], city_temp, preway[city_temp][1]);
      // now.add_time(cost[city_temp]);   // 此时的时间为之前的时间加上从前一个城市到city_temp的时间(包括等候时间)
      now = graph.GetRoute(preway[city_temp][0], city_temp, preway[city_temp][1]).end_time;
      temp_now.set_hour(now.GetHour()); // 只看到达时间,不看天数
      // now = graph.GetRoute(preway[city_temp][0], city_temp, preway[city_temp][1]).end_time;

      for (int j = 0; j < kCityNum; j++) //更新
      {
        if (is_count[j])
          continue;
        for (int k = 0; k < graph.Getsize(city_temp, j); k++)
        { //*****和LM的区别*****
          const Route &route = graph.GetRoute(city_temp, j, k);
          int wait_hour = route.start_time.hour_diff(temp_now); // 计算从当前时间开始需要等待的时间
          if (wait_hour < 0)                                    // 如果发车时间在now之前,想要搭乘这辆车就必须等候到其发车
            wait_hour += 24;
          int route_hour = route.end_time.hour_diff(route.start_time); // 路途上的时间
          find_min_cost.push_back(wait_hour + route_hour);
        }

        if (!find_min_cost.empty())
        {
          min = min_element(find_min_cost.begin(), find_min_cost.end());
          int newcost = *min + cost[city_temp];
          if (newcost < cost[j])
          {
            cost[j] = newcost;
            preway[j][0] = city_temp;
            preway[j][1] = distance(find_min_cost.begin(), min);
            // pretime[j] = now;
          }
          find_min_cost.clear();
        }
      }
    }
    for (int traceback = destination; traceback != origin; traceback = preway[traceback][0])
    {
      path.Append(graph, preway[traceback][0], traceback, preway[traceback][1]); //pretime[traceback]);
    }
  }
  //path.Reverse();
  //path.Show();
  return path;
}

Path Traveller::GetPathLTM(const CityGraph &graph, const std::vector<City_id> &plan, Time start_time, Time limit_time)
{
  int limit = limit_time.hour_diff(start_time);
  Path path;
  int least_money = kMaxInt;
  Path temp;
  auto temp_plan_shuffle = plan;
  DFSLTM(graph, plan, path, temp, 0, least_money, limit);
  std::random_device rd;
  std::mt19937 g(rd());
  if (plan.size() > 3)
    for (int i = 0; i < 5; i++)
    {
      shuffle(temp_plan_shuffle.begin() + 1, temp_plan_shuffle.end() - 1, g);
      while (std::equal(plan.begin(), plan.end(), temp_plan_shuffle.begin()))
        shuffle(temp_plan_shuffle.begin() + 1, temp_plan_shuffle.end() - 1, g);
      DFSLTM(graph, temp_plan_shuffle, path, temp, 0, least_money, limit);
    }
  return path;
}

void Traveller::DFSLTM(const CityGraph &graph, const std::vector<City_id> &plan, Path &path, Path temp, int layer, int &least_money, const int limit)
{
  if (layer == plan.size() - 1)
  {
    path = temp;
    least_money = path.GetTotalPrice();
    return;
  }
  int i = plan.at(layer);
  int j = plan.at(layer + 1);
  for (int k = 0; k < graph.Getsize(i, j); k++)
  {
    temp.Append(graph, i, j, k, 1);
    temp.FixTotalTime(graph, init_time_);
    if (temp.GetTotalPrice() < least_money && temp.GetTotalTime().GetLength() <= limit && temp.GetTotalTime().GetLength() + plan.size() - layer - 1 <= limit)
      DFSLTM(graph, plan, path, temp, layer + 1, least_money, limit);
    temp.Remove(graph);
  }
}

bool Traveller::SaveData() const
{
  std::vector<std::string> lines;
  std::string temp;
  int start_index = 0;

  std::ofstream create_name_flie(name_path, std::ofstream::app); // 防止文件不存在导致的打开失败
  std::ofstream create_file(save_path, std::ofstream::app);      // 防止文件不存在导致的打开失败

  std::ifstream name_fis(name_path);
  if (name_fis)
  {
    while (getline(name_fis, temp))
      if (temp == id_)
        break;
      else
        start_index++;
  }

  std::ifstream fis(save_path);
  if (fis)
  {
    while (getline(fis, temp))
      lines.push_back(temp);

    start_index *= SaveLines;

    if (start_index < lines.size())
    {
      int i = start_index + 1;

      lines[i++] = std::to_string(state_);
      lines[i++] = std::to_string(strategy_);

      temp = "";
      for (int j = 0; j < travelling_plan_.size(); j++)
        temp += std::to_string(travelling_plan_.at(j)) + " ";
      lines[i++] = temp;

      temp = "";
      for (auto j = touring_path_.cbegin(); j != touring_path_.cend(); j++)
        temp += std::to_string((*j).former_city) + " " + std::to_string((*j).current_city) + " " + std::to_string((*j).kth_way) + " ";
      lines[i++] = temp;

      lines[i++] = std::to_string(next_city_hour_left_);
      lines[i++] = std::to_string(position_pathnode_);
      lines[i++] = std::to_string(init_time_.GetHour());
    }
    else
    {
      lines.push_back(id_);
      lines.push_back(std::to_string(state_));
      lines.push_back(std::to_string(strategy_));

      temp = "";
      for (int j = 0; j < travelling_plan_.size(); j++)
        temp += std::to_string(travelling_plan_.at(j)) + " ";
      lines.push_back(temp);

      temp = "";
      for (auto j = touring_path_.cbegin(); j != touring_path_.cend(); j++)
        temp += std::to_string((*j).former_city) + " " + std::to_string((*j).current_city) + " " + std::to_string((*j).kth_way) + " ";
      lines.push_back(temp);

      lines.push_back(std::to_string(next_city_hour_left_));
      lines.push_back(std::to_string(position_pathnode_));
      lines.push_back(std::to_string(init_time_.GetHour()));
    }
  }
  else
    return false;

  std::ofstream fos(save_path);
  if (fos)
    std::for_each(lines.begin(), lines.end(), [&fos](const std::string &line) { fos << line << std::endl; });
  else
    return false;
  return true;
}

bool Traveller::LoadData(int cnt, const CityGraph &graph)
{
  if (cnt == -1)
    return false;

  int state_temp;
  int strategy_temp;
  std::string temp;

  std::ifstream in_stream(save_path);
  if (in_stream.is_open())
  {
    for (int i = 0; i < cnt * SaveLines; i++)
      getline(in_stream, temp); // 找位置

    in_stream >> id_;           // 第一行
    in_stream >> state_temp;    // 第二行
    in_stream >> strategy_temp; // 第三行

    if (state_temp == 0)
      state_ = STAY;
    else if (state_temp == 1)
      state_ = OFF;

    if (strategy_temp == 0)
      strategy_ = LEAST_MONEY;
    else if (strategy_temp == 1)
      strategy_ = LEAST_TIME;
    else if (strategy_temp == 2)
      strategy_ = LIMIT_TIME;

    getline(in_stream, temp); // 结束前一行

    getline(in_stream, temp); // 第四行
    std::istringstream sis(temp);
    int plantemp;
    while (sis >> plantemp)
    {
      // std::cout << plantemp << " "; ///////////
      travelling_plan_.push_back(plantemp);
    }
    // std::cout << std::endl;

    getline(in_stream, temp); // 第五行
    sis.str(temp);
    sis.clear();
    City_id former_city, current_city, k;
    while (sis >> former_city)
    {
      sis >> current_city;
      sis >> k;
      //std::cout << former_city << " " << current_city << " " << k << std::endl; ////////////
      touring_path_.Append(graph, former_city, current_city, k, 1);
    }

    in_stream >> next_city_hour_left_;
    //std::cout << next_city_hour_left_ << std::endl;///////////

    in_stream >> position_pathnode_;
    //std::cout << position_pathnode_ << std::endl;///////////
    int hour;
    in_stream >> hour;
    init_time_.set_hour(hour);
    touring_path_.FixTotalTime(graph, init_time_);
    return true;
  }
  return false;
}

inline void Traveller::UpdateState(const CityGraph &graph, Time now)
{
  if (state_ == OFF)
  {
    if (next_city_hour_left_ == 1)
    {
      if (position_pathnode_ == touring_path_.GetLen() - 1)
      {
        state_ = STAY;
        position_pathnode_ = -1;
        return;
      }
      else
      {
        position_pathnode_++;
        PathNode node = touring_path_.GetNode(position_pathnode_);
        Route route = graph.GetRoute(node.former_city, node.current_city, node.kth_way);
        PathNode nodebefore = touring_path_.GetNode(position_pathnode_ - 1);
        Route preroute = graph.GetRoute(nodebefore.former_city, nodebefore.current_city, nodebefore.kth_way);
        int diff_hour = route.start_time.hour_diff(Time(1, preroute.end_time.GetHour()));
        if (diff_hour < 0)
          diff_hour += 24;
        if (!diff_hour)
        {
          next_city_hour_left_ = route.end_time.hour_diff(route.start_time);
        }
        else
        {
          state_ = STAY;
          next_city_hour_left_ = diff_hour;
        }
      }
    }
    else
      next_city_hour_left_--;
  }
  else
  {
    if (position_pathnode_ != -1)
    {
      if (next_city_hour_left_ == 1)
      {
        int i = touring_path_.GetNode(position_pathnode_).former_city;
        int j = touring_path_.GetNode(position_pathnode_).current_city;
        int k = touring_path_.GetNode(position_pathnode_).kth_way;
        state_ = OFF;
        Route route = graph.GetRoute(i, j, k);
        next_city_hour_left_ = route.end_time.hour_diff(route.start_time);
      }
      else
        next_city_hour_left_--;
    }
  }
}

inline void Traveller::InitState(const CityGraph &graph)
{
  if (position_pathnode_ == -2)
    return;
  auto path_begin = touring_path_.cbegin();
  Route route = graph.GetRoute((*path_begin).former_city, (*path_begin).current_city, (*path_begin).kth_way);
  int left_hour = route.start_time.hour_diff(init_time_);
  position_pathnode_ = 0;
  if (left_hour < 0)
    left_hour += 24;
  if (left_hour)
  {
    state_ = STAY;
    next_city_hour_left_ = left_hour;
  }
  else
  {
    state_ = OFF;
    next_city_hour_left_ = route.end_time.hour_diff(route.start_time);
  }
}
#endif // SRC_TRAVELLER