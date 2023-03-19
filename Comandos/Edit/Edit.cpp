#include <iostream>
#include <string.h>
#include <fstream>

#include "Edit.h"
#include "../Estructura.h"
#include "../Mount/Mount.h"
#include "../../aux_funciones.h"

using namespace std;

bool EditarArchivo(string filePath, string contentPath, bool openSesion, string userSesion, string partitionId, PartitionNode *&firstNode){
    //* VALIDACIONES 
    if(!openSesion){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion " << partitionId << " no se encontró en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    //* Validando que la ruta de cont exista 
    cout << "--> Obteniendo datos de la ruta indicada en cont..." << endl;
    ifstream cont;
    cont.open(contentPath, ios::in);
    if(cont.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir la ruta " << contentPath << "\033[0m" << endl;
        cont.close();
        return false;
    }
    string content = "";
    string line;
    while(!cont.eof()){
        getline(cont, line);
        if(content != "") content += "\n";
        content += line;
    }
    cont.close();

    cout << "--> Obteniendo datos del disco..." << endl;
    fstream disco;
    string path = getDiskPath(firstNode, partitionId);
    disco.open(path, ios::in|ios::out|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << path << "\033[0m" << endl;
        disco.close();
        return false;
    }

    char aux;
    disco.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR. \033[0m" << endl;
        disco.close();
        return false;
    }
    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));

    string partitionName = getPartitionName(firstNode, partitionId);
    int numeroParticionPrimaria = 0, particionInicial = 0, numeroParticionExtendida = 0;
    char partStatus = '0';

    while(true){
        if(mb.mbr_partition_1.part_status != 'E'){
            if(mb.mbr_partition_1.part_name == partitionName){
                if(mb.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
            if(mb.mbr_partition_2.part_name == partitionName){
                if(mb.mbr_partition_2.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
            if(mb.mbr_partition_3.part_name == partitionName){
                if(mb.mbr_partition_3.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
            if(mb.mbr_partition_4.part_name == partitionName){
                if(mb.mbr_partition_4.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
        cout << "\033[0;91;49m[Error]: La partición " << partitionId << " no ha sido formateada. Utilice antes mkfs. \033[0m" << endl;
        disco.close();
        return false;
    }

    Superblock sbAux;
    //* EDIT PRIMARY PARTITION 
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO EDIT LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    //* Accediendo al archivo solicitado 
    Inode iAux;
    FolderBlock folderB;
    bool wasFound = false;
    int posicionActualInode = sbAux.s_inode_start;
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode)); // Root inode

    char *parte;
    string fileCopy = filePath, nombreArchivo = "";
    parte = strtok((char*)fileCopy.c_str(), "/");

    while(parte != NULL){
        if(iAux.i_type == '1'){
            cout << "\033[0;91;49m[Error]: el nombre " << parte << " corresponde a un archivo y no a un subdirectorio \033[0m" << endl;
            disco.close();
            return false;
        }

        nombreArchivo = parte;

        //* Buscando en bloques directos 
        for(int i = 0; i < 12; i++){
            if(iAux.i_block[i] == -1){
                cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << nombreArchivo
                << " de la ruta " << filePath << " en el árbol de directorios\033[0m" << endl;
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
                posicionActualInode = folderB.b_content[j].b_inodo;
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
            << " de la ruta " << filePath << " en el árbol de directorios\033[0m" << endl;
            disco.close();
            return false;
        }
        wasFound = false;
        parte = strtok(NULL, "/");
    }

    if(iAux.i_type == '0'){
        cout << "\033[0;91;49m[Error]: el nombre " << nombreArchivo << " indicado corresponde a un directorio, no a un archivo \033[0m" << endl;
        disco.close();
        return false;
    }

    //* Inodo del archivo "nombreArchivo" se encuentra en iAux 
    FileBlock fileB;
    memset(fileB.b_content, 0, 64);
    bool modificarBloque = false;
    bool modificarInode = false;
    int k = 0;

    //* Recorriendo bloques directos para escribir el nuevo contenido 
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1){
            //* CREANDO NUEVO BLOQUE ARCHIVO 
            modificarInode = true;
            iAux.i_block[i] = sbAux.s_first_blo;
            memset(fileB.b_content, 0, 64);
            disco.seekp(sbAux.s_first_blo);
            disco.write((char*)&fileB, sizeof(FileBlock));
            sbAux.s_first_blo += sbAux.s_block_s;
            sbAux.s_free_blocks_count -= 1;
            //* Escribiendo en el bitmap de bloques 
            disco.seekg(sbAux.s_bm_block_start);
            while(!disco.eof()){
                disco.read((char *)&aux, sizeof(char));
                if(!disco.eof() && aux == cero){
                    disco.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                    disco.write((char*)&uno, sizeof(char));
                    break;
                }
            }
        }

        disco.seekg(iAux.i_block[i]);
        disco.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] != 0) continue;
            //* ESCRIBIENDO EN EL BLOQUE ACTUAL 
            fileB.b_content[j] = content[k];
            modificarBloque = true;
            k++;
            if(k >= (int)content.length()) break;
        }
        if(modificarBloque){
            //* SOBRESCRIBIENDO BLOQUE DE ARCHIVOS 
            disco.seekp(iAux.i_block[i]);
            disco.write((char*)&fileB, sizeof(FileBlock));
            modificarBloque = false;
        }
        if(k >= (int)content.length()) break;
    }

    if(modificarInode){
        time(&iAux.i_mtime);
        //* SOBRESCRIBIENDO SUPERBLOQUE 
        disco.seekp(particionInicial);
        disco.write((char*)&sbAux, sizeof(Superblock));
    }else time(&iAux.i_atime);

    //* SOBRESCRIBIENDO INODO ARCHIVO 
    disco.seekp(posicionActualInode);
    disco.write((char*)&iAux, sizeof(Inode));

    disco.close();
    return true;
}

Edit _Edit(char *parametros){
    ///* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    ///* path
    string path = "";
    bool vPath = false;
    ///* cont
    string contenidoPath = "";
    bool vContenido = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            ///* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if(parametros[i] == 9 || parametros[i] == 32);
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter p
            ///* Reconocimiento del caracter c
            case 1: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 2;
                else if ((char)tolower(parametros[i]) == 'c') estado = 16;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> path
            ///* Reconocimiento del caracter a
            case 2: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter t
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 4;
                else if (parametros[i] == '#'){comentario = "", comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter h
            case 4:{
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'h') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario = parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento de la path con comillas
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vPath = false; path = ""; estado = 7;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vPath = false; path = ""; path += parametros[i]; estado = 8;}
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
            ///* Reconocimiento de la path sin comillas
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
            //TODO--> cont
            ///* Reconocimiento del caracter o
            case 16: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'o') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter n
            case 17: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'n') estado = 18;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter t
            case 18: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 19;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento del caracter =
            case 19: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 20;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            ///* Reconocimiento de la path con comillas
            case 20: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vContenido = false; contenidoPath = ""; estado = 21;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vContenido = false; contenidoPath = ""; contenidoPath += parametros[i]; estado = 22;}
                break;
            }
            case 21: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(contenidoPath.length() > 0) vContenido = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else contenidoPath += parametros[i];
                break;
            }
            ///* Reconocimiento de la path sin comillas
            case 22: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32) {
                    vContenido = true;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else contenidoPath += parametros[i];
                break;
            }
            ///* Parametro invalido
            case -1: {
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es invalido para el comando edit. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else parametroActual += parametros[i];
                break;
            }
            ///* Comentarios
            case -2: {
                comentario += parametros[i];
                break;
            }
        }///* End switch case
    }///* End for loop
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Edit e;
    if(vPath && vContenido){
        e.filePath = path;
        e.contentPath = contenidoPath;
        e.acceso = true;
        return e;
    }

    if(!vPath) cout << "\033[0;91;49m[Error]: El parametro \">path\" es obligatorio para eidtar el archivo. \033[0m" << endl;
    if(!vContenido) cout << "\033[0;91;49m[Error]: El parametro \">cont\" es obligatorio para editar el archivo. \033[0m" << endl;
    e.acceso = false;
    return e;
}