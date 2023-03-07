#include <iostream>
#include <string.h>
#include <fstream>

#include "Mkusr.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"
#include "../Mount/Mount.h"

using namespace std;


bool createUser(bool openSesion, string userSesion, string newUser, string password, string groupName, string partitionId, PartitionNode *&firstNode){
    /* VALIDACIONES */
    if(!openSesion){
        cout << "\033[0;91;49m> Error: no se encontró sesión activa\033[0m\n";
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m> La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m\n";
        return false;
    }

    string rootuser = "root";
    if(userSesion != rootuser){
        cout << "\033[0;91;49m> Error: la sesión activa corresponde al usuario " << userSesion << ", mkusr necesita que el usuario sea root \033[0m\n";
        return false;
    }

    cout << "> Obteniendo datos del disco...\n";
    fstream disk;
    string diskPath = getDiskPath(firstNode, partitionId);
    disk.open(diskPath, ios::in|ios::out|ios::binary);
    if(disk.fail()){
        cout << "\033[0;91;49m> Error: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << diskPath << "\033[0m\n";
        disk.close();
        return false;
    }

    char aux;
    disk.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "> Error interno: no vino char idMBR antes del MBR\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
        cout << "\033[0;91;49m> Error: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m\n";
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
            cout << "> Error interno: se llegó a un pointer = -1 en el inodo root sin encontrar el users.txt\n";
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
        cout << "> Error interno: se leyeron todos los bloques directos sin encontrar users.txt\n";
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
    int currentUserId = 0;
    bool groupExists = false;
    string contentcopy = content, linecopy = "", id = "", type = "", group = "", usr = "", pass = "";
    line = strtok((char*)contentcopy.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
        /* GID, Tipo, Grupo -> 1,G,MAX10\n */
        /* UID, Tipo, Grupo, Usuario, Contraseña -> 1,U,root,root,123\n */
        size_t pos = 0;
        pos = linecopy.find(",");
        id = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        type = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        group = linecopy.substr(0, pos);
        if(!groupExists && group == groupName){
            if(id == "0"){
                cout << "\033[0;91;49m> Error: El grupo " << group << " ha sido eliminado. \033[0m\n";
                disk.close();
                return false;
            }
            groupExists = true;
        }

        if(type == "G"){
            line = strtok(NULL, "\n");
            continue;
        }
        linecopy.erase(0, pos + 1);

        pos = linecopy.find(",");
        usr = linecopy.substr(0, pos);
        if(usr == newUser){
            cout << "\033[0;91;49m> Error: El usuario " << usr << " ya existe en la partición \033[0m\n";
            disk.close();
            return false;
        }
        linecopy.erase(0, pos + 1);
        pass = linecopy;

        currentUserId += 1;

        line = strtok(NULL, "\n");
    }

    if(!groupExists){
        cout << "\033[0;91;49m> Error: El grupo " << groupName << " no ha sido creado en la partición \033[0m\n";
        disk.close();
        return false;
    }

    string newContent = to_string(currentUserId + 1);
    newContent += ",U,";
    newContent += groupName;
    newContent += ",";
    newContent += newUser;
    newContent += ",";
    newContent += password;
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


Mkusr _Mkusr(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* user
    string usuario = "";
    bool vUsuario = false;
    //* password
    string contrasena = "";
    bool vContrasena = false;
    //* group
    string nombreGrupo = "";
    bool vGrupo = false;

    for(int i = 0; i <= (int)strlen(parametros); i++){
        switch (estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if (parametros[i] == '>') estado = 1;
                else if (parametros[i] == 9 || parametros[i] == 32) ; //* Extra
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado -1;
                break;
            }
            //* Reconocimiento del caracter u
            //* Reconocimiento del caracter p
            //* Reconocimiento del caracter i
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'u') estado = 2;
                else if ((char)tolower(parametros[i]) == 'p') estado = 9;
                else if ((char)tolower(parametros[i]) == 'g') estado = 16;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> User
            //* Reconocimiento del caracter s
            case 2: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 's') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter r
            case 4: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'r') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimento de comillas dobles
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {vUsuario = false; usuario = ""; estado = 7;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vUsuario = false; usuario = ""; usuario += parametros[i]; estado = 8;}
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(usuario.length() > 0) vUsuario = true;
                    else cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene un nombre de usuario. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else usuario += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    vUsuario = true;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else usuario += parametros[i];
                break;
            }
            //TODO--> Pass
            //* Reconocimiento del caracter a
            case 9: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'a') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter s
            case 10: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 's') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter s
            case 11: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 's') estado = 12;
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
            //* Reconocimiento de comillas dobles
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vContrasena = false; contrasena = "", estado = 14;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vContrasena = false; contrasena = ""; contrasena += parametros[i]; estado = 15;}
                break;
            }
            case 14: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(contrasena.length() > 0) vContrasena = true;
                    else cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene una contraseña. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado -2;}
                else contrasena += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 15: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(contrasena.length() > 0) vContrasena = true;
                    else cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene una contraseña. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else contrasena += parametros[i];
                break;
            }
            //TODO--> GRP
            //* Reconocimiendo del caracter r
            case 16: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'r') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter p
            case 17: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 18;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 18: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 19;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 19: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vGrupo = false; nombreGrupo = "", estado = 20;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vGrupo = false; nombreGrupo = ""; nombreGrupo += parametros[i]; estado = 21;}
                break;
            }
            case 20: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(nombreGrupo.length() > 0) vGrupo = true;
                    else cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene una contraseña. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado -2;}
                else nombreGrupo += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 21: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(nombreGrupo.length() > 0) vGrupo = true;
                    else cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene una contraseña. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreGrupo += parametros[i];
                break;
            }
            //* Reconocimiento de error sintactico
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
            //* Comentarios.
            case -2: {
                comentario += parametros[i];
                break;
            }
        }//* End switch case
    }//* End for loop
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Mkusr mu;
    if(vUsuario && vContrasena && vGrupo){
        mu.username = usuario;
        mu.password = contrasena;
        mu.groupname = nombreGrupo;
        mu.acceso = true;
        return mu;
    }

    if(!vUsuario) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">user\" para poder crear el usuario.\033[0m" << endl;
    if(!vContrasena) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">pass\" para poder crear el usuario.\033[0m" << endl;
    if(!vGrupo) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">grp\" para poder crear el usuario.\033[0m" << endl;

    mu.acceso = false;
    return mu;
}