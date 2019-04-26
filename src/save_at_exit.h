#ifndef SAVE_AT_EXIT
#define SAVE_AT_EXIT

#include <cstdlib>
#include <csignal>
#include <string>
#include <iostream>

#include "io.h"
#include "traveller.h"

static Traveller *p_traveller = nullptr;
static bool saved = false;

// 设置指向退出时需要保存的用户的指针
void setTravellerPtr(Traveller *traveller)
{
    p_traveller = traveller;
}

// 保存操作,用于接收正常退出
static void SaveDataOnExit()
{
    if (!saved && p_traveller != nullptr /*&& (*p_traveller).get_position() != -2*/)
    {
        std::cout << "saving..." << "\t";
        if ((*p_traveller).SaveData())
        {
            saved = true;   // 一旦保存成功,就设置该flag为true防止重复保存
            std::cout << "success." << std::endl;
        }
        else
            std::cout << "failed." << std::endl;
    }
}

// 用于abort(), terminate(), SIGINT的handler
void SaveDataOnExit(int sig)
{
    std::cout << "Receive signal " << sig << std::endl;
    SaveDataOnExit();
    std::exit(EXIT_FAILURE);    // 此时会再次调用SaveDataOnExit()
}

// 为信号及退出操作设置调用函数
void setSignalHandlers()
{
    std::signal(SIGABRT, SaveDataOnExit);   // 设置接收到信号时的调用函数
    std::signal(SIGINT, SaveDataOnExit);
    std::signal(SIGTERM, SaveDataOnExit);

    std::atexit(SaveDataOnExit);    // 设置正常退出时的调用函数
}

#endif // SAVE_AT_EXIT
