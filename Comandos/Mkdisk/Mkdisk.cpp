#include <iostream>
#include <string.h> //memset()
#include <cctype> // tolower()
#include <cstdlib> // rand()
#include <fstream>

#include "Mkdisk.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"

using namespace std;

bool CrearDisco(Mkdisk mk){
    //? Validación de la extensión del disco (dsk)
    string extension = "";
    for (int i = mk.diskName.length() - 1; i >= 0; i--){
        if (mk.diskName[i] == '.'){
            extension = mk.diskName.substr(i + 1, mk.diskName.length());
            break;
        }
    }

    // * Validación si el archivo no contiene extensión
    if (extension.length() == 0){
        cout << "\033[0;91;49m[Error]: " << mk.diskName << " no contiene una extensión. \033[0m" << endl;
        return false;
    }
    // * Validación si el archivo contiene una extension distinta a dsk.
    if (extension != "dsk"){
        cout << "\033[0;91;49m[Error]: " << mk.diskName << " no es de la extensión dsk. \033[0m" << endl;
        return false;
    }

    string comandoUbuntu;
    int ansUbuntu;

    try{
        //! Ubicando la ruta del disco donde se creará
        cout << "--> Ubicando la ruta del disco..." << endl;
        comandoUbuntu = "mkdir -p \"";
        comandoUbuntu += mk.diskPath;
        comandoUbuntu += "\"";
        cout << "--> " << comandoUbuntu << endl;
        ansUbuntu = system(comandoUbuntu.c_str());

        if (ansUbuntu == 0){
            // TODO --> Creamos el disco y todo.
            fstream nuevoDisco;
            string path = "";
            int factor;
            cout << "--> Creando el disco..." << endl;
            path = mk.diskPath;
            path += mk.diskName;
            nuevoDisco.open(path, ios::out | ios::binary);
            if(nuevoDisco.fail()) return false;

            if(mk.sizeUnit == 'K') factor = 1024;
            else factor = 1024*1024;
            for (int i = 0; i < (mk.diskSize*factor); i++){
                nuevoDisco.write((char *)&cero, sizeof(char));
            }
            nuevoDisco.close();

            cout << "--> Configurando MBR del disco... " << endl;
            MBR nuevoMBR;
            nuevoMBR.mbr_tamano = mk.diskSize * factor;
            time(&nuevoMBR.mbr_fecha_creacion);
            nuevoMBR.mbr_dsk_signature = rand() % (100 + 1);
            nuevoMBR.dsk_fit = mk.diskFit;

            //* Particiones vacias
            nuevoMBR.mbr_partition_1.part_status = 'E';
            nuevoMBR.mbr_partition_2.part_status = 'E';
            nuevoMBR.mbr_partition_3.part_status = 'E';
            nuevoMBR.mbr_partition_4.part_status = 'E';
            nuevoDisco.open(path, ios::in | ios::out | ios::binary);
            if(nuevoDisco.fail()) return false;
            nuevoDisco.seekp(0);
            nuevoDisco.write((char *)&idMBR, sizeof(char));
            nuevoDisco.write((char *)&nuevoMBR, sizeof(MBR));
            nuevoDisco.close();

            return true;
        }

    }catch (...){
        return false;
    }





    return false;
}

// //TODO --> Para la verificacion de numeros positivos
// bool isNumber(char c){
//     if(c >= 48 && c <= 57) return true;
//     return false;
// }

// //TODO para la verificacion de texto
// bool isLetter(char c){
//     if((c >= 65 && c <= 90) || (c >= 97 && c <= 122)) return true;
//     return false;
// }

Mkdisk _Mkdisk(char *parametros){
    
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";


    //! path (obligatorio)
    string diskPath = "", diskName ="";
    bool vPath = false;
    //! size (obligatorio)
    string diskSizeStr = "";
    int diskSizeInt = 0;
    bool vSize = false;
    //TODO  fit (opcional)
    string diskFitStr = "";
    char diskFitChar = '\0';
    bool vFit = false;
    //TODO  unit (opcional)
    string unitSizeStr = "";
    char unitSizeChar = '\0';
    bool vUnit = false;

    //* Empezamos verificando la cadena recibida caracter por caracter
    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
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
                else if ((char)tolower(parametros[i]) == 's') estado = 9;
                else if ((char)tolower(parametros[i]) == 'f') estado = 16;
                else if ((char)tolower(parametros[i]) == 'u') estado = 21;
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
                if (parametros[i] == '\"'){vPath = false; diskPath = ""; diskName = ""; estado = 7;}
                else if (parametros[i] == '/'){vPath = false; diskPath = ""; diskPath += parametros[i]; diskName = ""; estado = 8;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else estado = -1;
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if (parametros[i] == '\"') {
                    if ((diskPath.size() > 0) && (diskName.size() > 0)) vPath = true;
                    else if((diskPath.size() > 0) && (diskName.size() < 0)){
                        cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene un nombre para el disco. \033[0m" << endl;
                    } else cout << "\033[0;91;49m[Error]: " << parametroActual << " no hay una ruta para la creación del disco. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '/'){diskPath += diskName; diskPath += parametros[i]; diskName = "";}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de la Path sin comillas.
            case 8: {
                parametroActual += parametros[i];
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if ((diskPath.length() > 0) || (diskName.length() > 0)) vPath = true;
                    else if ((diskPath.length() > 0) || (diskName.length() < 0)){
                        cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene un nombre para el disco. \033[0m" << endl;
                    } else cout << "\033[0;91;49m[Error]: " << parametroActual << " no hay una ruta para la creación del disco. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '/') {diskPath += diskName; diskPath += parametros[i]; diskName = "";}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else diskName += parametros[i];
                break;
            }
            //* Reconocimiento del caracter i
            case 9: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'i') estado = 10;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
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
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 12;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del igual
            case 12: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 13;
                else if (parametros[i] == '#'){comentario = ""; comentario = parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de numero positivo y no negativo para el size
            //* Primero comenzamos reconociendo el signo menor (-)
            case 13 : {
                parametroActual += parametros[i];
                if (parametros[i] == '-'){vSize = false; diskSizeStr = parametros[i]; estado = 14;}
                else if (isNumber(parametros[i])){vSize = false; diskSizeStr = parametros[i]; estado = 15;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 14: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])){diskSizeStr += parametros[i]; estado = 15;}
                else if (parametros[i] == '#'){comentario = ""; comentario = parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 15: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])) diskSizeStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if (stoi(diskSizeStr) <= 0) cout << "\033[0;91;49m[Error]: el valor del parametro size debe ser entero positivo \033[0m" << endl;
                    else{
                        vSize = true;
                        diskSizeInt = stoi(diskSizeStr);
                    }
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter i
            case 16: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 17: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 18;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 18: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 19;
                else if (parametros[i] == '#'){comentario = ""; comentario = parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocemos que vengan letras (bf, wf, ff)
            case 19: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])){vFit = false; diskFitStr = parametros[i]; estado = 20;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 20: {
                parametroActual += parametros[i];
                if (isLetter(parametros[i])) diskFitStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(diskFitStr.c_str(), "bf") == 0){ vFit = true; diskFitChar = 'B';}
                    else if (strcasecmp(diskFitStr.c_str(), "wf") == 0){vFit = true; diskFitChar = 'W';}
                    else if (strcasecmp(diskFitStr.c_str(), "ff") == 0){vFit = true; diskFitChar = 'F';}
                    else cout << "\033[0;91;49m[Error]: No se reconocio el fit (" << parametroActual << ") del comando mkdisk. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter n
            case 21: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'n') estado = 22;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter i
            case 22: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'i') estado = 23;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 23: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 24;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocmiento del caracter =
            case 24: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 25;
                else if (parametros[i] == '#'){comentario = ""; comentario = parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de los valores del parametro unit (K, M)
            case 25: {
                parametroActual += parametros[i];
                if (isLetter(parametros[i])){vUnit = false; unitSizeStr = parametros[i]; estado = 26;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 26: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i]))unitSizeStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(unitSizeStr.c_str(), "k") == 0){vUnit = true; unitSizeChar = 'k';}
                    else if (strcasecmp(unitSizeStr.c_str(), "m") == 0){vUnit = true; unitSizeChar = 'm';}
                    else cout <<  "\033[0;91;49m[Error]: No se reconocio la unidad en el parametro " << parametroActual << " del comando mkdisk \033[0m\n";
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //! Error de parametros inválidos.
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
            //* Comentarios.
            case -2: {
                comentario += parametros[i];
                break;
            }
        }
    }
    if (comentario.length() > 0) cout << "\033[38;5;246m[comentario] > " << comentario << "\033[0m" << endl;

    Mkdisk mk;
    if (vPath && vSize){
        if(!vFit || diskFitChar == '\0') diskFitChar = 'F';
        if (!vUnit || unitSizeChar == '\0') unitSizeChar = 'M';

        mk.diskPath = diskPath;
        mk.diskName = diskName;
        mk.diskSize = diskSizeInt;
        mk.diskFit = diskFitChar;
        mk.sizeUnit = unitSizeChar;
        mk.acceso = true;
        return mk;
    }

    if(!vPath) cout << "\033[0;91;49m[Error]: La direccion \">path\" es obligatoria para la creación del disco!" << endl;
    if(!vSize) cout << "\033[0;91;49m[Error]: El tamaño \">size\" es obligatorio para la creación del disco!" << endl;
    mk.acceso = false;
    return mk;
}
