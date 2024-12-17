#ifndef SECTOR_HPP
#define SECTOR_HPP

#define BUFF_SIZE 128

struct sector
{
    unsigned long start;
    unsigned long end;
    char readAcces;
    char writeAcces;
    char exeAcces;
    char protectedA;
    int offset;
    int devMajor;
    int devMinor;
    int incode; // zero = unused
    char name[BUFF_SIZE];
};

#endif
