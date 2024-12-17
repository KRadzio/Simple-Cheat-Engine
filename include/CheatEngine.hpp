#ifndef CHEAT_ENGINE_HPP
#define CHEAT_ENGINE_HPP

// standard c libraires
#include <unistd.h>
#include <dirent.h>
#include <string.h>

// system c libraires
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>

// standard c++ libraires
#include <iostream>
#include <string>
#include <list>
#include <thread>
#include <mutex>

// project headers
#include "Sector.hpp"
#include "Gui.hpp"
#include "Value.hpp"

#define DOOM "/usr/bin/dsda-doom"
#define MAX_PATH_LENGTH 128

class CheatEngine
{
public:
    static CheatEngine &GetInstance();
    void MainLoop();

// helper methods
private:
    void AddUsefulSectors();
    void ScanForValue();
    void Rescan();
    void FreezeValues();
    void WriteValueBackToMemory(Value& v);

// DOOM specific
private:
     void FindPlayerStructAddress();
    void AddValuesToFreeze();

private:
    CheatEngine();
    ~CheatEngine();

private:
    // thread
    std::mutex mutex;
    std::thread freezeThread;
    // to agregate date
    std::list<unsigned long> addresesWithMatchingValue;
    std::list<Value> valuesToFreeze;
    std::list<Sector> sectorsToScan;
    // search
    long value = 0;
    unsigned long matches;
    unsigned long playerStructAddress = 0; // cheats for DOOM (the player struct is known so we can calculate the offsets from its start)
    unsigned int pid = 0;
    // other
    bool run = true;
    std::string filepath;
    std::string procName;
};

#endif
