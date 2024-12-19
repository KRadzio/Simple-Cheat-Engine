#ifndef CHEAT_ENGINE_HPP
#define CHEAT_ENGINE_HPP

// standard c libraires
#include <unistd.h>
#include <dirent.h>
#include <string.h>

// system c libraires
#include <errno.h>
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
#include "Value.hpp"
#include "Match.hpp"

#define DOOM "/usr/bin/dsda-doom"
#define MAX_PATH_LENGTH 128
#define BUFF_SIZE 128


class CheatEngine
{
public:
    static CheatEngine &GetInstance();
    void MainLoop();

// helper methods
private:
    void ChangePid();
    void Help();
    void ResetSearch();
    void DisplayMatches();
    void DisplayValuesToFreeze();
    void DisplaySectorsToScan();
    void DisplayMemoryUnderAddress();
    void WriteToAddres();
    void AddNewValueToFreeze();
    void RemoveValueToFreeze();
    void ScanForValue();
    void Rescan();
    void AddUsefulSectors();
    void StopFreezeThread();
    void FreezeValues();
    void WriteValueBackToMemory(Value &v);
    void ResetEverything();
    void AddNewFilePaths();
    bool IsTheProcessRunning(unsigned int pid);

    // DOOM specific
private:
    void FindPlayerStructAddress();
    void AddFullArsenal();
    void AddPlayerValuesToFreeze();

private:
    CheatEngine();
    ~CheatEngine();

private:
    // thread
    std::mutex mutex;
    std::thread freezeThread;
    // to agregate data
    std::list<Match> addresesWithMatchingValue; // clear after search is done or if the user wants to reset it
    std::list<Value> valuesToFreeze; // clear if the user wants it to or if the game closed
    std::list<Sector> sectorsToScan; // clear only after changing pid
    // search
    long valueToFind = 0;
    unsigned long matches = 0;
    unsigned long playerStructAddress = 0; // used in cheats for DOOM (the player struct is known so we can calculate the offsets from its start)
    unsigned int pid = 0;
    // other
    bool runMainLoop = true;
    bool runFreezing = true;
    std::string filepath = "";
    std::string procName = "";
};

#endif
