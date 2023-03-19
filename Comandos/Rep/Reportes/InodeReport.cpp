#include <iostream>
#include <fstream>

#include "../../Mount/Mount.h"
#include "../../Estructura.h"
#include "../../../aux_funciones.h"

using namespace std;

bool Reporte_inode(string partitionId, string reportPath, PartitionNode *&firstNode){
    //* Verificando que el id represente una partición montada 
    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
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

    //* while para poder finalizar las validaciones cuando se encuentre la partición
    //* solo se ejecuta una vez
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
    //* INODE REPORT PRIMARY PARTITION 
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
    }
    //* TODO REPORT INODE LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    string cadena = "digraph G {\n\trankdir=\"LR\"\n\tnode [shape=box fontname=\"Helvetica,Arial,sans-serif\" peripheries=0]\n\tlabel = \"Inodos\"";

    Inode iAux;
    disco.seekg(sbAux.s_inode_start);
    int totalInodes = sbAux.s_inodes_count - sbAux.s_free_inodes_count;
    //* COMO NO SE PROGRAMÓ LA ELIMINACIÓN DE INODOS, SE RECORREN SECUENCIALMENTE 
    for(int i = 0; i < totalInodes; i++){
        disco.read((char*)&iAux, sizeof(Inode));
        cadena += "\n\ta";
        cadena += to_string(i);
        cadena += " [label=<\n\t\t<TABLE border=\"3\" cellspacing=\"5\" cellpadding=\"10\" style=\"rounded\" bgcolor=\"lightblue\">";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"aquamarine3\"><B>Inodo ";
        cadena += to_string(i+1);
        cadena += "</B></TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_uid</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(iAux.i_uid);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_gid</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(iAux.i_gid);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_s</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(iAux.i_s);
        cadena += " bytes</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_atime</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">-</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_ctime</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        if(iAux.i_ctime != NULL) cadena += ctime(&iAux.i_ctime);
        else cadena += "-";
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_mtime</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        if(iAux.i_ctime != NULL) cadena += ctime(&iAux.i_mtime);
        else cadena += "-";
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_type</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += iAux.i_type;
        if(iAux.i_type == '0') cadena += " (carpeta)";
        else cadena += " (archivo)";
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">i_perm</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(iAux.i_perm);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t</TABLE>>];";
        if(i > 0){
            cadena += "\n\ta";
            cadena += to_string(i-1);
            cadena += " -> a";
            cadena += to_string(i);
            cadena += ";";
        }
    }

    cadena += "\n}";
    disco.close();
    return CrearReporte(cadena, reportPath);
}