#include <iostream>
#include <string.h>
#include <fstream>

#include "Mkdir.h"
#include "../Mount/Mount.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"

using namespace std;

bool CrearCarpeta(bool openSesion, string userSesion, string directoryPath, bool createParentFolders, string partitionId, PartitionNode *&firstNode){
    ///* VALIDACIONES 
    if(!openSesion){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

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
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR \033[0m" << endl;
        disco.close();
        return false;
    }
    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));

    string nombreParticion = getPartitionName(firstNode, partitionId);
    int numeroParticionPrimaria = 0, particionInicial = 0, numeroParticionExtendida = 0;;
    char partStatus = '0';

    while(true){
        if(mb.mbr_partition_1.part_status != 'E'){
            if(mb.mbr_partition_1.part_name == nombreParticion){
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
            if(mb.mbr_partition_2.part_name == nombreParticion){
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
            if(mb.mbr_partition_3.part_name == nombreParticion){
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
            if(mb.mbr_partition_4.part_name == nombreParticion){
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
        cout << "\033[0;91;49m[Error]: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m" << endl;
        disco.close();
        return false;
    }

    Superblock sbAux;
    //* MKDIR PRIMARY PARTITION 
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO MKDIR LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    //* UBICANDO EL INODO DE USERS.TXT Y SU CONTENIDO 
    cout << "--> Obteniendo datos del usuario..." << endl;

    Inode iAux;
    FolderBlock folderB;
    string name = "users.txt";
    bool wasFound = false;
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode)); // Root inode

    //* Buscando en bloques directos 
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1){
            cout << "\033[0;91;49m[Error]: se llegó a un pointer = -1 en el inodo root sin encontrar el users.txt \033[0m" << endl;
            disco.close();
            return false;
        }

        disco.seekg(iAux.i_block[i]);
        disco.read((char*)&folderB, sizeof(FolderBlock));

        //* Buscando en sus 4 espacios disponibles 
        for(int j = 0; j < 4; j++){
            if(folderB.b_content[j].b_inodo == -1) break;
            if(folderB.b_content[j].b_name != name) continue;

            //* Inodo de users.txt encontrado 
            disco.seekg(folderB.b_content[j].b_inodo);
            disco.read((char*)&iAux, sizeof(Inode));
            wasFound = true;
            break;
        }
        if(wasFound) break;
    }

    //* No se encontró en los bloques directos 
    if(!wasFound){
        cout << "\033[0;91;49m[Error]: se leyeron todos los bloques directos sin encontrar users.txt \033[0m" << endl;
        disco.close();
        return false;
    }
    // Inodo de users.txt se encuentra en iAux

    FileBlock fileB;
    string content = "";
    //* Recorriendo bloques directos para leer su contenido 
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1) break;
        disco.seekg(iAux.i_block[i]);
        disco.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] == 0) break;
            content += fileB.b_content[j];
        }
    }

    if(content.length() == (12*64)){
        // TODO seguir leyendo bloques indirectos
    }

    //* OBTENIENDO EL UID, Y EL GRUPO DEL USUARIO 

    char *line;
    int uid = -1;
    string contenidoPath = content;
    string linecopy = "", id = "", type = "", nombreGrupo = "", user = "", contrasena = "";
    line = strtok((char*)contenidoPath.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
        //* FORMATO DE users.txt:
        //* GID, Tipo, Grupo -> 1,G,MAX10\n
        //* UID, Tipo, Grupo, Usuario, Contraseña -> 1,U,root,root,123\n
        size_t pos = 0;
        pos = linecopy.find(",");
        id = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        type = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);

        if(type == "G"){
            line = strtok(NULL, "\n");
            continue;
        }

        pos = linecopy.find(",");
        nombreGrupo = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        user = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        contrasena = linecopy;

        if(user == userSesion){
            uid = stoi(id);
            //* user grupo -> nombreGrupo variable
            break;
        }

        line = strtok(NULL, "\n");
    }

    if(uid == -1){
        cout << "\033[0;91;49m[Error]: No se encontró al usuario " << userSesion << " en el archivo users.txt de la partición \033[0m" << endl;
        disco.close();
        return false;
    }

    //* OBTENIENDO EL GID DEL GRUPO AL QUE PERTENECE EL USUARIO 

    int gid = -1;
    string grupo = "";
    contenidoPath = content;
    line = strtok((char*)contenidoPath.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
        //* GID, Tipo, Grupo -> 1,G,MAX10\n 
        size_t pos = 0;
        pos = linecopy.find(",");
        id = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        type = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);

        if(type == "U"){
            line = strtok(NULL, "\n");
            continue;
        }

        grupo = linecopy;
        if(grupo == nombreGrupo){
            gid = stoi(id);
            break;
        }

        line = strtok(NULL, "\n");
    }

    if(gid == -1){
        cout << "\033[0;91;49m[Error]: No se encontró al grupo " << nombreGrupo << " del usuario " << userSesion << "en el archivo users.txt de la partición \033[0m" << endl;
        disco.close();
        return false;
    }

    //* CREANDO DIRECTORIO 
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode)); // Root inode
    int posicionActualInode = sbAux.s_inode_start;
    int posicionAnteriorInode = sbAux.s_inode_start;

    string directorio = directoryPath, nombreDirectorio = "";
    wasFound = false;
    size_t pos = 0;

    //* PARENT FOLDERS 
    while ((pos = directorio.find("/")) != string::npos) {
        nombreDirectorio = directorio.substr(0, pos);
        if(nombreDirectorio == "" || nombreDirectorio == "\0"){
            directorio.erase(0, pos + 1);
            continue;
        }

        //* Buscando en bloques directos 
        for(int i = 0; i < 12; i++){
            if(iAux.i_block[i] == -1){
                //*cout << "PUNTERO EN INODO NULO, CREANDO NUEVO BLOQUE CARPETA\n";
                if(!createParentFolders){
                    cout << "\033[0;91;49m[Error]: no se ha encontrado el subdirectorio " << nombreDirectorio
                    << " de la ruta " << directoryPath << " (use parámetro -p) \033[0m" << endl;
                    disco.close();
                    return false;
                }
                //* Preparando nuevo bloque para inodo del subdirectorio 
                for(int j = 0; j < 4; j++) memset(folderB.b_content[j].b_name, 0, 12);
                for(int j = 0; j < 4; j++) folderB.b_content[j].b_inodo = -1;
                //* actualizando inodo actual con nuevo bloque
                iAux.i_block[i] = sbAux.s_first_blo;
                time(&iAux.i_mtime);
                disco.seekp(posicionActualInode);
                disco.write((char*)&iAux, sizeof(Inode));
                //* Escribiendo el bloque carpeta 
                disco.seekp(sbAux.s_first_blo);
                disco.write((char*)&folderB, sizeof(FolderBlock));
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
            disco.read((char*)&folderB, sizeof(FolderBlock));
            //* Buscando en sus 4 espacios disponibles
            for(int j = 0; j < 4; j++){
                if(folderB.b_content[j].b_inodo == -1){
                    //*cout << "PUNTERO EN BLOQUE CARPETA NULO, CREANDO NUEVO INODO CARPETA\n";
                    if(!createParentFolders){
                        cout << "\033[0;91;49m[Error]: no se ha encontrado el subdirectorio " << nombreDirectorio
                        << " de la ruta " << directoryPath << " (use parámetro -p) \033[0m" << endl;
                        disco.close();
                        return false;
                    }
                    //* Actualizando bloque carpeta, agregando el nuevo inodo
                    int posParent = sbAux.s_first_ino;
                    folderB.b_content[j].b_inodo = sbAux.s_first_ino;
                    memset(folderB.b_content[j].b_name, 0, 12);
                    for(int k = 0; k < (int)nombreDirectorio.length() && k < 12; k++)
                        {folderB.b_content[j].b_name[k] = nombreDirectorio[k];}
                    disco.seekp(iAux.i_block[i]);
                    disco.write((char*)&folderB, sizeof(FolderBlock));
                    //* Creando nuevo inodo en actual bloque para subdirectorio 
                    cout << "--> Creando subdirectorio " << nombreDirectorio << "..." << endl;
                    iAux.i_uid = uid;
                    iAux.i_gid = gid;
                    iAux.i_s = -1;
                    iAux.i_atime  = NULL;
                    time(&iAux.i_ctime);
                    time(&iAux.i_mtime);
                    iAux.i_block[0] = sbAux.s_first_blo; // BLOQUE CON REFERENCIAS AL INODO
                    for(int j = 1; j < 15; j++) iAux.i_block[j] = -1;
                    iAux.i_type = '0'; // directorio
                    iAux.i_perm = 664;
                    //* Escribiendo el inodo carpeta del subdirectorio 
                    disco.seekp(sbAux.s_first_ino);
                    disco.write((char*)&iAux, sizeof(Inode));
                    sbAux.s_first_ino += sbAux.s_inode_s;
                    sbAux.s_free_inodes_count -= 1;
                    //* Escribiendo en el bitmap de inodos 
                    disco.seekg(sbAux.s_bm_inode_start);
                    while(!disco.eof()){
                        disco.read((char *)&aux, sizeof(char));
                        if(!disco.eof() && aux == cero){
                            disco.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                            disco.write((char*)&uno, sizeof(char));
                            break;
                        }
                    }
                    //* Escribiendo nuevo bloque carpeta con referencias para subdirectorio 
                    for(int j = 0; j < 4; j++) memset(folderB.b_content[j].b_name, 0, 12);
                    folderB.b_content[0].b_name[0] = '.';
                    folderB.b_content[0].b_inodo = folderB.b_content[j].b_inodo;
                    folderB.b_content[1].b_name[0] = '.';
                    folderB.b_content[1].b_name[1] = '.';
                    folderB.b_content[1].b_inodo = posicionActualInode;
                    folderB.b_content[2].b_inodo = -1;
                    folderB.b_content[3].b_inodo = -1;
                    //* Escribiendo el bloque carpeta 
                    disco.seekp(sbAux.s_first_blo);
                    disco.write((char*)&folderB, sizeof(FolderBlock));
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
                    //* Inodo encontrado 
                    posicionAnteriorInode = posicionActualInode;
                    posicionActualInode = posParent;
                    disco.seekg(posParent);
                    disco.read((char*)&iAux, sizeof(Inode));
                    wasFound = true;
                    break;
                }

                if(folderB.b_content[j].b_name != nombreDirectorio) continue;

                //* Inodo encontrado 
                posicionAnteriorInode = posicionActualInode;
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
            // TODO bloques indirectos
            disco.close();
            return false;
        }

        if(iAux.i_type == '1'){
            cout << "\033[0;91;49m[Error]: el nombre " << nombreDirectorio << " corresponde a un archivo, no a un subdirectorio \033[0m" << endl;
            disco.close();
            return false;
        }

        directorio.erase(0, pos + 1);
        wasFound = false;
    }

    //* Final directorio, parent inode -> iAux 
    cout << "--> Creando directorio " << directorio << "..." << endl;
    bool creado = false;
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1){
            //*cout << "PUNTERO EN INODO NULO, CREANDO NUEVO BLOQUE CARPETA\n";
            //* Preparando nuevo bloque para inodo del directorio 
            for(int j = 0; j < 4; j++) memset(folderB.b_content[j].b_name, 0, 12);
            for(int j = 0; j < 4; j++) folderB.b_content[j].b_inodo = -1;
            //* actualizando inodo padre con nuevo bloque
            iAux.i_block[i] = sbAux.s_first_blo;
            time(&iAux.i_mtime);
            disco.seekp(posicionActualInode);
            disco.write((char*)&iAux, sizeof(Inode));
            //* Escribiendo el bloque carpeta 
            disco.seekp(sbAux.s_first_blo);
            disco.write((char*)&folderB, sizeof(FolderBlock));
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
        disco.read((char*)&folderB, sizeof(FolderBlock));
        for(int j = 0; j < 4; j++){
            if(folderB.b_content[j].b_inodo == -1){
                //*cout << "PUNTERO EN BLOQUE CARPETA NULO, CREANDO NUEVO INODO CARPETA\n";
                //* Actualizando bloque carpeta, agregando el nuevo inodo
                int posParent = sbAux.s_first_ino;
                folderB.b_content[j].b_inodo = sbAux.s_first_ino;
                memset(folderB.b_content[j].b_name, 0, 12);
                for(int k = 0; k < (int)directorio.length() && k < 12; k++)
                    {folderB.b_content[j].b_name[k] = directorio[k];}
                disco.seekp(iAux.i_block[i]);
                disco.write((char*)&folderB, sizeof(FolderBlock));
                //* CREANDO INODO CARPETA DEL DIRECTORIO 
                iAux.i_uid = uid;
                iAux.i_gid = gid;
                iAux.i_s = -1;
                iAux.i_atime  = NULL;
                time(&iAux.i_ctime);
                time(&iAux.i_mtime);
                iAux.i_block[0] = sbAux.s_first_blo; // BLOQUE CON REFERENCIAS AL INODO
                for(int k = 1; k < 15; k++) iAux.i_block[k] = -1;
                iAux.i_type = '0'; // directorio
                iAux.i_perm = 664;
                //* Escribiendo el inodo carpeta del directorio 
                disco.seekp(sbAux.s_first_ino);
                disco.write((char*)&iAux, sizeof(Inode));
                sbAux.s_first_ino += sbAux.s_inode_s;
                sbAux.s_free_inodes_count -= 1;
                //* Escribiendo en el bitmap de inodos 
                disco.seekg(sbAux.s_bm_inode_start);
                while(!disco.eof()){
                    disco.read((char *)&aux, sizeof(char));
                    if(!disco.eof() && aux == cero){
                        disco.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                        disco.write((char*)&uno, sizeof(char));
                        break;
                    }
                }
                //* Escribiendo nuevo bloque carpeta con referencias para subdirectorio 
                for(int k = 0; k < 4; k++) memset(folderB.b_content[k].b_name, 0, 12);
                folderB.b_content[0].b_name[0] = '.';
                folderB.b_content[0].b_inodo = posParent;
                folderB.b_content[1].b_name[0] = '.';
                folderB.b_content[1].b_name[1] = '.';
                folderB.b_content[1].b_inodo = posicionActualInode;
                folderB.b_content[2].b_inodo = -1;
                folderB.b_content[3].b_inodo = -1;
                //* Escribiendo el bloque carpeta 
                disco.seekp(sbAux.s_first_blo);
                disco.write((char*)&folderB, sizeof(FolderBlock));
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
                creado = true;
                break;
            }

            if(folderB.b_content[j].b_name != directorio) continue;

            cout << "[Mensaje]: Ya existe el directorio " << directorio << ", indicado en la ruta " << directoryPath << endl;
            disco.close();
            return false;
        }
        if(creado) break;
    }

    //* Sobrescribiendo Superbloque con la información actualizada 
    disco.seekp(particionInicial);
    disco.write((char*)&sbAux, sizeof(Superblock));

    disco.close();
    return true;
}



Mkdir _Mkdir(char *parametros){
    ///* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    ///* path
    string path = "";
    bool vPath = false;
    ///* r
    bool crearCarpetasParents = false;

    for(int i = 0; i <= (int)strlen(parametros); i++){
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
            ///* Reconocimiento del caracter r
            case 1: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 2;
                else if ((char)tolower(parametros[i]) == 'r') {crearCarpetasParents = true; parametroActual = ""; estado = 0;}
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
            ///* Parametro invalido
            case -1: {
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es invalido para el comando mkdir. \033[0m" << endl;
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

    Mkdir md;
    if(vPath){
        md.directoryPath = path;
        md.createParentFolderS = crearCarpetasParents;
        md.acceso = true;
        return md;
    }
    //cout << "\033[0;91;49m[Error]: El parametro \">path\" es obligatorio para crear el archivo. \033[0m" << endl;
    md.acceso = false;
    return md;
}