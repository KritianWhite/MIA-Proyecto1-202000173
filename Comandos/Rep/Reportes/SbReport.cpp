#include <iostream>
#include <fstream>

#include "../../Mount/Mount.h"
#include "../../Estructura.h"
#include "../../../aux_funciones.h"

using namespace std;

bool Reporte_super_bloque(string partitionId, string reportPath, PartitionNode *&firstNode){
    /* Verificando que el id represente una partición montada */
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
        cout << "\033[0;91;49m[Error]: no vino char idMBR antes del MBR\033[0m" << endl;
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

    /* Validando que la partición contenga formato */
    if(partStatus != 'F'){
        cout << "\033[0;91;49m[Error]: La partición " << partitionId << " no ha sido formateada, utilice mkfs primero \033[0m" << endl;
        disco.close();
        return false;
    }

    Superblock sbAux;
    /* SUBERBLOCK REPORT PRIMARY PARTITION */
    if(numeroParticionPrimaria != 0){
        disco.seekg(particionInicial);
        disco.read((char*)&sbAux, sizeof(Superblock));
        disco.close();
    }
    // TODO REPORT SUPERBLOCK LOGICAL PARTITIONS
    else {
        disco.close();
        return false;
    }

    try{
        string cadena = "digraph G {\n\tnode [fontname=\"Helvetica,Arial,sans-serif\" peripheries=0]";
        cadena += "\n\ta0 [label=<\n\t\t<TABLE border=\"3\" cellspacing=\"5\" cellpadding=\"10\">";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"#4a6687\"><B>SUPERBLOQUE</B></TD>\n\t\t\t</TR>";

        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_filesystem_type</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">ext";
        cadena += to_string(sbAux.s_filesystem_type);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_inodes_count</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_inodes_count);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_blocks_count</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_blocks_count);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_free_blocks_count</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_free_blocks_count);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_free_inodes_count</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_free_inodes_count);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_mtime</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += ctime(&sbAux.s_mtime);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_umtime</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        if(sbAux.s_umtime != NULL) cadena += ctime(&sbAux.s_umtime);
        else cadena += "-";
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_mnt_count</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_mnt_count);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_magic</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_magic);
        cadena += " (0xEF53)</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_inode_s</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_inode_s);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_block_s</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_block_s);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_first_ino</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_first_ino);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_first_blo</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_first_blo);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_bm_inode_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_bm_inode_start);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_bm_block_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_bm_block_start);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_inode_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_inode_start);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\" bgcolor=\"white\">s_block_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\"  bgcolor=\"white\">";
        cadena += to_string(sbAux.s_block_start);
        cadena += "</TD>\n\t\t\t</TR>";

        cadena += "\n\t\t</TABLE>>];\n}";

        return CrearReporte(cadena, reportPath);

    }catch(...){
        return false;
    }
}