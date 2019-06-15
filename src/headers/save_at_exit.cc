#ifndef SAVE_AT_EXIT_CC
#define SAVE_AT_EXIT_CC

#pragma execution_character_set("utf-8")

#include <cstdlib>
#include <csignal>
#include <string>
#include <iostream>

#include "save_at_exit.h"
#include "io.h"
#include "traveller.h"
#include "log.h"

static Traveller *p_traveller = nullptr;
static bool saved = false;

void setTravellerPtr(Traveller *traveller) noexcept
{
    p_traveller = traveller;
}

void SaveDataOnExit()
{
    if (!saved && p_traveller != nullptr /*&& (*p_traveller).get_position() != -2*/)
    {
        std::cout << "saving..."
                  << "\t";
        if ((*p_traveller).saveData())
        {
            saved = true;
            Log::LogWrite("数据保存成功");
            std::cout << "success." << std::endl;
        }
        else
        {
            Log::LogWrite("数据保存失败或数据已保存");
            std::cout << "failed." << std::endl;
        }
    }
}

void SaveDataOnExit(int sig)
{
    Log::LogWrite(std::string("收到信号: ") + std::to_string(sig));
    std::cout << "Receive signal " << sig << std::endl;
    SaveDataOnExit();
    std::exit(EXIT_FAILURE);
}

void setSignalHandlers()
{
    Log::LogWrite("绑定信号处理函数");
    std::signal(SIGABRT, SaveDataOnExit);
    std::signal(SIGINT, SaveDataOnExit);
    std::signal(SIGTERM, SaveDataOnExit);
    std::atexit(SaveDataOnExit);
}

#endif // SAVE_AT_EXIT_CC
