#include <iostream>
#include <fstream>

#include "../../Mount/Mount.h"
#include "../../Estructura.h"
#include "../../../aux_funciones.h"

using namespace std;

bool Reporte_Disk(string partitionId, string reportPath, PartitionNode *&firstNode){
    /* Verificando que el id represente una partición montada */
    if (!isIdInList(firstNode, partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << partitionId << " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    cout << "--> Obteniendo datos del disco...\n";
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
    EBR ebrAux;
    disco.read((char *)&mb, sizeof(MBR));
    int espacioVacio, numeroParticionExtendida = -1, bloquesParticionExtendida = 0, espacioOcupadoExtendida = 0;
    int porcentaje;

    string cadena = "digraph G {\n\tnode[shape=box fontname=\"Helvetica,Arial,sans-serif\" peripheries=0]\n\tlabel=\"DISCO\"";
    cadena += "\n\ta1 [label=<\n\t\t<TABLE border=\"5\" cellspacing=\"6\" cellpadding=\"15\" style=\"filled\">";
    cadena += "\n\t\t<TR>\n\t\t<TD rowspan=\"2\" border=\"2\"><B>MBR</B></TD>"; // close row

    if(mb.mbr_partition_1.part_status != 'E'){
        espacioVacio = mb.mbr_partition_1.part_start - sizeof(char) - sizeof(MBR);
        if(espacioVacio > 0){
            // CASE 0 -> MBR [] P1
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }

        porcentaje = ((float)mb.mbr_partition_1.part_s/mb.mbr_tamano)*100;
        if(mb.mbr_partition_1.part_type == 'P'){
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>";
            cadena += mb.mbr_partition_1.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }else{
            numeroParticionExtendida = 1;
            disco.seekg(mb.mbr_partition_1.part_start);
            disco.read((char*)&ebrAux, sizeof(EBR));
            if(ebrAux.part_status == 'E'){
                bloquesParticionExtendida = 2; // first ebr & free space
                espacioOcupadoExtendida = mb.mbr_partition_1.part_s;
            }else{
                // TODO Eliminación de lógicas para ver validación de espacio libre
                espacioOcupadoExtendida = ebrAux.part_s;
                bloquesParticionExtendida = 2;
                while(ebrAux.part_next != -1){
                    disco.seekg(ebrAux.part_next);
                    disco.read((char*)&ebrAux, sizeof(EBR));
                    espacioOcupadoExtendida += ebrAux.part_s;
                    bloquesParticionExtendida += 2;
                }
                if(espacioOcupadoExtendida < mb.mbr_partition_1.part_s) bloquesParticionExtendida += 1; // free space
            }
            cadena += "\n\t\t<TD colspan=\"";
            cadena += to_string(bloquesParticionExtendida);
            cadena += "\" border=\"2\"><B>";
            cadena += mb.mbr_partition_1.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }else{
        // CASE 0 (all disk available)
        cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>100%</B></TD>";
    }


    if(mb.mbr_partition_2.part_status != 'E'){
        espacioVacio = mb.mbr_partition_2.part_start - (mb.mbr_partition_1.part_start + mb.mbr_partition_1.part_s);
        if(espacioVacio > 0){
            // CASE 1 (between P1 & P2)
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }

        porcentaje = ((float)mb.mbr_partition_2.part_s/mb.mbr_tamano)*100;
        if(mb.mbr_partition_2.part_type == 'P'){
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>";
            cadena += mb.mbr_partition_2.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }else{
            numeroParticionExtendida = 2;
            disco.seekg(mb.mbr_partition_2.part_start);
            disco.read((char*)&ebrAux, sizeof(EBR));
            if(ebrAux.part_status == 'E'){
                bloquesParticionExtendida = 2; // first ebr & free space
                espacioOcupadoExtendida = mb.mbr_partition_2.part_s;
            }else{
                // TODO Eliminación de lógicas para ver validación de espacio libre
                espacioOcupadoExtendida = ebrAux.part_s;
                bloquesParticionExtendida = 2;
                while(ebrAux.part_next != -1){
                    disco.seekg(ebrAux.part_next);
                    disco.read((char*)&ebrAux, sizeof(EBR));
                    espacioOcupadoExtendida += ebrAux.part_s;
                    bloquesParticionExtendida += 2;
                }
                if(espacioOcupadoExtendida < mb.mbr_partition_2.part_s) bloquesParticionExtendida += 1; // free space
            }
            cadena += "\n\t\t<TD colspan=\"";
            cadena += to_string(bloquesParticionExtendida);
            cadena += "\" border=\"2\"><B>";
            cadena += mb.mbr_partition_2.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }else if(mb.mbr_partition_1.part_status != 'E'){
        // CASE 1 (all space after P1)
        espacioVacio = mb.mbr_tamano - (mb.mbr_partition_1.part_start + mb.mbr_partition_1.part_s);
        if(espacioVacio > 0){
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }


    if(mb.mbr_partition_3.part_status != 'E'){
        espacioVacio = mb.mbr_partition_3.part_start - (mb.mbr_partition_2.part_start + mb.mbr_partition_2.part_s);
        if(espacioVacio > 0){
            // CASE 2 (between P2 & P3)
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }

        porcentaje = ((float)mb.mbr_partition_3.part_s/mb.mbr_tamano)*100;
        if(mb.mbr_partition_3.part_type == 'P'){
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>";
            cadena += mb.mbr_partition_3.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }else{
            numeroParticionExtendida = 3;
            disco.seekg(mb.mbr_partition_3.part_start);
            disco.read((char*)&ebrAux, sizeof(EBR));
            if(ebrAux.part_status == 'E'){
                bloquesParticionExtendida = 2; // first ebr & free space
                espacioOcupadoExtendida = mb.mbr_partition_3.part_s;
            }else{
                // TODO Eliminación de lógicas para ver validación de espacio libre
                espacioOcupadoExtendida = ebrAux.part_s;
                bloquesParticionExtendida = 2;
                while(ebrAux.part_next != -1){
                    disco.seekg(ebrAux.part_next);
                    disco.read((char*)&ebrAux, sizeof(EBR));
                    espacioOcupadoExtendida += ebrAux.part_s;
                    bloquesParticionExtendida += 2;
                }
                if(espacioOcupadoExtendida < mb.mbr_partition_3.part_s) bloquesParticionExtendida += 1; // free space
            }
            cadena += "\n\t\t<TD colspan=\"";
            cadena += to_string(bloquesParticionExtendida);
            cadena += "\" border=\"2\"><B>";
            cadena += mb.mbr_partition_3.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }else if(mb.mbr_partition_2.part_status != 'E'){
        // CASE 2 (all space after P2)
        espacioVacio = mb.mbr_tamano - (mb.mbr_partition_2.part_start + mb.mbr_partition_2.part_s);
        if(espacioVacio > 0){
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }


    if(mb.mbr_partition_4.part_status != 'E'){
        espacioVacio = mb.mbr_partition_4.part_start - (mb.mbr_partition_3.part_start + mb.mbr_partition_3.part_s);
        if(espacioVacio > 0){
            // CASE 3 (between P3 & P4)
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }

        porcentaje = ((float)mb.mbr_partition_4.part_s/mb.mbr_tamano)*100;
        if(mb.mbr_partition_4.part_type == 'P'){
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>";
            cadena += mb.mbr_partition_4.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }else{
            numeroParticionExtendida = 4;
            disco.seekg(mb.mbr_partition_4.part_start);
            disco.read((char*)&ebrAux, sizeof(EBR));
            if(ebrAux.part_status == 'E'){
                bloquesParticionExtendida = 2; // first ebr & free space
                espacioOcupadoExtendida = mb.mbr_partition_4.part_s;
            }else{
                // TODO Eliminación de lógicas para ver validación de espacio libre
                espacioOcupadoExtendida = ebrAux.part_s;
                bloquesParticionExtendida = 2;
                while(ebrAux.part_next != -1){
                    disco.seekg(ebrAux.part_next);
                    disco.read((char*)&ebrAux, sizeof(EBR));
                    espacioOcupadoExtendida += ebrAux.part_s;
                    bloquesParticionExtendida += 2;
                }
                if(espacioOcupadoExtendida < mb.mbr_partition_4.part_s) bloquesParticionExtendida += 1; // free space
            }
            cadena += "\n\t\t<TD colspan=\"";
            cadena += to_string(bloquesParticionExtendida);
            cadena += "\" border=\"2\"><B>";
            cadena += mb.mbr_partition_4.part_name;
            cadena += "<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }

        espacioVacio = mb.mbr_tamano - (mb.mbr_partition_4.part_start + mb.mbr_partition_4.part_s);
        if(espacioVacio > 0){
            // CASE 4 (between P4 & EOD)
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }else if(mb.mbr_partition_3.part_status != 'E'){
        // CASE 3 (all space after P3)
        espacioVacio = mb.mbr_tamano - (mb.mbr_partition_3.part_start + mb.mbr_partition_3.part_s);
        if(espacioVacio > 0){
            porcentaje = ((float)espacioVacio/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD rowspan=\"2\" border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }

    cadena += "\n\t\t</TR>"; // closign first row

    if(numeroParticionExtendida == -1){
        disco.close();
        cadena += "\n\t</TABLE>>];\n}";
        return CrearReporte(cadena, reportPath);
    }

    int tamanoParticion = 0;
    switch(numeroParticionExtendida){
        case 1: disco.seekg(mb.mbr_partition_1.part_start); tamanoParticion = mb.mbr_partition_1.part_s; break;
        case 2: disco.seekg(mb.mbr_partition_2.part_start); tamanoParticion = mb.mbr_partition_2.part_s; break;
        case 3: disco.seekg(mb.mbr_partition_3.part_start); tamanoParticion = mb.mbr_partition_3.part_s; break;
        case 4: disco.seekg(mb.mbr_partition_4.part_start); tamanoParticion = mb.mbr_partition_4.part_s; break;
    }
    disco.read((char*)&ebrAux, sizeof(EBR));

    cadena += "\n\t\t<TR>";
    if(ebrAux.part_status == 'E'){
        // first ebr & free space
        cadena += "\n\t\t<TD border=\"2\">EBR</TD>";
        porcentaje = ((float)espacioOcupadoExtendida/mb.mbr_tamano)*100;
        cadena += "\n\t\t<TD border=\"2\"  bgcolor=\"white\"><B>Libre<BR/>";
        cadena += to_string(porcentaje);
        cadena += "%</B></TD>";
    }else{
        // TODO Eliminación de lógicas para ver validación de espacio libre
        cadena += "\n\t\t<TD border=\"2\">EBR</TD>";
        porcentaje = ((float)ebrAux.part_s/mb.mbr_tamano)*100;
        cadena += "\n\t\t<TD border=\"2\"><B>Lógica<BR/>";
        cadena += to_string(porcentaje);
        cadena += "%</B></TD>";

        while(ebrAux.part_next != -1){
            disco.seekg(ebrAux.part_next);
            disco.read((char*)&ebrAux, sizeof(EBR));
            cadena += "\n\t\t<TD border=\"2\">EBR</TD>";
            porcentaje = ((float)ebrAux.part_s/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD border=\"2\"><B>Lógica<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
        if(espacioOcupadoExtendida < tamanoParticion){
            // free space
            porcentaje = ((float)(tamanoParticion - espacioOcupadoExtendida)/mb.mbr_tamano)*100;
            cadena += "\n\t\t<TD border=\"2\"><B>Libre<BR/>";
            cadena += to_string(porcentaje);
            cadena += "%</B></TD>";
        }
    }

    cadena += "\n\t\t</TR>";
    cadena += "\n\t</TABLE>>];\n}";

    disco.close();
    return CrearReporte(cadena, reportPath);
}

