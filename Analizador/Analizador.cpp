#include <iostream>
#include <cstdlib> // rand();
#include <string.h>
#include <fstream>
#include <limits>

#include "Analizador.h"

using namespace std;

bool sesionAbierta = false;
string nombreUsuario = "", pariticionID = "";

int Analizador(char *Comando, bool esScript){

    // Creamos unas variables de control
    int produccion = 1, cmmd = 0;
    bool incompleto = true, listo = false;

    // Vamos a separar el comando por partes por medio del espacio
    string nuevoComando = Comando;
    char *parte;
    parte = strtok(Comando, " ");

    while (parte != NULL){
        //& Primeramente vamos a reconocer el tipo de comando
        if (produccion == 1){
            // Comando execute
            if (strcasecmp(parte, "execute")==0){produccion = 2; cmmd = execute_command;}
            // Comando Pause
            else if (strcasecmp(parte, "pause")==0) produccion = 3;
            // Comando mkdisk
            else if (strcasecmp(parte, "mkdisk")==0){produccion = 2; cmmd = mkdisk_command;}
            // Reconocimiento de comentarios.
            else if (parte[0] == '#'){cout << "\033[38;5;246m(comentario) > " << parte << "\033[0m"; produccion = 4;}
        }


        // Cuando los usuarios existen verificamos la particion donde se encuentra el usuario
        if (produccion == 2 || produccion == 3){
            if (produccion == 2 && sesionAbierta){
                cout << "\033[0;1;49m(Sesion abierta " << nombreUsuario << " | Particion: " << pariticionID << ")\033[0m\n";
            }
        }else if (produccion != 4){
            cout << "\033[0;91;49m> Error: no se reconoce el comando " << parte << " \033[0m\n\n"; return 0;
        }

        // Programando el comando pause
        if (produccion == 3) return 2;

        

    }
}