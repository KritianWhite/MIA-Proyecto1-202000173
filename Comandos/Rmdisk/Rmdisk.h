#ifndef RMDISK_H
#define RMDISK_H

#include <iostream>
#include <string.h>
#include <cctype>

using namespace std;

bool EliminarDisco(string path);

struct Rmdisk{
    bool acceso;
    string path;
};


#endif // !RMDISK_H
