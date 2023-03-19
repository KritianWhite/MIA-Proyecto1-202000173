#include <iostream>
#include <string.h>
#include <fstream>

#include "Chgrp.h"
#include "../Mount/Mount.h"
#include "../../aux_funciones.h"
#include "../Estructura.h"

using namespace std;


bool CambiarGrupo(bool openSesion, string userSesion, string partitionId, string username, string newGroup, PartitionNode *&firstNode){
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
    if(userSesion != rootuser){
        cout << "\033[0;91;49m[Error]: la sesión activa corresponde al usuario " << userSesion << ", mkusr necesita que el usuario sea root \033[0m" << endl;
        return false;
    }

    if(username == rootuser){
        cout << "\033[0;91;49m[Error]: no se puede cambiar de grupo al usuario root \033[0m" << endl;
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

    string nombrePArticion = getPartitionName(firstNode, partitionId);
    int numeroParticionPrimaria = 0, particionInicial = 0, numeroParticionExtendida = 0;
    char partStatus = '0';

    while(true){
        if(mb.mbr_partition_1.part_status != 'E'){
            if(mb.mbr_partition_1.part_name == nombrePArticion){
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
            if(mb.mbr_partition_2.part_name == nombrePArticion){
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
            if(mb.mbr_partition_3.part_name == nombrePArticion){
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
            if(mb.mbr_partition_4.part_name == nombrePArticion){
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

    /* Validando que la partición contenga formato */
    if(partStatus != 'F'){
        cout << "\033[0;91;49m[Error]: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m " << endl;
        disco.close();
        return false;
    }

    Superblock sb;
    /* SUBERBLOCK PRIMARY PARTITION */
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sb, sizeof(Superblock));
    }
    // TODO SUPERBLOCK LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    /* LOOKING FOR THE USERS.TXT FILE */
    Inode iAux;
    FolderBlock folderB;
    string name = "users.txt";
    int iAuxPosicion = -1;
    bool wasFound = false;
    disco.seekg(sb.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode)); // Root inode

    /* Buscando en bloques directos */
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1){
            cout << "\033[0;91;49m[Error]: se llegó a un pointer = -1 en el inodo root sin encontrar el users.txt \033[0m" << endl;
            disco.close();
            return false;
        }

        disco.seekg(iAux.i_block[i]);
        disco.read((char*)&folderB, sizeof(FolderBlock));

        /* Buscando en sus 4 espacios disponibles */
        for(int j = 0; j < 4; j++){
            if(folderB.b_content[j].b_inodo == -1) break;
            if(folderB.b_content[j].b_name != name) continue;

            /* Inodo de users.txt encontrado */
            iAuxPosicion = folderB.b_content[j].b_inodo;
            disco.seekg(folderB.b_content[j].b_inodo);
            disco.read((char*)&iAux, sizeof(Inode));
            wasFound = true;
            break;
        }
        if(wasFound) break;
    }

    /* No se encontró en los bloques directos */
    if(!wasFound){
        cout << "\033[0;91;49m[Error]: se leyeron todos los bloques directos sin encontrar users.txt \033[0m" << endl;
        disco.close();
        return false;
    }

    /* Inodo de users.txt se encuentra en iAux */
    /* Recorriendo bloques directos para leer su contenido */
    FileBlock fileB;
    string content = "";
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1) break;
        disco.seekg(iAux.i_block[i]);
        disco.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] == 0) break;
            content += fileB.b_content[j];
        }
    }

    /* Validando que el nuevo grupo exista */

    char *line;
    size_t pos = 0;
    bool groupExists = false;
    string contentCopy = content, linecopy = "";
    string id = "", type = "", group = "";
    line = strtok((char*)contentCopy.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
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
        if(group == newGroup){
            if(id == "0"){
                cout << "\033[0;91;49m[Error]: El grupo " << group << " que se intenta asignar al usuario ha sido eliminado. \033[0m" << endl;
                disco.close();
                return false;
            }
            groupExists = true;
            break;
        }

        line = strtok(NULL, "\n");
    }

    if(!groupExists){
        cout << "\033[0;91;49m[Error]: El grupo " << newGroup << " no ha sido creado en la partición \033[0m" << endl;
        disco.close();
        return false;
    }

    pos = 0;
    bool cambiado = false;
    string newContent = "", usr = "", pass = "";
    contentCopy = content;
    linecopy = "";
    id = "";
    type = "";
    group = "";
    /* Leyendo el contenido de users.txt para cambiar grupo de usuario */
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
        if(!cambiado && usr == username){
            if(id == "0"){
                cout << "--> El usuario " << usr << " ha sido eliminado, por lo que no se le puede asignar un grupo" << endl;
                disco.close();
                return false;
            }
            cout << "--> Cambiando al usuario del grupo " << group << " al grupo " << newGroup << "...\n";
            newContent += id + "," + type + "," + newGroup + "," + usr + "," + pass + "\n";
            cambiado = true;
        }
        else newContent += id + "," + type + "," + group + "," + usr + "," + pass + "\n";
    }

    if(cambiado){
        /* Actualizando bloques con la nueva información */
        int k = 0;
        for(int i = 0; i < 12; i++){
            if(iAux.i_block[i] == -1){
                // TODO crear bloques, ya que es posible que el grupo posea más caracteres que el antiguo
                cout << "\033[0;91;49m[Error]: se llegó a un pointer vacío (-1), cuando el contenido debería ser <= al de antes \033[0m" << endl;
                disco.close();
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
            disco.seekp(iAux.i_block[i]);
            disco.write((char*)&fileB, sizeof(FileBlock));
            if(k >= (int)newContent.length()) break;
        }

        time(&iAux.i_mtime);
        /* Sobrescribiendo iAux (inodo users.txt) */
        disco.seekp(iAuxPosicion);
        disco.write((char*)&iAux, sizeof(Inode));

        disco.close();
        return true;
    }

    // TODO seguir leyendo bloques indirectos

    cout << "\033[0;91;49m[Error]: no se encontró al usuario " << username << " \033[0m" << endl;
    disco.close();
    return false;
}


Chgrp _Chgrp(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* user
    string usuario = "";
    bool vUsuario = false;
    //* group
    string nombreGrupo = "";
    bool vGrupo = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
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
            //* Reconocimiento del caracter g
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'u') estado = 2;
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
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para chgrp. \033[0m" << endl;
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

    Chgrp cg;
    if(vUsuario && vGrupo){
        cg.username = usuario;
        cg.newGroup = nombreGrupo;
        cg.acceso = true;
        return cg;
    }

    if(!vUsuario) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">user\" para poder cambiar el usuario al otro grupo.\033[0m" << endl;
    if(!vGrupo) cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">grp\" para poder cambiar el usuario al otro grupo.\033[0m" << endl;

    cg.acceso = false;
    return cg;
}