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
    // ImGui::Begin("Test",NULL);
    // ImGui::Text("aaaaaaaaaa");
    // ImGui::End();
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

    freezeThread = std::thread(&CheatEngine::FreezeHealth, this);

    printf("enter any key to stop\n");

    getchar();
    getchar();

    mutex.lock();
    run = false;
    mutex.unlock();

    freezeThread.join();
}

void CheatEngine::AddUsefulSectors()
{
    char *line = NULL;
    size_t len = 0;
    FILE *mapFile = fopen(filepath.c_str(), "r");
    struct sector s;

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

void CheatEngine::FindPlayerStructAddress()
{
    unsigned long address = 0;
    int status;
    ptrace(PT_ATTACH, pid, NULL, NULL);
    waitpid(pid, &status, 0);
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
    ptrace(PT_DETACH, pid, NULL, NULL);
    addresesWithMatchingValue.clear();
    // the player struct in DOOM starts 44 bytes before the health value
    playerStructAddress = address - 44;
}

void CheatEngine::FreezeHealth()
{
    unsigned int pid;
    unsigned long playerStructAddress;
    mutex.lock();
    pid = this->pid;
    playerStructAddress = this->playerStructAddress;
    mutex.unlock();

    ptrace(PTRACE_SEIZE, pid, NULL, NULL);
    int status;
    int runL = 1;
    while (runL)
    {
        ptrace(PTRACE_INTERRUPT, pid, NULL, NULL);
        waitpid(pid, &status, 0);
        // the health is stored in int (4 bytes) however ptrace writes a long value (and long has 8 bytes in linux)
        long health = ptrace(PTRACE_PEEKDATA, pid, playerStructAddress + 44, 0);
        long otherValue = health & 0xFFFFFFFF00000000;
        health = health & 0x00000000FFFFFFFF;
        if (health < 800)
        {
            // pointers are eight bytes long
            unsigned long mobjPtr = ptrace(PTRACE_PEEKDATA, pid, playerStructAddress, 0);
            long newhealth = 800 + otherValue; // we have to save the other value without overwriting it
            ptrace(PTRACE_POKEDATA, pid, playerStructAddress + 44, newhealth);
            // 
            health = ptrace(PTRACE_PEEKDATA, pid, mobjPtr + 196, 0);
            otherValue = health & 0xFFFFFFFF00000000;
            health = health & 0x00000000FFFFFFFF;
            newhealth = 800 + otherValue;
            ptrace(PTRACE_POKEDATA, pid, mobjPtr + 196, newhealth);
        }
        ptrace(PTRACE_CONT, pid, NULL, NULL);
        sleep(1);
        mutex.lock();
        if (!run)
            runL = 0;
        mutex.unlock();
    }
    ptrace(PT_DETACH, pid, NULL, NULL);
}