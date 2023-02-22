#include <iostream>
#include <string.h>
#include <cctype>

#include "Execute.h"

using namespace std;

Execute _Execute(char *parametros){
    //cout << "en el _Execute " << parametros << endl;
    // * Variables para la path
    string path = "";
    //? --> Verificación de que la path sea correcta
    bool vPath = false; 

    // TODO --> Variables de control
    int estado = 0;
    string parametroActual = "";
    string comentario = "";

    // * Recorremos los parametros letra por letra.
    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            //? --> Para el carácter >
            case 0: {
                parametroActual += parametros[i];
                if (parametros[i] == '>') estado = 1;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //? --> Para el carácter p
            case 1: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 2;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //? --> Para el carácter a
            case 2:{
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //? --> Para el carácter t
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //? --> Para el carácter h
            case 4: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'h') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //? --> Para el carácter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                //else if(parametros[i] == 12 || parametros[i] == 39) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO: execute -path=/home/Desktop/comandos.eea = 39
            //TODO: execute -path = 12
            //? --> Reconocimiento de path con comillas dobles ("X")
            case 6: {
                parametroActual += parametros[i];
                if (parametros[i] == '\"'){vPath = false; path = ""; estado = 7;}
                else if (parametros[i] == 9 || parametros[i] == 39) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vPath = false; path = ""; path += parametros[i]; estado = 8;}
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if (parametros[i] == '\"'){
                    if(path.length() > 0) vPath = true;
                    else cout << "Error: " << parametroActual << " posee una ruta vacia." << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //? --> Reconocimiento de path sin comillas dobles (X)
            case 8: {
                parametroActual += parametros[i];
                if (parametros[i] == 0 || parametros[i] == 12 || parametros[i] == 39){
                    vPath = true;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //? Error en el parametro
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 39){
                    cout << "Error: " << parametroActual << " es inválido para el comando execute." << endl;
                    parametroActual = "";
                    estado = 0;
                }else parametroActual += parametros[i];
                break;
            }
            //? Para el reconocimiento de comentarios
            case -2: {
                comentario += parametros[i];
                break;
            }
        } //! Final del switch..
    }//! Final del For..

    if (comentario.length() > 0){cout << "[Comentario]: " << comentario << endl;}

    Execute e;

    //TODO --> Hacemos la verificación que la path sea correcta/exista.
    if (vPath){
        e.path = path; 
        e.acceso = true; 
        return e;}
    
    cout << "Error: Falta el parametro obligatorio \">path\"." << endl;
    e.acceso = false;
    
    return e;
} 