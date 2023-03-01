#include <iostream>
#include <string.h>
#include <cctype>
#include <fstream>
#include <limits>

#include "Fdisk.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"

using namespace std;


Fdisk _Fdisk(char *parametros){

    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";

    //* path (obligatorio)
    string path = "";
    bool vPath = false;
    //* Nombre (obligatorio)
    string nombreParticion = "";
    bool vNombre = false;
    
    //TODO --> PARA LA CREACION DE UNA PARTICION
    //* tamaño de la particion (obligatorio)
    string tamanoParticionStr = "";
    int tamanoParticion = 0;
    bool vTamano = false;
    
    //* units (opcional B | K | M)
    string tamanoUnitStr = "";
    char tamanoUnit = '\0';
    bool vUnit = false;

    //* Tipo de partición (opcional P [default] | E | L)
    string tipoParticionStr = "";
    char tipoParticion = '\0';
    bool vTipo = false;

    // Fit (opcional F | B | W [default])
    string fitParticionStr = "";
    char fitParticion = '\0';
    bool vFit = false;

    //TODO --> PARA LA ELIMINACION DE UNA PARTICION
    //* Eliminar el tipo
    string eliminarTipo = "";
    bool vEliminado = false;

    //TODO --> PARA CAMBIAR EL TAMAÑO DE LA PRTICION
    //* Cambiar tamaño
    string cambiarTamanoStr = "";
    int cambiarTamano = 0;
    bool vAdd = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch (estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de caracters iniciales de cada parametro
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'p') estado = 2;
                else if((char)tolower(parametros[i]) == 's') estado = 9;
                else if ((char)tolower(parametros[i]) == 'n') estado = 16;
                else if ((char)tolower(parametros[i]) == 'u') estado = 23;
                else if ((char)tolower(parametros[i]) == 't') estado = 29;
                else if ((char)tolower(parametros[i]) == 'f') estado = 35;
                else if ((char)tolower(parametros[i]) == 'd') estado = 40;
                else if ((char)tolower(parametros[i]) == 'a') estado = 48;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> PATH
            //* Reconocimiento del caracter a
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
                if(parametros[i] == '\"'){vPath = false; path = ""; estado = 7;}
                else if (parametros[i] == '/'){vPath = false; path = ""; path += parametros[i]; estado = 8;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(path.length() > 0) vPath = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //* Reconocimiento de la path sin comillas
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(path.length() > 0) vPath = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //TODO --> SIZE
            //* Reconocimiento del caracter i
            case 9: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter z
            case 10: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'z') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 11: {
                parametroActual += parametros[0];
                if ((char)tolower(parametros[i]) == 'e') estado = 12;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 12: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 13;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de numero para el size (primeramente reconcemos el signo menos (-))
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == '-'){vTamano = false; tamanoParticionStr = parametros[i]; estado = 14;}
                else if (isNumber(parametros[i])){vTamano = false; tamanoParticionStr = parametros[i]; estado = 15;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que sean numeros enteros
            case 14: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])){tamanoParticionStr += parametros[i]; estado = 15;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 15: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])) tamanoParticionStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if (stoi(tamanoParticionStr) <= 0) cout << "\033[0;91;49m[Error]: El valor del parametro >size debe de ser entero positivo. \033[0m" << endl;
                    else {vTamano = true; tamanoParticion = stoi(tamanoParticionStr);}
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> NAME
            //* Reconocimiento del caracter a
            case 16: {
                //cout << "Llegue al 16" << endl;
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter m
            case 17: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'm') estado = 18;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter e
            case 18: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 19;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter =
            case 19: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 20;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del nombre con comillas
            case 20: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {vNombre = false; nombreParticion = ""; estado = 21;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vNombre = false; nombreParticion = ""; nombreParticion += parametros[i]; estado = 22;}
                break;
            }
            case 21: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(nombreParticion.length() > 0){
                        for(int i = 0; i < (int)nombreParticion.length(); i++){
                            nombreParticion[i] = tolower(nombreParticion[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[0;91;49m[Error]: \"" << parametroActual << "\" asigne un nombre a la particion. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreParticion += parametros[i];
                break;
            }
            //* Reconocimiento del nombre sin comillas
            case 22: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(nombreParticion.length() > 0){
                        for(int i = 0; i < (int)nombreParticion.length(); i++){
                            nombreParticion[i] = tolower(nombreParticion[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[0;91;49m[Error]: \"" << parametroActual << "\" asigne un nombre al disco. \033[0m" << endl;
                    parametroActual = ""; 
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreParticion += parametros[i];
                break;
            }
            //TODO --> UNIT
            //* Reconocimiento del caracter n
            case 23: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'n') estado = 24;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter i
            case 24: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'i') estado = 25;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 25: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 26;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 26: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 27;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos si realmente se estan recibiendo letras
            case 27: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])){vUnit = false; tamanoUnitStr = parametros[i]; estado = 28;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            // Hacemos la verificacion de las unidades
            case 28: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i]))tamanoUnitStr += parametros[i];
                else if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(tamanoUnitStr.c_str(), "b") == 0){vUnit = true; tamanoUnit = 'B';}
                    else if (strcasecmp(tamanoUnitStr.c_str(), "k") == 0){vUnit = true; tamanoUnit = 'K';}
                    else if (strcasecmp(tamanoUnitStr.c_str(), "m") == 0){vUnit = true; tamanoUnit = 'M';}
                    else cout << "\033[0;91;49m[Error]: No se reconocio la unidad en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> TYPE
            //* Reconocimiento del caracter y
            case 29: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'y') estado = 30;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter p
            case 30: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 31;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 31: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 32;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 32: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 33;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que realmente venga una letra
            case 33: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])){vTipo = false; tipoParticionStr = parametros[i]; estado = 34;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos los tipos de particiones
            case 34: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])) tipoParticionStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(tipoParticionStr.c_str(), "p") == 0){vTipo = true; tipoParticion = 'P';}
                    else if (strcasecmp(tipoParticionStr.c_str(), "e") == 0){vTipo = true; tipoParticion = 'E';}
                    else if (strcasecmp(tipoParticionStr.c_str(), "l") == 0){vTipo = true; tipoParticion = 'L';}
                    else cout << "\033[0;91;49m[Error]: No se reconocio el tipo de particion en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> FIT
            //* Reconocimiento del caracter i
            case 35: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 36;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 36: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 37;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 37: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 38;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que realmente venga una letra
            case 38: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])){vFit = true; fitParticionStr = parametros[i]; estado = 39;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Validamos el ajuste de espacio para la particion
            case 39: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])) fitParticionStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(fitParticionStr.c_str(), "bf") == 0){vFit = true; fitParticion = 'B';}
                    else if (strcasecmp(fitParticionStr.c_str(), "wf") == 0){vFit = true; fitParticion = 'W';}
                    else if (strcasecmp(fitParticionStr.c_str(), "ff") == 0){vFit = true; fitParticion = 'F';}
                    else cout << "\033[0;91;49m[Error]: No se reconocio el ajuste de espacio para la particion en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> DELETE
            //* Reconocimiento del caracter e
            case 40: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 41;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter l
            case 41: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'l') estado = 42;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 42: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 43;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 43: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 44;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 44: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 45;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 45: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 46;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que sean letras realmente
            case 46: {
                parametroActual += parametros[i];
                if (isLetter(parametros[i])){vEliminado = false; eliminarTipo = parametros[i]; estado = 47;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Realizamos la verificacion de que el valor del parametro sea full
            case 47: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])) eliminarTipo += parametros[i];
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(eliminarTipo.c_str(), "full") == 0){vEliminado = true; eliminarTipo = "full";}
                    else cout << "\033[0;91;49m[Error]: No se reconocio full para la eliminacion de la particion en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> ADD
            //* reconocimiento del caracter d
            case 48: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 49;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* reconocimiento del caracter d
            case 49: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 50;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 50: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 51;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos primeramente el signo negativo (-)
            case 51: {
                parametroActual += parametros[i];
                if (parametros[i] == '-') {vAdd = false; cambiarTamanoStr = parametros[i]; estado = 52;}
                else if (isNumber(parametros[i])){vAdd = false; cambiarTamanoStr = parametros[i]; estado = 53;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos de que realmente sea numero entero
            case 52: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])){cambiarTamanoStr += parametros[i]; estado = 53;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 53: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])) cambiarTamanoStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    vAdd = true;
                    cambiarTamano = stoi(cambiarTamanoStr);
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }




            //* Parametro invalido
            case -1: {
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es invalido para el comando fdisk \033[0m" << endl;
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
    }//* End for loop

    if (comentario.length() > 0) cout << "\033[38;5;246m[comentario] > " << comentario << "\033[0m" << endl;

    Fdisk fd;

    //* Primero vamos a crear la particion
    if(vPath && vNombre){
        if(vTamano){
            if(!vAdd && !vEliminado){
                fd.diskPath = path;
                fd.partitionName = nombreParticion;
                fd.partitionSize = tamanoParticion;
                if(vUnit) fd.sizeUnit = tamanoUnit;
                else fd.sizeUnit = 'K';
                if(vTipo) fd.partitionType = tipoParticion;
                else fd.partitionType = 'P';
                if(vFit) fd.partitionFit = fitParticion;
                else fd.partitionFit = 'W';
                fd.deletionType = "";
                fd.resizePartition = false;
                fd.acceso = true;
                return fd;
            }

            //* Verificamos que los parametros Add y delete no vengan en la misma instruccion
            if(vAdd && vEliminado) {
                cout << "\033[0;91;49m[Error]: Los parametros \">Add\" y \">Delete\"" <<
                    " no pueden venir en la misma instruccion del parametro \">size\""<< 
                    " en el comando fdisk. \033[0m" << endl;
                fd.acceso = false;
                return fd;
            }
        }

        //* Eliminamos particion
        if(vEliminado){
            if(!vAdd && !vTamano){
                fd.diskPath = path;
                fd.partitionName = nombreParticion;
                fd.deletionType = eliminarTipo;
                fd.resizePartition = false;
                fd.acceso = true;
                return fd;
            }

            //* Verificamos que los parametros Add y Size no vengan en la misma instruccion
            if(vAdd && vTamano){
                cout << "\033[0;91;49m[Error]: Los parametros \">Add\" y \">size\"" <<
                    " no pueden venir en la misma instruccion del parametro \">Delete\""<< 
                    " en el comando fdisk. \033[0m" << endl;
                fd.acceso = false;
                return fd;
            }
        }

        // Aumentar o disminuir tamaño de particion
        if(vAdd){
            if(!vTamano){
                fd.diskPath = path;
                fd.partitionName = nombreParticion;
                fd.sizetoChange = cambiarTamano;
                if(vUnit) fd.sizeUnit = tamanoUnit;
                else fd.sizeUnit = 'K';
                fd.deletionType = "";
                fd.resizePartition = true;
                fd.acceso = true;
                return fd;
            }

            cout << "\033[0;91;49m[Error]: Los parametros \">Add\" y \">size\" no pueden venir en" <<
             " la misma instruccioin del comando fdisk. \033[0m" << endl;
            fd.acceso = false;
            return fd;
        }

        cout << "\033[0;91;49m[Error]: Faltan parametros para poder realizar la particion. \033[0m" << endl;
        fd.acceso = false;
        return fd;
    }

    if(!vPath && !vNombre){
        cout << "\033[0;91;49m[Error]: Los parametros \">path\" y \">name\" son obligatorios para la creacion de la particion. \033[0m" << endl;
        fd.acceso = false;
        return fd;
    }

}

