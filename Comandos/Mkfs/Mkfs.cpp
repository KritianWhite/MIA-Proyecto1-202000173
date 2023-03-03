#include <iostream>
#include <string.h> // memset()...
#include <cmath>    // floor() para n inodos

#include "Mkfs.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"

using namespace std;

Mkfs _Mkfs(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* ID
    string particionID = "";
    bool vID = false;
    //* type
    string tipoFormato = "";
    bool vTipo = false;
    //* filesystem (fs)
    string fileSystemStr = "";
    int fileSystemInt = 0;
    bool vFS = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter i
            //* Reconocimiento del caracter t
            //* Reconocimiento del caracter f
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 2;
                else if ((char)tolower(parametros[i]) == 't') estado = 7;
                else if ((char)tolower(parametros[i]) == 'f') estado = 14;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> PATH
            //* Reconocimiento del caracter d
            case 2: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 3: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 4: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vID = false; particionID = ""; estado = 5;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vID = false; particionID = ""; particionID += parametros[i]; estado = 6;}
                break;
            }
            case 5: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(particionID.length() > 0){
                        vID = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un id vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else particionID += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(particionID.length() > 0) vID = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un id vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else particionID += parametros[i];
                break;
            }
            //TODO--> TYPE
            //*Reconocimiento del caracter y
            case 7: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'y') estado = 8;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //*Reconocimiento del caracter p
            case 8: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 9;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //*Reconocimiento del caracter e
            case 9: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 10: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de las comillas dobles
            case 11: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vTipo = false; tipoFormato = ""; estado = 12;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if(parametros[i] == '#' ){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vTipo = false; tipoFormato = ""; tipoFormato += parametros[i]; estado = 13;}
                break;
            }
            case 12: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if (tipoFormato.length() > 0){
                        if(strcasecmp(tipoFormato.c_str(), "full") == 0){
                            vTipo = true; tipoFormato = "full";
                        }else cout << "\033[;91;49m[Error]: El tipo de formato \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un tipo de formato vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else tipoFormato += parametros[i];
                break;
            }
            //* Reconocimiento sin las comillas dobles
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(tipoFormato.length() > 0){
                        if(strcasecmp(tipoFormato.c_str(), "full") == 0){
                            vTipo = true; tipoFormato = "full";
                        }else cout << "\033[;91;49m[Error]: El tipo de formato \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un tipo de formato vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else tipoFormato += parametros[i];
                break;
            }
            //TODO--> FILESYSTEM
            //* Reconocimiento del caracter s
            case 14: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 's') estado = 15;
                else if (parametros[i] == '#'){comentario = "";comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 15: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 16;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 16: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vFS = false; fileSystemStr = ""; estado = 17;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vFS = false; fileSystemStr = ""; fileSystemStr += parametros[i]; estado = 18;}
                break;
            }
            case 17: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(fileSystemStr.length() > 0){
                        if(strcasecmp(fileSystemStr.c_str(), "2fs") == 0){
                            vFS = true; fileSystemInt = 2;
                        }else if (strcasecmp(fileSystemStr.c_str(), "3fs") == 0){
                            vFS = true; fileSystemInt = 3;
                        }else cout << "\033[;91;49m[Error]: El sistema de archivo \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un sistema de archivo vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else fileSystemStr += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 18: {
                parametroActual += parametros[i];
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(fileSystemStr.length() > 0){
                        if (strcasecmp(fileSystemStr.c_str(), "2fs") == 0){
                            vFS = true; fileSystemInt = 2;
                        }else if (strcasecmp(fileSystemStr.c_str(), "3fs") == 0){
                            vFS = true; fileSystemInt = 3;
                        }else cout << "\033[;91;49m[Error]: El sistema de archivo \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un sistema de archivo vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else fileSystemStr += parametros[i];
                break;
            }
            //* Error de sintaxis
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para mkfs. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }
                else{
                    parametroActual += parametros[i];
                    break;
                }
            }
            //* Comentario
            case -2: {
                comentario += parametros[i];
                break;
            }
        }//* End switch case
    }//* End for loop
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;
}