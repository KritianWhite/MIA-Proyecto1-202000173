#ifndef LOGIN_H
#define LOGIN_H

#include <iostream>

using namespace std;

bool LoginUser(bool openSesion, string partitionId, string username, string password, PartitionNode *&firstNode);

struct Login
{
    bool acceso;
    string username;
    string password;
    string partitionId;
};


#endif // !LOGIN_H