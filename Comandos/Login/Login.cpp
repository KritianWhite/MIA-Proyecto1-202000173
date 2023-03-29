#include <iostream>
#include <string.h>
#include <fstream>

#include "Login.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"
#include "../Mount/Mount.h"


using namespace std;

bool LoginUser(bool openSesion, string partitionId, string username, string password, PartitionNode *&firstNode){
    if(openSesion){
        cout << "\033[0;91;49m[Error]: Ya se encuentra una sesión activa (use logout primero) \033[0m" << endl;
        return false;
    }

    /* Verificando que el id represente una partición montada */
    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    cout << "--> Obteniendo datos del disco..." << endl;
    ifstream disk;
    string diskPath = getDiskPath(firstNode, partitionId);
    disk.open(diskPath, ios::in|ios::binary);
    if(disk.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << diskPath << ".\033[0m" << endl;
        disk.close();
        return false;
    }

    char aux;
    disk.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "\033[0;91;49m[Error]: No vino ch ar idMBR antes del MBR. Error interno.\033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
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
        cout << "\033[0;91;49m[Error]:  La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m" << endl;
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
    bool wasFound = false;
    disk.seekg(sb.s_inode_start);
    disk.read((char*)&inodeAux, sizeof(Inode)); // Root inode

    /* Buscando en bloques directos */
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1){
            cout << "\033[0;91;49m[Error]: se llegó a un pointer = -1 en el inodo root sin encontrar el users.txt \033[0m" << endl;
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

    /* Buscando al usuario que inició sesión */
    char *line;
    string linecopy = "", id = "", type = "", group = "", user = "", pass = "";
    line = strtok((char*)content.c_str(), "\n");
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
        group = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pos = linecopy.find(",");
        user = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);
        pass = linecopy;

        if(user == username && pass == password){
            disk.close();
            if(id != "0") return true;
            cout << "\033[0;91;49m[Error]: El usuario " << user << " ha sido eliminado. \033[0m" << endl;
            return false;
        }

        line = strtok(NULL, "\n");
    }

    disk.close();
    return false;
}


Login _Login(char* parametros){
    //* Variables de control
    string parametroActual = "";
    string comentario = "";
    int estado = 0;
    //* User
    string usuario = "";
    bool vUser = false;
    //* Paswword
    string contrasena = "";
    bool vContrasena = false;
    //* Particion id
    string particionID = "";
    bool vID = false;

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
                else if ((char)tolower(parametros[i]) == 'i') estado = 20;
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
            //TODO--> Password
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
                if ((char)tolower(parametros[i]) == 's') estado = 16;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter w
            case 12: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'w') estado = 13;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter o
            case 13: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'o') estado = 14;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter r
            case 14: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'r') estado = 15;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter d
            case 15: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'd') estado = 16;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 16: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 17: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vContrasena = false; contrasena = "", estado = 18;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vContrasena = false; contrasena = ""; contrasena += parametros[i]; estado = 19;}
                break;
            }
            case 18: {
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
            case 19: {
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
            //TODO--> Id
            //* Reconocimiento del caracter d
            case 20: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 21;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 21: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 22;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 22: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vID = false; particionID = ""; estado = 23;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vID = false; particionID = ""; particionID += parametros[i]; estado = 24;}
                break;
            }
            case 23: {
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
            //* Reconocimiento sin comillas dobles
            case 24: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(particionID.length() > 0) vID = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un id vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else particionID += parametros[i];
                break;
            }
            //* Reconocimiento de error sintactico
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para login. \033[0m" << endl;
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

    Login lg;
    if(vUser && vContrasena && vID){
        lg.username = usuario;
        lg.password = contrasena;
        lg.partitionId = particionID;
        lg.acceso = true;
        return lg;
    }

    if(!vUser) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">user\" para poder iniciar sesion.\033[0m" << endl;
    if(!vContrasena) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">password\" para poder iniciar sesion.\033[0m" << endl;
    if(!vID) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">id\" para poder iniciar sesion.\033[0m" << endl;

    lg.acceso = false;
    return lg;
}
