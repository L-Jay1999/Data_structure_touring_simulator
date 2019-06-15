#ifndef SRC_TRAVELLER_H
#define SRC_TRAVELLER_H

#include <vector>
#include <string>
#include <iostream>
#include <cstdint>

#include "time_format.h"
#include "user_type.h"
#include "path.h"

class CityGraph;
extern const std::string save_path;
extern const std::string name_path;

class Traveller // 旅行者
{
    struct DFSLeastMoneyParWarp;
    struct DFSLeastTimeParWarp;

public:
    Traveller() = default;
    Traveller(const std::string &id) : id_(id) {}
    Traveller(std::string &&id) noexcept : id_(std::move(id)) {}

    // 显示旅客id
    const std::string &get_ID() const noexcept { return id_; }
    void set_id(const std::string &name) { id_ = name; }

    // 打印旅客路径
    const Path &get_path() const noexcept { return touring_path_; }

    // 为旅客计划一条路径
    Path schedulePath(const CityGraph &graph, const std::vector<City_id> &plan, Strategy s, Time t = Time(), Time limit = Time());
    Path schedulePath(const CityGraph &graph, Strategy s, Time t = Time(), Time limit = Time())
    {
        return schedulePath(graph, travelling_plan_, s, t, limit);
    }

    // 设置旅行路径
    void set_path(Path &path) noexcept
    {
        touring_path_ = path;
        position_pathnode_ = -1;
    }

    // 打印旅客计划
    void printPlan() const;

    const std::vector<City_id> &get_plan() const { return travelling_plan_; }

    void set_plan(std::vector<City_id> &&plan) noexcept { travelling_plan_ = std::move(plan); }
    void set_plan(const std::vector<City_id> &plan) { travelling_plan_ = plan; }
    void set_plan(const std::initializer_list<City_id> &il) { travelling_plan_ = std::vector<City_id>(il); }

    // 保存当前旅客信息
    bool saveData() const;
    bool loadData(int cnt, const CityGraph &graph);

    void append_plan(City_id city) { travelling_plan_.push_back(city); }

    int get_position() const noexcept { return position_pathnode_; }

    int get_left_hour() const noexcept { return next_city_hour_left_; }
    int get_off_hours(const CityGraph &graph, int cnt);
    int get_stay_hours(const CityGraph &graph, int cnt);

    void initState(const CityGraph &graph);

    void updateState(const CityGraph &graph);
    TravellerState get_state() const { return state_; }

    void set_strategy(Strategy strategy) noexcept { strategy_ = strategy; }
    void set_strategy(int strategy) noexcept { strategy_ = static_cast<Strategy>(strategy); }
    Strategy get_strategy() const noexcept { return strategy_;}

    void set_init_time(const Time &t) noexcept { init_time_ = t; }
    const Time &get_init_time() const noexcept { return init_time_; }

private:
    std::string id_;                       // 旅客id
    TravellerState state_ = STAY;          // 旅客当前状态 *
    Strategy strategy_ = LEAST_MONEY;      // 旅行策略
    std::vector<City_id> travelling_plan_; // 旅行计划 <起点>, <中继点>... , <终点>
    Path touring_path_;                    // 旅行路径
    int next_city_hour_left_ = 0;          // 到下一个城市的剩余多少小时 *
    int position_pathnode_ = -2;           // 当前在第k个pathnode上, -2代表没有出行计划，-1代表有出行计划但没到出发时间 *
    Time init_time_; // 最开始时的时间 *

    //std::vector<PathNode>::iterator next_city_; // 路径中的下一个城市 *
    Path GetPathLeastMoney(const CityGraph &graph, const std::vector<City_id> &plan);
    Path GetPathLeastTime(const CityGraph &graph, const std::vector<City_id> &plan, Time now);
    Path GetPathLTM(const CityGraph &graph, const std::vector<City_id> &plan, Time now, Time limit);
    void DFSLeastTime(const CityGraph &graph, const std::vector<City_id> &plan, Path &path, Path &temp_path, DFSLeastTimeParWarp &par_warp);
    void DFSLeastMoney(const std::vector<std::vector<int>> &price_matrix, std::vector<int> &path, std::vector<int> &temp_path, DFSLeastMoneyParWarp &par_warp);
    void DFSLTM(const CityGraph &graph, const std::vector<City_id> &plan, Path &path, Path temp, int layer, int &least_money, const int limit);
};

#endif // SRC_TRAVELLER_H
