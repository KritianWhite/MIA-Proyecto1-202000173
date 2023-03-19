#include <iostream>
#include <string.h>
#include <fstream>
#include <regex>

#include "Find.h"
#include "../Estructura.h"
#include "../Mount/Mount.h"
#include "../../aux_funciones.h"

using namespace std;

string RecursividadArchivos(string regularExp, ifstream &disk, Inode &actualInode, int posActualInode, string tabs){
    FolderBlock folderBlock_aux;
    string cadena = "", cadenaAux ="", result = "", parent = ".", previousParent = "..";

    //* Recorriendo bloques directos del inodo actual 
    for(int i = 0; i < 12; i++){
        if(actualInode.i_block[i] == -1) break;

        //* Bloque carpeta 
        disk.seekg(actualInode.i_block[i]);
        disk.read((char*)&folderBlock_aux, sizeof(FolderBlock));
        //* Generando el árbol de cada apt ocupado recursivamente 
        for(int j = 0; j < 4; j++){
            if(folderBlock_aux.b_content[j].b_name == parent) continue;
            if(folderBlock_aux.b_content[j].b_name == previousParent) continue;
            if(folderBlock_aux.b_content[j].b_inodo == -1) break;

            disk.seekg(folderBlock_aux.b_content[j].b_inodo);
            disk.read((char*)&actualInode, sizeof(Inode));

            if(regularExp == "*"){
                if(regex_match(folderBlock_aux.b_content[j].b_name, regex("(.+)")))
                    cadenaAux += tabs + folderBlock_aux.b_content[j].b_name + "\n";
            }
            else if(regularExp == "?"){
                if(regex_match(folderBlock_aux.b_content[j].b_name, regex("(.)")))
                    cadenaAux += tabs + folderBlock_aux.b_content[j].b_name + "\n";
            }
            else if(regularExp == "?.*"){
                if(regex_match(folderBlock_aux.b_content[j].b_name, regex("(.\\.(.+))")))
                    cadenaAux += tabs + folderBlock_aux.b_content[j].b_name + "\n";
            }
            else if(regularExp == "*.?"){
                if(regex_match(folderBlock_aux.b_content[j].b_name, regex("((.+)\\..)")))
                    cadenaAux += tabs + folderBlock_aux.b_content[j].b_name + "\n";
            }
            else if(regularExp == "?.?") {
                if(regex_match(folderBlock_aux.b_content[j].b_name, regex("(.\\..)")))
                    cadenaAux += tabs + folderBlock_aux.b_content[j].b_name + "\n";
            }

            if(actualInode.i_type == '0'){
                result = RecursividadArchivos(regularExp, disk, actualInode, folderBlock_aux.b_content[j].b_inodo, tabs + "|_");
            }


            if(cadenaAux == "" && result != ""){
                cadena += tabs + folderBlock_aux.b_content[j].b_name + "\n" + result;
            }else{
                cadena += cadenaAux + result;
            }

            result = "";
            cadenaAux = "";
        }
        //* Recuperando el inodo actual 
        disk.seekg(posActualInode);
        disk.read((char*)&actualInode, sizeof(Inode));
    }

    return cadena;
}


bool BuscarArchivos(bool openSesion, string partitionId, string originPath, string regularExpression, PartitionNode *&firstNode){
    //* VALIDACIONES 
    if(!openSesion){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    if(regularExpression != "*" && regularExpression != "?" && regularExpression != "?.*" && regularExpression != "*.?" && regularExpression != "?.?"){
        cout << "\033[0;91;49m[Error]: No se reconoció la expresión regular " << regularExpression << "\033[0m" << endl;
        return false;
    }

    cout << "--> Obteniendo datos del disco..." << endl;
    ifstream disco;
    string path = getDiskPath(firstNode, partitionId);
    disco.open(path, ios::in|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << path << "\033[0m" << endl;
        disco.close();
        return false;
    }

    char aux;
    disco.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR\033[0m" << endl;
        disco.close();
        return false;
    }
    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));

    string nombreParticion = getPartitionName(firstNode, partitionId);
    int numeroParticionPrimaria = 0, particionInicial = 0, numeroParticionExtendidia = 0;
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
            if(mb.mbr_partition_1.part_type == 'E') numeroParticionExtendidia = 1;
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
            if(mb.mbr_partition_2.part_type == 'E') numeroParticionExtendidia = 2;
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
            if(mb.mbr_partition_3.part_type == 'E') numeroParticionExtendidia = 3;
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
            if(mb.mbr_partition_4.part_type == 'E') numeroParticionExtendidia = 4;
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
    //* SUBERBLOCK PRIMARY PARTITION 
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO SUPERBLOCK LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    //* Accediendo al archivo solicitado 
    Inode iAux;
    FolderBlock folderB;
    bool wasFound = false;
    int posicionInode = sbAux.s_inode_start;
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode)); // Root inode

    if(originPath != "/"){
        char *parte;
        string fileCopy = originPath, nombreArchivo = "";
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
                    cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << nombreArchivo
                    << " de la ruta " << originPath << " en el árbol de directorios\033[0m" << endl;
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
                    posicionInode = folderB.b_content[j].b_inodo;
                    disco.seekg(folderB.b_content[j].b_inodo);
                    disco.read((char*)&iAux, sizeof(Inode));
                    wasFound = true;
                    break;
                }
                if(wasFound) break;
            }

            //* No se encontró en los bloques directos 
            if(!wasFound){
                cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << nombreArchivo
                << " de la ruta " << originPath << " en el árbol de directorios\033[0m" << endl;
                disco.close();
                return false;
            }
            wasFound = false;
            parte = strtok(NULL, "/");
        }

        if(iAux.i_type == '1'){
            cout << "\033[0;91;49m[Error]: el nombre " << nombreArchivo << " indicado corresponde a un archivo, no a un directorio \033[0m" << endl;
            disco.close();
            return false;
        }
    }

    //* Inodo carpeta de la ruta origen se encuentra en iAux 
    string result = "--> " + originPath + "\n";

    result += RecursividadArchivos(regularExpression, disco, iAux, posicionInode, "|_");
    cout << result << endl;

    disco.close();
    return true;
}

Find _Find(char *parametros){
    string originPath = "";
    bool vPath = false;
    string regularExpression = "";
    bool vExpresion = false;
    int estado = 0;
    string parametroActual = "";
    string comentario = "";

    for(int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'p') estado = 2;
                else if((char)tolower(parametros[i]) == 'n') estado = 10;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 2: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 3;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 3: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 4;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 4: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'h') estado = 5;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 5: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 7;
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }

            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {vPath = false; originPath = ""; estado = 8;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vPath = false; originPath = ""; originPath += parametros[i]; estado = 9;}
                break;
            }
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(originPath.length() > 0) vPath = true;
                    else cout << "Error: " << parametroActual << " posee una ruta vacia \n";
                    parametroActual = "";
                    estado = 0;
                }
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else originPath += parametros[i];
                break;
            }
            case 9: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    vPath = true;
                    parametroActual = "";
                    estado = 0;
                }
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else originPath += parametros[i];
                break;
            }
            case 10: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 11;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 11: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'm') estado = 12;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 12: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 13;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 15;
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }

            case 15: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {vExpresion = false; regularExpression = ""; estado = 16;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vExpresion = false; regularExpression = ""; regularExpression += parametros[i]; estado = 17;}
                break;
            }
            case 16: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(regularExpression.length() > 0) vExpresion = true;
                    else {cout << "Error: " << parametroActual << " posee un nombre vacío \n";}

                    parametroActual = "";
                    estado = 0;
                }
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {regularExpression += parametros[i];}
                break;
            }
            case 17: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(regularExpression.length() > 0) vExpresion = true;
                    else {cout << "Error: " << parametroActual << " posee un nombre vacío \n";}

                    parametroActual = "";
                    estado = 0;
                }
                else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
                else {regularExpression += parametros[i];}
                break;
            }
            case -1: {
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "Error: " << parametroActual << " es invalido para el comando find \n";
                    parametroActual = "";
                    estado = 0;
                }
                else parametroActual += parametros[i];
                break;
            }
            case -2: {
                comentario += parametros[i];
                break;
            }
        } // end: switch
    } // end: for char in parametros

    if(comentario.length() > 0)
        {}

    Find f;

    if(vPath && vExpresion){
        f.originPath = originPath;
        f.regularExpression = regularExpression;
        f.acceso = true;
        return f;
    }

    string mssg = "Error: Faltan los parametros obligatorios";
    if(!vPath) mssg += " -path";
    if(!vExpresion) mssg += " -name";
    mssg += " para poder ejecutar el comando find \n\n";
    cout << mssg;
    f.acceso = false;
    return f;
}