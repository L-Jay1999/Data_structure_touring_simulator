#ifndef SRC_IO
#define SRC_IO

#include <cctype>
#include <iostream>
#include <limits>
#include <string>
#include <sstream>
#include <vector>
#include <set>
#include <algorithm>
#include <iterator>
#include <fstream>
#include <exception>

#include "traveller.h"
#include "id_map.h"
#include "user_type.h"
#include "path.h"

#if defined(_WIN32) || (defined(__CYGWIN__) && !defined(_WIN32)) || defined(__MINGW32__) || defined(__MINGW64__)
#include <windows.h>
#endif // _WIN32

extern const std::string name_path;
extern const int kCityNum;

//欢迎界面
int Welcome(Traveller &t);

//功能菜单,返回一个操作代码
int Menu(const IDMap &im, Traveller &traveller);

//预定行程
std::vector<City_id> Request(const IDMap &im);

// 打印错误信息
void ErrorMsg(const std::string &err_msg);

// 打印账户列表
bool PrintNameList();

void Status();

// 根据所提供的traveller添加账号
inline bool AddAccount(const Traveller &traveller) { return traveller.SaveData(); }
// 根据用户名添加账号
bool AddAccount(const std::string &account_name)
{
    std::ofstream fos(name_path, std::ofstream::app);
    if (fos)
    {
        fos << account_name << std::endl;
        return true;
    }
    else
        return false;
}

void MapSearch();

// 返回账户名称所在的行数,若账户名称不存在则返回-1
int AccountCheck(const std::string &id);

// 提供界面供用户选择是否采用该路线
bool PathConfirm();

// 给出界面让用户选择策略
Strategy InputStrategy(Time &init_time, Time &limit_time);

// 打印用户的信息
void PrintTravellerInfo(const CityGraph &graph, const IDMap &id_map, const Time &now, const Traveller &traveller);

// 打印界面友好的路径
std::ostream &PrintPath(const CityGraph &graph, const IDMap &id_map, const Path &path, std::ostream &os = std::cout);
std::ostream &PrintPath(const CityGraph &graph, const IDMap &id_map, const Path &path, const int index, bool showtotal = false, std::ostream &os = std::cout);

// 改变模拟的速度
double getSimulateSpeed();

// 提供设置选项菜单
eSettings SettingsMenu();

// 设置控制台字体大小
bool SetConsoleFontSize();

// 验证账户名称是否合法
inline bool IsValidName(const std::string &name_str)
{
    for (auto i = name_str.begin(); i != name_str.end(); ++i)
        if (!std::isalnum(*i))
            return false;
    return true;
}

// 返回给定字符串中第一个数字,如果没有数字,则返回一个不是数字的字符
char FindFirstDigit(const std::string &op_str)
{
    for (auto i = op_str.begin(); i != op_str.end(); ++i)
        if (std::isdigit(*i))
            return *i;
    return 'A';
}

// 返回给定字符串中第一个字母,如果没有字母,则返回一个不是字母的字符
char FindFirstAlpha(const std::string &op_str)
{
    for (auto i = op_str.begin(); i != op_str.end(); ++i)
        if (std::isalpha(*i))
            return *i;
    return '0';
}

// 清除屏幕上的信息
inline bool ClearScreen()
{
#if defined(_WIN32) || (defined(__CYGWIN__) && !defined(_WIN32)) || defined(__MINGW32__) || defined(__MINGW64__)

    static const HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获得标准输出流的句柄

    CONSOLE_SCREEN_BUFFER_INFO csbiStdOut; // 用于接收标准输出流的缓存的信息
    COORD topLeft = {0, 0};                // 设置光标最后位于终端的坐标

    std::cout.flush(); // 刷新缓冲区,防止数据残留,避免清屏后出现清屏前的信息

    if (!GetConsoleScreenBufferInfo(hStdOut, &csbiStdOut))
    {
        abort();
    }
    DWORD length = csbiStdOut.dwSize.X * csbiStdOut.dwSize.Y; // 写入缓冲区的长度,等于标准输出流缓冲区的size大小

    DWORD written;

    bool bWirteSuccess = FillConsoleOutputCharacter(
        hStdOut,   // 指定标准输出为要写入的流
        TEXT(' '), // 用空格覆盖标准输出流控制台的每一个字符
        length,    // 写入的长度
        topLeft,   // 从何处开始写入
        &written   // 写入了多少字符
    );

    SetConsoleCursorPosition(hStdOut, topLeft); // 设置标准输出流所在控制台的光标位置

    return bWirteSuccess;

#elif defined(__linux__)
    std::cout << "\x1B[2J\x1B[H";
    return true;
#endif
}

// 输出两个城市的所有路线
void PrintRoutes(const CityGraph &graph, const IDMap &id_map);

// 获得时间
Time InputLimitTime();
Time InputInitTime();

// 判断所给城市是否处于旅行计划中
bool IsInplan(const std::vector<City_id> &plan, City_id city);

int Welcome(Traveller &traveller)
{
    std::cout << "|----------------------------------------------|" << std::endl;
    std::cout << std::endl;
    std::cout << "|                                              |" << std::endl;
    std::cout << std::endl;
    std::cout << "|                   Welcome!                   |" << std::endl;
    std::cout << std::endl;
    std::cout << "|                                              |" << std::endl;
    std::cout << std::endl;
    std::cout << "|----------------------------------------------|" << std::endl;
    std::cout << std::endl;
    std::cout << "选择注册还是已有账号登陆？" << std::endl;
    std::cout << "注册：S           登陆：L" << std::endl;
    std::string option_str;
    char option;
    while (1)
    {
        std::getline(std::cin, option_str);
        std::cin.clear();
        option = FindFirstAlpha(option_str);

        if (option == 'S' || option == 's')
        {
            std::cout << "请输入你想注册的账号：";
            std::string accout_name;
            getline(std::cin, accout_name);
            std::cin.clear();

            while (!IsValidName(accout_name))
            {
                ErrorMsg("非法的用户名，请重新输入：");
                getline(std::cin, accout_name);
                std::cin.clear();
            }

            while (AccountCheck(accout_name) != -1)
            {
                std::cout << "该账号已被注册，请重新输入：";
                getline(std::cin, accout_name);
                std::cin.clear();

                while (!IsValidName(accout_name))
                {
                    ErrorMsg("非法的用户名，请重新输入：");
                    getline(std::cin, accout_name);
                    std::cin.clear();
                }
            }
            AddAccount(accout_name);
            traveller.set_id(accout_name);
            std::cout << "你已注册账号：" << accout_name << std::endl;
            return -1;
        }
        else if (option == 'l' || option == 'L')
        {
            std::cout << "已注册的账号列表如下：" << std::endl;
            PrintNameList();

            std::cout << "请输入您的账号：";
            std::string accout_name;
            getline(std::cin, accout_name);
            std::cin.clear();

            while (!IsValidName(accout_name))
            {
                ErrorMsg("非法的用户名,请重新输入");
                getline(std::cin, accout_name);
                std::cin.clear();
            }

            while (AccountCheck(accout_name) == -1)
            {
                std::string option_str;
                char option;

                std::cout << "该账号不存在，是否需要注册该账号?[Y/N]" << std::endl;

                std::getline(std::cin, option_str);
                std::cin.clear();
                option = FindFirstAlpha(option_str);
                while (option != 'Y' && option != 'N')
                {
                    std::cout << "无效的选项，请重新输入" << std::endl;
                    std::getline(std::cin, option_str);
                    std::cin.clear();
                    option = FindFirstAlpha(option_str);
                }
                if (option == 'Y')
                {
                    AddAccount(accout_name);
                    traveller.set_id(accout_name);
                    return -1;
                }
                else if (option == 'N')
                {
                    std::cout << "请重新输入您的账号：";
                    getline(std::cin, accout_name);
                    std::cin.clear();

                    while (!IsValidName(accout_name))
                    {
                        ErrorMsg("非法的用户名，请重新输入：");
                        getline(std::cin, accout_name);
                        std::cin.clear();
                    }
                }
            }
            return AccountCheck(accout_name);
        }
        else if (option == 'q' || option == 'Q')
        {
            exit(0);
        }
        else
        {
            std::cout << "无效的选项，请重新输入：";
        }
    }
}

//功能菜单,返回一个操作代码
int Menu(const IDMap &im, Traveller &traveller)
{
    std::cout << std::endl;
    std::cout << "输入对应数字获取功能：" << std::endl
              << "1、预定行程" << std::endl
              << "2、状态查询" << std::endl
              << "3、路线查询" << std::endl
              << "4、模拟旅行" << std::endl
              << "5、设    置" << std::endl
              << "6、退出程序" << std::endl;
    std::string buf;
    char num;
    std::vector<City_id> plan;
    while (std::getline(std::cin, buf))
    {
        if (!buf.size())
        {
            // std::cout << "!!!" << std::endl;
            continue;
        }
        num = FindFirstDigit(buf);
        int operate_code = num - '0';

        if (operate_code == SCHEDULE)
        {
            ClearScreen();
            std::cout << "-------------------------------------------------" << std::endl;
            return operate_code;
        }
        else if (operate_code == INQUIRE_STATE)
        {
            ClearScreen();
            std::cout << "-------------------------------------------------" << std::endl;
            return operate_code;
        }
        else if (operate_code == INQUIRE_PATH)
        {
            ClearScreen();
            std::cout << "-------------------------------------------------" << std::endl;
            return operate_code;
        }
        else if (operate_code == SIMULATE)
        {
            ClearScreen();
            return operate_code;
        }
        else if (operate_code == SETTINGS)
        {
            ClearScreen();
            return operate_code;
        }
        else if (operate_code == EXIT)
        {
            return operate_code;
        }
        else
        {
            std::cout << "无效的选项，请重新输入：";
        }
    }
    std::cin.clear();
}

//预定行程
//
//
//
//
// TODO: 判断输入是否为数字
std::vector<City_id> Request(const IDMap &im)
{
    std::vector<City_id> res;
    std::string id;
    City_id temp_id;

    std::cout << "当前支持的城市有：" << std::endl;
    for (int i = 0; i < im.GetCityMapSize(); i++)
    {
        std::cout << i + 1 << " : " << im.GetCityStr(i) << std::endl;
    }

    std::cout << "请输入您的当前城市(输入数字)：";
    while (1)
    {
        if (!std::cin.good())
            std::cin.clear();
        std::cin >> id;
        if (std::cin.good())
        {
            while (1)
            {
                try
                {
                    temp_id = std::stoi(id) - 1;
                    break;
                }
                catch (const std::exception &e)
                {
                    std::cout << "请输入数字：";
                }
                std::cin >> id;
                std::cin.clear();
            }

            if (temp_id < im.GetCityMapSize() && temp_id >= 0)
            {
                std::cout << "你选择的当前城市是：" << im.GetCityStr(temp_id) << std::endl;
                res.push_back(temp_id);
                break;
            }
            else
            {
                ErrorMsg("无效的输入，请重新输入");
            }
        }
    }

    std::cout << "请输入您希望经过的城市(以q结束)：";
    while (1)
    {
        if (!std::cin.good())
            std::cin.clear();
        std::cin >> id;
        if (std::cin.good())
        {
            while (1)
            {
                try
                {
                    if (id == "q")
                        break;

                    temp_id = std::stoi(id) - 1;
                    break;
                }
                catch (const std::exception &e)
                {
                    std::cout << "请输入数字或q：";
                }
                std::cin >> id;
                std::cin.clear();
            }

            if (id == "q")
                break;

            if (temp_id >= im.GetCityMapSize() || temp_id < 0)
                ErrorMsg("无效的城市，请重新输入");
            else if (temp_id == res.front())
                ErrorMsg("与起点重复，请重新输入");
            else if (IsInplan(res, temp_id))
                ErrorMsg("已存在于计划中，请重新输入");
            else
            {
                res.push_back(temp_id);
            }
        }
    }
    std::cout << "你选择经过的城市是：";
    std::for_each(++res.begin(), res.end(), [&](City_id city_id) { std::cout << im.GetCityStr(city_id) << " "; });
    std::cout << std::endl;

    std::cout << "请输入您的目的城市：";
    while (1)
    {
        if (!std::cin.good())
            std::cin.clear();
        std::cin >> id;
        if (std::cin.good())
        {
            while (1)
            {
                try
                {
                    temp_id = std::stoi(id) - 1;
                    break;
                }
                catch (const std::exception &e)
                {
                    std::cout << "请输入数字：";
                }
                std::cin >> id;
                std::cin.clear();
            }

            if (std::find(res.begin(), res.end(), temp_id) != res.end())
            {
                ErrorMsg("重复的城市，请重新输入");
            }
            else if (temp_id >= im.GetCityMapSize() || temp_id < 0)
                ErrorMsg("无效的城市，请重新输入");
            else
            {
                std::cout << "你选择的目的城市是：" << im.GetCityStr(temp_id) << std::endl;
                res.push_back(temp_id);
                break;
            }
        }
    }
    return res;
}

/*inline void Status()
{
    //traveller.();//关于traveller的输出
}*/

// 打印错误信息
// TODO : 同时输出至日志
void ErrorMsg(const std::string &err_msg)
{
    std::cout << err_msg << std::endl;
}

/*inline void MapSearch()
{
    //citygraph.()//关于图的输出
}*/

// 返回账户名称所在的行数,若账户名称不存在则返回-1
inline int AccountCheck(const std::string &id)
{
    std::vector<std::string> namelist; // unused parameter
    std::ifstream in_stream(name_path);
    if (in_stream.is_open())
    {
        int cnt = 0;
        std::string temp;
        while (getline(in_stream, temp))
        {
            if (temp == id)
            {
                return cnt;
            }
            cnt++;
        }
    }
    return -1;
}

// 打印账户列表
inline bool PrintNameList()
{
    std::string line_buf;
    std::ifstream fis(name_path);
    if (fis)
    {
        while (std::getline(fis, line_buf))
            std::cout << line_buf << std::endl;
        return true;
    }
    else
        return false;
}

inline bool PathConfirm()
{
    std::string option_str;
    char option;
    std::cout << "是否选择该条路线?[Y/N]" << std::endl;
    while (1)
    {
        std::getline(std::cin, option_str);

        if (!option_str.size())
            continue;
        option = FindFirstAlpha(option_str);
        if (option == 'Y' || option == 'y')
            return true;
        else if (option == 'N' || option == 'n')
            return false;
        else
        {
            ErrorMsg("无效的输入，请重新输入");
            std::cin.clear();
        }
    }
}

inline Strategy InputStrategy(Time &init_time, Time &limit_time)
{
    std::string strategy_str;
    int strategy;
    std::cout << "输入旅行策略" << std::endl
              << "1. 最少金钱" << std::endl
              << "2. 最少时间" << std::endl
              << "3. 限定时间内最少金钱" << std::endl;
    while (1)
    {
        std::getline(std::cin, strategy_str);
        std::cin.clear();
        if (!strategy_str.size())
            continue;
        strategy = FindFirstDigit(strategy_str) - '0';

        switch (strategy - 1)
        {
        case 0:
            init_time = InputInitTime();
            return LEAST_MONEY;
        case 1:
            init_time = InputInitTime();
            return LEAST_TIME;
        case 2:
            init_time = InputInitTime();
            limit_time = InputLimitTime();
            return LIMIT_TIME;
        default:
            ErrorMsg("无效的输入，请重新输入");
            break;
        }
    }
}

std::ostream &PrintPath(const CityGraph &graph, const IDMap &id_map, const Path &path, std::ostream &os)
{
    return PrintPath(graph, id_map, path, 0, true);
}

std::ostream &PrintPath(const CityGraph &graph, const IDMap &id_map, const Path &path, const int index, bool showtotal, std::ostream &os)
{
    std::string comp("三个字");
    std::string wrap[] = {"\t\t", "\t"};
    //std::cout << "为你定制的路线为：" << std::endl;
    std::cout << "始发地"
              << "\t\t"
              << "目的地"
              << "\t\t"
              << "方式" << '\t'
              << "出发时间" << '\t'
              << "到达时间" << '\t'
              << "价格" << '\t'
              << "完成" << '\t' << std::endl;
    for (int path_node = 0; path_node < index; path_node++)
    {
        int i = path.GetNode(path_node).former_city;
        int j = path.GetNode(path_node).current_city;
        int k = path.GetNode(path_node).kth_way;
        Route route = graph.GetRoute(i, j, k);
        auto former_city_str = id_map.GetCityStr(i);
        auto current_city_str = id_map.GetCityStr(j);
        std::cout << former_city_str << wrap[former_city_str.size() > comp.size()]
                  << current_city_str << wrap[current_city_str.size() > comp.size()]
                  << id_map.GetTransStr(route.transport_type) << '\t';
        RouteShow(route.start_time, route.end_time);
        std::cout << route.price << '\t'
                  << "O" << '\t' << std::endl;
    }

    for (int path_node = index; path_node < path.GetLen(); path_node++)
    {
        int i = path.GetNode(path_node).former_city;
        int j = path.GetNode(path_node).current_city;
        int k = path.GetNode(path_node).kth_way;
        Route route = graph.GetRoute(i, j, k);
        auto former_city_str = id_map.GetCityStr(i);
        auto current_city_str = id_map.GetCityStr(j);
        std::cout << former_city_str << wrap[former_city_str.size() > comp.size()]
                  << current_city_str << wrap[current_city_str.size() > comp.size()]
                  << id_map.GetTransStr(route.transport_type) << '\t';
        RouteShow(route.start_time, route.end_time);
        std::cout << route.price << '\t'
                  << "X" << '\t' << std::endl;
    }
    if (showtotal)
    {
        std::cout << "总价格花费：" << path.GetTotalPrice() << std::endl;
        std::cout << "总时间花费：" << path.GetTotalTime().GetLength() << "h" << std::endl;
    }
    return os;
}

void PrintTravellerInfo(const CityGraph &graph, const IDMap &id_map, const Time &now, const Traveller &traveller)
{
    auto plan = traveller.get_plan();
    auto path = traveller.get_path();
    auto position = traveller.get_position();

    //std::cout << "用户名: " <<traveller.get_ID() << std::endl;
    if (traveller.get_state() == STAY)
    {
        if (position == -2)
            std::cout << "当前无出行计划" << std::endl;
        else if (position == -1)
        {
            std::cout << std::endl;
            std::cout << "你的始发地是：" << id_map.GetCityStr(plan.front()) << std::endl;
            std::cout << "你的目的地是：" << id_map.GetCityStr(plan.back()) << std::endl;

            if (plan.size() > 2)
            {
                std::cout << "你的途经城市有：";
                for (int i = 1; i < plan.size() - 1; i++)
                {
                    std::cout << id_map.GetCityStr(plan.at(i)) << " ";
                }
                std::cout << std::endl;
            }
            PrintPath(graph, id_map, path);
        }
        else
        {
            PrintPath(graph, id_map, path, position);
            std::cout << "当前在：" << id_map.GetCityStr(path.GetNode(position).former_city) << std::endl;
            std::cout << "离出发还有：" << traveller.get_left_hour() << "h" << std::endl;
        }
    }
    else
    {
        PrintPath(graph, id_map, path, position);
        std::cout << "距目的地 " << id_map.GetCityStr(path.GetNode(position).current_city) << " 还有：" << traveller.get_left_hour() << "h" << std::endl;
    }
}

void PrintRoutes(const CityGraph &graph, const IDMap &id_map)
{
    std::string id;
    City_id start_city;
    City_id target_city;
    std::cout << "输入两个城市，获取两城市间的所有路线" << std::endl;
    for (int i = 0; i < id_map.GetCityMapSize(); i++)
    {
        std::cout << i + 1 << " : " << id_map.GetCityStr(i) << std::endl;
    }
    std::cout << "请输入始发城市：";
    while (1)
    {
        if (!std::cin.good())
            std::cin.clear();
        std::cin >> id;
        if (std::cin.good())
        {
            start_city = std::stoi(id) - 1;
            if (start_city < id_map.GetCityMapSize() && start_city >= 0)
            {
                std::cout << "你选择的始发城市是：" << id_map.GetCityStr(start_city) << std::endl;
                break;
            }
            else
            {
                ErrorMsg("无效的输入");
                std::cout << "请输入始发城市：";
            }
        }
    }
    std::cout << "请输入目的城市：";
    while (1)
    {
        if (!std::cin.good())
            std::cin.clear();
        std::cin >> id;
        if (std::cin.good())
        {
            target_city = std::stoi(id) - 1;
            if (target_city < id_map.GetCityMapSize() && target_city >= 0)
            {
                std::cout << "你选择的目的城市是：" << id_map.GetCityStr(target_city) << std::endl;
                break;
            }
            else
            {
                ErrorMsg("无效的输入");
                std::cout << "请输入始发城市：";
            }
        }
    }
    int size = graph.Getsize(start_city, target_city);
    if (!size)
    {
        std::cout << "两城市间无路线" << std::endl;
    }
    else
    {
        std::string comp("三个字");
        std::string wrap[] = {"\t\t", "\t"};
        std::cout << "序号" << '\t'
                  << "始发地"
                  << "\t\t"
                  << "目的地"
                  << "\t\t"
                  << "方式" << '\t'
                  << "出发时间" << '\t'
                  << "到达时间" << '\t'
                  << "价格" << '\t' << std::endl;

        for (int k = 0; k < size; k++)
        {
            Route route = graph.GetRoute(start_city, target_city, k);

            std::string cityi = id_map.GetCityStr(start_city);
            std::string cityj = id_map.GetCityStr(target_city);
            std::string method = id_map.GetTransStr(route.transport_type);

            std::cout << k + 1 << '\t'
                      << cityi << wrap[cityi.size() > comp.size()]
                      << cityj << wrap[cityj.size() > comp.size()]
                      << method << '\t';
            RouteShow(route.start_time, route.end_time);
            std::cout << route.price << '\t' << std::endl;
        }
    }
}

Time InputLimitTime()
{
    std::string info;
    int day;
    int hour;

    while (1)
    {
        std::cout << "输入你希望第几天内到达(1代表当天)：";
        if (!std::cin.good())
            std::cin.clear();

        std::cin >> info;
        if (std::cin.good())
        {
            day = std::stoi(info);
            if (day >= 1)
            {
                std::cout << "第" << day << "天到达" << std::endl;
                break;
            }
            else
            {
                ErrorMsg("无效的输入，请重新输入");
            }
        }
    }

    while (1)
    {
        std::cout << "输入你希望到达时刻(输入小时数)：";
        if (!std::cin.good())
            std::cin.clear();

        std::cin >> info;
        if (std::cin.good())
        {
            hour = std::stoi(info);
            if (hour >= 0 && hour < 24)
            {
                std::cout << hour << "点前" << std::endl;
                break;
            }
            else
            {
                ErrorMsg("无效的输入，请重新输入");
            }
        }
    }
    return Time(day, hour);
}

Time InputInitTime()
{
    std::string info;
    int hour;

    while (1)
    {
        std::cout << "输入预计出发时间开始查找(输入小时数)：";
        if (!std::cin.good())
            std::cin.clear();

        std::cin >> info;
        if (std::cin.good())
        {
            hour = std::stoi(info);
            if (hour >= 0 && hour < 24)
            {
                std::cout << "输入预计出发时间为：" << hour << ":00" << std::endl;
                break;
            }
            else
            {
                ErrorMsg("无效的输入，请重新输入");
            }
        }
    }
    return Time(1, hour);
}

inline bool IsInplan(const std::vector<City_id> &plan, City_id city)
{
    return std::find(plan.begin(), plan.end(), city) != plan.end();
}

double getSimulateSpeed()
{
    std::cout << "请输入两次模拟的间隔时间(在0.5秒和10秒之间)：";
    std::string line;
    double sleep_ms;
    std::exception excpt;

    while (1)
    {
        try
        {
            getline(std::cin, line);
            std::cin.clear();
            if (!line.size())
                continue;
            else
            {
                sleep_ms = std::stod(line);
                if (sleep_ms < 0.5 || sleep_ms > 10)
                    throw excpt;
                break;
            }
        }
        catch (std::exception excpt)
        {
            std::cout << "输入有误，请重新输入" << std::endl;
        }
    }
    return 1000 * sleep_ms;
}

eSettings SettingsMenu()
{
    std::cout << "输入对应数字获取功能：" << std::endl
              << "1、改变模拟速度" << std::endl
              << "2、改变控制台字体大小(Windows Only)" << std::endl;
    std::string line;
    int option;
    while (1)
    {
        getline(std::cin, line);
        std::cin.clear();

        if (!line.size())
            continue;

        option = FindFirstDigit(line);

        if (option == '1')
            return SIMULATION_SPEED;
        else if (option == '2')
            return CONSOLE_FONT_SIZE;
        else
        {
            std::cout << "无效的输入，请重新输入" << std::endl;
        }
    }
}

bool SetConsoleFontSize()
{
#if defined(_WIN32) || (defined(__CYGWIN__) && !defined(_WIN32)) || defined(__MINGW32__) || defined(__MINGW64__)

    static HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出流的句柄
    static CONSOLE_FONT_INFOEX cfiStdOut;                    // 记录标准输出所使用的字体的信息
    SHORT sNewX, sNewY;
    cfiStdOut.cbSize = sizeof(CONSOLE_FONT_INFOEX); // 必须设置cbSize为CONSOLE_FONT_SIZE的大小以满足GetCurrentConsoleFontEx()的要求

    GetCurrentConsoleFontEx(hStdOut, FALSE, &cfiStdOut);    // 获得标准输出所使用的字体的信息
    GetConsoleFontSize(hStdOut, cfiStdOut.nFont);           // 获得字体大小

    // std::cout << cfiStdOut.FaceName << " "
    //           << "X: " << cfiStdOut.dwFontSize.X << " Y: " << cfiStdOut.dwFontSize.Y << std::endl;
    std::cout << "当前字体大小: " << cfiStdOut.dwFontSize.Y << std::endl;
    std::cout << "请输入新的字体大小: ";
    while (1)
    {
        std::cin >> sNewY;
        std::cin.clear();
        if (sNewY < 1)
        {
            std::cout << "输入有误，请重新输入" << std::endl;
        }
        else
            break;
    }
    // cfiStdOut.dwFontSize.X = sNewX;
    cfiStdOut.dwFontSize.Y = sNewY;

    SetCurrentConsoleFontEx(hStdOut, FALSE, &cfiStdOut);    // 设置字体大小

#else
    std::cout << "该设置目前仅支持Windows系统" << std::endl;
#endif
}
#endif //SRC_IO