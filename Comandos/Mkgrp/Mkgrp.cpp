#include <iostream>
#include <string.h>
#include <fstream>

#include "Mkgrp.h"

using namespace std;


bool CrearGrupo(bool openSesion, string user, string partitionId, string groupName, PartitionNode *&firstNode){
    /* VALIDACIONES */
    if(!openSesion){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    string rootuser = "root";
    if(user != rootuser){
        cout << "\033[0;91;49m[Error]: la sesión activa corresponde al usuario " << user << ", mkgrp necesita que el usuario sea root \033[0m" << endl;
        return false;
    }

    cout << "--> Obteniendo datos del disco..." << endl;
    fstream disk;
    string diskPath = getDiskPath(firstNode, partitionId);
    disk.open(diskPath, ios::in|ios::out|ios::binary);
    if(disk.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << diskPath << "\033[0m" << endl;
        disk.close();
        return false;
    }

    char aux;
    disk.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "\033[0;91;49m[Error]:  no vino char idMBR antes del MBR. \033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
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

    Superblock sb;
    /* SUBERBLOCK PRIMARY PARTITION */
    if(numberPrimaryPartition != 0){
        disk.seekg(partitionStart);
        disk.read((char*)&sb, sizeof(Superblock));
    }
    // TODO SUPERBLOCK LOGICAL PARTITIONS
    else {
        disk.close();
        return false;
    }

    /* LOOKING FOR THE USERS.TXT FILE */
    Inode inodeAux;
    FolderBlock folderB;
    string name = "users.txt";
    int inodeAuxPos = -1;
    bool wasFound = false;
    disk.seekg(sb.s_inode_start);
    disk.read((char*)&inodeAux, sizeof(Inode)); // Root inode

    /* Buscando en bloques directos */
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1){
            cout << "\033[0;91;49m[Error]:se llegó a un pointer = -1 en el inodo root sin encontrar el users.txt\033[0m" << endl;
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
            inodeAuxPos = folderB.b_content[j].b_inodo;
            disk.seekg(folderB.b_content[j].b_inodo);
            disk.read((char*)&inodeAux, sizeof(Inode));
            wasFound = true;
            break;
        }
        if(wasFound) break;
    }

    /* No se encontró en los bloques directos */
    if(!wasFound){
        cout << "\033[0;91;49m[Error]: se leyeron todos los bloques directos sin encontrar users.txt \033[0m" << endl;
        disk.close();
        return false;
    }

    /* Inodo de users.txt se encuentra en inodeAux */
    FileBlock fileB;
    string content = "";
    /* Recorriendo bloques directos para leer su contenido */
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1) break;
        disk.seekg(inodeAux.i_block[i]);
        disk.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] == 0) break;
            content += fileB.b_content[j];
        }
    }

    if(content.length() == (12*64)){
        // TODO seguir leyendo bloques indirectos
    }

    char *line;
    int currentGroupId = 0;
    string contentcopy = content, linecopy = "", id = "", type = "", group = "";
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
            cout << "\033[0;91;49m[Error]: El grupo " << group << " ya existe en la partición. \033[0m\n";
            disk.close();
            return false;
        }
        currentGroupId += 1;

        line = strtok(NULL, "\n");
    }

    string newContent = to_string(currentGroupId + 1);
    newContent += ",G,";
    newContent += groupName;
    newContent += "\n";
    bool wasWritten = false;
    int k = 0, blockPos = -1;

    /* Recorriendo bloques directos del inodo de users.txt */
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1){
            // se llegó a un pointer vacío, por lo que requiere un nuevo bloque
            // escribiendolo en el bitmap de bloques
            disk.seekg(sb.s_bm_block_start);
            while(!disk.eof()){
                disk.read((char *)&aux, sizeof(char));
                if(!disk.eof() && aux == cero){
                    disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                    disk.write((char*)&uno, sizeof(char));
                    break;
                }
            }
            blockPos = sb.s_first_blo;
            inodeAux.i_block[i] = blockPos;
            sb.s_first_blo += sb.s_block_s;
            sb.s_free_blocks_count -= 1;
            memset(fileB.b_content, 0, 64);
        }else{
            // existe el bloque de archivos
            blockPos = inodeAux.i_block[i];
            disk.seekg(inodeAux.i_block[i]);
            disk.read((char*)&fileB, sizeof(FileBlock));
        }
        // escribiendo el nuevo contenido
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] != 0) continue;
            fileB.b_content[j] = newContent[k];
            wasWritten = true;
            k++;
            if(k >= (int)newContent.length()) break;
        }
        // escribiendo o sobrescribiendo el bloque con el contenido nuevo, según el caso
        if(wasWritten){
            disk.seekp(blockPos);
            disk.write((char*)&fileB, sizeof(FileBlock));
            time(&inodeAux.i_mtime);
            if(k >= (int)newContent.length()) break;
            /* Falta contenido por escribirse */
            memset(fileB.b_content, 0, 64);
            wasWritten = false;
        }
    }

    /* Sobrescribiendo inodeAux (inodo users.txt) */
    disk.seekp(inodeAuxPos);
    disk.write((char*)&inodeAux, sizeof(Inode));
    /* Sobrescribiendo superbloque */
    disk.seekp(partitionStart);
    disk.write((char*)&sb, sizeof(Superblock));

    disk.close();
    return true;
}


Mkgrp _Mkgrp(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* Name
    string nombreGrupo = "";
    bool vNombre = false;

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
            //TODO--> NAME
            //* Reconocimiento del caracter n
            case 1: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'n') estado = 2;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter a
            case 2: {
                //cout << "Llegue al 16" << endl;
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter m
            case 3: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'm') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter e
            case 4: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter =
            case 5: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de las comillas dobles
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {vNombre = false; nombreGrupo = ""; estado = 7;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vNombre = false; nombreGrupo = ""; nombreGrupo += parametros[i]; estado = 8;}
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(nombreGrupo.length() > 0) vNombre = true;
                    else cout << "\033[0;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vacio. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreGrupo += parametros[i];
                break;
            }
            //* Reconocimiento sin las comillas dobles
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(nombreGrupo.length() > 0)vNombre = true;
                    else cout << "\033[0;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vacio. \033[0m" << endl;
                    parametroActual = ""; 
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreGrupo += parametros[i];
                break;
            }

            //* Error de sintaxis
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para mkgrp. \033[0m" << endl;
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

    Mkgrp mg;
    if(vNombre){
        mg.groupName = nombreGrupo;
        mg.acceso = true;
        return mg;
    }

    cout << "\033[0;91;49m[Error]: Falta el parametro obligatorio \">name\" para poder crear el grupo.\033[0m" << endl;
    mg.acceso = false;
    return mg;
}