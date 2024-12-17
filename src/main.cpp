
#include "CheatEngine.hpp"


// std::list<unsigned long> addresses;
// pthread_mutex_t mutex;
// int run = 1;

// struct sector
// {
//     unsigned long start;
//     unsigned long end;
//     char readAcces;
//     char writeAcces;
//     char exeAcces;
//     char protectedA;
//     int offset;
//     int devMajor;
//     int devMinor;
//     int incode; // zero = unused
//     char name[BUFF_SIZE];
// };

// struct dataForThread
// {
//     unsigned int pid;
//     unsigned long playerStructAddress;
// };

// // think about how to get the value
// void ScanForValue(unsigned long value, unsigned int pid, int *matches, struct sector *s)
// {

//     ptrace(PT_ATTACH, pid, NULL, NULL);
//     int status;

//     waitpid(pid, &status, 0);
//     int stepSize = 0;
//     if (value <= 0xFF) // 255 one byte
//         stepSize = 1;
//     else if (value <= 0xFFFF) // 65535 // two bytes
//         stepSize = 2;
//     else if (value <= 0xFFFFFFFF) // 4294967295 // four bytes
//         stepSize = 4;
//     else // eight bytes
//         stepSize = 8;

//     u_int8_t oneByte;
//     u_int16_t twoBytes;
//     u_int32_t fourBytes;
//     u_int64_t eightBytes;

//     printf("%lx %lx\n", s->start, s->end);
//     printf("%lu\n", sizeof(long));

//     for (unsigned long i = s->start; i < s->end; i += sizeof(long))
//     {
//         for (unsigned long j = 0; j < sizeof(long); j += stepSize)
//         {
//             if ((i + j) % 8 == 0)
//                 eightBytes = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
//             if ((i + j) % 4 == 0)
//                 fourBytes = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
//             if ((i + j) % 2 == 0)
//             {
//                 twoBytes = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
//                 oneByte = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
//             }
//             if ((i + j) % 2 == 1)
//                 oneByte = ptrace(PTRACE_PEEKDATA, pid, i + j, 0);
//             if (eightBytes == value || fourBytes == value || twoBytes == value || oneByte == value)
//             {
//                 addresses.emplace_front(i + j);
//                 (*matches)++;
//             }
//             oneByte = 0;
//             twoBytes = 0;
//             fourBytes = 0;
//             eightBytes = 0;
//         }
//     }
//     printf("Found %d matches\n", *matches);
//     ptrace(PT_DETACH, pid, NULL, NULL);

//     // 55d3afc217ec
// }

// void Rescan(unsigned long value, unsigned int pid, int *matches)
// {
//     int status;

//     ptrace(PT_ATTACH, pid, NULL, NULL);
//     waitpid(pid, &status, 0);
//     *matches = 0;

//     u_int8_t oneByte;
//     u_int16_t twoBytes;
//     u_int32_t fourBytes;
//     u_int64_t eightBytes;

//     auto i = addresses.begin();
//     while (i != addresses.end())
//     {
//         if ((*i) % 8 == 0)
//             eightBytes = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
//         if ((*i) % 4 == 0)
//             fourBytes = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
//         if ((*i) % 2 == 0)
//         {
//             twoBytes = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
//             oneByte = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
//         }
//         if ((*i) % 2 == 1)
//             oneByte = ptrace(PTRACE_PEEKDATA, pid, *i, 0);
//         if (eightBytes == value || fourBytes == value || twoBytes == value || oneByte == value)
//         {
//             printf("%lx\n", *i);
//             i++;
//             (*matches)++;
//         }
//         else
//             i = addresses.erase(i);
//         oneByte = 0;
//         twoBytes = 0;
//         fourBytes = 0;
//         eightBytes = 0;
//     }
//     printf("Found %d matches\n", *matches);
//     ptrace(PT_DETACH, pid, NULL, NULL);
// }

// unsigned long FindPlayerStruct(unsigned int pid)
// {
//     unsigned long address = 0;
//     int status;
//     ptrace(PT_ATTACH, pid, NULL, NULL);
//     waitpid(pid, &status, 0);
//     for (auto i : addresses)
//     {
//         int ammo1 = ptrace(PTRACE_PEEKDATA, pid, i + 220, 0);
//         int ammo2 = ptrace(PTRACE_PEEKDATA, pid, i + 224, 0);
//         if (ammo1 == 200 && ammo2 == 50)
//         {
//             address = i;
//             break;
//         }
//     }
//     ptrace(PT_DETACH, pid, NULL, NULL);
//     addresses.clear();
//     return address;
// }

// void *FreezeHealth(void *data)
// {
//     struct dataForThread *inter = (struct dataForThread *)data;
//     unsigned int pid = inter->pid;
//     unsigned long playerStructAddress = inter->playerStructAddress;

//     ptrace(PTRACE_SEIZE, pid, NULL, NULL);
//     int status;
//     int runL = 1;
//     while (runL)
//     {
//         ptrace(PTRACE_INTERRUPT, pid, NULL, NULL);
//         waitpid(pid, &status, 0);
//         int health = ptrace(PTRACE_PEEKDATA, pid, playerStructAddress, 0);
//         if (health < 800)
//         {
//             unsigned long mobjPtr = ptrace(PTRACE_PEEKDATA, pid, playerStructAddress - 44, 0);
//             int newhealth = 800;
//             ptrace(PTRACE_POKEDATA, pid, mobjPtr + 196, newhealth);
//             ptrace(PTRACE_POKEDATA, pid, playerStructAddress, newhealth);
//         }
//         ptrace(PTRACE_CONT, pid, NULL, NULL);
//         sleep(1);
//         pthread_mutex_lock(&mutex);
//         if (!run)
//             runL = 0;
//         pthread_mutex_unlock(&mutex);
//     }
//     ptrace(PT_DETACH, pid, NULL, NULL);

//     return NULL;
// }

int main()
{
    CheatEngine::GetInstance().MainLoop();
    // pthread_mutex_init(&mutex, NULL);

    // unsigned int pid = 0;

    // char path[BUFF_SIZE];
    // char name[BUFF_SIZE];
    // char nameT[BUFF_SIZE];
    // int nameLength;
    // char *line = NULL;
    // size_t len = 0;

    // scanf("%u", &pid);

    // snprintf(path, sizeof(path), "/proc/%u/maps", pid);

    // snprintf(name, sizeof(name), "/proc/%u/exe", pid);

    // nameLength = readlink(name, nameT, BUFF_SIZE - 1);

    // printf("%s\n", nameT);

    // // DIR *dir = opendir(path2);

    // FILE *mapFile = fopen(path, "r");

    // struct sector s;

    // int lineCounter = 0;

    // unsigned long value = 0;

    // scanf("%lu", &value);

    // int matches = 0;

    // while (getline(&line, &len, mapFile) != -1)
    // {
    //     sscanf(line, "%lx-%lx %c%c%c%c %x %x:%x %d %s", &s.start, &s.end, &s.readAcces, &s.writeAcces,
    //            &s.exeAcces, &s.protectedA, &s.offset, &s.devMajor, &s.devMinor, &s.incode, s.name);

    //     if (strcmp(s.name, nameT) == 0)

    //     {
    //         ScanForValue(value, pid, &matches, &s);
    //         printf("%d) %lx-%lx %c%c%c%c %x %x:%x %d %s \n", lineCounter, s.start, s.end, s.readAcces, s.writeAcces,
    //                s.exeAcces, s.protectedA, s.offset, s.devMajor, s.devMinor, s.incode, s.name);
    //     }

    //     lineCounter++;
    // }
    // fclose(mapFile);

    // scanf("%lu", &value);

    // Rescan(value, pid, &matches);

    // scanf("%lu", &value);

    // Rescan(value, pid, &matches);

    // unsigned long playerStructAddress = FindPlayerStruct(pid);

    // printf("%lx \n", playerStructAddress);

    // struct dataForThread dt;

    // dt.pid = pid;
    // dt.playerStructAddress = playerStructAddress;

    // pthread_t freezeThread;

    // pthread_create(&freezeThread, NULL, FreezeHealth, (void *)(&dt));

    // printf("enter any key to stop\n");

    // getchar();
    // getchar();

    // pthread_mutex_lock(&mutex);
    // run = 0;
    // pthread_mutex_unlock(&mutex);

    // pthread_join(freezeThread, NULL);

    // closedir(dir);
    // sizeof(long) == 8

    // sleep(10);

    return 0;
}