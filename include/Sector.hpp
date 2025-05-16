#ifndef SECTOR_HPP
#define SECTOR_HPP

#define BUFF_SIZE 128

struct Sector
{
    unsigned long start;
    unsigned long end;
    char readAcces;
    char writeAcces;
    char exeAcces;
    char protectedA;
    unsigned int offset;
    unsigned int devMajor;
    unsigned int devMinor;
    int incode; // zero = unused
    char name[BUFF_SIZE];
};

#endif
