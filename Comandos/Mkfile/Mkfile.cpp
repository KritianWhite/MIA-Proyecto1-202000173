#include <iostream>
#include <string>
#include <string.h>
#include <fstream>

#include "Mkfile.h"
#include "../Estructura.h"
#include "../Mount/Mount.h"
#include "../../aux_funciones.h"

using namespace std;

bool CrearArchivo(bool openSesion, string userSesion, Mkfile mf, string partitionId, PartitionNode *&firstNode){
    /* VALIDACIONES */
    if(!openSesion){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    /* Validando que, si viene cont, la ruta exista, y preparando el contenido del archivo */
    string content = "";
    if(mf.contentPath != ""){
        cout << "--> Obteniendo datos de la ruta indicada en cont..." << endl;
        ifstream cont;
        cont.open(mf.contentPath, ios::in);
        if(cont.fail()){
            cout << "\033[0;91;49m[Error]: No se ha podido abrir la ruta " << mf.contentPath << "\033[0m" << endl;
            cont.close();
            return false;
        }
        string line;
        while(!cont.eof()){
            getline(cont, line);
            if(content != "") content += "\n";
            content += line;
        }
        cont.close();
    }
    else if(mf.fileSize > 0){
        // TODO QUITAR RESTRICCIÓN DE SOLO DIRECTOS
        for(int i = 0; i < mf.fileSize && i < (64*12); i++){
            content += to_string(i%10);
        }
    }

    cout << "--> Obteniendo datos del disco..." << endl;
    fstream disk;
    string diskPath = getDiskPath(firstNode, partitionId);
    disk.open(diskPath, ios::in|ios::out|ios::binary);
    if(disk.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << diskPath << ".\033[0m" << endl;
        disk.close();
        return false;
    }

    char aux;
    disk.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR. \033[0m" << endl;
        disk.close();
        return false;
    }
    MBR mbrDisk;
    disk.read((char *)&mbrDisk, sizeof(MBR));

    string partitionName = getPartitionName(firstNode, partitionId);
    int numberPrimaryPartition = 0, partitionStart = 0;
    char partStatus = '0';
    int numberExtendedPartition = 0;

    // while para poder finalizar las validaciones cuando se encuentre la partición
    // solo se ejecuta una vez
    while(true){
        if(mbrDisk.mbr_partition_1.part_status != 'E'){
            if(mbrDisk.mbr_partition_1.part_name == partitionName){
                if(mbrDisk.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas.\033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 1;
                partitionStart = mbrDisk.mbr_partition_1.part_start;
                partStatus = mbrDisk.mbr_partition_1.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_1.part_type == 'E') numberExtendedPartition = 1;
        }
        if(mbrDisk.mbr_partition_2.part_status != 'E'){
            if(mbrDisk.mbr_partition_2.part_name == partitionName){
                if(mbrDisk.mbr_partition_2.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas.\033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 2;
                partitionStart = mbrDisk.mbr_partition_2.part_start;
                partStatus = mbrDisk.mbr_partition_2.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_2.part_type == 'E') numberExtendedPartition = 2;
        }
        if(mbrDisk.mbr_partition_3.part_status != 'E'){
            if(mbrDisk.mbr_partition_3.part_name == partitionName){
                if(mbrDisk.mbr_partition_3.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas.\033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 3;
                partitionStart = mbrDisk.mbr_partition_3.part_start;
                partStatus = mbrDisk.mbr_partition_3.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_3.part_type == 'E') numberExtendedPartition = 3;
        }
        if(mbrDisk.mbr_partition_4.part_status != 'E'){
            if(mbrDisk.mbr_partition_4.part_name == partitionName){
                if(mbrDisk.mbr_partition_4.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas.\033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 4;
                partitionStart = mbrDisk.mbr_partition_4.part_start;
                partStatus = mbrDisk.mbr_partition_4.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_4.part_type == 'E') numberExtendedPartition = 4;
        }
        break;
    }

    /* Validando que la partición contenga formato */
    if(partStatus != 'F'){
        cout << "\033[0;91;49m[Error]: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m" << endl;
        disk.close();
        return false;
    }

    Superblock sbAux;
    /* MKFILE PRIMARY PARTITION */
    if(numberPrimaryPartition != 0){
        disk.seekg(partitionStart);
        disk.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO MKFILE LOGICAL PARTITIONS
    else {
        disk.close();
        return false;
    }

    /* UBICANDO EL INODO DE USERS.TXT Y SU CONTENIDO */
    cout << "--> Obteniendo datos del usuario...\n";

    Inode inodeAux;
    FolderBlock folderB;
    string name = "users.txt";
    bool wasFound = false;
    disk.seekg(sbAux.s_inode_start);
    disk.read((char*)&inodeAux, sizeof(Inode)); // Root inode

    /* Buscando en bloques directos */
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1){
            cout << "\033[0;91;49m[Error]: se llegó a un pointer = -1 en el inodo root sin encontrar el users.txt\033[0m" << endl;
            disk.close();
            return false;
        }
        disk.seekg(inodeAux.i_block[i]);
        disk.read((char*)&folderB, sizeof(FolderBlock));

        /* Buscando en sus 4 espacios disponibles */
        for(int j = 0; j < 4; j++){
            if(folderB.b_content[j].b_inodo == -1) break;
            if(folderB.b_content[j].b_name != name) continue;

            /* Inodo de users.txt encontrado */
            disk.seekg(folderB.b_content[j].b_inodo);
            disk.read((char*)&inodeAux, sizeof(Inode));
            wasFound = true;
            break;
        }
        if(wasFound) break;
    }

    /* No se encontró en los bloques directos */
    if(!wasFound){
        cout << "\033[0;91;49m[Error]: se leyeron todos los bloques directos sin encontrar users.txt\033[0m" << endl;
        disk.close();
        return false;
    }
    // Inodo de users.txt se encuentra en inodeAux

    FileBlock fileB;
    string usersContent = "";
    /* Recorriendo bloques directos para leer su contenido */
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1) break;
        disk.seekg(inodeAux.i_block[i]);
        disk.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] == 0) break;
            usersContent += fileB.b_content[j];
        }
    }

    if(usersContent.length() == (12*64)){
        // TODO seguir leyendo bloques indirectos
    }

    /* OBTENIENDO EL UID, Y EL GRUPO DEL USUARIO */

    char *line;
    int uid = -1;
    string contentcopy = usersContent;
    string linecopy = "", id = "", type = "", groupName = "", user = "", pass = "";
    line = strtok((char*)contentcopy.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
        /* FORMATO DE users.txt:
        * GID, Tipo, Grupo -> 1,G,MAX10\n
        * UID, Tipo, Grupo, Usuario, Contraseña -> 1,U,root,root,123\n
        */
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
        groupName = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        user = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pass = linecopy;

        if(user == userSesion){
            uid = stoi(id);
            // user group -> groupName variable
            break;
        }

        line = strtok(NULL, "\n");
    }

    if(uid == -1){
        cout << "\033[0;91;49m[Error]: No se encontró al usuario " << userSesion << " en el archivo users.txt de la partición \033[0m" << endl;
        disk.close();
        return false;
    }

    /* OBTENIENDO EL GID DEL GRUPO AL QUE PERTENECE EL USUARIO */

    int gid = -1;
    string group = "";
    contentcopy = usersContent;
    line = strtok((char*)contentcopy.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
        /* GID, Tipo, Grupo -> 1,G,MAX10\n */
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

        group = linecopy;
        if(group == groupName){
            gid = stoi(id);
            break;
        }

        line = strtok(NULL, "\n");
    }

    if(gid == -1){
        cout << "\033[0;91;49m[Error]: No se encontró al grupo " << groupName << " del usuario " << userSesion << "en el archivo users.txt de la partición \033[0m" << endl;
        disk.close();
        return false;
    }

    /* CREANDO ARCHIVO */
    disk.seekg(sbAux.s_inode_start);
    disk.read((char*)&inodeAux, sizeof(Inode)); // Root inode
    int posCurrentInode = sbAux.s_inode_start;
    int posPreviosInode = sbAux.s_inode_start;

    string filename = mf.filePath, directoryName = "";
    wasFound = false;
    size_t pos = 0;

    /* PARENT FOLDERS */
    while ((pos = filename.find("/")) != string::npos) {
        directoryName = filename.substr(0, pos);
        if(directoryName == "" || directoryName == "\0"){
            filename.erase(0, pos + 1);
            continue;
        }

        /* Buscando en bloques directos */
        for(int i = 0; i < 12; i++){
            if(inodeAux.i_block[i] == -1){
                //cout << "PUNTERO EN INODO NULO, CREANDO NUEVO BLOQUE CARPETA\n";
                if(!mf.createParentFolders){
                    cout << "\033[0;91;49m[Error]: no se ha encontrado el subdirectorio " << directoryName
                    << " de la ruta " << mf.filePath << " (use parámetro -p) \033[0m" << endl;
                    disk.close();
                    return false;
                }
                /* Preparando nuevo bloque para inodo del subdirectorio */
                for(int j = 0; j < 4; j++) memset(folderB.b_content[j].b_name, 0, 12);
                for(int j = 0; j < 4; j++) folderB.b_content[j].b_inodo = -1;
                // actualizando inodo actual con nuevo bloque
                inodeAux.i_block[i] = sbAux.s_first_blo;
                time(&inodeAux.i_mtime);
                disk.seekp(posCurrentInode);
                disk.write((char*)&inodeAux, sizeof(Inode));
                /* Escribiendo el bloque carpeta */
                disk.seekp(sbAux.s_first_blo);
                disk.write((char*)&folderB, sizeof(FolderBlock));
                sbAux.s_first_blo += sbAux.s_block_s;
                sbAux.s_free_blocks_count -= 1;
                /* Escribiendo en el bitmap de bloques */
                disk.seekg(sbAux.s_bm_block_start);
                while(!disk.eof()){
                    disk.read((char *)&aux, sizeof(char));
                    if(!disk.eof() && aux == cero){
                        disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                        disk.write((char*)&uno, sizeof(char));
                        break;
                    }
                }
            }

            disk.seekg(inodeAux.i_block[i]);
            disk.read((char*)&folderB, sizeof(FolderBlock));
            // Buscando en sus 4 espacios disponibles
            for(int j = 0; j < 4; j++){
                if(folderB.b_content[j].b_inodo == -1){
                    //cout << "PUNTERO EN BLOQUE CARPETA NULO, CREANDO NUEVO INODO CARPETA\n";
                    if(!mf.createParentFolders){
                        cout << "\033[0;91;49m[Error]: no se ha encontrado el subdirectorio " << directoryName
                        << " de la ruta " << mf.filePath << " (use parámetro -p) \033[0m" << endl;
                        disk.close();
                        return false;
                    }
                    // Actualizando bloque carpeta, agregando el nuevo inodo
                    int posParent = sbAux.s_first_ino;
                    folderB.b_content[j].b_inodo = sbAux.s_first_ino;
                    memset(folderB.b_content[j].b_name, 0, 12);
                    for(int k = 0; k < (int)directoryName.length() && k < 12; k++)
                        {folderB.b_content[j].b_name[k] = directoryName[k];}
                    disk.seekp(inodeAux.i_block[i]);
                    disk.write((char*)&folderB, sizeof(FolderBlock));
                    /* Creando nuevo inodo en actual bloque para subdirectorio */
                    cout << "--> Creando subdirectorio " << directoryName << "..." << endl;
                    inodeAux.i_uid = uid;
                    inodeAux.i_gid = gid;
                    inodeAux.i_s = -1;
                    inodeAux.i_atime  = NULL;
                    time(&inodeAux.i_ctime);
                    time(&inodeAux.i_mtime);
                    inodeAux.i_block[0] = sbAux.s_first_blo; // BLOQUE CON REFERENCIAS AL INODO
                    for(int j = 1; j < 15; j++) inodeAux.i_block[j] = -1;
                    inodeAux.i_type = '0'; // filename
                    inodeAux.i_perm = 664;
                    /* Escribiendo el inodo carpeta del subdirectorio */
                    disk.seekp(sbAux.s_first_ino);
                    disk.write((char*)&inodeAux, sizeof(Inode));
                    sbAux.s_first_ino += sbAux.s_inode_s;
                    sbAux.s_free_inodes_count -= 1;
                    /* Escribiendo en el bitmap de inodos */
                    disk.seekg(sbAux.s_bm_inode_start);
                    while(!disk.eof()){
                        disk.read((char *)&aux, sizeof(char));
                        if(!disk.eof() && aux == cero){
                            disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                            disk.write((char*)&uno, sizeof(char));
                            break;
                        }
                    }
                    /* Escribiendo nuevo bloque carpeta con referencias para subdirectorio */
                    for(int j = 0; j < 4; j++) memset(folderB.b_content[j].b_name, 0, 12);
                    folderB.b_content[0].b_name[0] = '.';
                    folderB.b_content[0].b_inodo = folderB.b_content[j].b_inodo;
                    folderB.b_content[1].b_name[0] = '.';
                    folderB.b_content[1].b_name[1] = '.';
                    folderB.b_content[1].b_inodo = posCurrentInode;
                    folderB.b_content[2].b_inodo = -1;
                    folderB.b_content[3].b_inodo = -1;
                    /* Escribiendo el bloque carpeta */
                    disk.seekp(sbAux.s_first_blo);
                    disk.write((char*)&folderB, sizeof(FolderBlock));
                    sbAux.s_first_blo += sbAux.s_block_s;
                    sbAux.s_free_blocks_count -= 1;
                    /* Escribiendo en el bitmap de bloques */
                    disk.seekg(sbAux.s_bm_block_start);
                    while(!disk.eof()){
                        disk.read((char *)&aux, sizeof(char));
                        if(!disk.eof() && aux == cero){
                            disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                            disk.write((char*)&uno, sizeof(char));
                            break;
                        }
                    }
                    /* Inodo encontrado */
                    posPreviosInode = posCurrentInode;
                    posCurrentInode = posParent;
                    disk.seekg(posParent);
                    disk.read((char*)&inodeAux, sizeof(Inode));
                    wasFound = true;
                    break;
                }

                if(folderB.b_content[j].b_name != directoryName) continue;

                /* Inodo encontrado */
                posPreviosInode = posCurrentInode;
                posCurrentInode = folderB.b_content[j].b_inodo;
                disk.seekg(folderB.b_content[j].b_inodo);
                disk.read((char*)&inodeAux, sizeof(Inode));
                wasFound = true;
                break;
            }
            if(wasFound) break;
        }

        /* No se encontró en los bloques directos */
        if(!wasFound){
            // TODO bloques indirectos
            disk.close();
            return false;
        }

        if(inodeAux.i_type == '1'){
            cout << "\033[0;91;49m[Error]: el nombre " << directoryName << " corresponde a un archivo, no a un subdirectorio \033[0m" << endl;
            disk.close();
            return false;
        }

        filename.erase(0, pos + 1);
        wasFound = false;
    }

    /* Final filename, parent inode -> inodeAux */
    cout << "--> Creando archivo " << filename << "..." << endl;

    // Cálculo de bloques necesarios para escribir el archivo
    float blocksNum = (float)content.length() / 64;
    int fileBlocks = (int)blocksNum;
    if(blocksNum > fileBlocks) fileBlocks += 1;

    bool created = false;
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1){
            //cout << "PUNTERO EN INODO NULO, CREANDO NUEVO BLOQUE CARPETA\n";
            /* Preparando nuevo bloque para inodo del archivo */
            for(int j = 0; j < 4; j++) memset(folderB.b_content[j].b_name, 0, 12);
            for(int j = 0; j < 4; j++) folderB.b_content[j].b_inodo = -1;
            // actualizando inodo padre con nuevo bloque
            inodeAux.i_block[i] = sbAux.s_first_blo;
            time(&inodeAux.i_mtime);
            disk.seekp(posCurrentInode);
            disk.write((char*)&inodeAux, sizeof(Inode));
            /* Escribiendo el bloque carpeta */
            disk.seekp(sbAux.s_first_blo);
            disk.write((char*)&folderB, sizeof(FolderBlock));
            sbAux.s_first_blo += sbAux.s_block_s;
            sbAux.s_free_blocks_count -= 1;
            /* Escribiendo en el bitmap de bloques */
            disk.seekg(sbAux.s_bm_block_start);
            while(!disk.eof()){
                disk.read((char *)&aux, sizeof(char));
                if(!disk.eof() && aux == cero){
                    disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                    disk.write((char*)&uno, sizeof(char));
                    break;
                }
            }
        }
        disk.seekg(inodeAux.i_block[i]);
        disk.read((char*)&folderB, sizeof(FolderBlock));
        for(int j = 0; j < 4; j++){
            if(folderB.b_content[j].b_inodo == -1){
                //cout << "PUNTERO EN BLOQUE CARPETA NULO, CREANDO NUEVO INODO CARPETA\n";
                // Actualizando bloque carpeta, agregando el nuevo inodo
                int posParent = sbAux.s_first_ino;
                folderB.b_content[j].b_inodo = sbAux.s_first_ino;
                memset(folderB.b_content[j].b_name, 0, 12);
                for(int k = 0; k < (int)filename.length() && k < 12; k++)
                    {folderB.b_content[j].b_name[k] = filename[k];}
                disk.seekp(inodeAux.i_block[i]);
                disk.write((char*)&folderB, sizeof(FolderBlock));
                /* CREANDO INODO CARPETA DEL DIRECTORIO */
                inodeAux.i_uid = uid;
                inodeAux.i_gid = gid;
                inodeAux.i_s = (int)content.length();
                inodeAux.i_atime  = NULL;
                time(&inodeAux.i_ctime);
                time(&inodeAux.i_mtime);

                for(int i = 0; i < 12; i++){
                    if(i < fileBlocks) inodeAux.i_block[i] = sbAux.s_first_blo + (i)*sbAux.s_block_s;
                    else inodeAux.i_block[i] = -1;
                }
                inodeAux.i_type = '1'; // file
                inodeAux.i_perm = 664;
                /* Escribiendo el inodo archivo */
                disk.seekp(sbAux.s_first_ino);
                disk.write((char*)&inodeAux, sizeof(Inode));
                sbAux.s_first_ino += sbAux.s_inode_s;
                sbAux.s_free_inodes_count -= 1;
                /* Escribiendo en el bitmap de inodos */
                disk.seekg(sbAux.s_bm_inode_start);
                while(!disk.eof()){
                    disk.read((char *)&aux, sizeof(char));
                    if(!disk.eof() && aux == cero){
                        disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                        disk.write((char*)&uno, sizeof(char));
                        break;
                    }
                }


                /* CREACIÓN DE BLOQUES ARCHIVO */
                memset(fileB.b_content, 0, 64);
                int maxContent = 0;
                for(int i = 0; i < (int)content.length(); i++){
                    fileB.b_content[maxContent] = content[i];
                    maxContent += 1;
                    if(maxContent == 64){
                        /* Escribiendo el bloque carpeta para pasar al otro */
                        disk.seekp(sbAux.s_first_blo);
                        disk.write((char*)&fileB, sizeof(FileBlock));
                        sbAux.s_first_blo += sbAux.s_block_s;
                        sbAux.s_free_blocks_count -= 1;
                        /* Escribiendo en el bitmap de bloques */
                        disk.seekg(sbAux.s_bm_block_start);
                        while(!disk.eof()){
                            disk.read((char *)&aux, sizeof(char));
                            if(!disk.eof() && aux == cero){
                                disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                                disk.write((char*)&uno, sizeof(char));
                                break;
                            }
                        }
                        memset(fileB.b_content, 0, 64); // limpiando el content para volver a llenarlo
                        maxContent = 0;
                    }
                }
                if(maxContent > 0){
                    /* Escribiendo el úlitmo bloque carpeta que no llegó a ocupar los 64 bytes */
                    disk.seekp(sbAux.s_first_blo);
                    disk.write((char*)&fileB, sizeof(FileBlock));
                    sbAux.s_first_blo += sbAux.s_block_s;
                    sbAux.s_free_blocks_count -= 1;
                    /* Escribiendo en el bitmap de bloques */
                    disk.seekg(sbAux.s_bm_block_start);
                    while(!disk.eof()){
                        disk.read((char *)&aux, sizeof(char));
                        if(!disk.eof() && aux == cero){
                            disk.seekp(-1, ios::cur);
                            disk.write((char*)&uno, sizeof(char));
                            break;
                        }
                    }
                }

                created = true;
                break;
            }

            if(folderB.b_content[j].b_name != filename) continue;

            cout << "[Mensaje]: Ya existe el archivo " << filename << ", indicado en la ruta " << mf.filePath << endl;
            disk.close();
            return false;
        }
        if(created) break;
    }

    /* Sobrescribiendo Superbloque con la información actualizada */
    disk.seekp(partitionStart);
    disk.write((char*)&sbAux, sizeof(Superblock));

    disk.close();
    return true;
}


Mkfile _Mkfile(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* path
    string path = "";
    bool vPath = false;
    //* r
    bool createParentFolders = false;
    //* size
    string tamano = "";
    int tamanoArchivo = 0;
    bool vTamano = false;
    //* contenido path
    string contenidoPath = "";
    bool vContenido = false;

    for(int i = 0; i <= (int)strlen(parametros); i++){
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
            //* Reconocimiento del caracter s
            //* Reconocimiento del caracter r
            //* Reconocimiento del caracter c
            case 1: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 2;
                else if ((char)tolower(parametros[i]) == 's') estado = 9;
                else if ((char)tolower(parametros[i]) == 'r') {createParentFolders = true; parametroActual = ""; estado = 0;}
                else if ((char)tolower(parametros[i]) == 'c') estado = 16;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> path
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
            //* Reconocimiento de la path con comillas
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
                if(parametros[i] == '-'){vTamano = false; tamano = parametros[i]; estado = 14;}
                else if (isNumber(parametros[i])){vTamano = false; tamano = parametros[i]; estado = 15;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que sean numeros enteros
            case 14: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])){tamano += parametros[i]; estado = 15;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 15: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])) tamano += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if (stoi(tamano) <= 0) cout << "\033[0;91;49m[Error]: El valor del parametro >size debe de ser entero positivo. \033[0m" << endl;
                    else {vTamano = true; tamanoArchivo = stoi(tamano);}
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> cont
            //* Reconocimiento del caracter o
            case 16: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'o') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter n
            case 17: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'n') estado = 18;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 18: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 19;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 19: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 20;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de la path con comillas
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
            //* Reconocimiento de la path sin comillas
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
            //* Parametro invalido
            case -1: {
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es invalido para el comando mkfile. \033[0m" << endl;
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
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Mkfile mf;
    if(vPath){
        mf.filePath = path;
        mf.createParentFolders = createParentFolders;
        if(vTamano) mf.fileSize = tamanoArchivo;
        else mf.fileSize = 0;
        if(vContenido) mf.contentPath = contenidoPath;
        else mf.contentPath = "";
        mf.acceso = true;
        return mf;
    }
    //if(!vPath) cout << "\033[0;91;49m[Error]: El parametro \">path\" es obligatorio para crear el archivo. \033[0m" << endl;
    mf.acceso = false;
    return mf;
}
