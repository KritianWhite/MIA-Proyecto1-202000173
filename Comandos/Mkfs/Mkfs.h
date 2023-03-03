#ifndef MKFS_H
#define MKFS_H

#include <iostream>

using namespace std;


struct Mkfs
{
    bool acceso;
    string partitionId;
    string formatType; //* Full
    int fileSystem;    //* 2 | 3 (2fs | 3fs en comando)
};



#endif // !MKFS_H