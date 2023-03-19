#include <iostream>
#include <string.h>
#include <fstream>

#include "Rename.h"
#include "../Mount/Mount.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"

using namespace std;

bool CambiarNombre_Archivo(bool openSesion, string partitionId, string path, string oldName, string newName, PartitionNode *&firstNode){
    if(!openSesion){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId 
        << " no fue encontrada en la lista de particiones montadas (use mount"
        <<" para ver esta lista) \033[0m" << endl;
        return false;
    }

    cout << "--> Obteniendo datos del disco..." << endl;
    fstream disco;
    string diskPath = getDiskPath(firstNode, partitionId);
    disco.open(diskPath, ios::in|ios::out|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " 
        << partitionId << ", con ruta " << diskPath << "\033[0m" << endl;
        disco.close();
        return false;
    }

    char aux;
    disco.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR \033[0m" << endl;
        disco.close();
        return false;
    }
    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));

    string nombreParticion = getPartitionName(firstNode, partitionId);
    int numeroParticionPrimaria = 0, particionInicial = 0, numeroParticionExtendida = 0;
    char partStatus = '0';

    while(true){
        if(mb.mbr_partition_1.part_status != 'E'){
            if(mb.mbr_partition_1.part_name == nombreParticion){
                if(mb.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 1;
                particionInicial = mb.mbr_partition_1.part_start;
                partStatus = mb.mbr_partition_1.part_status;
                break;
            }
            if(mb.mbr_partition_1.part_type == 'E') numeroParticionExtendida = 1;
        }
        if(mb.mbr_partition_2.part_status != 'E'){
            if(mb.mbr_partition_2.part_name == nombreParticion){
                if(mb.mbr_partition_2.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 2;
                particionInicial = mb.mbr_partition_2.part_start;
                partStatus = mb.mbr_partition_2.part_status;
                break;
            }
            if(mb.mbr_partition_2.part_type == 'E') numeroParticionExtendida = 2;
        }
        if(mb.mbr_partition_3.part_status != 'E'){
            if(mb.mbr_partition_3.part_name == nombreParticion){
                if(mb.mbr_partition_3.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 3;
                particionInicial = mb.mbr_partition_3.part_start;
                partStatus = mb.mbr_partition_3.part_status;
                break;
            }
            if(mb.mbr_partition_3.part_type == 'E') numeroParticionExtendida = 3;
        }
        if(mb.mbr_partition_4.part_status != 'E'){
            if(mb.mbr_partition_4.part_name == nombreParticion){
                if(mb.mbr_partition_4.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 4;
                particionInicial = mb.mbr_partition_4.part_start;
                partStatus = mb.mbr_partition_4.part_status;
                break;
            }
            if(mb.mbr_partition_4.part_type == 'E') numeroParticionExtendida = 4;
        }
        break;
    }

    //* Validando que la partición contenga formato 
    if(partStatus != 'F'){
        cout << "\033[0;91;49m[Error]: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m" << endl;
        disco.close();
        return false;
    }

    Superblock sbAux;
    //* RENAME PRIMARY PARTITION 
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO RENAME LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    Inode iAux;
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode)); // Root inode

    //* Control variables 
    FolderBlock folderB;
    bool wasFound = false;

    char *parte;
    string fileCopy = path, nombreArchivo = "";
    parte = strtok((char*)fileCopy.c_str(), "/");

    while(parte != NULL){
        if(iAux.i_type == '1'){
            cout << "\033[0;91;49m[Error]: el nombre " << parte << " corresponde a un archivo, no a un subdirectorio \033[0m" << endl;
            disco.close();
            return false;
        }

        nombreArchivo = parte;

        //* Buscando en bloques directos 
        for(int i = 0; i < 12; i++){
            if(iAux.i_block[i] == -1){
                cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << nombreArchivo << " de la ruta " << path << " en el árbol de directorios\033[0m" << endl;
                disco.close();
                return false;
            }

            disco.seekg(iAux.i_block[i]);
            disco.read((char*)&folderB, sizeof(FolderBlock));

            //* Buscando en sus 4 espacios disponibles 
            for(int j = 0; j < 4; j++){
                if(folderB.b_content[j].b_inodo == -1) break;
                if(folderB.b_content[j].b_name != nombreArchivo) continue;

                //* Inodo de parte encontrado 
                disco.seekg(folderB.b_content[j].b_inodo);
                disco.read((char*)&iAux, sizeof(Inode));
                wasFound = true;
                break;
            }
            if(wasFound) break;
        }

        //* No se encontró en los bloques directos 
        if(!wasFound){
            cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << nombreArchivo << " de la ruta " << path << " en el árbol de directorios\033[0m" << endl;
            disco.close();
            return false;
        }
        wasFound = false;
        parte = strtok(NULL, "/");
    }

    if(iAux.i_type == '1'){
        cout << "\033[0;91;49m[Error]: el nombre " << nombreArchivo << " indicado corresponde a un archivo, no a un subdirectorio (ruta: " << path << ") \033[0m" << endl;
        disco.close();
        return false;
    }

    //* Inodo padre (path) se encuentra en iAux 

    //* Buscando en bloques directos 
    wasFound = false;
    int posicionBloque = 0;
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1){
            cout << "\033[0;91;49m[Error]: no se ha encontrado el archivo/directorio " << oldName << " de la ruta " << path << " en el árbol de directorios\033[0m" << endl;
            disco.close();
            return false;
        }

        disco.seekg(iAux.i_block[i]);
        posicionBloque = iAux.i_block[i];
        disco.read((char*)&folderB, sizeof(FolderBlock));

        //* Buscando en sus 4 espacios disponibles 
        for(int j = 0; j < 4; j++){
            if(folderB.b_content[j].b_inodo == -1) break;
            if(folderB.b_content[j].b_name != oldName) continue;

            //* Inodo de oldName encontrado 
            cout << "--> Cambiando el nombre de " << oldName << " a " << newName << "..." << endl;
            memset(folderB.b_content[j].b_name, 0, 12);
            for(int k = 0; k < (int)newName.length() && k < 12; k++)
                folderB.b_content[j].b_name[k] = newName[k];

            wasFound = true;
            break;
        }
        if(wasFound) break;
    }

    //* No se encontró en los bloques directos 
    if(!wasFound){
        cout << "\033[0;91;49m[Error]: no se ha encontrado el archivo/directorio " << oldName << " de la ruta " << path << " en el árbol de directorios\033[0m" << endl;
        disco.close();
        return false;
    }

    //* Acutalizando bloque 
    disco.seekp(posicionBloque);
    disco.write((char*)&folderB, sizeof(FolderBlock));

    disco.close();
    return true;
}


Rename _Rename(char *parametros){
    ///* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    ///* path
    string path = "";
    bool vPath = false;
    ///* name
    string nombre = "", nombreViejo = "";
    bool vNombre = false;

    for(int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            ///* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if (parametros[i] == '>') estado = 1;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = 2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter p
            ///* Reconocimiento del caracter n
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'p') estado = 2;
                else if((char)tolower(parametros[i]) == 'n') estado = 9;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> PATH
            ///* Reconocimiento del caracter p
            case 2: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter t
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter h
            case 4: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'h') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1; 
                break;
            }
            ///* Reconocimiento del caraacter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento de la path con comillas
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vPath = false; path = ""; path = ""; nombreViejo = ""; estado = 7;}
                else if (parametros[i] == '/'){vPath = false; path = ""; path += parametros[i]; nombreViejo = ""; path = ""; estado = 8;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if((path.length() > 0) && (nombreViejo.length() > 0)) vPath = true;
                    else if ((path.length() > 0) && (path.length() < 0)){
                        cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta, pero no proporciona el nombre del archivo. \033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if(parametros[i] == '/'){path += nombreViejo; path += parametros[i]; nombreViejo = "";}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreViejo += parametros[i];
                break;
            }
            ///* Reconocimiento de la path sin comillas
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if((path.length() > 0) && (nombreViejo.length() > 0)) vPath = true;
                    else if ((path.length() > 0) && (nombreViejo.length() < 0)){
                        cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta, pero no proporciona el nombre del archivo. \033[0m" << endl;
                    }
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if(parametros[i] == '/'){path += nombreViejo; path += parametros[i]; nombreViejo = "";}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreViejo += parametros[i];
                break;
            }
            //TODO--> Name
            ///* Reconocimiento del caracter a
            case 9: {
                //cout << "Llegue al 16" << endl;
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            ///* Reconocimiento del caracter m
            case 10: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'm') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            ///* Reconocimiento del caracter e
            case 11: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 12;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            ///* Reconocimiento del caracter =
            case 12: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 13;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento de doble comillas
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vNombre = false; nombre = ""; estado = 14;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vNombre = false; nombre = ""; nombre += parametros[i]; estado = 15;}
                break;
            }
            case 14: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(nombre.length() > 0){
                        //for (int i = 0; i < (int)nombre.length(); i++){
                        //    nombre[i] = tolower(nombre[i]);
                        //}
                        vNombre = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vacío. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombre += parametros[i];
                break;
            }
            ///* Reconocimiento del nombre sin comillas
            case 15: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(nombre.length() > 0){
                        // for(int i = 0; i < (int)nombre.length(); i++){
                        //     nombre[i] = tolower(nombre[i]);
                        // }
                        vNombre = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombre += parametros[i];
                break;
            }
            ///* Error sintactico
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para rename. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }
                else{
                    parametroActual += parametros[i];
                    break;
                }
            }
            ///* Comentarios
            case -2: {
                comentario += parametros[i];
                break;
            }
        }///* End switch case
    }///* End for loop
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Rename re;
    if(vPath && vNombre){
        re.path = path;
        re.oldName = nombreViejo;
        re.newName = nombre;
        re.acceso = true;
        return re;
    }
    if(!vPath) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">path\" para poder montar la partición.\033[0m" << endl;
    if(!vNombre) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">name\" para poder montar la partición.\033[0m" << endl;
    re.acceso = false;
    return re;
}