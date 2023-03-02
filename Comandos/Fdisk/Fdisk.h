#ifndef FDISK_H
#define FDISK_H

bool CrearParticion(string diskPath, string partitionName, int partitionSize, char sizeUnit, char partitionType, char partitionFit);
bool EliminarParticion(string diskPath, string partitionName, string deletionType);

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

#endif // FDISK_H
