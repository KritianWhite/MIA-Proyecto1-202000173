#ifndef FDISK_H
#define FDISK_H



//bool isNumber(char c);
//bool isLetter(char c);

char idMBR_ = 'a';
char nullChar_ = '\0';

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
