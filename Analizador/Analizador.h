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
    mount_command = 5,
    unmount_command = 6,
    mkfs_command = 7,
    login_command = 8,
    logout_command = 9,
    mkgrp_command = 10,
    rmgrp_command = 11,
    mkusr_command = 12,
    rmusr_command = 13,
    mkfile_command = 14,
    cat_command = 15,
    rep_command = 16
};



#endif // ANALIZADOR_H