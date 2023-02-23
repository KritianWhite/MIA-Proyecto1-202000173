#ifndef FDISK_H
#define FDISK_H

#include <iostream>

using namespace std;

bool CrearParticion(string diskPath, string partitionName, int partitionSize, char sizeUnit, char partitionType, char partitionFit);
bool EliminarParticion(string diskPath, string partitionName, string deletionType);
bool isNumber(char c);
bool isLetter(char c);

struct Fdisk{
    bool acceso;
    string diskPath;
    string partitionName;
    int partitionSize;
    char sizeUnit;
    char partitionType;
    char partitionFit;
    string deletionType;
    bool resizePartition;
    int sizetoChange;
};

#endif // !FDISK_H
