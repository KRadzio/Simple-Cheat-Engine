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
    printf("Welcome to simple cheat engine \n\n");
    // print info
    Help();
    char opt;
    while (opt != 'q')
    {
        printf("Simple Cheat Engine Prompt>");
        std::cin >> opt;
        switch (opt)
        {
        case 'h':
            Help();
            break;
        case 'p':
            ChangePid();
            break;
        case 'v':
            DisplaySectorsToScan();
            break;
        case 's':
            ScanForValue();
            break;
        case 'a':
            Rescan();
            break;
        case 'l':
            DisplayMatches();
            break;
        case 'd':
            DisplayMemoryUnderAddress();
            break;
        case 'w':
            WriteToAddres();
            break;
        case 'f':
            AddNewValueToFreeze();
            break;
        case 'j':
            DisplayValuesToFreeze();
            break;
        case 'c':
            if (!freezeThread.joinable())
            {
                runFreezing = true;
                freezeThread = std::thread(&CheatEngine::FreezeValues, this);
            }
            else
                printf("Freezing alredy running\n");
            break;
        case 'e':
            StopFreezeThread();
            break;
        case 'u':
            RemoveValueToFreeze();
            break;
        case 'k':
            StopFreezeThread();
            valuesToFreeze.clear();
            break;
        case 'n':
            system("clear");
            break;
        case 'b':
            if (procName == DOOM)
            {
                FindPlayerStructAddress();
                runFreezing = true;
                freezeThread = std::thread(&CheatEngine::FreezeValues, this);
            }
            else
                printf("This option can not be used on other games\n");
            break;
        default:
            break;
        }
    }
    // the user may not have stoped it
    StopFreezeThread();
    valuesToFreeze.clear();
}

void CheatEngine::ChangePid()
{
    printf("Enter new pid\n");
    unsigned int tmp;
    std::cin >> tmp;
    if (tmp == 0)
    {
        printf("Pid can not be zero, the previous pid is not changed\n");
        return;
    }
    pid = tmp;
    ResetEverything();
    AddNewFilePaths();
    AddUsefulSectors();
    printf("\nNew pid set: %u\n", pid);
    printf("Determined which sectors to scann\n");
}

void CheatEngine::Help()
{
    // help menu
    printf("IMPORTANT: THIS INFO CAN BE DISPLAYED AFTER CHOOSING THE 'Help' OPTION\n");
    printf("h) Help\n");
    printf("p) Change pid\n");
    printf("v) Display sectors which will be scanned\n");
    printf("s) Scan for value\n");
    printf("a) Scan again\n");
    printf("l) List matches\n");
    printf("d) Print memory under address\n");
    printf("w) Write to memory address\n");
    printf("f) Add a value to freeze\n");
    printf("j) List values to freeze\n");
    printf("c) Continue the freezing\n");
    printf("e) Stop freezing\n");
    printf("u) Remove a value to freeze\n");
    printf("k) Clear all values to freeze and stop freezing\n");
    printf("n) Clear screen\n");
    printf("q) Quit\n");
    printf("WARNING: FREEZING VALUES WILL BE STOPED AFTER SCANNING OR READING/WRITING TO MEMORY\n");
    printf("If the game is Classic DOOM the following option is available\n");
    printf("b) Find player struct addres and auto add values to freeze\n");
    printf("NOTE: Finding the player struct addres should be done when ther is x < 10 matches\n");
    printf("NOTE: The player struct is found by inserting PLAYER HEALTH VALUE\n");
}

void CheatEngine::ResetSearch()
{
    matches = 0;
    valueToFind = 0;
    if (procName == DOOM)
        playerStructAddress = 0;
    addresesWithMatchingValue.clear();
}

void CheatEngine::DisplayMatches()
{
    for (auto m : addresesWithMatchingValue)
    {
        printf("Addres: %lx", m.address);
        if (m.type == EXECUTABLE)
            printf(" In code sector ");
        else
            printf(" In data sector ");
        printf("size in bytes: %d\n", m.size);
    }
}

void CheatEngine::DisplayValuesToFreeze()
{
    mutex.lock();
    for (auto vf : valuesToFreeze)
        printf("Addres: %lx value frozen: %ld size in bytes: %d\n", vf.address, vf.value, vf.size);
    mutex.unlock();
}

void CheatEngine::DisplaySectorsToScan()
{
    printf("Form left to right) Start: End: Flags: Offset: DevMajor: DevMinor: Incode: Name of the file:\n");
    for (auto s : sectorsToScan)
    {
        printf("%lx-%lx %c%c%c%c %x %x:%x %d %s", s.start, s.end, s.readAcces, s.writeAcces,
               s.exeAcces, s.protectedA, s.offset, s.devMajor, s.devMinor, s.incode, s.name);
        if (s.exeAcces == 'x')
            printf(" (code sector)\n");
        else
            printf(" (data sector)\n");
    }
}

void CheatEngine::DisplayMemoryUnderAddress()
{
    if (pid == 0)
    {
        printf("Cannot read the engine is not attached to a process\n");
        return;
    }
    if (!IsTheProcessRunning(pid))
    {
        printf("The process has ended can not scan\n");
        return;
    }

    if (freezeThread.joinable())
    {
        StopFreezeThread();
        printf("Freezing stoped to read memory\n");
    }

    printf("Enter address (IN HEX), and number of bytes ahead of the addres to display\n");
    printf("Memory will be rounded to eight bytes\n");
    unsigned long address;
    unsigned long numberOfBytes;
    std::cin >> std::hex >> address >> std::dec;
    std::cin >> numberOfBytes;
    int8_t currByte;

    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    for (unsigned long i = 0; i < numberOfBytes; i += sizeof(long))
    {
        long bytesRead = ptrace(PTRACE_PEEKDATA, pid, address + i, NULL);
        printf("%lx)", address + i);
        for (unsigned long i = 0; i < sizeof(long); i++)
        {
            currByte = bytesRead;
            bytesRead = bytesRead >> 8;
            printf(" %ux ", currByte);
        }
        printf("\n");
    }
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

void CheatEngine::WriteToAddres()
{
    if (pid == 0)
    {
        printf("Cannot write the engine is not attached to a process\n");
        return;
    }

    if (!IsTheProcessRunning(pid))
    {
        printf("The process has ended can not write\n");
        return;
    }
    if (freezeThread.joinable())
    {
        StopFreezeThread();
        printf("Freezing stoped to write a value to memory\n");
    }

    Value v;
    printf("Insert address to write in (IN HEX), value to be written and size in bytes. In that order\n");
    std::cin >> std::hex >> v.address >> std::dec;
    std::cin >> v.value;
    std::cin >> v.size;

    ptrace(PTRACE_ATTACH, pid, NULL, NULL);
    WriteValueBackToMemory(v);
    ptrace(PTRACE_DETACH, pid, NULL, NULL);
}

void CheatEngine::AddNewValueToFreeze()
{
    Value v;
    printf("Insert address (IN HEX), value at which the variable will be frozen and size in bytes. In that order\n");
    std::cin >> std::hex >> v.address >> std::dec;
    std::cin >> v.value;
    std::cin >> v.size;
    mutex.lock();
    for (auto vf : valuesToFreeze)
    {
        if (v.address == vf.address)
        {
            vf.size = v.size;
            vf.value = v.value;
            break;
        }
    }
    // not in the list
    valuesToFreeze.push_back(v);
    mutex.unlock();
}

void CheatEngine::RemoveValueToFreeze()
{
    printf("Insert the value address (IN HEX)\n");
    unsigned long address;
    std::cin >> std::hex >> address >> std::dec;
    mutex.lock();
    auto i = valuesToFreeze.begin();
    while (i != valuesToFreeze.end())
    {
        if (i->address == address)
            i = valuesToFreeze.erase(i);
        else
            i++;
    }
    mutex.unlock();
}

void CheatEngine::ScanForValue()
{
    if (sectorsToScan.empty())
    {
        printf("Can not scan the list is empty\n");
        return;
    }

    if (!IsTheProcessRunning(pid))
    {
        printf("The process has ended can not scan\n");
        return;
    }

    if (freezeThread.joinable())
    {
        StopFreezeThread();
        printf("Freezeing stoped in order to scan for new value\n");
    }
    ResetSearch();

    printf("Insert value to search\n");
    std::cin >> valueToFind;

    ptrace(PT_ATTACH, pid, NULL, NULL);
    int status;
    // wait for the process to stop
    waitpid(pid, &status, 0);

    // the value is unsigned and can be stored on diffrent amount of bytes
    int stepSize = 0;
    if (valueToFind <= 127 && valueToFind >= -128) // 255 one byte
        stepSize = 1;
    else if (valueToFind <= 32767 && valueToFind >= -32768) // 65535 // two bytes
        stepSize = 2;
    else if (valueToFind <= 2147483647 && valueToFind >= -2147483648) // 4294967295 // four bytes
        stepSize = 4;
    else // eight bytes
        stepSize = 8;

    int8_t oneByte;
    int16_t twoBytes;
    int32_t fourBytes;
    int64_t eightBytes;

    for (auto s : sectorsToScan)
    {
        printf("Scanning: %lx-%lx", s.start, s.end);

        if (s.exeAcces == 'x')
            printf(" (code sector) ");
        else
            printf(" (data sector) ");

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
                if (eightBytes == valueToFind || fourBytes == valueToFind || twoBytes == valueToFind || oneByte == valueToFind)
                {
                    Match m;
                    if (oneByte == valueToFind)
                        m.size = 1;
                    else if (twoBytes == valueToFind)
                        m.size = 2;
                    else if (fourBytes == valueToFind)
                        m.size = 4;
                    else
                        m.size = 8;
                    m.address = i + j;
                    if (s.exeAcces == 'x')
                        m.type = EXECUTABLE;
                    else
                        m.type = NOT_EXECUTABLE;
                    addresesWithMatchingValue.emplace_front(m);
                    matches++;
                }
                oneByte = 0;
                twoBytes = 0;
                fourBytes = 0;
                eightBytes = 0;
            }
        }
        printf(" DONE\n");
    }
    ptrace(PT_DETACH, pid, NULL, NULL);
    printf("Found %lu matches\n", matches);
}

void CheatEngine::Rescan()
{

    if (sectorsToScan.empty())
    {
        printf("Can not scan the list is empty\n");
        return;
    }

    if (!IsTheProcessRunning(pid))
    {
        printf("The process has ended can not scan\n");
        return;
    }
    if (freezeThread.joinable())
    {
        StopFreezeThread();
        printf("Freezeing stoped in order to scan for value\n");
    }

    printf("Enter value again after it changes\n");
    std::cin >> valueToFind;

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
        if ((i->address) % 8 == 0)
            eightBytes = ptrace(PTRACE_PEEKDATA, pid, i->address, 0);
        if ((i->address) % 4 == 0)
            fourBytes = ptrace(PTRACE_PEEKDATA, pid, i->address, 0);
        if ((i->address) % 2 == 0)
        {
            twoBytes = ptrace(PTRACE_PEEKDATA, pid, i->address, 0);
            oneByte = ptrace(PTRACE_PEEKDATA, pid, i->address, 0);
        }
        if ((i->address) % 2 == 1)
            oneByte = ptrace(PTRACE_PEEKDATA, pid, i->address, 0);
        if (eightBytes == valueToFind || fourBytes == valueToFind || twoBytes == valueToFind || oneByte == valueToFind)
        {
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
    printf("%lu matches after rescan\n", matches);
}

void CheatEngine::AddUsefulSectors()
{
    char *line = NULL;
    size_t len = 0;
    FILE *mapFile = fopen(filepath.c_str(), "r");

    if (mapFile == NULL && errno == ENOENT)
    {
        printf("Process with pid: %u does not exist\n", pid);
        return;
    }
    struct Sector s;

    while (getline(&line, &len, mapFile) != -1)
    {
        sscanf(line, "%lx-%lx %c%c%c%c %x %x:%x %d %s", &s.start, &s.end, &s.readAcces, &s.writeAcces,
               &s.exeAcces, &s.protectedA, &s.offset, &s.devMajor, &s.devMinor, &s.incode, s.name);

        if (strcmp(s.name, procName.c_str()) == 0)
        {
            sectorsToScan.push_back(s);
            printf("%lx-%lx %c%c%c%c %x %x:%x %d %s WILL BE SCANED", s.start, s.end, s.readAcces, s.writeAcces,
                   s.exeAcces, s.protectedA, s.offset, s.devMajor, s.devMinor, s.incode, s.name);
            if (s.exeAcces == 'x')
                printf(" (code sector)\n");
            else
                printf(" (data sector)\n");
        }
        else
            printf("%lx-%lx %c%c%c%c %x %x:%x %d %s WILL NOT BE SCANED\n", s.start, s.end, s.readAcces, s.writeAcces,
                   s.exeAcces, s.protectedA, s.offset, s.devMajor, s.devMinor, s.incode, s.name);
    }
    fclose(mapFile);
}

void CheatEngine::StopFreezeThread()
{
    mutex.lock();
    runFreezing = false;
    mutex.unlock();

    if (freezeThread.joinable())
        freezeThread.join();
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
    while (runL && IsTheProcessRunning(pid))
    {
        ptrace(PTRACE_INTERRUPT, pid, NULL, NULL);
        waitpid(pid, &status, 0);
        mutex.lock();
        if (procName == DOOM)
        {
            unsigned long mobjPtr = ptrace(PTRACE_PEEKDATA, pid, playerStructAddress, 0);
            // the player either restarted the map or changed map or loaded a save
            if (!valuesToFreeze.empty())
            {
                if (mobjPtr != valuesToFreeze.begin()->address - 196)
                {
                    valuesToFreeze.begin()->address = mobjPtr + 196;
                    AddFullArsenal();
                }
            }
        }
        for (auto v : valuesToFreeze)
            WriteValueBackToMemory(v);
        mutex.unlock();
        ptrace(PTRACE_CONT, pid, NULL, NULL);
        // sleep fore half a second to not update to often
        usleep(500000);
        mutex.lock();
        if (!runFreezing)
            runL = 0;
        mutex.unlock();
    }
    ptrace(PT_DETACH, pid, NULL, NULL);
    if (!IsTheProcessRunning(pid))
        printf("Freezing stoped becouse the process finished\n");
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
    if (!IsTheProcessRunning(pid))
    {
        printf("The process has ended can not find player struct\n");
        return;
    }
    if (freezeThread.joinable())
    {
        StopFreezeThread();
        printf("Freezeing stoped in order to search for player struct\n");
    }
    unsigned long address = 0;
    int status;

    ptrace(PT_ATTACH, pid, NULL, NULL);
    waitpid(pid, &status, 0);
    // search relative to health pos
    for (auto i : addresesWithMatchingValue)
    {
        int ammo1 = ptrace(PTRACE_PEEKDATA, pid, i.address + 220, 0);
        int ammo2 = ptrace(PTRACE_PEEKDATA, pid, i.address + 224, 0);
        // the player may have backpack
        if ((ammo1 == 200 && ammo2 == 50) || (ammo1 == 400 && ammo2 == 100))
        {
            address = i.address;
            break;
        }
    }
    if (address != 0)
    {
        // clear to avoid dupes
        if (!valuesToFreeze.empty())
        {
            valuesToFreeze.clear();
            printf("All values deleted. Inserted player specific values\n");
        }
        // the player struct in DOOM starts 44 bytes before the health value
        playerStructAddress = address - 44;
        printf("Player struct address found: %lx \n", playerStructAddress);
        AddFullArsenal();
        AddPlayerValuesToFreeze();
        ptrace(PT_DETACH, pid, NULL, NULL);
    }
    else
        printf("Can not find player\n");
}

void CheatEngine::AddFullArsenal()
{
    Value v;
    v.size = 4;
    v.value = 1;
    for (int i = 0; i < 9; i++)
    {
        v.address = playerStructAddress + 204 + i * 4;
        WriteValueBackToMemory(v);
    }
    printf("Added all weapons\n");
}

void CheatEngine::AddPlayerValuesToFreeze()
{
    mutex.lock();
    // relative to the begining of the struct
    // mobj
    Value v;
    unsigned long mobjPtr = ptrace(PTRACE_PEEKDATA, pid, playerStructAddress, 0);
    v.address = mobjPtr + 196;
    v.value = 200;
    v.size = 4;
    valuesToFreeze.push_back(v);
    // player health
    v.address = playerStructAddress + 44;
    v.value = 200;
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
    // bullet ammo
    v.address = playerStructAddress + 240;
    v.value = 400;
    v.size = 4;
    valuesToFreeze.push_back(v);
    // shell ammo
    v.address = playerStructAddress + 244;
    v.value = 100;
    v.size = 4;
    valuesToFreeze.push_back(v);
    // plasma ammo
    v.address = playerStructAddress + 248;
    v.value = 600;
    v.size = 4;
    valuesToFreeze.push_back(v);
    // rocket ammo
    v.address = playerStructAddress + 252;
    v.value = 100;
    v.size = 4;
    valuesToFreeze.push_back(v);
    mutex.unlock();
    printf("Added player health, armor and ammo to freeze list\n");
}

void CheatEngine::ResetEverything()
{
    StopFreezeThread();
    addresesWithMatchingValue.clear();
    valuesToFreeze.clear();
    sectorsToScan.clear();
    valueToFind = 0;
    matches = 0;
    playerStructAddress = 0;
    filepath = "";
    procName = "";
    runFreezing = true;
}

void CheatEngine::AddNewFilePaths()
{
    filepath = "/proc/" + std::to_string(pid) + "/exe";
    char tmp[MAX_PATH_LENGTH];
    int length = readlink(filepath.c_str(), tmp, MAX_PATH_LENGTH - 1);
    tmp[length] = '\0';
    procName = tmp;
    printf("New proces name: %s \n", procName.c_str());
    filepath = "/proc/" + std::to_string(pid) + "/maps";
}

bool CheatEngine::IsTheProcessRunning(unsigned int pid)
{
    std::string path = "/proc/" + std::to_string(pid) + "/status";
    FILE *file = fopen(path.c_str(), "r");
    if (file == NULL && errno == ENOENT)
        return false;
    char *line = NULL;
    char status;
    unsigned long len;

    while (getline(&line, &len, file) != -1)
    {
        if (strncmp(line, "State:\t", sizeof("State:\t") - 1) == 0)
        {
            status = line[sizeof("State:\t") - 1];
            break;
        }
    }
    fclose(file);
    if (status == 'Z' || status == 'X')
        return false;
    else
        return true;
}
