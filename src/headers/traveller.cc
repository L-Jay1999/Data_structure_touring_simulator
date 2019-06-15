#ifndef SRC_TRAVELLER_CC
#define SRC_TRAVELLER_CC

#pragma execution_character_set("utf-8")

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
#include <queue>
#include <utility>
#include <thread>
#include <atomic>

#include "traveller.h"
#include "user_type.h"
#include "path.h"
#include "time_format.h"
#include "city_graph.h"
#include "log.h"

#ifdef TEST_GET_PATH
extern int call_counter_time;
extern int call_counter_money;
extern int depth_counter;
#endif // TEST_GET_PATH

static constexpr int SaveLines = 8;
const std::string save_path = "../data/traveller_data.txt";
const std::string name_path = "../data/namelist.txt";
static constexpr int kMaxInt = INT32_MAX; // 0x7fffffff

struct Traveller::DFSLeastMoneyParWarp
{
	int temp_price;
	int *path_price;
	int *t_price;
	bool *isMeet;
	int current;
	int depth;
	int min_price;
};

struct Traveller::DFSLeastTimeParWarp
{
	Time t;
	City_id current;
	int *total_hour;
	bool *isMeet;
	int depth;
	std::vector<City_id> *temp;
};

void Traveller::DFSLeastMoney(const std::vector<std::vector<int>> &price_matrix, std::vector<int> &path, std::vector<int> &temp_path, DFSLeastMoneyParWarp &par_warp)
{
#ifdef TEST_GET_PATH
	call_counter_money++;
#endif												 // TEST_GET_PATH
	if (par_warp.current == price_matrix.size() - 1) // 到达终点时判断递归深度(路径长度)是否符合要求
	{
		if (par_warp.depth == price_matrix.size() - 1) // 路径长度是否符合要求
		{
			path = temp_path;
			*par_warp.path_price = par_warp.temp_price;
			*par_warp.t_price = par_warp.temp_price;
			return;
#ifdef TEST_GET_PATH
			depth_counter++;
			std::cout << par_warp.path_price << std::endl;
#endif // TEST_GET_PATH
		}
		else
			return;
	}

	int current_save = par_warp.current;

	for (int i = 0; i != price_matrix.size(); ++i)
	{
		if (i == par_warp.current || par_warp.isMeet[i])
			continue;
		else
		{
			par_warp.isMeet[i] = true;
			par_warp.temp_price += price_matrix[par_warp.current][i];
			if (par_warp.temp_price <= *par_warp.path_price && (price_matrix.size() - 2 - par_warp.depth) * par_warp.min_price <= *par_warp.path_price - par_warp.temp_price)
			{
				temp_path.push_back(i);
				par_warp.depth++, par_warp.current = i; // 进入更深层的递归时保存当前的状态
				DFSLeastMoney(price_matrix, path, temp_path, par_warp);
				par_warp.depth--, par_warp.current = current_save; // 还原状态
				temp_path.pop_back();
			}
			par_warp.temp_price -= price_matrix[par_warp.current][i]; // 还原状态
			par_warp.isMeet[i] = false;
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
            *par_warp.total_hour = path.get_total_time().to_hour();
			return;
#ifdef TEST_GET_PATH
			depth_counter++;
            std::cout << path.get_total_time().to_hour() << std::endl;
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
			(*par_warp.temp)[0] = plan[par_warp.current], (*par_warp.temp)[1] = plan[i];
			temp_time = init_time_;
			path_save = temp_path;
			par_warp.isMeet[i] = true;
            temp_path.append(GetPathLeastTime(graph, *par_warp.temp, par_warp.t));
            temp_path.fixTotalTime(graph, init_time_);
            if (temp_path.get_total_time().to_hour() < *par_warp.total_hour && plan.size() - 1 - par_warp.depth < *par_warp.total_hour - temp_path.get_total_time().to_hour())
			{
                par_warp.current = i, par_warp.depth++, par_warp.t = temp_time.add_time(temp_path.get_total_time());
				DFSLeastTime(graph, plan, path, temp_path, par_warp);
				par_warp.current = temp_current, par_warp.depth--, par_warp.t = time_save;
			}
			par_warp.isMeet[i] = false;
			temp_path = path_save;
		}
	}
}

Path Traveller::schedulePath(const CityGraph &graph, const std::vector<City_id> &plan, Strategy s, Time start_time, Time limit)
{
	Log::LogWrite("安排路线中");
	init_time_ = start_time;
	strategy_ = s;
	travelling_plan_ = plan;

	if (plan.size() < 2)
		throw plan.size();
	if (s == LEAST_MONEY)
	{
		std::vector<std::vector<Path>> path_matrix;
		std::vector<std::vector<int>> price_matrix;
		int min_price = kMaxInt;
		Path res;
        int path_price = GetPathLeastMoney(graph, plan).get_total_price();

		std::vector<City_id> temp_plan_shuffle = plan;
		int temp_path_price;

		std::random_device rd;
		std::mt19937 g(rd());
		for (int i = 0; i < 10; i++)
		{
			shuffle(temp_plan_shuffle.begin() + 1, temp_plan_shuffle.end() - 1, g);
            temp_path_price = GetPathLeastMoney(graph, temp_plan_shuffle).get_total_price();
			if (temp_path_price < path_price)
				path_price = temp_path_price;
		}

		size_t sz = plan.size();
		bool *min_price_is_meet = new bool[sz]();
		std::vector<City_id> temp_plan{0, 1};

		for (int i = 0; i != sz; ++i)
		{
			min_price_is_meet[i] = false;
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
                price_matrix[i].push_back(path_matrix[i][j].get_total_price());
				if (price_matrix[i].back() && min_price > price_matrix[i].back())
					min_price = price_matrix[i].back();
			}
		}

		delete[] min_price_is_meet;

		std::vector<int> res_path;
		if (sz > 10)
		{
			int t_price[31]{};
			bool(*is_meet)[31] = new bool[sz][31]();
			int temp_prices[31]{};
			DFSLeastMoneyParWarp par_warps[31]{};
			std::vector<int> res_paths[31]{};
			std::vector<int> temp_paths[31]{};

			std::vector<std::thread> t_vec;
			for (int i = 0; i < sz - 2; i++)
			{
				temp_prices[i] = price_matrix[0][i + 1];
				is_meet[i][0] = true, is_meet[i][i + 1] = true;
				par_warps[i] = {temp_prices[i], &path_price, &t_price[i], is_meet[i], i + 1, 1, min_price};
				temp_paths[i].push_back(0);
				temp_paths[i].push_back(i + 1);
				t_vec.emplace_back(&Traveller::DFSLeastMoney, this, std::ref(price_matrix), std::ref(res_paths[i]), std::ref(temp_paths[i]), std::ref(par_warps[i]));
			}

			for (auto &t : t_vec)
			{
				t.join();
			}

			int temp_p = kMaxInt;
			for (int i = 0; i < sz - 2; i++)
			{
				if (path_price == t_price[i])
				{
					res_path = res_paths[i];
					break;
				}
			}
			delete[] is_meet;
		}
		else
		{
			int t_price;
			bool *is_meet = new bool[sz] {};
			is_meet[0] = true;
			DFSLeastMoneyParWarp par_warp = {0, &path_price, &t_price,is_meet, 0, 0, min_price};
			std::vector<int> temp_path;
			res_path.push_back(0);
			temp_path.push_back(0);
			DFSLeastMoney(price_matrix, res_path, temp_path, par_warp);
			delete[] is_meet;
		}
		for (int i = 1; i != res_path.size(); ++i)
            res.append(path_matrix[res_path[i - 1]][res_path[i]]);
        res.fixTotalTime(graph, start_time);
		return res;
		// return GetPathLeastMoney(graph, plan);
	}
	else if (s == LEAST_TIME)
	{
		Path res = GetPathLeastTime(graph, plan, start_time);
        res.fixTotalTime(graph, start_time);
		int i = 0;
		auto sz = plan.size();

		if (plan.size() == 2)
			return res;
		else
		{
			if (sz > 2)
			{
                int total_hour = res.get_total_time().to_hour();
				bool(*is_meet)[31] = new bool[sz][31]();
				DFSLeastTimeParWarp par_warps[31]{};
				Path res_paths[31];
				Path temp_paths[31];
				std::vector<std::thread> t_vec;
				std::vector<City_id> temp[31];

				for (int i = 0; i < sz - 2; i++)
				{
					is_meet[i][0] = true, is_meet[i][i + 1] = true;
					temp[i] = {0, i + 1};
					temp_paths[i] = GetPathLeastTime(graph, {plan[0], plan[i + 1]}, start_time);
                    temp_paths[i].fixTotalTime(graph, start_time);
                    Time s_time = temp_paths[i].get_total_time();
					par_warps[i] = {s_time.add_time(start_time), i + 1, &total_hour, is_meet[i], 1, &temp[i]};
					t_vec.emplace_back(&Traveller::DFSLeastTime, this, std::ref(graph), std::ref(plan), std::ref(res_paths[i]), std::ref(temp_paths[i]), std::ref(par_warps[i]));
				}

				for (auto &t : t_vec)
				{
					t.join();
				}

				for (int i = 0; i < sz - 2; i++)
				{
                    if (total_hour == res_paths[i].get_total_time().to_hour())
					{
						res = res_paths[i];
						break;
					}
				}

				delete[] is_meet;
			}
			else
			{
                int total_hour = res.get_total_time().to_hour();
				bool *isMeet = new bool[sz]();
				Path temp_path;
				isMeet[0] = true;
				std::vector<City_id> temp{0, 1};
				DFSLeastTimeParWarp par_warp = {start_time,  0, &total_hour, isMeet, 0, &temp};
				DFSLeastTime(graph, plan, res, temp_path, par_warp);
				delete[] isMeet;
			}
		}
		return res;
	}
	else // if (s == LIMIT_TIME)
	{
		Path a = GetPathLTM(graph, plan, start_time, limit);
        if (a.get_len() == 0)
		{
			Log::LogWrite("没有符合要求的路线");
			std::cout << "未找到符合要求路线" << std::endl;
		}
		return a;
	}
}

Path Traveller::GetPathLeastMoney(const CityGraph &graph, const std::vector<City_id> &plan)
{
	City_id destination; // 终点
	City_id origin;		 // 起点
	Path path;
	std::priority_queue<std::pair<int /*k*/, int /*price*/>> find_min_cost;
	std::pair<int, int> temp_pair;
	auto empty_pq(find_min_cost);

	for (int cnt = plan.size() - 1; cnt > 0; cnt--)
	{
		destination = plan[cnt];
		origin = plan[cnt - 1];
		int cost[kCityNum];		 // 记录最小花费
		int preway[kCityNum][2]; // preway[cityA][] = {CityB, transport_index_from_CityB_to_CityA}
		bool is_count[kCityNum] = {false};

		for (int j = 0; j < kCityNum; j++) //对数据进行初始化
		{
			if (j == origin)
				continue;
            for (int k = 0; k < graph.get_size(origin, j); k++) // 将所有从origin到j的价格push到find_min_cost中
			{
                temp_pair.first = k, temp_pair.second = graph.get_route(origin, j, k).price;
				find_min_cost.push(temp_pair);
			}

			if (!find_min_cost.empty()) // 如果可以从origin到j
			{
				auto &min = find_min_cost.top();
				cost[j] = min.second;
				preway[j][0] = origin;
				preway[j][1] = min.first;
				find_min_cost = empty_pq;
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
                for (int k = 0; k < graph.get_size(city_temp, j); k++)
				{
                    temp_pair.first = k, temp_pair.second = graph.get_route(city_temp, j, k).price;
                    find_min_cost.push(std::make_pair(k, graph.get_route(city_temp, j, k).price));
				}
				if (!find_min_cost.empty())
				{
					auto &min = find_min_cost.top();
					int newcost = min.second + cost[city_temp];
					if (newcost < cost[j])
					{
						cost[j] = newcost;
						preway[j][0] = city_temp;
						preway[j][1] = min.first;
					}
					find_min_cost = empty_pq;
				}
			}
		}
		for (int traceback = destination; traceback != origin; traceback = preway[traceback][0])
		{
            path.append(graph, preway[traceback][0], traceback, preway[traceback][1]);
		}
	}
	//path.Reverse();
	//path.Show();
	return path;
}

Path Traveller::GetPathLeastTime(const CityGraph &graph, const std::vector<City_id> &plan, Time now)
{
	City_id destination; // 终点
	City_id origin;		 // 起点
	Path path;
	std::vector<int> find_min_cost;
	Time temp_now;
	std::vector<int>::iterator min;
	for (int cnt = plan.size() - 1; cnt > 0; cnt--)
	{
		destination = plan[cnt];
		origin = plan[cnt - 1];
		int cost[kCityNum];		 // 记录最小花费
		int preway[kCityNum][2]; // preway[cityA][] = {CityB, transport_index_from_CityB_to_CityA}
		// Time pretime[kCityNum];
		bool is_count[kCityNum] = {false};

		for (int j = 0; j < kCityNum; j++) //对数据进行初始化
		{
			if (j == origin)
				continue;
            for (int k = 0; k < graph.get_size(origin, j); k++) // 将所有从origin到j的价格push到find_min_cost中
			{												   //*****和LM的区别*****
                const Route &route = graph.get_route(origin, j, k);
                temp_now.set_hour(now.get_hour());
				int wait_hour = route.start_time.hour_diff(temp_now); // 计算从当前时间开始需要等待的时间
				if (wait_hour < 0)									  // 如果发车时间在now之前,想要搭乘这辆车就必须等候到其发车
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
			// Route temp_route = graph.get_route(preway[city_temp][0], city_temp, preway[city_temp][1]);
			// now.add_time(cost[city_temp]);   // 此时的时间为之前的时间加上从前一个城市到city_temp的时间(包括等候时间)
            now = graph.get_route(preway[city_temp][0], city_temp, preway[city_temp][1]).end_time;
            temp_now.set_hour(now.get_hour()); // 只看到达时间,不看天数
			// now = graph.get_route(preway[city_temp][0], city_temp, preway[city_temp][1]).end_time;

			for (int j = 0; j < kCityNum; j++) //更新
			{
				if (is_count[j])
					continue;
                for (int k = 0; k < graph.get_size(city_temp, j); k++)
				{ //*****和LM的区别*****
                    const Route &route = graph.get_route(city_temp, j, k);
					int wait_hour = route.start_time.hour_diff(temp_now); // 计算从当前时间开始需要等待的时间
					if (wait_hour < 0)									  // 如果发车时间在now之前,想要搭乘这辆车就必须等候到其发车
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
            path.append(graph, preway[traceback][0], traceback, preway[traceback][1]); //pretime[traceback]);
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
        least_money = path.get_total_price();
		return;
	}
	int i = plan.at(layer);
	int j = plan.at(layer + 1);
    for (int k = 0; k < graph.get_size(i, j); k++)
	{
        temp.append(graph, i, j, k, 1);
        temp.fixTotalTime(graph, init_time_);
        if (temp.get_total_price() < least_money && temp.get_total_time().get_length() <= limit && temp.get_total_time().get_length() + plan.size() - layer - 1 <= limit)
			DFSLTM(graph, plan, path, temp, layer + 1, least_money, limit);
        temp.remove(graph);
	}
}

bool Traveller::saveData() const
{
	std::vector<std::string> lines;
	std::string temp;
	int start_index = 0;

	std::ofstream create_name_flie(name_path, std::ofstream::app); // 防止文件不存在导致的打开失败
	std::ofstream create_file(save_path, std::ofstream::app);	  // 防止文件不存在导致的打开失败

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
            lines[i++] = std::to_string(init_time_.get_hour());
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
            lines.push_back(std::to_string(init_time_.get_hour()));
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

bool Traveller::loadData(int cnt, const CityGraph &graph)
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

		in_stream >> id_;			// 第一行
		in_stream >> state_temp;	// 第二行
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
            touring_path_.append(graph, former_city, current_city, k, 1);
		}

		in_stream >> next_city_hour_left_;
		//std::cout << next_city_hour_left_ << std::endl;///////////

		in_stream >> position_pathnode_;
		//std::cout << position_pathnode_ << std::endl;///////////
		int hour;
		in_stream >> hour;
		init_time_.set_hour(hour);
        touring_path_.fixTotalTime(graph, init_time_);
		return true;
	}
	return false;
}

void Traveller::updateState(const CityGraph &graph)
{
	if (state_ == OFF)
	{
		if (next_city_hour_left_ == 1)
		{
            if (position_pathnode_ == touring_path_.get_len() - 1)
			{
				state_ = STAY;
				position_pathnode_ = -1;
				return;
			}
			else
			{
				position_pathnode_++;
                PathNode node = touring_path_.getNode(position_pathnode_);
                Route route = graph.get_route(node.former_city, node.current_city, node.kth_way);
                PathNode node_before = touring_path_.getNode(position_pathnode_ - 1);
                Route pre_route = graph.get_route(node_before.former_city, node_before.current_city, node_before.kth_way);
                int diff_hour = route.start_time.hour_diff(Time(1, pre_route.end_time.get_hour()));
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
                int i = touring_path_.getNode(position_pathnode_).former_city;
                int j = touring_path_.getNode(position_pathnode_).current_city;
                int k = touring_path_.getNode(position_pathnode_).kth_way;
				state_ = OFF;
                Route route = graph.get_route(i, j, k);
				next_city_hour_left_ = route.end_time.hour_diff(route.start_time);
			}
			else
				next_city_hour_left_--;
		}
	}
}

void Traveller::initState(const CityGraph &graph)
{
	auto path_begin = touring_path_.cbegin();
    Route route = graph.get_route((*path_begin).former_city, (*path_begin).current_city, (*path_begin).kth_way);
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

void Traveller::printPlan() const
{
	std::for_each(travelling_plan_.begin(), travelling_plan_.end(), [](City_id city) { std::cout << city << " "; });
	std::cout << std::endl;
}

int Traveller::get_off_hours(const CityGraph &graph, int cnt)
{
    int i = touring_path_.getNode(cnt).former_city;
    int j = touring_path_.getNode(cnt).current_city;
    int k = touring_path_.getNode(cnt).kth_way;
    Route route = graph.get_route(i, j, k);
	return route.end_time.hour_diff(route.start_time);
}

int Traveller::get_stay_hours(const CityGraph &graph, int cnt)
{
    PathNode node = touring_path_.getNode(cnt);
    Route route = graph.get_route(node.former_city, node.current_city, node.kth_way);
	int diff_hour;
	if (cnt == 0)
	{
		diff_hour = route.start_time.hour_diff(init_time_);
	}
	else
	{
        PathNode node_before = touring_path_.getNode(cnt - 1);
        Route pre_route = graph.get_route(node_before.former_city, node_before.current_city, node_before.kth_way);
        diff_hour = route.start_time.hour_diff(Time(1, pre_route.end_time.get_hour()));
	}
	if (diff_hour < 0)
		diff_hour += 24;
	else if (diff_hour > 23)
		diff_hour -= 24;
	return diff_hour;
}

#endif // SRC_TRAVELLER_CC
