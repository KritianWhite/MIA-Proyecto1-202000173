#ifndef ANALIZADOR_H
#define ANALIZADOR_H

#include <iostream>
#include <cstdlib> // rand();
#include <string.h>
#include <fstream>
#include <limits>

using namespace std;

// TODO --> Declaracion de funciones para el archivo Analizador.cpp
int Analizador(char *Comando, bool esScript);


// TODO --> COMANDOS
enum commands {
    execute_command = 1,
    mkdisk_command = 2,
    rmdisk_command = 3,
    fdisk_command = 4,
    mount_command = 5
};



#endif // ANALIZADOR_H