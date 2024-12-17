#ifndef CHEAT_ENGINE_HPP
#define CHEAT_ENGINE_HPP

#include <unistd.h>
#include <dirent.h>

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <mutex>

#include "Sector.hpp"
#include "Gui.hpp"

class CheatEngine
{
public:
    static CheatEngine &GetInstance();
    void MainLoop();

private:
    void ScanForValue();
    void Rescan();
    unsigned long FindPlayerStructAddress();
    static void FreezeHealth();

private:
    CheatEngine();
    ~CheatEngine();

private:
    std::mutex mutex;
    long value = 0;
    std::list<unsigned long> addresesWithMatchingValue;
    unsigned long playerStructAddress = 0;
    unsigned int pid = 0;
    bool run = true;
};

#endif
