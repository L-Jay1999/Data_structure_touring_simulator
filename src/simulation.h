#ifndef SRC_SIMULATE
#define SRC_SIMULATE

#include <ctime>
#include <cstdlib>
#include <chrono>
#include <thread>

#include "io.h"
#include "traveller.h"
#include "city_graph.h"
#include "user_type.h"
#include "path.h"
#include "log.h"
#include "id_map.h"

static Time current_time;
static int sleep_millsecs = 500;

// 返回每两次调用间的间隔毫秒数
std::chrono::duration<double, std::milli> Timer()
{
    static int count = 0;
    static std::chrono::time_point<std::chrono::system_clock> start_time;
    static std::chrono::time_point<std::chrono::system_clock> end_time;
    if (count % 2)
    {
        end_time = std::chrono::system_clock::now();
        count++;
        return end_time - start_time;
    }
    else
    {

        start_time = std::chrono::system_clock::now();
        count++;
        return start_time - start_time;
    }
}

// 接受一个值,根据值的奇偶来返回时间间隔,当count为奇数时返回该次调用与上次count为偶数时调用的时间间隔
std::chrono::duration<double, std::milli> Timer(int count)
{
    static std::chrono::time_point<std::chrono::system_clock> start_time;
    static std::chrono::time_point<std::chrono::system_clock> end_time;
    if (count % 2)
    {
        end_time = std::chrono::system_clock::now();
        return end_time - start_time;
    }
    else
    {
        start_time = std::chrono::system_clock::now();
        return start_time - start_time;
    }
}

void setSleepMillsecs(int sleep_ms)
{
    sleep_millsecs = sleep_ms;
}

void SleepFor(int millseconds)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(sleep_millsecs - millseconds));
}

void InitializeSimulator(const Time &start_time)
{
    current_time = start_time;
}

#if defined(_WIN32) || (defined(__CYGWIN__) && !defined(_WIN32)) || defined(__MINGW32__) || defined(__MINGW64__)

// 返回一个offscreen的buf的句柄
static HANDLE InitOutBuf()
{
    HANDLE hTempOutBuf = CreateConsoleScreenBuffer(
        GENERIC_READ | GENERIC_WRITE,       // 设置读写权限
        FILE_SHARE_READ | FILE_SHARE_WRITE, // 设置读写共享模式
        NULL,
        CONSOLE_TEXTMODE_BUFFER, // 文本模式缓冲区
        NULL);                   // 保留字
    CONSOLE_CURSOR_INFO cci;
    cci.bVisible = FALSE;
    cci.dwSize = 1;
    SetConsoleCursorInfo(hTempOutBuf, &cci); // 设置该缓冲区的光标不可见
    return hTempOutBuf;
}

// 利用双缓冲进行模拟,避免闪屏
void Simulate(Traveller &traveller, const CityGraph &city_graph, const IDMap &id_map)
{
    if (traveller.get_position() == -2)
    {
        ErrorMsg("当前没有出行计划");
        return;
    }
    static bool out_buf_inited = false;
    static HANDLE hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
    static HANDLE hOutBuf;

    if (!out_buf_inited)
    {
        hOutBuf = InitOutBuf();
        out_buf_inited = true;
    }

    SetConsoleActiveScreenBuffer(hOutBuf); // 设置控制台显示offscreen的缓冲区

    CONSOLE_SCREEN_BUFFER_INFO csbi; // 使得OutBuf的各项信息与StdOut相同
    GetConsoleScreenBufferInfo(hStdOut, &csbi);
    SMALL_RECT srctRect = csbi.srWindow;
    COORD coordBufSize = csbi.dwSize;
    COORD coordBufCoord = {0, 0};
    CHAR_INFO *data_buffer = new CHAR_INFO[coordBufSize.X * coordBufSize.Y];

    int count = 0;
    while (traveller.get_position() != -1)
    {
        Timer(count++);
        ClearScreen();

        Path path = traveller.get_path();
        int position = traveller.get_position();

        City_id current_city = path.GetNode(position).former_city;
        City_id next_city = path.GetNode(position).current_city;
        int route_index = path.GetNode(position).kth_way;
        Trans_id transport_type = city_graph.GetRoute(current_city, next_city, route_index).transport_type;
        std::cout << "当前时间 : " << current_time.GetDay() << " 日 " << current_time.GetHour() << "时" << std::endl;
        //PrintPath(city_graph, id_map, traveller.get_path(), traveller.get_position());
        PrintTravellerInfo(city_graph, id_map, current_time, traveller);

        ReadConsoleOutput(hStdOut, data_buffer, coordBufSize, coordBufCoord, &srctRect);  // 从StdOut读出所有字符,存入data_buffer中
        WriteConsoleOutput(hOutBuf, data_buffer, coordBufSize, coordBufCoord, &srctRect); // 将data_buffer中的字符写入到OutBuf之中

        traveller.UpdateState(city_graph, current_time);
        current_time.add_time(1);
        auto duration = Timer(count++);
        // std::cout << duration.count();
        SleepFor(duration.count());
        // std::cin >> click;
    }
    ClearScreen();
    std::cout << "当前时间 : " << current_time.GetDay() << " 日 " << current_time.GetHour() << "时" << std::endl;
    std::cout << "到达目的地" << std::endl;
    PrintPath(city_graph, id_map, traveller.get_path(), traveller.get_path().GetLen());

    SetConsoleActiveScreenBuffer(hStdOut); // 切换控制台使用的缓冲区为标准输出
    delete[] data_buffer;                  // 释放内存
}

#else

void Simulate(Traveller &traveller, const CityGraph &city_graph, const IDMap &id_map)
{
   
    if (traveller.get_position() == -2)
    {
        ErrorMsg("当前没有出行计划");
        return;
    }
    int count = 0;

    while (traveller.get_position() != -1)
    {
        //char click;
        Timer(count++);
        ClearScreen();
        std::cout << traveller.get_position() << std::endl;
        Path path = traveller.get_path();
        int position = traveller.get_position();

        City_id current_city = path.GetNode(position).former_city;
        City_id next_city = path.GetNode(position).current_city;
        int route_index = path.GetNode(position).kth_way;
        Trans_id transport_type = city_graph.GetRoute(current_city, next_city, route_index).transport_type;
        std::cout << "当前时间 : " << current_time.GetDay() << " 日 " << current_time.GetHour() << "时" << std::endl;
        //PrintPath(city_graph, id_map, traveller.get_path(), traveller.get_position());
        PrintTravellerInfo(city_graph, id_map, current_time, traveller);
        traveller.UpdateState(city_graph, current_time);
        current_time.add_time(1);
        auto duration = Timer(count++);
        // std::cout << duration.count();
        SleepFor(duration.count());
        // std::cin >> click;
    }

    ClearScreen();
    std::cout << "当前时间 : " << current_time.GetDay() << " 日 " << current_time.GetHour() << "时" << std::endl;
    std::cout << "到达目的地" << std::endl;
    PrintPath(city_graph, id_map, traveller.get_path(), traveller.get_path().GetLen());
}
#endif
#endif // SRC_SIMULATE