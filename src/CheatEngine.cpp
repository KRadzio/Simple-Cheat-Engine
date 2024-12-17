#include "CheatEngine.hpp"

CheatEngine::CheatEngine() {}
CheatEngine::~CheatEngine() {}

CheatEngine &CheatEngine::GetInstance()
{
    static CheatEngine *instance = new CheatEngine;
    return *instance;
}

void CheatEngine::MainLoop()
{
    Value v;
    v.address = 1000;
    v.size = 2;
    v.value = 5;
    WriteValueBackToMemory(v);
    std::cout << "Enter pid \n";
    std::cin >> pid;
    filepath = "/proc/" + std::to_string(pid) + "/exe";
    char tmp[MAX_PATH_LENGTH];
    int length = readlink(filepath.c_str(), tmp, MAX_PATH_LENGTH - 1);
    tmp[length] = '\0';
    procName = tmp;
    std::cout << procName << std::endl;

    filepath = "/proc/" + std::to_string(pid) + "/maps";
    AddUsefulSectors();

    std::cout << "Insert health value\n";
    std::cin >> value;

    ScanForValue();

    std::cout << "Found: " << matches << " matches\n";

    std::cout << "Enter health value again after it changes\n";
    std::cin >> value;
    Rescan();

    FindPlayerStructAddress();

    printf("%lx \n", playerStructAddress);

    freezeThread = std::thread(&CheatEngine::FreezeValues, this);

    printf("enter any key to stop\n");

    getchar();
    getchar();

    mutex.lock();
    run = false;
    mutex.unlock();

    if (freezeThread.joinable())
        freezeThread.join();
    valuesToFreeze.clear();
}

void CheatEngine::AddUsefulSectors()
{
    char *line = NULL;
    size_t len = 0;
    FILE *mapFile = fopen(filepath.c_str(), "r");
    struct Sector s;

    while (getline(&line, &len, mapFile) != -1)
    {
        sscanf(line, "%lx-%lx %c%c%c%c %x %x:%x %d %s", &s.start, &s.end, &s.readAcces, &s.writeAcces,
               &s.exeAcces, &s.protectedA, &s.offset, &s.devMajor, &s.devMinor, &s.incode, s.name);

        if (strcmp(s.name, procName.c_str()) == 0)
        {
            sectorsToScan.push_back(s);
            printf("%lx-%lx %c%c%c%c %x %x:%x %d %s \n", s.start, s.end, s.readAcces, s.writeAcces,
                   s.exeAcces, s.protectedA, s.offset, s.devMajor, s.devMinor, s.incode, s.name);
        }
    }
    fclose(mapFile);
}

void CheatEngine::ScanForValue()
{
    ptrace(PT_ATTACH, pid, NULL, NULL);
    int status;
    // wait for the process to stop
    waitpid(pid, &status, 0);

    // the value is unsigned and can be stored on diffrent amount of bytes
    int stepSize = 0;
    if (value <= 127 && value >= -128) // 255 one byte
        stepSize = 1;
    else if (value <= 32767 && value >= -32768) // 65535 // two bytes
        stepSize = 2;
    else if (value <= 2147483647 && value >= -2147483648) // 4294967295 // four bytes
        stepSize = 4;
    else // eight bytes
        stepSize = 8;

    int8_t oneByte;
    int16_t twoBytes;
    int32_t fourBytes;
    int64_t eightBytes;

    for (auto s : sectorsToScan)
    {
        printf("%lx %lx\n", s.start, s.end);
        for (unsigned long i = s.start; i < s.end; i += sizeof(long))
        {
            for (unsigned long j = 0; j < sizeof(long); j += stepSize)
            {
                if ((i + j) % 8 == 0)
                    eightBytes = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
                if ((i + j) % 4 == 0)
                    fourBytes = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
                if ((i + j) % 2 == 0)
                {
                    twoBytes = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
                    oneByte = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
                }
                if ((i + j) % 2 == 1)
                    oneByte = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
                if (eightBytes == value || fourBytes == value || twoBytes == value || oneByte == value)
                {
                    addresesWithMatchingValue.emplace_front(i + j);
                    matches++;
                }
                oneByte = 0;
                twoBytes = 0;
                fourBytes = 0;
                eightBytes = 0;
            }
        }
    }
    ptrace(PT_DETACH, pid, NULL, NULL);
}

void CheatEngine::Rescan()
{
    int status;

    ptrace(PT_ATTACH, pid, NULL, NULL);
    waitpid(pid, &status, 0);
    matches = 0;

    int8_t oneByte;
    int16_t twoBytes;
    int32_t fourBytes;
    int64_t eightBytes;

    auto i = addresesWithMatchingValue.begin();
    while (i != addresesWithMatchingValue.end())
    {
        // the value is unsigned and can be stored on diffrent amount of bytes
        // this time the number of bytes can be determined by the addres the value is at
        if ((*i) % 8 == 0)
            eightBytes = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
        if ((*i) % 4 == 0)
            fourBytes = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
        if ((*i) % 2 == 0)
        {
            twoBytes = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
            oneByte = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
        }
        if ((*i) % 2 == 1)
            oneByte = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
        if (eightBytes == value || fourBytes == value || twoBytes == value || oneByte == value)
        {
            printf("%lx\n", *i);
            i++;
            matches++;
        }
        else
            i = addresesWithMatchingValue.erase(i);
        oneByte = 0;
        twoBytes = 0;
        fourBytes = 0;
        eightBytes = 0;
    }
    ptrace(PT_DETACH, pid, NULL, NULL);
}

void CheatEngine::FreezeValues()
{
    unsigned int pid;
    mutex.lock();
    pid = this->pid;
    mutex.unlock();

    ptrace(PTRACE_SEIZE, pid, NULL, NULL);
    int status;
    int runL = 1;
    while (runL)
    {
        ptrace(PTRACE_INTERRUPT, pid, NULL, NULL);
        waitpid(pid, &status, 0);
        mutex.lock();
        for (auto v : valuesToFreeze)
            WriteValueBackToMemory(v);
        mutex.unlock();
        ptrace(PTRACE_CONT, pid, NULL, NULL);
        sleep(1);
        mutex.lock();
        if (!run)
            runL = 0;
        mutex.unlock();
    }
    ptrace(PT_DETACH, pid, NULL, NULL);
}

void CheatEngine::WriteValueBackToMemory(Value &v)
{
    long mask = 0xFFFFFFFFFFFFFFFF;
    mask = mask << (v.size * 8);
    long newValue = ptrace(PTRACE_PEEKDATA, pid, v.address, 0);
    // to not override the other bytes that may be used by other things in game
    newValue = newValue & mask;
    newValue += v.value;
    ptrace(PTRACE_POKEDATA, pid, v.address, newValue);
}

void CheatEngine::FindPlayerStructAddress()
{
    unsigned long address = 0;
    int status;
    ptrace(PT_ATTACH, pid, NULL, NULL);
    waitpid(pid, &status, 0);
    // search relative to health pos
    for (auto i : addresesWithMatchingValue)
    {
        int ammo1 = ptrace(PTRACE_PEEKDATA, pid, i + 220, 0);
        int ammo2 = ptrace(PTRACE_PEEKDATA, pid, i + 224, 0);
        // the player may have backpack
        if ((ammo1 == 200 && ammo2 == 50) || (ammo1 == 400 && ammo2 == 100))
        {
            address = i;
            break;
        }
    }

    // the player struct in DOOM starts 44 bytes before the health value
    playerStructAddress = address - 44;
    AddValuesToFreeze();
    ptrace(PT_DETACH, pid, NULL, NULL);
    addresesWithMatchingValue.clear();
}

void CheatEngine::AddValuesToFreeze()
{
    // relative to the begining of the struct
    // mobj
    Value v;
    unsigned long mobjPtr = ptrace(PTRACE_PEEKDATA, pid, playerStructAddress, 0);
    v.address = mobjPtr + 196;
    v.value = 800;
    v.size = 4;
    valuesToFreeze.push_back(v);
    // player health
    v.address = playerStructAddress + 44;
    v.value = 800;
    v.size = 4;
    valuesToFreeze.push_back(v);
    // player armor
    v.address = playerStructAddress + 48;
    v.value = 200;
    v.size = 4;
    valuesToFreeze.push_back(v);
    //  armor flag
    v.address = playerStructAddress + 64;
    v.value = 2;
    v.size = 4;
    valuesToFreeze.push_back(v);
    //  ammo
    v.address = playerStructAddress + 252;
    v.value = 100;
    v.size = 4;
    valuesToFreeze.push_back(v);
}