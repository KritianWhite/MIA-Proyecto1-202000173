#include <iostream>
#include <string.h>
#include <cctype>
#include <fstream>

#include "Mount.h"
#include "../../aux_funciones.h"

using namespace std;


Mount _Mount(char *parametros){
    //* Variables de control
    int estado = 0;
    string comentario = "";
    string parametroActual = "";
    //* Path
    string path = "";
    string nombreDisco = "";
    bool vPath = false;
    //* Nombre
    string nombreParticion = "";
    bool vNombre = false;

    for(int i = 0; i <= (int)strlen(parametros); i++){
        switch (estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if (parametros[i] == '>') estado = 1;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = 2;}
                else estado = -1;
                break;
            }
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'p') estado = 2;
                else if((char)tolower(parametros[i]) == 'n') estado = 9;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> PATH
            //* Reconocimiento del caracter p
            case 2: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter h
            case 4: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'h') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1; 
                break;
            }
            //* Reconocimiento del caraacter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de la path con comillas
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vPath = false; path = ""; nombreDisco = ""; estado = 7;}
                else if (parametros[i] == '/'){vPath = false; path = ""; path += parametros[i]; nombreDisco = ""; estado = 8;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if((path.length() > 0) && (nombreDisco.length() > 0)) vPath = true;
                    else if ((path.length() > 0) && (nombreDisco.length() < 0)){
                        cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta, pero no proporciona el nombre del disco. \033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if(parametros[i] == '/'){path += nombreDisco; path += parametros[i]; nombreDisco = "";}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreDisco += parametros[i];
                break;
            }
            //* Reconocimiento de la path sin comillas
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if((path.length() > 0) && (nombreDisco.length() > 0)) vPath = true;
                    else if ((path.length() > 0) && (nombreDisco.length() < 0)){
                        cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta, pero no proporciona el nombre del disco. \033[0m" << endl;
                    }
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if(parametros[i] == '/'){path += nombreDisco; path += parametros[i]; nombreDisco = "";}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreDisco += parametros[i];
                break;
            }
            //TODO--> Name
            //* Reconocimiento del caracter a
            case 9: {
                //cout << "Llegue al 16" << endl;
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter m
            case 10: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'm') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter e
            case 11: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 12;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter =
            case 12: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 13;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de doble comillas
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vNombre = false; nombreParticion = ""; estado = 14;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vNombre = false; nombreParticion = ""; nombreParticion += parametros[i]; estado = 15;}
                break;
            }
            case 14: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(nombreParticion.length() > 0){
                        for (int i = 0; i < (int)nombreParticion.length(); i++){
                            nombreParticion[i] = tolower(nombreParticion[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vacío. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreParticion += parametros[i];
                break;
            }
            //* Reconocimiento del nombre sin comillas
            case 15: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(nombreParticion.length() > 0){
                        for(int i = 0; i < (int)nombreParticion.length(); i++){
                            nombreParticion[i] = tolower(nombreParticion[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreParticion += parametros[i];
                break;
            }

            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para mkdisk. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }
                else{
                    parametroActual += parametros[i];
                    break;
                }
            }
            case -2: {
                comentario += parametros[i];
                break;
            }
        }//* End switch case
    }//* End for loop
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Mount mnt;
    if(vPath && vNombre){
        mnt.diskPath = path;
        mnt.diskName = nombreDisco;
        mnt.partitionName = nombreParticion;
        mnt.acceso = true;
        return mnt;
    }

    //* Si en dado caso no viene la path o el nombre usaremos como funcion interna
    //* para poder mostrar las particiones montadas
    if(!vPath && !vNombre){
        mnt.diskPath = "";
        mnt.diskName = "";
        mnt.partitionName = "";
        mnt.acceso = true;
        return mnt;
    } 

    if(!vPath) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">path\" para poder montar la partición.\033[0m" << endl;
    if(!vNombre) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">name\" para poder montar la partición.\033[0m" << endl;

    mnt.acceso = false;
    return mnt;
}

