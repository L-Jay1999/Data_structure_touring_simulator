#ifndef SRC_CG_CC
#define SRC_CG_CC

#pragma execution_character_set("utf-8")

#include <iostream>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

#include "city_graph.h"
#include "user_type.h"
#include "time_format.h"
#include "id_map.h"
#include "log.h"

static const std::string flight_file_path = "../data/flight_extract_with_id.txt";
static const std::string train_file_path = "../data/train_extract_with_id.txt";
static const std::string car_file_path = "../data/car_extract_with_id.txt";

CityGraph::CityGraph()
{
    if(LoadCityGraph(flight_file_path, 2) &&
       LoadCityGraph(train_file_path, 1) &&
       LoadCityGraph(car_file_path, 0))
        Log::LogWrite("车次数据加载成功");
    else
    {
        Log::LogWrite("车次数据读取失败");
    }
}

bool CityGraph::LoadCityGraph(const std::string &name, int type)
{ //将飞机火车汽车数据加载到程序中
    Log::LogWrite(std::string("加载 ") + name + " 中");
    std::ifstream stream(name);

    if (stream)
    {
        if (type != 1)
        { //飞机汽车的数据
            int data[5];
            while (!stream.eof())
            {
                for (int i = 0; i < 5; i++)
                {
                    stream >> data[i];
                }
                Route temp = {type, 0, Time(data[2]), Time(data[3]), data[4]};
                city_[data[0]][data[1]].push_back(temp);
            }
        }
        else
        { //火车的数据
            int data[7];
            std::string line;
            while (getline(stream, line))
            {
                std::istringstream ss(line);
                for (int i = 0; i < 7; i++)
                {
                    ss >> data[i];
                }
                Route temp = {type, 0, Time(data[2]), Time(data[3]), data[6]};
                city_[data[0]][data[1]].push_back(temp);
            }
        }
    }
    else
    {
        Log::LogWrite(std::string("无法打开 ") + name);
        return false;
    }
    return true;
}

void CityGraph::show(City_id former_city, City_id current_city, int k) const
{
    if (former_city != current_city)
    {
        std::cout << former_city << ' ' << current_city << ' ' << k << ' '
                  << city_[former_city][current_city].at(k).start_time.get_day() << ' '
                  << city_[former_city][current_city].at(k).start_time.get_hour() << ' '
                  << city_[former_city][current_city].at(k).end_time.get_day() << ' '
                  << city_[former_city][current_city].at(k).end_time.get_hour() << ' '
                  << city_[former_city][current_city].at(k).price << std::endl;
    }
    else
        std::cout << "No data!" << std::endl;
}
#endif // SRC_CG_CC
