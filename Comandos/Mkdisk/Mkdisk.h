#ifndef MKDISK_H
#define MKDISK_H

#include <iostream>

using namespace std;

struct Mkdisk{
    bool acceso;
    string diskPath;
    string diskName;
    int diskSize;
    char diskFit; // F (default) | B | W
    char sizeUnit; // K | M (default)
};

#endif // !MKDISK_H
