#include <iostream>
#include <string.h>
#include <fstream>

#include "../../Mount/Mount.h"
#include "../../Estructura.h"
#include "../../../aux_funciones.h"

using namespace std;

bool Reporte_File(string partitionId, string reportPath, string filePath, PartitionNode *&firstNode){
    //* Verificando que el id represente una partición montada 
    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    if(filePath == ""){
        cout << "\033[0;91;49m[Error]: Proporcione una ruta para poder generar el reporte file (parámetro -ruta) \033[0m" << endl;
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
    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));

    string nombreParticion = getPartitionName(firstNode, partitionId);
    int numeroParticionPrimaria = 0, particionInicial = 0;
    char partStatus = '0';
    int numeroParticionExtendida = 0;

    while(true){
        if(mb.mbr_partition_1.part_status != 'E'){
            if(mb.mbr_partition_1.part_name == nombreParticion){
                if(mb.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
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
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\033[0m" << endl;
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
    //* FILE REPORT PRIMARY PARTITION 
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO REPORT FILE LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    //* Accediendo al archivo solicitado 
    Inode iAux;
    FolderBlock folderB;
    bool wasFound = false;
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode)); // Root inode

    char *parte;
    string fileCopy = filePath, nombreAchivo = "";
    parte = strtok((char*)fileCopy.c_str(), "/");

    while(parte != NULL){
        if(iAux.i_type == '1'){
            cout << "\033[0;91;49m[Error]: el nombre " << parte << " corresponde a un archivo, no a un subdirectorio \033[0m" << endl;
            disco.close();
            return false;
        }

        nombreAchivo = parte;

        //* Buscando en bloques directos 
        for(int i = 0; i < 12; i++){
            if(iAux.i_block[i] == -1){
                cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << nombreAchivo
                << " de la ruta " << filePath << " en el árbol de directorios\033[0m" << endl;
                disco.close();
                return false;
            }

            disco.seekg(iAux.i_block[i]);
            disco.read((char*)&folderB, sizeof(FolderBlock));

            //* Buscando en sus 4 espacios disponibles 
            for(int j = 0; j < 4; j++){
                if(folderB.b_content[j].b_inodo == -1) break;
                if(folderB.b_content[j].b_name != nombreAchivo) continue;

                //* Inodo de parte encontrado 
                disco.seekg(folderB.b_content[j].b_inodo);
                disco.read((char*)&iAux, sizeof(Inode));
                wasFound = true;
                break;
            }
            if(wasFound) break;
        }

        //* No se encontró en los bloques directos 
        if(!wasFound){
            cout << "\033[0;91;49m[Error]: no se ha encontrado el directorio " << nombreAchivo
            << " de la ruta " << filePath << " en el árbol de directorios\033[0m" << endl;
            disco.close(); 
            return false;
        }
        wasFound = false;
        parte = strtok(NULL, "/");
    }

    if(iAux.i_type == '0'){
        cout << "\033[0;91;49m[Error]: el nombre " << nombreAchivo << " indicado corresponde a un directorio, no a un archivo \033[0m" << endl;
        disco.close();
        return false;
    }

    //* Inodo del archivo "nombreAchivo" se encuentra en iAux 
    FileBlock fileB;
    string archivoTexto = "";
    //* Recorriendo bloques directos para leer su contenido 
    for(int i = 0; i < 12; i++){
        if(iAux.i_block[i] == -1) break;
        disco.seekg(iAux.i_block[i]);
        disco.read((char*)&fileB, sizeof(FileBlock));
        for(int j = 0; j < 64; j++){
            if(fileB.b_content[j] == 0) break;
            archivoTexto += fileB.b_content[j];
        }
    }

    if(archivoTexto.length() == (12*64)){
        // TODO seguir leyendo bloques indirectos
    }

    disco.close();
    return CrearReporte_Texto(archivoTexto, reportPath);
}