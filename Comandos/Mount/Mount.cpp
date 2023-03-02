#include <iostream>
#include <string.h>
#include <cctype>
#include <fstream>

#include "Mount.h"
#include "../../aux_funciones.h"
#include "../Estructura.h"

using namespace std;

bool ParticionMount(Mount mnt, PartitionNode *&primero){
    
    //* Variables de control
    ifstream disco;
    string pathCompleto = "";
    string nombreDisco = "";
    char auxiliar;

    string extension = "";
    for (int i = mnt.diskName.length() -1; i >= 0; i--){
        if(mnt.diskName[i] == '.'){
            extension = mnt.diskName.substr(i + 1, mnt.diskName.length());
            break;
        }
    }
    nombreDisco = mnt.diskName.substr(0, mnt.diskName.length() - extension.length() - 1);
    cout << "--> Obteniendo datos del disco..." << endl;
    pathCompleto +=  mnt.diskPath;
    pathCompleto += mnt.diskName;
    disco.open(pathCompleto, ios::in|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco en " << pathCompleto << ". \033[0m" << endl;
        disco.close();
        return false;
    }

    disco.read((char *)&auxiliar, sizeof(char));
    if(auxiliar != idMBR){
        cout << "\033[0;91;49m[Error]: No vino char idMBR antes de MBR. Error interno. \033[0m" << endl;
        disco.close();
        return false;
    }

    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));

    bool existeParticion = false;
    int numeroParticionExt = 0;

    while(true){
        if(mb.mbr_partition_1.part_status != 'E'){
            if(mb.mbr_partition_1.part_name == mnt.partitionName){
                if(mb.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: No puede montarse la particion " << mnt.partitionName 
                    << " ya que es extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                existeParticion = true;
                break;
            }
            if(mb.mbr_partition_1.part_type == 'E') numeroParticionExt = 1;
        }
        if(mb.mbr_partition_2.part_status != 'E'){
            if(mb.mbr_partition_2.part_name == mnt.partitionName){
                if(mb.mbr_partition_2.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: No puede montarse la particion " << mnt.partitionName 
                    << " ya que es extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                existeParticion = true;
                break;
            }
            if(mb.mbr_partition_2.part_type == 'E') numeroParticionExt = 2;
        }
        if(mb.mbr_partition_3.part_status != 'E'){
            if(mb.mbr_partition_3.part_name == mnt.partitionName){
                if(mb.mbr_partition_3.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: No puede montarse la particion " << mnt.partitionName 
                    << " ya que es extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                existeParticion = true;
                break;
            }
            if(mb.mbr_partition_3.part_type == 'E') numeroParticionExt = 3;
        }
        if(mb.mbr_partition_4.part_status != 'E'){
            if(mb.mbr_partition_4.part_name == mnt.partitionName){
                if (mb.mbr_partition_4.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: No puede montarse la particion " << mnt.partitionName 
                    << " ya que es extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                existeParticion = true;
                break;
            }
            if(mb.mbr_partition_4.part_type == 'E') existeParticion = 4;
        }
        break;
    }//* End while loop

    //* Validamos que la particion exista en el disco duro
    if(!existeParticion){
        if(numeroParticionExt == 0){
            cout << "\033[0;91;49m[Error]: La particion " << mnt.partitionName 
            << " no pudo encontrarse en el disco con ruta " << 
            pathCompleto << ". \033[0m" << endl;
            disco.close();
            return false;
        }

        //* Verificamos si es logica
        EBR ebAux;
        switch(numeroParticionExt){
            case 1: {
                disco.seekg(mb.mbr_partition_1.part_start);
                disco.read((char *)&ebAux, sizeof(EBR));
                break;
            }
            case 2: {
                disco.seekg(mb.mbr_partition_2.part_start);
                disco.read((char *)&ebAux, sizeof(EBR));
                break;
            }
            case 3: {
                disco.seekg(mb.mbr_partition_3.part_start);
                disco.read((char *)&ebAux, sizeof(EBR));
                break;
            }
            case 4: {
                disco.seekg(mb.mbr_partition_4.part_start);
                disco.read((char *)&ebAux, sizeof(EBR));
                break;
            }
        }

        //* Si no hay particiones logicas
        if(ebAux.part_status == 'E'){
            cout << "\033[0;91;49m[Error]: La particion " << mnt.partitionName 
            << " no pudo encontrarse en el disco con ruta " << 
            pathCompleto << ". \033[0m" << endl;
            disco.close();
            return false;
        }

        //* Recorriendo las particiones logicas para encontrar el nombre
        int contador = 1;
        do{
            if(ebAux.part_name == mnt.partitionName){
                existeParticion = true;
                break;
            }

            if(ebAux.part_next == -1) break;

            disco.seekg(ebAux.part_next);
            disco.read((char *)&ebAux, sizeof(EBR));
            contador += 1;
        }while(contador <= 12);

        if (contador > 12){
            cout << "\033[0;91;49m[Error]: Se detectaron mas de 12 particiones logicas (ver su creación en fdisk). \033[0m" << endl;
            disco.close();
            return false;
        }

        if(!existeParticion){
            cout << "\033[0;91;49m[Error]: La particion " << mnt.partitionName 
            << " no pudo encontrarse en el disco con ruta " << 
            pathCompleto << ". \033[0m" << endl;
            disco.close();
            return false;
        }
        //* En este punto, la particion existe, por lo que continuamos
        cout << "--> Partición logica detectada..." << endl;
    }else cout << "--> Partición primaria detectada..." << endl;

    //* Validamos que la particion no se encuentre montada
    if(isPartitionMounted(primero, mnt.partitionName, pathCompleto)){
        cout << "\033[0;91;49m[Error]: La particion " << mnt.partitionName << " del disco " << pathCompleto
        << " ya se encuentra montada en memoria. ID: " << getPartitionId(primero, mnt.partitionName) << ". \033[0m" << endl;
        disco.close();
        return false;
    }

    cout << "--> Montando la partición en memoria..." << endl;
    bool insert = insertMountedPartition(primero, mnt.partitionName, nombreDisco, pathCompleto);
    if(!insert){
        cout << "\033[0;91;49m[Error]: La particion " << mnt.partitionName << " del disco " << pathCompleto
        << " no pudo montarse. (Posible causa: eliminación de particiones, pero no en la lista enlazada. \033[0m" << endl;
        disco.close();
        return false;
    }

    disco.close();
    return true;
}

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

