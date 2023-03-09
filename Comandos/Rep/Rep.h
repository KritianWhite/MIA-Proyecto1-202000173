#ifndef REP_H
#define REP_H

#include <iostream>

using namespace std;

bool CrearReporte(string graphvizString, string reportPath);
bool CrearReporte_Texto(string fileText, string reportPath);
struct Rep
{
    bool acceso;
    string reportName;
    string reportPath;
    string partitionId;
    string directoryPath;
};


#endif // !REP_H