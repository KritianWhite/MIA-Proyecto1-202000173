#include <iostream>
#include <string.h>

#include "Unmount.h"
#include "../Mount/Mount.h"

using namespace std;


bool ParticionUnmount(string partitionId, PartitionNode *&primero){
    cout << "--> comprobando que la particion esté montada..." << endl;
    if(!isIdInList(primero, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con el id " << partitionId << 
        " no se pudo encontrar en la lista de particiones montadas. \033[0m" << endl;
        return false;
    }

    bool eliminar = deletePartitionFromList(primero, partitionId);

    if(!eliminar){
        cout << "\033[0;91;49m[Error]: La particion con el id " << partitionId << 
        " no pudo desmontarse. Error interno. \033[0m" << endl;
        return false;
    }

    return true;
}


Unmount _Unmount(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "";
    string comentario = "";
    //* ID
    string particionID = "";
    bool vID = false;

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
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 2;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
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
            //* Reconocimiento de las comillas dobles
            case 4: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vID = false; particionID = ""; estado = 5;}
                else if(parametros[i] == 9 || parametros[i] == 32);
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
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
            //* Reconocimiento sin las comillas dobles
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(particionID.length() > 0){
                        vID = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un id vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else particionID += parametros[i];
                break;
            }


            //* Error de sintaxis
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
            //* Comentario
            case -2: {
                comentario += parametros[i];
                break;
            }
        }//* End switch case
    }//* End for loop
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Unmount umnt;

    if(vID){
        umnt.partitionId = particionID;
        umnt.acceso = true;
        return umnt;
    }

    cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">id\" para poder desmontar la partición.\033[0m" << endl;
    umnt.acceso = false;
    return umnt;
}