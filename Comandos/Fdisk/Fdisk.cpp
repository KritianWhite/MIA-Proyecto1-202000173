#include <iostream>
#include <string.h>
#include <cctype>
#include <fstream>
#include <limits>

#include "Fdisk.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"

using namespace std;


bool CrearParticion(string diskPath, string partitionName, int partitionSize, char sizeUnit, char partitionType, char partitionFit){
    //* Variables de control
    fstream disco;
    string nombres[4];
    int bytesParticion = 0, espacio_vacio[5];
    int primaria = 0, extendida = 0, numeroExtPart = 0; //* numberExtPart para ubicar la particion extendida, para crear lógicas
    char auxiliar;

    cout << "--> Obteniendo datos del disco..." << endl;
    disco.open(diskPath, ios::in|ios::out|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco en \"" << diskPath << "\" \033[0m" << endl;
        disco.close();
        return false;
    }

    disco.read((char *)&auxiliar, sizeof(char));
    if(auxiliar != idMBR){
        cout << "\033[0;91;49m[Error]: Error interno (idMBR). \033[0m" << endl;
        disco.close();
        return false;
    }

    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));
    for (int i = 0; i < 5; i++){
        /****/
        espacio_vacio[i] = 0;
    }
    
    //* Para particion 1
    if(mb.mbr_partition_1.part_status != 'E'){
        nombres[0] = mb.mbr_partition_1.part_name;
        switch(mb.mbr_partition_1.part_type){
            case 'P': {
                primaria += 1;
                break;
            }
            case 'E': {
                extendida += 1;
                numeroExtPart = 1;
                break;
            }
        }
        espacio_vacio[0] = mb.mbr_partition_1.part_start - sizeof(char) - sizeof(MBR);
    }else {
        nombres[0] = "";
        espacio_vacio[0] = mb.mbr_tamano - sizeof(char) - sizeof(MBR);
    }

    //* Para particion 2
    if(mb.mbr_partition_2.part_status != 'E'){
        nombres[1] = mb.mbr_partition_2.part_name;
        switch (mb.mbr_partition_2.part_start){
            case 'P': {
                primaria += 1;
                break;
            }
            case 'E': {
                extendida += 1;
                numeroExtPart = 2;
                break;
            }
        }
        espacio_vacio[1] = mb.mbr_partition_2.part_start - (mb.mbr_partition_1.part_start + mb.mbr_partition_1.part_s);
    }else {
        nombres[1] = "";
        if(mb.mbr_partition_1.part_status != 'E'){
            espacio_vacio[1] = mb.mbr_tamano - (mb.mbr_partition_1.part_start + mb.mbr_partition_1.part_s);
        }else ;
    }

    //* Para particion 3
    if(mb.mbr_partition_3.part_status != 'E'){
        nombres[2] = mb.mbr_partition_3.part_name;
        switch (mb.mbr_partition_3.part_type){
            case 'P': {
                primaria += 1;
                break;
            }
            case 'E': {
                extendida += 1;
                numeroExtPart = 3;
                break;
            }
        }
        espacio_vacio[2] = mb.mbr_partition_3.part_start - (mb.mbr_partition_2.part_start + mb.mbr_partition_2.part_s);
    }else{
        nombres[2] = "";
        if(mb.mbr_partition_2.part_status != 'E'){
            espacio_vacio[2] = mb.mbr_tamano - (mb.mbr_partition_2.part_start + mb.mbr_partition_2.part_s);
        }else ;
    }

    if(mb.mbr_partition_4.part_status != 'E'){
        nombres[3] = mb.mbr_partition_4.part_name;
        switch(mb.mbr_partition_4.part_type){
            case 'P': {
                primaria += 1;
                break;
            }
            case 'E': {
                extendida += 1;
                numeroExtPart = 4;
                break;
            }
        }
        espacio_vacio[3] = mb.mbr_partition_4.part_start - (mb.mbr_partition_3.part_start + mb.mbr_partition_3.part_s);
        espacio_vacio[4] = mb.mbr_tamano - (mb.mbr_partition_4.part_start + mb.mbr_partition_4.part_s);
    }else{
        nombres[3] = "";
        if(mb.mbr_partition_3.part_status != 'E'){
            espacio_vacio[3] = mb.mbr_tamano - (mb.mbr_partition_3.part_start + mb.mbr_partition_3.part_s);
            espacio_vacio[4] = 0;
        }else;
    }

    //* Validamos que el nombre no este repetido
    for(int i = 0; i < 4; i++){
        if(nombres[i] == partitionName){
            cout <<"\033[0;91;49m[Error]: El nombre \"" << partitionName << "\" de la partición que se intenta crear ya existe en el disco \033[0m" << endl;
            disco.close();
            return false;
        }
    }

    //* Validamos distribucion de particiones
    cout << "--> Validando distribución de particiones..." << endl;
    if(partitionType != 'L' && (primaria + extendida) == 4){
        cout << "\033[0;91;49m[Error]: limite de particiones alcanzado, existen " << to_string(primaria) << 
        " particiones primarias y " << to_string(extendida) << " particiones extendidas en el disco duro \033[0m" << endl;
        disco.close();
        return false;
    }
    if(partitionType == 'E' && extendida == 1){
        cout << "\033[0;91;49m[Error]: Limite de particiones extendidas alcanzado. (Solo puede haber una " <<
        "particion extendida en el disco duro). \033[0m" << endl;
        disco.close();
        return false;
    }
    if(partitionType == 'L' && extendida == 0){
        cout << "\033[0;91;49m[Error]: No se puede crear la particion lógica debido a que no existe una " <<
        "particion extendida en el disco duro. \033[0m" << endl;
        disco.close();
        return false;
    }

    //* Validamos el espacio disponible
    cout << "--> Validando que haya espacio suficiente..." << endl;
    switch(sizeUnit){
        case 'B' : bytesParticion = partitionSize; break;
        case 'K' : bytesParticion = partitionSize * 1024; break;
        case 'M' : bytesParticion = partitionSize * 1024 * 1024; break;
        default : cout << "\033[0;91;49m[Error]: Error interno: \""<< sizeUnit <<"\" no se reconoce para sizeUnit (K|B|M). \033[0m" << endl;
    }

    if(partitionType == 'E' && bytesParticion < (int)sizeof(EBR)){
        cout << "\033[0;91;49m[Error]: El tamaño de la partición extendida debe ser minimo de \""<< to_string(sizeof(EBR)) <<
        " bytes\" para poder escribir su EBR. Tamaño asignado: \""<< to_string(bytesParticion) <<" bytes\". \033[0m" << endl;
        disco.close();
        return false;
    }

    if(partitionType == 'L' && bytesParticion < (int)sizeof(EBR)){
        cout << "\033[0;91;49m[Error]: El tamaño de la partición logica debe ser minimo de \""<< to_string(sizeof(EBR)) <<
        " bytes\" para poder escribir su EBR. Tamaño asignado: \""<< to_string(bytesParticion) <<" bytes\". \033[0m" << endl;
        disco.close();
        return false;
    }

    if(mb.dsk_fit != 'F' && mb.dsk_fit != 'W' && mb.dsk_fit != 'B'){
        cout << "\033[0;91;49m[Error]: Error interno. \"" << mb.dsk_fit << "\" no se reconoce como Fit para el MBR (F|B|W). \033[0m" << endl;
        disco.close();
        return false;
    }

    //TODO--> Particiones primarias y extendidas
    if(partitionType == 'P' || partitionType == 'E'){

        if(mb.dsk_fit == 'F'){
            cout << "--> Creando nueva particion con nombre \"" << partitionName << "\" de  tamaño \"" << to_string(bytesParticion) 
            << " bytes\" utilizando First fit (ajuste de colocacion del disco)..." << endl;
        }
        else if (mb.dsk_fit == 'W'){
            cout << "--> Creando nueva particion con nombre \"" << partitionName << "\" de  tamaño \"" << to_string(bytesParticion) 
            << " bytes\" utilizando Worst fit (ajuste de colocacion del disco)..." << endl;
        }
        else if(mb.dsk_fit == 'B'){
            cout << "--> Creando nueva particion con nombre \"" << partitionName << "\" de  tamaño \"" << to_string(bytesParticion) 
            << " bytes\" utilizando Best fit (ajuste de colocacion del disco)..." << endl;
        }
        for (int case_number = 0; case_number < 5; case_number++){
            if(espacio_vacio[case_number] >= bytesParticion){
                Partition nuevaParticion;
                nuevaParticion.part_status = 'A';
                nuevaParticion.part_type = partitionType;
                nuevaParticion.part_fit = partitionFit;
                nuevaParticion.part_s = bytesParticion;
                //* Limpiamos el char
                memset(nuevaParticion.part_name, 0, 16);
                for(int i = 0; i < (int)partitionName.length() && i < 16; i++){
                    nuevaParticion.part_name[i] = partitionName[i];
                }

                switch(case_number){
                    case 0: {
                        //* Todo el disco si particion 1 no existe
                        if(mb.mbr_partition_1.part_status == 'E'){
                            nuevaParticion.part_start = sizeof(char) + sizeof(MBR);
                            mb.mbr_partition_1 = nuevaParticion;
                        }
                        //* En medio del MBR y particion 1
                        else if(mb.mbr_partition_4.part_status == 'E'){
                            mb.mbr_partition_4 = mb.mbr_partition_3;
                            mb.mbr_partition_3 = mb.mbr_partition_2;
                            mb.mbr_partition_2 = mb.mbr_partition_1;
                            nuevaParticion.part_start = sizeof(char) + sizeof(MBR);
                            mb.mbr_partition_1 = nuevaParticion;
                        }else {
                            cout << "\033[0;91;49m[Error]: El disco ya cuenta con 4 particiones.\033[0m" << endl;
                            disco.close();
                           return false;
                        }
                        break;
                    }
                    case 1: {
                        //* todo el espacio despues de particion 1 si particion 2 no existe
                        if(mb.mbr_partition_2.part_status == 'E'){
                            nuevaParticion.part_start = mb.mbr_partition_1.part_start + mb.mbr_partition_1.part_s;
                            mb.mbr_partition_2 = nuevaParticion;
                        }
                        //* En medio de particion 1 y particion 2
                        else if (mb.mbr_partition_4.part_status == 'E'){
                            mb.mbr_partition_4 = mb.mbr_partition_3;
                            mb.mbr_partition_3 = mb.mbr_partition_2;
                            nuevaParticion.part_start = mb.mbr_partition_1.part_start + mb.mbr_partition_1.part_s;
                            mb.mbr_partition_2 = nuevaParticion;
                        }else{
                            cout << "\033[0;91;49m[Error]: El disco ya cuenta con 4 particiones.\033[0m" << endl;
                            disco.close();
                           return false;
                        }
                        break;
                    }
                    case 2: {
                        //* todo el espacio despues de particion 2 si particion 3 no existe
                        if(mb.mbr_partition_3.part_status == 'E'){
                            nuevaParticion.part_start = mb.mbr_partition_2.part_start + mb.mbr_partition_2.part_s;
                            mb.mbr_partition_3 = nuevaParticion;
                        }
                        //* En medio de particion 2 y particion 3
                        else if (mb.mbr_partition_4.part_status = 'E'){
                            mb.mbr_partition_4 = mb.mbr_partition_3;
                            nuevaParticion.part_start = mb.mbr_partition_2.part_start + mb.mbr_partition_2.part_s;
                            mb.mbr_partition_3 = nuevaParticion;
                        }else{
                            cout << "\033[0;91;49m[Error]: El disco ya cuenta con 4 particiones.\033[0m" << endl;
                            disco.close();
                           return false;
                        }
                        break;
                    }
                    case 3: {
                        //* todo el espacio despues de particion 3 si particion 4 no existe
                        if(mb.mbr_partition_4.part_status == 'E'){
                            nuevaParticion.part_start = mb.mbr_partition_3.part_start + mb.mbr_partition_3.part_s;
                            mb.mbr_partition_4 = nuevaParticion;
                        }
                        //* Si esta en medio de particion 3 y particion 4 seria un error
                        else{
                            cout << "\033[0;91;49m[Error]: El disco ya cuenta con 4 particiones.\033[0m" << endl;
                            disco.close();
                           return false;
                        }
                        break;
                    }
                }//* End switch case

                //* Crando primer EBR si es extendida
                if(partitionType == 'E'){
                    cout << "--> Configurando primer EBR para la particion extendia creada..." << endl;
                    EBR primerEBR;
                    primerEBR.part_status = 'E';
                    primerEBR.part_next = -1;
                    primerEBR.part_start = nuevaParticion.part_start;
                    disco.seekp(nuevaParticion.part_start);
                    disco.write((char *)&primerEBR, sizeof(EBR));
                }
                //* Actualizando MBR en el disco
                disco.seekp(1);
                disco.write((char *)&mb, sizeof(MBR));
                disco.close();
                return true;
            }//* End if case
        }//* End for loop

        switch (sizeUnit){
            case 'B': {
                cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion \"" << to_string(bytesParticion) 
                << " bytes\" (" << to_string(partitionSize) << " B) \033[0m" << endl;
                break;
            }
            case 'K': {
                cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion \"" << to_string(bytesParticion) 
                << " bytes\" (" << to_string(partitionSize) << " KB) \033[0m" << endl;
                break;
            }
            case 'M': {
                cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion \"" << to_string(bytesParticion) 
                << " bytes\" (" << to_string(partitionSize) << " MB) \033[0m" << endl;
                break;
            }
        }

        // cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion \"" << to_string(bytesParticion) 
        // << " bytes\" (" << to_string(partitionSize) << ") \033[0m" << endl;
        disco.close();
        return false;

    }//* End if primeria y extendida

    //TODO--> Particion logica (el array de espacios_vacio no lo usamos acá)
    EBR ebr_aux;
    int bytesDisponibles;
    switch (numeroExtPart){
        case 1: {
            disco.seekg(mb.mbr_partition_1.part_start);
            disco.read((char*)&ebr_aux, sizeof(EBR));
            bytesDisponibles = mb.mbr_partition_1.part_s;
            break;
        }
        case 2: {
            disco.seekg(mb.mbr_partition_2.part_start);
            disco.read((char*)&ebr_aux, sizeof(EBR));
            bytesDisponibles = mb.mbr_partition_2.part_s;
            break;
        }
        case 3: {
            disco.seekg(mb.mbr_partition_3.part_start);
            disco.read((char*)&ebr_aux, sizeof(EBR));
            bytesDisponibles = mb.mbr_partition_3.part_s;
            break;
        }
        case 4: {
            disco.seekg(mb.mbr_partition_4.part_start);
            disco.read((char*)&ebr_aux, sizeof(EBR));
            bytesDisponibles = mb.mbr_partition_4.part_s;
            break;
        }
        default: {
            cout << "\033[0;91;49m[Error]: Error critico: vino \"" << numeroExtPart << "\" como numero de particion extendida. \033[0m" << endl;
            disco.close();
            break;
        }
    }

    //TODO--> Particion extendida vacia.
    if(ebr_aux.part_status == 'E'){
        if(bytesParticion > bytesDisponibles){
            switch (sizeUnit){
                case 'B': {
                    cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion logica de \"" << to_string(bytesParticion) 
                    << " bytes\" (" << to_string(partitionSize) << " B). Tamaño de la particion extendida: " << to_string(bytesDisponibles) << " bytes. \033[0m" << endl;
                    break;
                }
                case 'K': {
                    cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion logica de \"" << to_string(bytesParticion) 
                    << " bytes\" (" << to_string(partitionSize) << " KB)Tamaño de la particion extendida: " << to_string(bytesDisponibles) << " bytes. \033[0m" << endl;
                    break;
                }
                case 'M': {
                    cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion logica de \"" << to_string(bytesParticion) 
                    << " bytes\" (" << to_string(partitionSize) << " MB) Tamaño de la particion extendida: " << to_string(bytesDisponibles) << " bytes. \033[0m" << endl;
                    break;
                }
            }
            disco.close();
            return false;
        }

        cout << "--> Creando nueva partición lógica " << partitionName << " de " << to_string(bytesParticion) << " bytes..." << endl;
        ebr_aux.part_status = 'A';
        ebr_aux.part_fit = partitionFit;
        ebr_aux.part_s = bytesParticion;
        memset(ebr_aux.part_name, 0, 16);
        for(int i = 0; i < (int)partitionName.length() && i < 16; i++){
            ebr_aux.part_name[i] = partitionName[i];
        }

        //* Sobreescribimos el EBR
        disco.seekp(ebr_aux.part_start);
        disco.write((char*)&ebr_aux, sizeof(EBR));
        disco.close();
        return true;
    } //* End if case particiones logicas

    //TODO--> Particion extendida con particiones logicas
    int particionLogica = 1;
    do{
        bytesDisponibles = bytesDisponibles - ebr_aux.part_s;
        if(ebr_aux.part_next == -1) break;

        disco.seekg(ebr_aux.part_next);
        disco.read((char*)&ebr_aux, sizeof(EBR));
        particionLogica += 1;
    }while(particionLogica < 12); //* Limite de particiones logicas (12)

    if(particionLogica == 12){
        cout << "\033[0;91;49m[Error]: Límite de particiones logicas alcanzado. Solo se permiten 12 particiones logicas.\033[0m" << endl;
        disco.close();
        return false;
    }

    if(bytesParticion > bytesDisponibles){
        switch (sizeUnit){
            case 'B': {
                cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion logica de \"" << to_string(bytesParticion) 
                << " bytes\" (" << to_string(partitionSize) << " B). Espacio disponible: " << to_string(bytesDisponibles) << " bytes. \033[0m" << endl;
                break;
            }
            case 'K': {
                cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion logica de \"" << to_string(bytesParticion) 
                << " bytes\" (" << to_string(partitionSize) << " KB). Espacio disponible: " << to_string(bytesDisponibles) << " bytes. \033[0m" << endl;
                break;
            }
            case 'M': {
                cout << "\033[0;91;49m[Error]: No se ha encontrado un espacio contiguo para la particion logica de \"" << to_string(bytesParticion) 
                << " bytes\" (" << to_string(partitionSize) << " MB). Espacio disponible: " << to_string(bytesDisponibles) << " bytes. \033[0m" << endl;
                break;
            }
        }
        disco.close();
        return false;
    }

    ebr_aux.part_next = ebr_aux.part_start + ebr_aux.part_s;

    cout << "--> Creando la nueva particion lógica " << partitionName << " de tamaño " << to_string(bytesParticion) << " bytes..." << endl;
    EBR nuevo_EBR;
    nuevo_EBR.part_status = 'A';
    nuevo_EBR.part_fit = partitionFit;
    nuevo_EBR.part_start = ebr_aux.part_next;
    nuevo_EBR.part_s = bytesParticion;
    nuevo_EBR.part_next = -1;
    memset(nuevo_EBR.part_name, 0, 16);
    for(int i = 0; i < (int)partitionName.length() && i < 16; i++){
        nuevo_EBR.part_name[i] = partitionName[i];
    }

    //* Sobreescribimos el anterior EBR
    disco.seekp(ebr_aux.part_start);
    disco.write((char*)&ebr_aux, sizeof(EBR));
    //* Escribiendo el nuevo EBR
    disco.seekp(nuevo_EBR.part_start);
    disco.write((char*)&nuevo_EBR, sizeof(EBR));
    disco.close();
    return true;
}

bool EliminarParticion(string diskPath, string partitionName, string deletionType){
    //* Variables de control
    fstream disco;
    int numeroExPart = 0, numeroParticion = 0, particionInicial = 0, tamanoParticion = 0;
    bool particionExtEliminada = false;

    cout << "--> Obteniendo los datos del disco..." << endl;
    disco.open(diskPath, ios::in|ios::out|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco en \"" << diskPath << "\" \033[0m" << endl;
        disco.close();
        return false;
    }

    char auxiliar;
    disco.read((char *)&auxiliar, sizeof(char));
    if(auxiliar != idMBR){
        cout << "\033[0;91;49m[Error]: Error interno. Problemas con id MBR. \033[0m" << endl;
        disco.close();
        return false;
    }

    MBR mb;
    disco.read((char*)&mb, sizeof(MBR));
    while(true){
        if(mb.mbr_partition_1.part_status != 'E'){
            if(mb.mbr_partition_1.part_type == 'E') numeroExPart += 1;
            if(mb.mbr_partition_1.part_name == partitionName){
                if(mb.mbr_partition_1.part_type == 'E') particionExtEliminada = true;
                particionInicial = mb.mbr_partition_1.part_start;
                tamanoParticion = mb.mbr_partition_1.part_s;
                numeroParticion = 1;
                break;
            }
        }

        if(mb.mbr_partition_2.part_status != 'E'){
            if(mb.mbr_partition_2.part_type == 'E') numeroExPart = 2;
            if(mb.mbr_partition_2.part_name == partitionName){
                if(mb.mbr_partition_2.part_type == 'E') particionExtEliminada = true;
                particionInicial = mb.mbr_partition_2.part_start;
                tamanoParticion = mb.mbr_partition_2.part_s;
                numeroParticion = 2;
                break;
            }
        }

        if(mb.mbr_partition_3.part_status != 'E'){
            if(mb.mbr_partition_3.part_type == 'E') numeroExPart = 3;
            if(mb.mbr_partition_3.part_name == partitionName){
                if(mb.mbr_partition_3.part_type == 'E') particionExtEliminada = true;
                particionInicial = mb.mbr_partition_3.part_start;
                tamanoParticion = mb.mbr_partition_3.part_s;
                numeroParticion = 3;
                break;

            }
        }

        if(mb.mbr_partition_4.part_status != 'E'){
            if(mb.mbr_partition_4.part_type == 'E') numeroExPart = 4;
            if(mb.mbr_partition_4.part_name == partitionName){
                if(mb.mbr_partition_4.part_type == 'E') particionExtEliminada = true;
                particionInicial = mb.mbr_partition_4.part_start;
                tamanoParticion = mb.mbr_partition_4.part_s;
                numeroParticion = 4;
                break;
            }
        }
        break;
    } //* End while loop

    if(particionInicial == 0){
        if(numeroExPart == 0){
            cout << "\033[0;91;49m[Error]: No se pudo encontrar la partición \"" << partitionName << "\". \033[0m" << endl;
            disco.close();
            return false;
        }
        disco.close();
        return false;
    }

    //* Eliminacion de particion primaria o extendida
    if(particionExtEliminada) cout << "[Mensaje]: Se detecto que la particion es extendida por lo que tambien serán eliminadas las particiones logicas que esta prosea." << endl;
    cout << "--> ¿Desea eliminar la particion " << partitionName << " permanentemente? Presione 1 \"SI\" desea eliminar o 2 si \"NO\" desea eliminar. " << endl;
    int opcion;
    cin >> opcion; 
    if(!cin) throw "\033[0;91;49m[Error]: Ingrese un numero entero. \033[0m";
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    //cout << "\033[0;92;49m[Correcto]:  \033[0m"

    if(opcion == 2){
        cout << "[Mensaje]: la particion no será eliminada. " << endl;
        disco.close();
        return false;
    }else if (opcion != 1){
        cout << "\033[0;91;49m[Error]: Ingrese un número válido.  \033[0m";
        disco.close();
        return false;
    }

    cout << "--> Se esta eliminando la particion " << partitionName << "..." << endl;
    disco.seekp(particionInicial);
    for(int i = 0; i < tamanoParticion; i++){
        disco.write((char*)&nullChar, sizeof(char));
    }

    cout << "--> Se está actualizando el MBR del disco..." << endl;
    switch(numeroParticion){
        case 1: {
            mb.mbr_partition_1 = mb.mbr_partition_2;
            mb.mbr_partition_2 = mb.mbr_partition_3;
            mb.mbr_partition_3 = mb.mbr_partition_4;
            break;
        }
        case 2: {
            mb.mbr_partition_2 = mb.mbr_partition_3;
            mb.mbr_partition_3 = mb.mbr_partition_4;
            break;
        }
        case 3: {
            mb.mbr_partition_3 = mb.mbr_partition_4;
            break;
        }
    }
    mb.mbr_partition_4.part_status = 'E';
    mb.mbr_partition_4.part_s = -1;
    memset(mb.mbr_partition_4.part_name, 0, 16);

    //* Sobreescribmos el MBR
    disco.seekp(1);
    disco.write((char*)&mb, sizeof(MBR));
    disco.close();

    return true;
}

Fdisk _Fdisk(char *parametros){

    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";

    //* path (obligatorio)
    string path = "";
    bool vPath = false;
    //* Nombre (obligatorio)
    string nombreParticion = "";
    bool vNombre = false;
    
    //TODO --> PARA LA CREACION DE UNA PARTICION
    //* tamaño de la particion (obligatorio)
    string tamanoParticionStr = "";
    int tamanoParticion = 0;
    bool vTamano = false;
    
    //* units (opcional B | K | M)
    string tamanoUnitStr = "";
    char tamanoUnit = '\0';
    bool vUnit = false;

    //* Tipo de partición (opcional P [default] | E | L)
    string tipoParticionStr = "";
    char tipoParticion = '\0';
    bool vTipo = false;

    // Fit (opcional F | B | W [default])
    string fitParticionStr = "";
    char fitParticion = '\0';
    bool vFit = false;

    //TODO --> PARA LA ELIMINACION DE UNA PARTICION
    //* Eliminar el tipo
    string eliminarTipo = "";
    bool vEliminado = false;

    //TODO --> PARA CAMBIAR EL TAMAÑO DE LA PRTICION
    //* Cambiar tamaño
    string cambiarTamanoStr = "";
    int cambiarTamano = 0;
    bool vAdd = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch (estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de caracters iniciales de cada parametro
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'p') estado = 2;
                else if((char)tolower(parametros[i]) == 's') estado = 9;
                else if ((char)tolower(parametros[i]) == 'n') estado = 16;
                else if ((char)tolower(parametros[i]) == 'u') estado = 23;
                else if ((char)tolower(parametros[i]) == 't') estado = 29;
                else if ((char)tolower(parametros[i]) == 'f') estado = 35;
                else if ((char)tolower(parametros[i]) == 'd') estado = 40;
                else if ((char)tolower(parametros[i]) == 'a') estado = 48;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> PATH
            //* Reconocimiento del caracter a
            case 2: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter h
            case 4: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'h') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1; 
                break;
            }
            //* Reconocimiento del caraacter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de la path con comillas
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vPath = false; path = ""; estado = 7;}
                else if (parametros[i] == '/'){vPath = false; path = ""; path += parametros[i]; estado = 8;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(path.length() > 0) vPath = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //* Reconocimiento de la path sin comillas
            case 8: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(path.length() > 0) vPath = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //TODO --> SIZE
            //* Reconocimiento del caracter i
            case 9: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter z
            case 10: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'z') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 11: {
                parametroActual += parametros[0];
                if ((char)tolower(parametros[i]) == 'e') estado = 12;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 12: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 13;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de numero para el size (primeramente reconcemos el signo menos (-))
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == '-'){vTamano = false; tamanoParticionStr = parametros[i]; estado = 14;}
                else if (isNumber(parametros[i])){vTamano = false; tamanoParticionStr = parametros[i]; estado = 15;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que sean numeros enteros
            case 14: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])){tamanoParticionStr += parametros[i]; estado = 15;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 15: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])) tamanoParticionStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if (stoi(tamanoParticionStr) <= 0) cout << "\033[0;91;49m[Error]: El valor del parametro >size debe de ser entero positivo. \033[0m" << endl;
                    else {vTamano = true; tamanoParticion = stoi(tamanoParticionStr);}
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> NAME
            //* Reconocimiento del caracter a
            case 16: {
                //cout << "Llegue al 16" << endl;
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter m
            case 17: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'm') estado = 18;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter e
            case 18: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 19;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter =
            case 19: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 20;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del nombre con comillas
            case 20: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {vNombre = false; nombreParticion = ""; estado = 21;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vNombre = false; nombreParticion = ""; nombreParticion += parametros[i]; estado = 22;}
                break;
            }
            case 21: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(nombreParticion.length() > 0){
                        for(int i = 0; i < (int)nombreParticion.length(); i++){
                            nombreParticion[i] = tolower(nombreParticion[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[0;91;49m[Error]: \"" << parametroActual << "\" asigne un nombre a la particion. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreParticion += parametros[i];
                break;
            }
            //* Reconocimiento del nombre sin comillas
            case 22: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(nombreParticion.length() > 0){
                        for(int i = 0; i < (int)nombreParticion.length(); i++){
                            nombreParticion[i] = tolower(nombreParticion[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[0;91;49m[Error]: \"" << parametroActual << "\" asigne un nombre al disco. \033[0m" << endl;
                    parametroActual = ""; 
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombreParticion += parametros[i];
                break;
            }
            //TODO --> UNIT
            //* Reconocimiento del caracter n
            case 23: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'n') estado = 24;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter i
            case 24: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'i') estado = 25;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 25: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 26;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 26: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 27;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos si realmente se estan recibiendo letras
            case 27: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])){vUnit = false; tamanoUnitStr = parametros[i]; estado = 28;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            // Hacemos la verificacion de las unidades
            case 28: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i]))tamanoUnitStr += parametros[i];
                else if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(tamanoUnitStr.c_str(), "b") == 0){vUnit = true; tamanoUnit = 'B';}
                    else if (strcasecmp(tamanoUnitStr.c_str(), "k") == 0){vUnit = true; tamanoUnit = 'K';}
                    else if (strcasecmp(tamanoUnitStr.c_str(), "m") == 0){vUnit = true; tamanoUnit = 'M';}
                    else cout << "\033[0;91;49m[Error]: No se reconocio la unidad en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> TYPE
            //* Reconocimiento del caracter y
            case 29: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'y') estado = 30;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter p
            case 30: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 31;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 31: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 32;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 32: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 33;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que realmente venga una letra
            case 33: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])){vTipo = false; tipoParticionStr = parametros[i]; estado = 34;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos los tipos de particiones
            case 34: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])) tipoParticionStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(tipoParticionStr.c_str(), "p") == 0){vTipo = true; tipoParticion = 'P';}
                    else if (strcasecmp(tipoParticionStr.c_str(), "e") == 0){vTipo = true; tipoParticion = 'E';}
                    else if (strcasecmp(tipoParticionStr.c_str(), "l") == 0){vTipo = true; tipoParticion = 'L';}
                    else cout << "\033[0;91;49m[Error]: No se reconocio el tipo de particion en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> FIT
            //* Reconocimiento del caracter i
            case 35: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 36;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 36: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 37;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 37: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 38;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que realmente venga una letra
            case 38: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])){vFit = true; fitParticionStr = parametros[i]; estado = 39;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Validamos el ajuste de espacio para la particion
            case 39: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])) fitParticionStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(fitParticionStr.c_str(), "bf") == 0){vFit = true; fitParticion = 'B';}
                    else if (strcasecmp(fitParticionStr.c_str(), "wf") == 0){vFit = true; fitParticion = 'W';}
                    else if (strcasecmp(fitParticionStr.c_str(), "ff") == 0){vFit = true; fitParticion = 'F';}
                    else cout << "\033[0;91;49m[Error]: No se reconocio el ajuste de espacio para la particion en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> DELETE
            //* Reconocimiento del caracter e
            case 40: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 41;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter l
            case 41: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'l') estado = 42;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 42: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 43;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 43: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 44;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter e
            case 44: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 45;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 45: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 46;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos que sean letras realmente
            case 46: {
                parametroActual += parametros[i];
                if (isLetter(parametros[i])){vEliminado = false; eliminarTipo = parametros[i]; estado = 47;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Realizamos la verificacion de que el valor del parametro sea full
            case 47: {
                parametroActual += parametros[i];
                if(isLetter(parametros[i])) eliminarTipo += parametros[i];
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(strcasecmp(eliminarTipo.c_str(), "full") == 0){vEliminado = true; eliminarTipo = "full";}
                    else cout << "\033[0;91;49m[Error]: No se reconocio full para la eliminacion de la particion en el parametro " << parametroActual << " del comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO --> ADD
            //* reconocimiento del caracter d
            case 48: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 49;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* reconocimiento del caracter d
            case 49: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 50;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 50: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 51;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos primeramente el signo negativo (-)
            case 51: {
                parametroActual += parametros[i];
                if (parametros[i] == '-') {vAdd = false; cambiarTamanoStr = parametros[i]; estado = 52;}
                else if (isNumber(parametros[i])){vAdd = false; cambiarTamanoStr = parametros[i]; estado = 53;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Verificamos de que realmente sea numero entero
            case 52: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])){cambiarTamanoStr += parametros[i]; estado = 53;}
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            case 53: {
                parametroActual += parametros[i];
                if(isNumber(parametros[i])) cambiarTamanoStr += parametros[i];
                else if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    vAdd = true;
                    cambiarTamano = stoi(cambiarTamanoStr);
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Parametro invalido
            case -1: {
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es invalido para el comando fdisk \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else parametroActual += parametros[i];
                break;
            }
            //* Comentarios
            case -2: {
                comentario += parametros[i];
                break;
            }
        }//* End switch case
    }//* End for loop

    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Fdisk fd;

    //* Primero vamos a crear la particion
    if(vPath && vNombre){
        if(vTamano){
            if(!vAdd && !vEliminado){
                fd.diskPath = path;
                fd.partitionName = nombreParticion;
                fd.partitionSize = tamanoParticion;
                if(vUnit) fd.sizeUnit = tamanoUnit;
                else fd.sizeUnit = 'K';
                if(vTipo) fd.partitionType = tipoParticion;
                else fd.partitionType = 'P';
                if(vFit) fd.partitionFit = fitParticion;
                else fd.partitionFit = 'W';
                fd.deletionType = "";
                fd.resizePartition = false;
                fd.acceso = true;
                return fd;
            }

            //* Verificamos que los parametros Add y delete no vengan en la misma instruccion
            if(vAdd && vEliminado) {
                cout << "\033[0;91;49m[Error]: Los parametros \">Add\" y \">Delete\"" <<
                    " no pueden venir en la misma instruccion del parametro \">size\""<< 
                    " en el comando fdisk. \033[0m" << endl;
                fd.acceso = false;
                return fd;
            }
        }

        //* Eliminamos particion
        if(vEliminado){
            if(!vAdd && !vTamano){
                fd.diskPath = path;
                fd.partitionName = nombreParticion;
                fd.deletionType = eliminarTipo;
                fd.resizePartition = false;
                fd.acceso = true;
                return fd;
            }

            //* Verificamos que los parametros Add y Size no vengan en la misma instruccion
            if(vAdd && vTamano){
                cout << "\033[0;91;49m[Error]: Los parametros \">Add\" y \">size\"" <<
                    " no pueden venir en la misma instruccion del parametro \">Delete\""<< 
                    " en el comando fdisk. \033[0m" << endl;
                fd.acceso = false;
                return fd;
            }
        }

        // Aumentar o disminuir tamaño de particion
        if(vAdd){
            if(!vTamano){
                fd.diskPath = path;
                fd.partitionName = nombreParticion;
                fd.sizetoChange = cambiarTamano;
                if(vUnit) fd.sizeUnit = tamanoUnit;
                else fd.sizeUnit = 'K';
                fd.deletionType = "";
                fd.resizePartition = true;
                fd.acceso = true;
                return fd;
            }

            cout << "\033[0;91;49m[Error]: Los parametros \">Add\" y \">size\" no pueden venir en" <<
             " la misma instruccioin del comando fdisk. \033[0m" << endl;
            fd.acceso = false;
            return fd;
        }

        cout << "\033[0;91;49m[Error]: Faltan parametros para poder realizar la particion. \033[0m" << endl;
        fd.acceso = false;
        return fd;
    }

    if(!vPath && !vNombre){
        cout << "\033[0;91;49m[Error]: Los parametros \">path\" y \">name\" son obligatorios para la creacion de la particion. \033[0m" << endl;
        fd.acceso = false;
        return fd;
    }

}

