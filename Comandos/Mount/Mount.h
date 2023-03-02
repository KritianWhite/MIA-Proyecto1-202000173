#ifndef MOUNT_H
#define MOUNT_H

#include <iostream>

using namespace std;



struct Mount{
    bool acceso;
    string diskPath;
    string diskName;
    string partitionName;
};

struct MountedPartition{
    string partitionId;
    string partitionName; // para obtener el vínculo entre id y partición en MBR
    string diskPath; // para acceder al disco en donde se encuentra la partición
};

struct PartitionNode{
    MountedPartition mountedP;
    PartitionNode* next;
};




#endif // !MOUNT_H
