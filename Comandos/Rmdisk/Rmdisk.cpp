#include <iostream>
#include <string.h>
#include <cctype>

#include "Rmdisk.h"

using namespace std;

bool EliminarDisco(string path){
    string comandoUbuntu;
    int ansUbuntu;
    try{
        cout << "--> Comprobando que exista el disco..." << endl;
        comandoUbuntu = "[ -f \"";
        comandoUbuntu += path;
        comandoUbuntu += "\" ]";
        ansUbuntu = system(comandoUbuntu.c_str());

        //* Comprobamos la existencia
        if(ansUbuntu == 0){
            cout << "--> Eliminando el disco..." << endl;
            comandoUbuntu = "rm \"";
            comandoUbuntu += path;
            comandoUbuntu += "\"";
            ansUbuntu = system(comandoUbuntu.c_str());
            //* Comprobamos que el disco si se haya eliminado correctamente
            if(ansUbuntu == 0){
                //cout << "\033[0;92;49m[Correcto]: El disco se ha eliminado correctamente... \033[0m" << endl;
                return true;
            }
        }else{
            comandoUbuntu = "[ -d ";
            comandoUbuntu += path;
            comandoUbuntu += " ]";
            ansUbuntu = system(comandoUbuntu.c_str());
            //* Comprobamos que existe la ruta del disco, y si en dado caso el disco no existe
            if (ansUbuntu == 0){
                cout << "\033[0;91;49m[Error]: La ruta \"" << path << "\" no pertenece a un directorio \033[0m" << endl;
            }else{
                cout << "\033[0;91;49m[Error]: No existe el disco con la ruta \"" << path << "\" \033[0m" << endl;
            }
        }
        return false;
    }catch(...){
        return false;
    }
}

Rmdisk _Rmdisk(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";

    string diskPath = "";
    bool vPath = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch (estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if(parametros[i] == 9 || parametros[i] == 32);
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter p
            case 1: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 2;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter a
            case 2: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 4;
                else if (parametros[i] == '#'){comentario = "", comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter h
            case 4:{
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'h') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario = parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de la Path con comillas.
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vPath = false; diskPath = ""; estado = 7;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vPath = false; diskPath = ""; diskPath += parametros[i]; estado = 8;}
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') vPath = true;
                else cout << "\033[0;91;49m[Error]: " << parametroActual << " posee una ruta vacia \033[0m" << endl;
                parametroActual = "";
                estado = 0;
            }
            //* Reconocimiento de la path sin comillas
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    vPath = true;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else diskPath +=parametros[i];
                break;
            }
            //* Reconocimiento de errores de parametros
            case -1: {
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es invalido para el comando rmdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else parametroActual += parametros[i];
                break;
            }
            //* Comentarios
            case -2: {
                comentario += parametros[i];
                break;
            }
        }//* End switch case
    }//* End For loop
    
    if(comentario.length() > 0) cout << "\033[38;5;246m[comentario] > " << comentario << "\033[0m" << endl;

    Rmdisk rm;
    if(vPath){
        rm.path = diskPath;
        rm.acceso = true;
        return rm;
    }

    cout << "\033[0;91;49m[Error]: Falta el parametro obligatorio \">path\" para poder eliminar un disco \033[0m" << endl;
    rm.acceso = false;
    return rm;
}