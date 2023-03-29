#include <iostream>
#include <string.h>
#include <fstream>

#include "../../Mount/Mount.h"
#include "../../Estructura.h"
#include "../../../aux_funciones.h"

using namespace std;

string getUsersContent(ifstream& disco, Superblock& sbAux){
    Inode inodeAux;
    FolderBlock folderB;
    string name = "users.txt";
    bool wasFound = false;
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&inodeAux, sizeof(Inode)); // Root inode

    //* Buscando en bloques directos 
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1){
            cout << "\033[0;91;49m[Error]: se llegó a un pointer = -1 en el inodo root sin encontrar el users.txt\033[0m" << endl;
            return "";
        }

        disco.seekg(inodeAux.i_block[i]);
        disco.read((char*)&folderB, sizeof(FolderBlock));

        //* Buscando en sus 4 espacios disponibles 
        for(int j = 0; j < 4; j++){
            if(folderB.b_content[j].b_inodo == -1) break;
            if(folderB.b_content[j].b_name != name) continue;

            //* Inodo de users.txt encontrado 
            disco.seekg(folderB.b_content[j].b_inodo);
            disco.read((char*)&inodeAux, sizeof(Inode));
            wasFound = true;
            break;
        }
        if(wasFound) break;
    }

    //* No se encontró en los bloques directos 
    if(!wasFound){
        cout << "\033[0;91;49m[Error]: se leyeron todos los bloques directos sin encontrar users.txt \033[0m" << endl;
        return "";
    }


    FileBlock fileB;
    string content = "";
    //* Recorriendo bloques directos para leer su contenido 
    for(int i = 0; i < 12; i++){
        if(inodeAux.i_block[i] == -1) break;
        disco.seekg(inodeAux.i_block[i]);
        disco.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] == 0) break;
            content += fileB.b_content[j];
        }
    }

    if(content.length() == (12*64)){
        // TODO seguir leyendo bloques indirectos
    }

    return content;
}

string getPermissions(char num){
    switch(num){
        case '0': return "---";
        case '1': return "--x";
        case '2': return "-w-";
        case '3': return "-wx";
        case '4': return "r--";
        case '5': return "r-x";
        case '6': return "rw-";
        case '7': return "rwx";
        default: return "";
    }
}

string getOwner(int uid, string content){
    char *line;
    string contentcopy = content;
    string linecopy = "", id = "", type = "", groupName = "", user = "", pass = "";
    line = strtok((char*)contentcopy.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
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
        if(stoi(id) != uid){
            line = strtok(NULL, "\n");
            continue;
        }

        pos = linecopy.find(",");
        groupName = linecopy.substr(0, pos);
        linecopy.erase(0, pos + 1);

        pos = linecopy.find(",");
        user = linecopy.substr(0, pos);
        return user;

        linecopy.erase(0, pos + 1);
        pass = linecopy;

        line = strtok(NULL, "\n");
    }
    return "";
}

string getGroup(int gid, string content){
    char *line;
    string contentcopy = content;
    string linecopy = "", id = "", type = "", groupName = "";
    line = strtok((char*)contentcopy.c_str(), "\n");
    while(line != NULL){
        linecopy = line;
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
        if(stoi(id) != gid){
            line = strtok(NULL, "\n");
            continue;
        }

        groupName = linecopy;
        return groupName;

        line = strtok(NULL, "\n");
    }
    return "";
}

string recursivePerm(ifstream &disco, Inode &actualInode, int posActualInode, char* inodeName, string usersContent){
    string cadena = "", parent = ".", previousParent = "..", perm = "";

    //* Creando reporte de inodo 
    cadena += "\n\t\t<TR>\n\t\t<TD border=\"2\"  bgcolor=\"white\">";
    perm = to_string(actualInode.i_perm);
    for(int j = 0; j < 3; j++) cadena += getPermissions(perm[j]);
    cadena += "</TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"white\">";
    cadena += getOwner(actualInode.i_uid, usersContent);
    cadena += "</TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"white\">";
    cadena += getGroup(actualInode.i_gid, usersContent);
    cadena += "</TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"white\">";
    cadena += to_string(actualInode.i_s);
    cadena += "</TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"white\">";
    if(actualInode.i_ctime != NULL) cadena += ctime(&actualInode.i_mtime);
    else cadena += "-";
    cadena += "</TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"white\">";
    cadena += actualInode.i_type;
    if(actualInode.i_type == '0') cadena += " (carpeta)";
    else cadena += " (archivo)";
    cadena += "</TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"white\">";
    cadena += inodeName;
    cadena += "</TD>\n\t\t</TR>";

    if(actualInode.i_type == '1') return cadena; // Inodo archivo

    //* Recorriendo bloques directos del inodo carpeta actual 
    FolderBlock folderBlock_aux;
    for(int i = 0; i < 12; i++){
        if(actualInode.i_block[i] == -1) break;

        //* Bloque carpeta 
        disco.seekg(actualInode.i_block[i]);
        disco.read((char*)&folderBlock_aux, sizeof(FolderBlock));

        //* Generando el reporte de permisos de cada apt ocupado recursivamente 
        for(int j = 0; j < 4; j++){
            if(folderBlock_aux.b_content[j].b_inodo == -1) break;
            if(folderBlock_aux.b_content[j].b_name == parent) continue;
            if(folderBlock_aux.b_content[j].b_name == previousParent) continue;

            disco.seekg(folderBlock_aux.b_content[j].b_inodo);
            disco.read((char*)&actualInode, sizeof(Inode));
            cadena += recursivePerm(disco, actualInode, folderBlock_aux.b_content[j].b_inodo, folderBlock_aux.b_content[j].b_name, usersContent);
        }

        //* Recuperando el inodo actual 
        disco.seekg(posActualInode);
        disco.read((char*)&actualInode, sizeof(Inode));
    }

    return cadena;
}

bool Reporte_ls(string partitionId, string reportPath, string filePath, PartitionNode *&firstNode){
    //* Verificando que el id represente una partición montada 
    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    if(filePath == ""){
        cout << "\033[0;91;49m[Error]: Proporcione una ruta para poder generar el reporte ls (parámetro -ruta) \033[0m" << endl;
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
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR \033[0m" << endl;
        disco.close();
        return false;
    }
    MBR mbrdisco;
    disco.read((char *)&mbrdisco, sizeof(MBR));

    string partitionName = getPartitionName(firstNode, partitionId);
    int numberPrimaryPartition = 0, partitionStart = 0;
    char partStatus = '0';
    int numberExtendedPartition = 0;

    // while para poder finalizar las validaciones cuando se encuentre la partición
    // solo se ejecuta una vez
    while(true){
        if(mbrdisco.mbr_partition_1.part_status != 'E'){
            if(mbrdisco.mbr_partition_1.part_name == partitionName){
                if(mbrdisco.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numberPrimaryPartition = 1;
                partitionStart = mbrdisco.mbr_partition_1.part_start;
                partStatus = mbrdisco.mbr_partition_1.part_status;
                break;
            }
            if(mbrdisco.mbr_partition_1.part_type == 'E') numberExtendedPartition = 1;
        }
        if(mbrdisco.mbr_partition_2.part_status != 'E'){
            if(mbrdisco.mbr_partition_2.part_name == partitionName){
                if(mbrdisco.mbr_partition_2.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numberPrimaryPartition = 2;
                partitionStart = mbrdisco.mbr_partition_2.part_start;
                partStatus = mbrdisco.mbr_partition_2.part_status;
                break;
            }
            if(mbrdisco.mbr_partition_2.part_type == 'E') numberExtendedPartition = 2;
        }
        if(mbrdisco.mbr_partition_3.part_status != 'E'){
            if(mbrdisco.mbr_partition_3.part_name == partitionName){
                if(mbrdisco.mbr_partition_3.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numberPrimaryPartition = 3;
                partitionStart = mbrdisco.mbr_partition_3.part_start;
                partStatus = mbrdisco.mbr_partition_3.part_status;
                break;
            }
            if(mbrdisco.mbr_partition_3.part_type == 'E') numberExtendedPartition = 3;
        }
        if(mbrdisco.mbr_partition_4.part_status != 'E'){
            if(mbrdisco.mbr_partition_4.part_name == partitionName){
                if(mbrdisco.mbr_partition_4.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
                    disco.close();
                    return false;
                }
                numberPrimaryPartition = 4;
                partitionStart = mbrdisco.mbr_partition_4.part_start;
                partStatus = mbrdisco.mbr_partition_4.part_status;
                break;
            }
            if(mbrdisco.mbr_partition_4.part_type == 'E') numberExtendedPartition = 4;
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
    //* LS REPORT PRIMARY PARTITION 
    if(numberPrimaryPartition != 0){
        disco.seekg(partitionStart);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO REPORT LS LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    //* Accediendo al archivo solicitado 
    Inode inodeAux;
    FolderBlock folderB;
    bool wasFound = false;
    int posActualInode = sbAux.s_inode_start;
    char* nameActualInode = "/";
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&inodeAux, sizeof(Inode)); // Root inode

    char *parte;
    string fileCopy = filePath, fileName = "";
    parte = strtok((char*)fileCopy.c_str(), "/");

    while(parte != NULL){
        if(inodeAux.i_type == '1'){
            cout << "\033[0;91;49m[Error]: el nombre " << parte << " corresponde a un archivo, no a un subdirectorio \033[0m" << endl;
            disco.close();
            return false;
        }

        fileName = parte;

        //* Buscando en bloques directos 
        for(int i = 0; i < 12; i++){
            if(inodeAux.i_block[i] == -1){
                cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << fileName
                << " de la ruta " << filePath << " en el árbol de directorios\033[0m" << endl;
                disco.close();
                return false;
            }

            disco.seekg(inodeAux.i_block[i]);
            disco.read((char*)&folderB, sizeof(FolderBlock));

            //* Buscando en sus 4 espacios disponibles 
            for(int j = 0; j < 4; j++){
                if(folderB.b_content[j].b_inodo == -1) break;
                if(folderB.b_content[j].b_name != fileName) continue;

                //* Inodo de parte encontrado 
                posActualInode = folderB.b_content[j].b_inodo;
                nameActualInode = folderB.b_content[j].b_name;
                disco.seekg(folderB.b_content[j].b_inodo);
                disco.read((char*)&inodeAux, sizeof(Inode));
                wasFound = true;
                break;
            }
            if(wasFound) break;
        }

        //* No se encontró en los bloques directos 
        if(!wasFound){
            cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << fileName
            << " de la ruta " << filePath << " en el árbol de directorios\033[0m" << endl;
            disco.close();
            return false;
        }
        wasFound = false;
        parte = strtok(NULL, "/");
    }

    //* Inodo del archivo "fileName" se encuentra en inodeAux 

    string cadena = "digraph G {\n\tnode [shape=box fontname=\"Helvetica,Arial,sans-serif\" peripheries=0]\n\tlabel = \"LS Report\"";
    cadena += "\n\ta0 [label=<\n\t\t<TABLE border=\"3\" cellspacing=\"5\" cellpadding=\"10\">";
    cadena += "\n\t\t<TR>\n\t\t<TD border=\"2\"  bgcolor=\"#4a6687\"><B>Permisos</B></TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"#4a6687\"><B>Owner</B></TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"#4a6687\"><B>Grupo</B></TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"#4a6687\"><B>Size (bytes)</B></TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"#4a6687\"><B>Fecha</B></TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"#4a6687\"><B>Tipo</B></TD>";
    cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"#4a6687\"><B>Nombre</B></TD>\n\t\t</TR>";

    string usersContent = getUsersContent(disco, sbAux);
    cadena += recursivePerm(disco, inodeAux, posActualInode, nameActualInode, usersContent);
    cadena += "\n\t\t</TABLE>>];\n}";

    disco.close();
    return CrearReporte(cadena, reportPath);

}