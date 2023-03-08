#include <iostream>
#include <string.h>
#include <fstream>

#include "Cat.h"
#include "../Estructura.h"
#include "../Mount/Mount.h"
#include "../../aux_funciones.h"

using namespace std;

bool MostrarContenido_Archivo(bool openSesion, string partitionId, vector<string>& filepathList, PartitionNode *&firstNode){
    if(!openSesion){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }

    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    cout << "--> Obteniendo datos del disco...\n";
    ifstream disk;
    string diskPath = getDiskPath(firstNode, partitionId);
    disk.open(diskPath, ios::in|ios::binary);
    if(disk.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << diskPath << "\033[0m" << endl;
        disk.close();
        return false;
    }

    char aux;
    disk.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR \033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas \033[0m" << endl;
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
        cout << "\033[0;91;49m[Error]: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m " << endl;
        disk.close();
        return false;
    }

    Superblock sbAux;
    /* CAT PRIMARY PARTITION */
    if(numberPrimaryPartition != 0){
        disk.seekg(partitionStart);
        disk.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO CAT LOGICAL PARTITIONS
    else {
        disk.close();
        return false;
    }

    /* Control variables */
    Inode inodeAux;
    FolderBlock folderB;
    bool wasFound = false;
    char *portion;
    string fileCopy = "", fileName = "";
    FileBlock fileB;
    string fileText = "";

    /* Accediendo a cada archivo solicitado */
    for(int i = 0; i < filepathList.size(); i++){
        disk.seekg(sbAux.s_inode_start);
        disk.read((char*)&inodeAux, sizeof(Inode)); // Root inode

        wasFound = false;
        fileCopy = filepathList[i];
        fileName = "";
        portion = strtok((char*)fileCopy.c_str(), "/");

        while(portion != NULL){
            if(inodeAux.i_type == '1'){
                cout << "\033[0;91;49m[Error]: el nombre " << portion << " corresponde a un archivo, no a un subdirectorio \033[0m" << endl;
                disk.close();
                return false;
            }

            fileName = portion;

            /* Buscando en bloques directos */
            for(int i = 0; i < 12; i++){
                if(inodeAux.i_block[i] == -1){
                    cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << fileName
                    << " de la ruta " << filepathList[i] << " en el árbol de directorios\033[0m" << endl;
                    disk.close();
                    return false;
                }

                disk.seekg(inodeAux.i_block[i]);
                disk.read((char*)&folderB, sizeof(FolderBlock));

                /* Buscando en sus 4 espacios disponibles */
                for(int j = 0; j < 4; j++){
                    if(folderB.b_content[j].b_inodo == -1) break;
                    if(folderB.b_content[j].b_name != fileName) continue;

                    /* Inodo de portion encontrado */
                    disk.seekg(folderB.b_content[j].b_inodo);
                    disk.read((char*)&inodeAux, sizeof(Inode));
                    wasFound = true;
                    break;
                }
                if(wasFound) break;
            }

            /* No se encontró en los bloques directos */
            if(!wasFound){
                cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << fileName
                << " de la ruta " << filepathList[i] << " en el árbol de directorios\033[0m " << endl;
                disk.close();
                return false;
            }
            wasFound = false;
            portion = strtok(NULL, "/");
        }

        if(inodeAux.i_type == '0'){
            cout << "\033[0;91;49m[Error]: el nombre " << fileName << " indicado corresponde a un directorio, no a un archivo \033[0m" << endl;
            disk.close();
            return false;
        }

        /* Inodo del archivo "fileName" se encuentra en inodeAux */
        /* Recorriendo bloques directos para leer su contenido */
        fileText = "";
        for(int i = 0; i < 12; i++){
            if(inodeAux.i_block[i] == -1) break;
            disk.seekg(inodeAux.i_block[i]);
            disk.read((char*)&fileB, sizeof(FileBlock));
            for(int j = 0; j < 64; j++){
                if(fileB.b_content[j] == 0) break;
                fileText += fileB.b_content[j];
            }
        }

        if(fileText.length() == (12*64)){
            // TODO seguir leyendo bloques indirectos
        }
        cout << "--> Mostrando archivo: " << filepathList[i] << endl;
        cout << "\033[38;5;21m[Contenido]: "  << fileText << ".\033[0m" << endl;
    }
    return true;
}




Cat _Cat(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* fileN
    Cat nuevoComandoCat;
    string path = "";

    for(int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter f
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'f') estado = 2;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter i
            case 2: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 3;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter l
            case 3: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'l') estado = 4;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 4: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 5;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 5: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 6;
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {path = "", estado = 7;}
                else if (parametros[i] == 9 || parametros[i] == 32);
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {path = ""; path += parametros[i]; estado = 8;}
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(path.length() > 0) nuevoComandoCat.filePathList.push_back(path);
                    else cout << "\033[0;91;49m[Error]: " << parametroActual << " no contiene una ruta. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    nuevoComandoCat.filePathList.push_back(path);
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //* Reconocimiento de error sintactico
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para cat. \033[0m" << endl;
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

    if(nuevoComandoCat.filePathList.size() > 0){
        nuevoComandoCat.acceso = true;
        return nuevoComandoCat;
    }

    cout << "\033[;91;49m[Error]: Faltan el parametro obligatorio \">FileN\" para poder mostrar el contenido de un archivo. \033[0m" << endl;
    nuevoComandoCat.acceso = false;
    return nuevoComandoCat;
}