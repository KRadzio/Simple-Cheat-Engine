#include <stdio.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <dirent.h>

#define BUFF_SIZE 128

struct sector
{
    unsigned long start;
    unsigned long end;
    char readAcces;
    char writeAcces;
    char exeAcces;
    char protected;
    int offset;
    int devMajor;
    int devMinor;
    int incode; // zero = unused
    char name[BUFF_SIZE];
};

int main()
{
    unsigned int pid = 0;

    char path[BUFF_SIZE];
    char path2[BUFF_SIZE];
    char *line = NULL;
    size_t len = 0;

    scanf("%u", &pid);

    ptrace(PT_ATTACH, pid, NULL, NULL);

    snprintf(path, sizeof(path), "/proc/%u/maps", pid);

    snprintf(path2, sizeof(path2), "/proc/%u/map_files", pid);

    DIR *dir = opendir(path2);

    FILE *mapFile = fopen(path, "r");

    struct sector s;

    int lineCounter = 0;

    long value = 0;

    scanf("%ld", &value);

    int matches = 0;

    struct dirent *dn;

    while ((dn = readdir(dir)) != NULL)
    {
        sscanf(dn->d_name, "%lx-%lx", &s.start, &s.end);
        for (unsigned long i = s.start; i < s.end; i += sizeof(long))
        {
            int valueFound = ptrace(PTRACE_PEEKDATA, pid, i, 0);
            // printf("%ld\n", valueFound);
            if (valueFound == value)
                matches++;
        }
        // printf("%s\n", dn->d_name);
    }

    while (getline(&line, &len, mapFile) != -1)
    {
        // printf("%s", line);

        sscanf(line, "%lx-%lx %c%c%c%c %x %x:%x %d %s", &s.start, &s.end, &s.readAcces, &s.writeAcces,
               &s.exeAcces, &s.protected, &s.offset, &s.devMajor, &s.devMinor, &s.incode, s.name);

        // if (s.incode != 0)
        //{

        //}
        // printf("%d) %lx-%lx %c%c%c%c %x %x:%x %d %s \n", lineCounter, s.start, s.end, s.readAcces, s.writeAcces,
        // s.exeAcces, s.protected, s.offset, s.devMajor, s.devMinor, s.incode, s.name);
        lineCounter++;
    }

    fclose(mapFile);

    closedir(dir);
    // sizeof(long) == 8

    printf("Found %d matches\n", matches);

    // sleep(10);

    ptrace(PT_DETACH, pid, NULL, NULL);

    return 0;
}