#include <iostream>
#include <fstream>

//#include "../Rep.cpp"
#include "../../Mount/Mount.h"
#include "../../Estructura.h"
#include "../../../aux_funciones.h"

using namespace std;

bool Reporte_MBR(string partitionId, string reportPath, PartitionNode *&firstNode){
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
        cout << "\033[0;91;49m[Error]: No vino char idMBR antes del MBR\033[0m" << endl;
        disco.close();
        return false;
    }
    MBR mbrDisk;
    disco.read((char *)&mbrDisk, sizeof(MBR));

    string cadena = "digraph G {\n\trankdir=\"TD\"\n\tnode [shape=box fontname=\"Helvetica,Arial,sans-serif\" peripheries=0]";
    cadena += "\n\ta0 [label=<\n\t\t<TABLE border=\"3\" cellspacing=\"5\" cellpadding=\"10\" style=\"filled\">";
    cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"#bd538a\"><B>MBR</B></TD>\n\t\t\t</TR>";
    cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">mbr_tamano</TD>";
    cadena += "\n\t\t\t<TD border=\"2\" >";
    cadena += to_string(mbrDisk.mbr_tamano);
    cadena += " bytes</TD>\n\t\t\t</TR>";
    cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">mbr_fecha_creacion</TD>";
    cadena += "\n\t\t\t<TD border=\"2\" >";
    cadena += ctime(&mbrDisk.mbr_fecha_creacion);
    cadena += "</TD>\n\t\t\t</TR>";
    cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">mbr_dsk_signature</TD>";
    cadena += "\n\t\t\t<TD border=\"2\" >";
    cadena += to_string(mbrDisk.mbr_dsk_signature);
    cadena += "</TD>\n\t\t\t</TR>";
    cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">dsk_fit</TD>";
    cadena += "\n\t\t\t<TD border=\"2\" >";
    cadena += mbrDisk.dsk_fit;
    cadena += "</TD>\n\t\t\t</TR>";


    int numberExtendedPartition = 0;

    cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"#8653bd\">Particion 1</TD>\n\t\t\t</TR>";
    if(mbrDisk.mbr_partition_1.part_status != 'E'){
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_status</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += '1';
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_type</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_1.part_type;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_fit</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_1.part_fit;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_1.part_start);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_s</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_1.part_s);
        cadena += " bytes</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_name</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_1.part_name;
        cadena += "</TD>\n\t\t\t</TR>";
        if(mbrDisk.mbr_partition_1.part_type == 'E') numberExtendedPartition = 1;
    }

    cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"#8653bd\">Particion 2</TD>\n\t\t\t</TR>";
    if(mbrDisk.mbr_partition_2.part_status != 'E'){
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_status</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += '1';
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_type</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_2.part_type;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_fit</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_2.part_fit;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_2.part_start);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_s</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_2.part_s);
        cadena += " bytes</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_name</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_2.part_name;
        cadena += "</TD>\n\t\t\t</TR>";
        if(mbrDisk.mbr_partition_2.part_type == 'E') numberExtendedPartition = 2;
    }

    cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"#8653bd\">Particion 3</TD>\n\t\t\t</TR>";
    if(mbrDisk.mbr_partition_3.part_status != 'E'){
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_status</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += '1';
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_type</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_3.part_type;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_fit</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_3.part_fit;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_3.part_start);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_s</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_3.part_s);
        cadena += " bytes</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_name</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_3.part_name;
        cadena += "</TD>\n\t\t\t</TR>";
        if(mbrDisk.mbr_partition_3.part_type == 'E') numberExtendedPartition = 3;
    }

    cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"#8653bd\">Particion 4</TD>\n\t\t\t</TR>";
    if(mbrDisk.mbr_partition_4.part_status != 'E'){
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_status</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += '1';
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_type</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_4.part_type;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_fit</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_4.part_fit;
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_start</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_4.part_start);
        cadena += "</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_s</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += to_string(mbrDisk.mbr_partition_4.part_s);
        cadena += " bytes</TD>\n\t\t\t</TR>";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_name</TD>";
        cadena += "\n\t\t\t<TD border=\"2\" >";
        cadena += mbrDisk.mbr_partition_4.part_name;
        cadena += "</TD>\n\t\t\t</TR>";
        if(mbrDisk.mbr_partition_4.part_type == 'E') numberExtendedPartition = 4;
    }

    cadena += "\n\t\t</TABLE>>];";

    if(numberExtendedPartition == 0){
        cadena += "\n}";
        return CrearReporte(cadena, reportPath);
    }

    EBR ebrAux;
    switch(numberExtendedPartition){
        case 1: disco.seekg(mbrDisk.mbr_partition_1.part_start); break;
        case 2: disco.seekg(mbrDisk.mbr_partition_2.part_start); break;
        case 3: disco.seekg(mbrDisk.mbr_partition_3.part_start); break;
        case 4: disco.seekg(mbrDisk.mbr_partition_4.part_start); break;
    }

    disco.read((char*)&ebrAux, sizeof(EBR));

    int actualNode = 1;
    while(true){
        cadena += "\n\ta";
        cadena += to_string(actualNode);
        cadena += " [label=<\n\t\t<TABLE border=\"3\" cellspacing=\"5\" cellpadding=\"10\" style=\"filled\">";
        cadena += "\n\t\t\t<TR>\n\t\t\t<TD colspan=\"2\" border=\"2\"  bgcolor=\"#bd538a\"><B>EBR</B></TD>\n\t\t\t</TR>";
        if(ebrAux.part_next != -1 || actualNode > 1){
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_status</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >";
            cadena += '1';
            cadena += "</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_fit</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >";
            cadena += ebrAux.part_fit;
            cadena += "</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_start</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >";
            cadena += to_string(ebrAux.part_start);
            cadena += "</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_s</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >";
            cadena += to_string(ebrAux.part_s);
            cadena += " bytes</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_next</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >";
            cadena += to_string(ebrAux.part_next);
            cadena += "</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_name</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >";
            cadena += ebrAux.part_name;
            cadena += "</TD>\n\t\t\t</TR>";
        }else{
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_status</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >";
            cadena += '0';
            cadena += "</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_fit</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >-</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_start</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >-</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_s</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >-</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_next</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >-1</TD>\n\t\t\t</TR>";
            cadena += "\n\t\t\t<TR>\n\t\t\t<TD border=\"2\">part_name</TD>";
            cadena += "\n\t\t\t<TD border=\"2\" >-</TD>\n\t\t\t</TR>";
        }

        cadena += "\n\t\t</TABLE>>];";

        if(actualNode == 1) cadena += "\n\ta0 -> a1[dir=none color=\"#00000000\"];";
        else{
            cadena += "\n\ta";
            cadena += to_string(actualNode - 1);
            cadena += " -> a";
            cadena += to_string(actualNode);
            cadena += ";";
        }
        if(ebrAux.part_next == -1) break;
        disco.seekg(ebrAux.part_next);
        disco.read((char*)&ebrAux, sizeof(EBR));
        actualNode++;
    }

    cadena += "\n}";
    disco.close();
    return CrearReporte(cadena, reportPath);
}