#include <iostream>
#include <string.h>
#include <fstream>

#include "Rmusr.h"
#include "../Estructura.h"
#include "../Mount/Mount.h"
#include "../../aux_funciones.h"

using namespace std;

bool EliminarUsuario(bool openSesion, string userSesion, string partitionId, string rmUsername, PartitionNode *&firstNode){
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

    if(rmUsername == rootuser){
        cout << "\033[0;91;49m> Error: el usuario root no se puede eliminar \033[0m\n";
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
    /* Recorriendo bloques directos para leer su contenido */
    FileBlock fileB;
    string content = "";
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1) break;
        disk.seekg(inodeAux.i_block[i]);
        disk.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] == 0) break;
            content += fileB.b_content[j];
        }
    }

    size_t pos = 0;
    bool deleted = false;
    string contentCopy = content, newContent = "", linecopy = "";
    string id = "", type = "", group = "", usr = "", pass = "";
    /* Leyendo el contenido de users.txt para cambiar id de usuario */
    while((pos = contentCopy.find("\n")) != string::npos){
        linecopy = contentCopy.substr(0, pos);
        contentCopy.erase(0, pos + 1);

        /* GID, Tipo, Grupo -> 1,G,MAX10\n */
        /* UID, Tipo, Grupo, Usuario, Contraseña -> 1,U,root,root,123\n */

        pos = linecopy.find(",");
        id = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        type = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        group = linecopy.substr(0, pos);

        if(type == "G"){
            newContent += id + "," + type + "," + group + "\n";
            continue;
        }

        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        usr = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pass = linecopy;
        if(!deleted && usr == rmUsername){
            if(id == "0"){
                cout << "> El usuario " << usr << " ya ha sido eliminado previamente\n";
                disk.close();
                return false;
            }
            newContent += "0," + type + "," + group + "," + usr + "," + pass + "\n";
            deleted = true;
        }
        else newContent += id + "," + type + "," + group + "," + usr + "," + pass + "\n";
    }

    if(deleted){
        /* Actualizando bloques con la nueva información */
        int k = 0;
        for(int i = 0; i < 12; i++){
            if(inodeAux.i_block[i] == -1){
                cout << "> Error interno: se llegó a un pointer vacío (-1), cuando el contenido debería ser <= al de antes\n";
                disk.close();
                return false;
            }
            // existe el bloque de archivos
            // escribiendo el nuevo contenido
            memset(fileB.b_content, 0, 64);
            for(int j = 0; j < 64; j++){
                fileB.b_content[j] = newContent[k];
                k++;
                if(k >= (int)newContent.length()) break;
            }
            disk.seekp(inodeAux.i_block[i]);
            disk.write((char*)&fileB, sizeof(FileBlock));
            if(k >= (int)newContent.length()) break;
        }
        time(&inodeAux.i_mtime);
        /* Sobrescribiendo inodeAux (inodo users.txt) */
        disk.seekp(inodeAuxPos);
        disk.write((char*)&inodeAux, sizeof(Inode));

        disk.close();
        return true;
    }

    // TODO seguir leyendo bloques indirectos

    cout << "\033[0;91;49m> Error: no se encontró al usuario " << rmUsername << " \033[0m\n";
    disk.close();
    return false;
}



Rmusr _Rmusr(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* name 
    string usuario = "";
    bool vUser = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
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
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'u') estado = 2;
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
                if(parametros[i] == '\"') {vUser = false; usuario = ""; estado = 7;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vUser = false; usuario = ""; usuario += parametros[i]; estado = 8;}
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(usuario.length() > 0) vUser = true;
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
                    vUser = true;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else usuario += parametros[i];
                break;
            }
            //* Reconocimiento de error sintactico
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para rmusr. \033[0m" << endl;
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

    Rmusr ru;
    if(vUser){
        ru.username = usuario;
        ru.acceso = true;
        return ru;
    }

    cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">user\" para ppoder eliminar el usuario.\033[0m" << endl;
    ru.acceso = false;
    return ru;
}