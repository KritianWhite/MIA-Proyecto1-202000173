#ifndef MKFILE_H
#define MKFILE_H

#include <iostream>

using namespace std;

struct Mkfile
{
    bool acceso;
    string filePath;
    bool createParentFolders;
    int fileSize;
    string contentPath;
};


#endif // !MKFILE_H