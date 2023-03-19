#include <iostream>
#include <fstream>

#include "../../Mount/Mount.h"
#include "../../Estructura.h"
#include "../../../aux_funciones.h"

using namespace std;


string Recursividad(ifstream &disco, Inode &actualInode, int posActualInode, char* inodeName, int &nodeCounter, int parentNode, int parentPointer){
    string cadena = "", parent = ".", previousParent = "..";
    int num_inode = nodeCounter, numeroBloque;

    //* Creando inodo 
    cadena += "\"node";
    cadena += to_string(nodeCounter);
    cadena += "\" [\n\t\tlabel = \"<f0>Inodo ";
    cadena += inodeName;
    cadena += "|i_uid: ";
    cadena += to_string(actualInode.i_uid);
    cadena += "|i_gid: ";
    cadena += to_string(actualInode.i_gid);
    cadena += "|i_s: ";
    cadena += to_string(actualInode.i_s);
    cadena += "|i_atime: ";
    if (actualInode.i_atime != NULL) cadena += ctime(&actualInode.i_atime);
    else cadena += "-";
    cadena += "|i_ctime: ";
    cadena += ctime(&actualInode.i_ctime);
    cadena += "|i_mtime: ";
    cadena += ctime(&actualInode.i_mtime);
    cadena += "|i_type: ";
    if(actualInode.i_type == '0') cadena += "Carpeta";
    else cadena += "Archivo\n\t\t";

    for(int i = 0; i < 12; i++){
        if(actualInode.i_block[i] == -1) break;
        cadena += "|<f";
        cadena += to_string(i+1);
        cadena += ">apt";
        cadena += to_string(i);
        cadena += ": ";
        cadena += to_string(actualInode.i_block[i]);
    }
    if(actualInode.i_type == '0') cadena += "\"\n\t\tshape = \"record\"\n\t\tfillcolor = \"white:lightblue\"\n\t];\n\t";
    else cadena += "\"\n\t\tshape = \"record\"\n\t\tfillcolor = \"white:greenyellow\"\n\t];\n\t";
    nodeCounter++;

    FolderBlock folderBlock_aux;
    FileBlock fileBlock_aux;
    //* Recorriendo bloques directos del inodo actual 
    for(int i = 0; i < 12; i++){
        if(actualInode.i_block[i] == -1) break;

        //* Bloque carpeta 
        if(actualInode.i_type == '0'){
            disco.seekg(actualInode.i_block[i]);
            disco.read((char*)&folderBlock_aux, sizeof(FolderBlock));
            numeroBloque = nodeCounter;
            cadena += "\"node";
            cadena += to_string(nodeCounter);
            cadena += "\" [\n\t\tlabel = \"<f0>Bloque Carpeta\n\t\t";
            //* Recorriendo los 4 espacios del bloque carpeta 
            for(int j = 0; j < 4; j++){
                cadena += "|<f";
                cadena += to_string(j+1);
                cadena += ">apt";
                cadena += to_string(j);
                cadena += ": ";
                cadena += to_string(folderBlock_aux.b_content[j].b_inodo);
                if(folderBlock_aux.b_content[j].b_inodo != -1){
                    cadena += "|Nombre: ";
                    cadena += folderBlock_aux.b_content[j].b_name;
                }
            }
            cadena += "\"\n\t\tshape = \"record\"\n\t\tfillcolor = \"lightblue:blue\"\n\t];\n\t";
            cadena += "\"node";
            cadena += to_string(num_inode);
            cadena += "\":f";
            cadena += to_string(i + 1);
            cadena += " -> \"node";
            cadena += to_string(nodeCounter);
            cadena += "\":f0;\n\t";

            nodeCounter++;

            //* Generando el árbol de cada apt ocupado recursivamente 
            for(int j = 0; j < 4; j++){
                if(folderBlock_aux.b_content[j].b_inodo == -1) break;
                if(folderBlock_aux.b_content[j].b_name == parent) continue;
                if(folderBlock_aux.b_content[j].b_name == previousParent) continue;

                disco.seekg(folderBlock_aux.b_content[j].b_inodo);
                disco.read((char*)&actualInode, sizeof(Inode));
                cadena += Recursividad(disco, actualInode, folderBlock_aux.b_content[j].b_inodo, folderBlock_aux.b_content[j].b_name, nodeCounter, numeroBloque, j+1);
            }

            //* Recuperando el inodo actual 
            disco.seekg(posActualInode);
            disco.read((char*)&actualInode, sizeof(Inode));
        }
        //* Bloque archivo 
        else{
            disco.seekg(actualInode.i_block[i]);
            disco.read((char*)&fileBlock_aux, sizeof(FileBlock));
            cadena += "\"node";
            cadena += to_string(nodeCounter);
            cadena += "\" [\n\t\tlabel = \"<f0>Bloque Archivo\n\t\t|";
            for(int j = 0; j < 64; j++){
                if(fileBlock_aux.b_content[j] == 0) break;
                if(fileBlock_aux.b_content[j] == '\n') cadena += "\\n";
                else cadena += fileBlock_aux.b_content[j];
            }
            cadena += "\"\n\t\tshape = \"record\"\n\t\tfillcolor = \"greenyellow:green4\"\n\t];\n\t";

            cadena += "\"node";
            cadena += to_string(num_inode);
            cadena += "\":f";
            cadena += to_string(i + 1);
            cadena += " -> \"node";
            cadena += to_string(nodeCounter);
            cadena += "\":f0;\n\t";

            nodeCounter++;
        }
    }

    if(parentNode != -1){
        //* Uniendo el inodo con su bloque padre 
        cadena += "\"node";
        cadena += to_string(parentNode);
        cadena += "\":f";
        cadena += to_string(parentPointer);
        cadena += " -> \"node";
        cadena += to_string(num_inode);
        cadena += "\":f0;\n\t";
    }

    return cadena;
}

bool Reporte_tree(string partitionId, string reportPath, PartitionNode *&firstNode){
    //* Verificando que el id represente una partición montada 
    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m> La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m\n";
        return false;
    }

    cout << "> Obteniendo datos del disco...\n";
    ifstream disco;
    string path = getDiskPath(firstNode, partitionId);
    disco.open(path, ios::in|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m> Error: No se ha podido abrir el disco asociado a la partición " << partitionId <<
        ", con ruta " << path << "\033[0m\n";
        disco.close();
        return false;
    }

    char aux;
    disco.read((char *)&aux, sizeof(char));
    if(aux != idMBR){
        cout << "> Error interno: no vino char idMBR antes del MBR\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
                    cout << "> Error interno: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas\n";
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
        cout << "\033[0;91;49m> Error: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m\n";
        disco.close();
        return false;
    }

    Superblock sbAux;
    //* TREE REPORT PRIMARY PARTITION 
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    // TODO REPORT TREE LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    //* INODO RAIZ 
    Inode iAux;
    disco.seekg(sbAux.s_inode_start);
    disco.read((char*)&iAux, sizeof(Inode));
    string cadena = "";

    cadena += "digraph g {\n\tfontname=\"Helvetica,Arial,sans-serif\"\n\tnode [fontname=\"Helvetica,Arial,sans-serif\" style=\"filled\"]"
    "\n\tedge [fontname=\"Helvetica,Arial,sans-serif\"]\n\tgraph [ rankdir = \"LR\" ];\n\t";

    //* Llenando el árbol recursivamente 
    int nodeCounter = 0;
    cadena += Recursividad(disco, iAux, sbAux.s_inode_start, "/", nodeCounter, -1, -1);
    cadena += "}";

    return CrearReporte(cadena, reportPath);
}