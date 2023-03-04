#include <iostream>
#include <string.h> // memset()...
#include <cmath>    // floor() para n inodos
#include <fstream>

#include "Mkfs.h"
#include "../Estructura.h"
#include "../../aux_funciones.h"

using namespace std;


int Calc_NumeroInodos(int partitionSize, int fileSystem, bool isLogicalPartition){
    /*
    *FORMULA
    *partition_size = Superblock + InodeBitmap + BlockBitmap + Inodes + Blocks
    * partition.part_s = sizeof(Superblock) + n + 3n + n*sizeof(Inode) + 3n*sizeof(Block)
    *partition.part_s - sizeof(Superblock) = 4n + n*(sizeof(Inode) + 3*sizeof(Block))
    *n*(4 + sizeof(Inode) + 3*sizeof(Block)) = partition.part_s - sizeof(Superblock)
    *
    *numerator = partition.part_s - sizeof(Superblock)   [- sizeof(EBR) for logical partitions]
    *denominator = 4 + sizeof(Inode) + 3*sizeof(Block)
    *n = numerator / denominator
    *
    *-> Available inodes: n
    *-> Available blocks: 3n
    */

    float numerador, denominador, resultado;
    int inodes = 0;
    if(!isLogicalPartition){
        //* EXT2
        numerador = partitionSize - sizeof(Superblock);
        //* 4 char para bitmaps, 1 inodo, 3 bloques
        denominador = 4*sizeof(char) + sizeof(Inode) + 3*sizeof(FileBlock);
        resultado = numerador / denominador;
        //* Redondeando el numero al entero mas cercano
        int inodes = floor(resultado);
        return inodes;
    }

    return inodes;
}

bool FormatoParticion(Mkfs mf, PartitionNode *&primerNodo){
    //* Verificamos que el id representa una particion montada
    if(!isIdInList(primerNodo, mf.partitionId)){
        cout << "[Mensaje]: La particion con id " << mf.partitionId << " no fue encontrada " <<
        " en la lista de particiones montadas (use mount para ver la lista de particiones)" << endl;
        return false; 
    }

    cout << "--> Obteniendo datos del disco..." << endl;
    fstream disco;
    string path = getDiskPath(primerNodo, mf.partitionId);
    disco.open(path, ios::in|ios::out|ios::binary);
    if(disco.fail()){
        cout << "\033[0;91;49m[Error]: No se puede abrir el disco asociado a la particion " << 
        mf.partitionId << " de la ruta " << path << "\033[0m" << endl;
        disco.close();
        return false;
    }

    char auxiliar;
    disco.read((char *)&auxiliar, sizeof(char));
    if(auxiliar != idMBR){
        cout << "\033[0;91;49m[Error]: No vino char idMBR antes del MBR. Error interno. \033[0m" << endl;
        disco.close();
        return false;
    }

    MBR mb;
    disco.read((char *)&mb, sizeof(MBR));
    string nombreParticion = getPartitionId(primerNodo, mf.partitionId);
    int numeroParticionPrimaria = 0, particionInicial = 0, tamanoParticion = 0;
    char partStatus = '0';
    int numeroParticionExtendidA = 0;

    while (true){
        if(mb.mbr_partition_1.part_start != 'E'){
            if(mb.mbr_partition_1.part_name == nombreParticion){
                if(mb.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La particion montada no coincide con una extendida, " <<
                    " solo pueden montarse primarias o extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 1;
                particionInicial = mb.mbr_partition_1.part_start;
                tamanoParticion = mb.mbr_partition_1.part_s;
                partStatus = mb.mbr_partition_1.part_status;
                break;
            }
            if(mb.mbr_partition_1.part_type == 'E') numeroParticionExtendidA = 1;
        }
        if(mb.mbr_partition_2.part_start != 'E'){
            if(mb.mbr_partition_2.part_name == nombreParticion){
                if(mb.mbr_partition_2.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La particion montada no coincide con una extendida, " <<
                    " solo pueden montarse primarias o extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 2;
                particionInicial = mb.mbr_partition_2.part_start;
                tamanoParticion = mb.mbr_partition_2.part_s;
                partStatus = mb.mbr_partition_2.part_status;
                break;
            }
            if(mb.mbr_partition_2.part_type == 'E') numeroParticionExtendidA = 2;
        }
        if(mb.mbr_partition_3.part_start != 'E'){
            if(mb.mbr_partition_3.part_name == nombreParticion){
                if(mb.mbr_partition_3.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La particion montada no coincide con una extendida, " <<
                    " solo pueden montarse primarias o extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 3;
                particionInicial = mb.mbr_partition_3.part_start;
                tamanoParticion = mb.mbr_partition_3.part_s;
                partStatus = mb.mbr_partition_3.part_status;
                break;
            }
            if (mb.mbr_partition_3.part_type == 'E') numeroParticionExtendidA = 3;
        }
        if(mb.mbr_partition_4.part_start != 'E'){
            if(mb.mbr_partition_4.part_name == nombreParticion){
                if(mb.mbr_partition_4.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La particion montada no coincide con una extendida, " <<
                    " solo pueden montarse primarias o extendida. \033[0m" << endl;
                    disco.close();
                    return false;
                }
                numeroParticionPrimaria = 4;
                particionInicial = mb.mbr_partition_4.part_start;
                tamanoParticion = mb.mbr_partition_4.part_s;
                partStatus = mb.mbr_partition_4.part_status;
                break;
            }
            if(mb.mbr_partition_4.part_type == 'E') numeroParticionExtendidA = 4;
        }
        break;
    }//* End while loop

    //* Mkfs particion primaria
    if(numeroParticionPrimaria != 0){
        if(partStatus == 'F'){
            cout << "--> La particion " << nombreParticion << " ya cuenta con un sistema de archivos. " <<
            "¿Desea formatear la unidad? (Tenga en cuenta que se perderan sus datos). Presion 1(Sí)/2(No): ";
            int opcion;
            cin >> opcion; if(!cin) throw "\033[0;91;49m[Error]: Ingrese un numero entero \033[0m";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if(opcion == 2){
                cout << "[Mensaje]: El formateo de la particion se ha cancelado." << endl;
                disco.close();
                return false;
            }else if(opcion != 1){
                cout << "\033[0;91;49m[Error]: Ingrese un numero valido. \033[0m" << endl;
                disco.close();
                return false;
            }
        }

        cout << "--> Se esta formateando " << mf.formatType << " en la particion " << mf.partitionId << "..." << endl;
        disco.seekp(particionInicial);
        for(int i = 0; i < tamanoParticion; i++){
            disco.write((char*)&cero, sizeof(char));
        }
        //* EXT2
        cout << "--> configurando superbloque..." << endl;
        int inodes = Calc_NumeroInodos(tamanoParticion, mf.fileSystem, false);

        Superblock sb;
        sb.s_filesystem_type = mf.fileSystem;
        sb.s_inodes_count = inodes;
        sb.s_blocks_count = 3*inodes;
        sb.s_free_blocks_count = 3*inodes; //* cambiaran conforme se agreguen los bloques
        sb.s_free_inodes_count = inodes; //* cambiaran conforme se agreguen inodos
        time(&sb.s_mtime);
        sb.s_umtime = NULL;
        sb.s_mnt_count = 1;
        sb.s_magic = 61257; //* 0xEF53
        sb.s_inode_s = sizeof(Inode);
        sb.s_block_s = sizeof(FolderBlock);
        sb.s_first_ino = particionInicial + sizeof(Superblock) + inodes + 3*inodes; //* Cambiaran conforme se agreguen bloques
        sb.s_first_blo = particionInicial + sizeof(Superblock) + inodes + 3*inodes + inodes*sizeof(Inode); //* Cambiaran conforme se agreguen bloques
        sb.s_bm_inode_start = particionInicial + sizeof(Superblock);
        sb.s_bm_block_start = particionInicial +sizeof(Superblock) + inodes;
        sb.s_inode_start = particionInicial + sizeof(Superblock) + inodes + 3*inodes;
        sb.s_block_start = particionInicial + sizeof(Superblock) + inodes + 3*inodes + inodes*sizeof(Inode);

        cout << "--> Creando directorio root (/)..." << endl;
        //* Creacion de inodo carpeta
        Inode root;
        root.i_uid = 1; //* id user: 1->root
        root.i_gid = 1; //* id group: 1-->root
        root.i_s = -1; //* folder
        root.i_atime = NULL;
        time(&root.i_ctime);
        time(&root.i_mtime);
        root.i_block[0] = sb.s_first_blo; //* Bloque archivos para users.txt
        for (int i = 1; i < 15; i++){
            root.i_block[i] = -1;
        }
        root.i_type = '0'; //* folder
        root.i_perm = 775; // rwx rwx r-x

        //* Escribiendo el inodo carpeta "/"
        disco.seekp(sb.s_first_ino);
        disco.write((char*)&root, sizeof(Inode));
        sb.s_first_ino += sb.s_inode_s;
        sb.s_free_inodes_count -= 1;
        //* Escribiendo el bitmap de inodos
        disco.seekg(sb.s_bm_inode_start);
        while(!disco.eof()){
            disco.read((char *)&auxiliar, sizeof(char));
            if(!disco.eof() && auxiliar == cero){
                //* 1 antes del cero encontrado para escribir el 1
                disco.seekp(-1, ios::cur);
                disco.write((char*)&uno, sizeof(char));
                break;
            }
        }

        cout << "--> Creando archivo /user.text ..." << endl;
        //* Creacion de bloque carpeta para guardar el archivo inodo que viene
        FolderBlock FB;
        string users = "users.txt";
        //* FB.b_content[0] -> .
        memset(FB.b_content[0].b_name, 0, 12);
        FB.b_content[0].b_name[0] = '.';
        FB.b_content[0].b_inodo = sb.s_inode_start;
        //* FB.b_content[1] -> ..
        memset(FB.b_content[1].b_name, 0, 12);
        FB.b_content[1].b_name[0] = '.';
        FB.b_content[1].b_name[1] = '.';
        FB.b_content[1].b_inodo = sb.s_inode_start;
        //* FB.b_content[2] -> users.txt
        memset(FB.b_content[2].b_name, 0, 12);
        for (int j = 0; j < (int)users.length() && j < 12; j++){
            FB.b_content[2].b_name[j] = users[j];
        }
        //* Ya se tomo en cuenta el inodo raiz
        FB.b_content[2].b_inodo = sb.s_first_ino;
        //* FB.b_content[3] --> vacio
        memset(FB.b_content[3].b_name, 0, 12);
        FB.b_content[3].b_inodo = -1;

        //* Escribiendo el bloque carpeta
        disco.seekp(sb.s_first_blo);
        disco.write((char*)&FB, sizeof(FolderBlock));
        sb.s_first_blo += sb.s_block_s;
        sb.s_free_blocks_count -= 1;
        //* Escribiendo en el bitmap de bloques
        disco.seekg(sb.s_bm_block_start);
        while(!disco.eof()){
            disco.read((char*)&auxiliar, sizeof(char));
            if(!disco.eof() && auxiliar == cero){
                //* 1 antes del cero encontrado para escribir el 1
                disco.seekp(-1, ios::cur);
                disco.write((char*)&uno, sizeof(char));
                break;
            }
        }

        //TODO--> Formato de users.txt
        //* GID, Tipo, Grupo -> 1,1,MAX10
        //* UID, Tipo, Grupo, Usuario, Contraseña -> 1,1,MAX10,MAX10
        string contenidoUsuario = "1,G,root\n";
        contenidoUsuario += "1,U,root,root,123\n";

        //* Cálculo de bloques necesarios para escribir el archivo
        float numeroBloques = (float)contenidoUsuario.length() / 64;
        int bloquesArchivo = (int)numeroBloques;
        if(numeroBloques > bloquesArchivo) bloquesArchivo += 1;

        //* Creacion de inodo archivo /users.txt
        Inode I;
        //* id user: 1 -> root
        I.i_uid = 1;
        //* id group: 1 ->root
        I.i_gid = 1;
        I.i_s = (int)contenidoUsuario.length();
        I.i_atime = NULL;
        time(&I.i_ctime);
        time(&I.i_mtime);
        for(int i = 0; i < 15; i++){
            if (i < bloquesArchivo) I.i_block[i] = sb.s_first_blo + (i)*sb.s_block_s;
            else I.i_block[i] = -1;
        }
        //* File
        I.i_type = '1';
        //* rwx rwx r--
        I.i_perm = 774;

        //* Escribiendo el inodo archivo "/users.txt"
        disco.seekp(sb.s_first_ino);
        disco.write((char*)&I, sizeof(Inode));
        sb.s_first_ino += sb.s_inode_s;
        sb.s_free_inodes_count -= 1;

        //* Escribiendo en el bitmap de inodos
        disco.seekg(sb.s_bm_inode_start);
        while(!disco.eof()){
            disco.read((char*)&auxiliar, sizeof(char));
            if(!disco.eof() && auxiliar == cero){
                //* 1 antes del cero encontrado para escribir el 1
                disco.seekp(-1, ios::cur);
                disco.write((char*)&uno, sizeof(char));
                break;
            }
        }

        //* Creación de bloques archivo para el archivo /users.txt
        FileBlock fileB;
        memset(fileB.b_content, 0, 64);
        int contenidoMaximo = 0;
        for(int i = 0; i < (int)contenidoUsuario.length(); i++){
            fileB.b_content[contenidoMaximo] = contenidoUsuario[i];
            contenidoMaximo += 1;
            if(contenidoMaximo == 64){
                //* Escribiendo el bloque carpeta para pasar al otro
                disco.seekp(sb.s_first_blo);
                disco.write((char*)&fileB, sizeof(FileBlock));
                sb.s_first_blo += sb.s_block_s;
                sb.s_free_blocks_count -= 1;
                //* Escribiendo en el bitmap de bloques
                disco.seekg(sb.s_bm_block_start);
                while(!disco.eof()){
                    disco.read((char*)&auxiliar, sizeof(char));
                    if(!disco.eof() && auxiliar == cero){
                        // 1 antes del cero encontrado para escribir el 1
                        disco.seekp(-1, ios::cur);
                        disco.write((char*)&uno, sizeof(char));
                        break;
                    }
                }
                //* Limpiando el content para volver a llenarlo
                memset(fileB.b_content, 0, 64);
                contenidoMaximo = 0;
            }
        }//* End for loop

        if (contenidoMaximo > 0){
            //* Escribiendo el ultimo bloque carpeta que no llego a ocupar los 64 bytes
            disco.seekp(sb.s_first_blo);
            disco.write((char*)&fileB, sizeof(FileBlock));
            sb.s_first_blo += sb.s_block_s;
            sb.s_free_blocks_count -= 1;
            //* Escribiendo en el bitmap de bloques
            disco.seekp(sb.s_bm_block_start);
            while(!disco.eof()){
                disco.read((char*)&auxiliar, sizeof(char));
                if(!disco.eof() && auxiliar == cero){
                    disco.seekp(-1, ios::cur);
                    disco.write((char*)&uno, sizeof(char));
                    break;
                }
            }
        }
        //* Escribiendo superbloque con la informacion actualizada
        disco.seekp(particionInicial);
        disco.write((char*)&sb, sizeof(Superblock));

        //* Actualizando y escribiendo MBR
        switch(numeroParticionPrimaria){
            case 1: {
                mb.mbr_partition_1.part_status = 'F'; break;
            }
            case 2: {
                mb.mbr_partition_2.part_status = 'F'; break;
            }
            case 3: {
                mb.mbr_partition_3.part_status = 'F'; break;
            }
            case 4: {
                mb.mbr_partition_4.part_status = 'F'; break;
            }
            default: {
                cout << "\033[0;91;49m[Error]: Se esperaba que la particion estuviera entre 1 y 4, vino: " <<
                numeroParticionPrimaria << ". \033[0m" << endl;
                disco.close();
                return false;
            }
        }
        disco.seekp(1);
        disco.write((char*)&mb, sizeof(MBR));
        disco.close();
        return true;
    }//* End if case
    disco.close();
    return false;
}


Mkfs _Mkfs(char* parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* ID
    string particionID = "";
    bool vID = false;
    //* type
    string tipoFormato = "";
    bool vTipo = false;
    //* filesystem (fs)
    string fileSystemStr = "";
    int fileSystemInt = 0;
    bool vFS = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter i
            //* Reconocimiento del caracter t
            //* Reconocimiento del caracter f
            case 1: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'i') estado = 2;
                else if ((char)tolower(parametros[i]) == 't') estado = 7;
                else if ((char)tolower(parametros[i]) == 'f') estado = 14;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> PATH
            //* Reconocimiento del caracter d
            case 2: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 3: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 4;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 4: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vID = false; particionID = ""; estado = 5;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vID = false; particionID = ""; particionID += parametros[i]; estado = 6;}
                break;
            }
            case 5: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(particionID.length() > 0){
                        vID = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un id vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else particionID += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(particionID.length() > 0) vID = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un id vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else particionID += parametros[i];
                break;
            }
            //TODO--> TYPE
            //*Reconocimiento del caracter y
            case 7: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'y') estado = 8;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //*Reconocimiento del caracter p
            case 8: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 9;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //*Reconocimiento del caracter e
            case 9: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'e') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 10: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de las comillas dobles
            case 11: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vTipo = false; tipoFormato = ""; estado = 12;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if(parametros[i] == '#' ){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vTipo = false; tipoFormato = ""; tipoFormato += parametros[i]; estado = 13;}
                break;
            }
            case 12: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if (tipoFormato.length() > 0){
                        if(strcasecmp(tipoFormato.c_str(), "full") == 0){
                            vTipo = true; tipoFormato = "full";
                        }else cout << "\033[;91;49m[Error]: El tipo de formato \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un tipo de formato vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else tipoFormato += parametros[i];
                break;
            }
            //* Reconocimiento sin las comillas dobles
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(tipoFormato.length() > 0){
                        if(strcasecmp(tipoFormato.c_str(), "full") == 0){
                            vTipo = true; tipoFormato = "full";
                        }else cout << "\033[;91;49m[Error]: El tipo de formato \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un tipo de formato vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else tipoFormato += parametros[i];
                break;
            }
            //TODO--> FILESYSTEM
            //* Reconocimiento del caracter s
            case 14: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 's') estado = 15;
                else if (parametros[i] == '#'){comentario = "";comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 15: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 16;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 16: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vFS = false; fileSystemStr = ""; estado = 17;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vFS = false; fileSystemStr = ""; fileSystemStr += parametros[i]; estado = 18;}
                break;
            }
            case 17: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(fileSystemStr.length() > 0){
                        if(strcasecmp(fileSystemStr.c_str(), "2fs") == 0){
                            vFS = true; fileSystemInt = 2;
                        }else if (strcasecmp(fileSystemStr.c_str(), "3fs") == 0){
                            vFS = true; fileSystemInt = 3;
                        }else cout << "\033[;91;49m[Error]: El sistema de archivo \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un sistema de archivo vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else fileSystemStr += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 18: {
                parametroActual += parametros[i];
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(fileSystemStr.length() > 0){
                        if (strcasecmp(fileSystemStr.c_str(), "2fs") == 0){
                            vFS = true; fileSystemInt = 2;
                        }else if (strcasecmp(fileSystemStr.c_str(), "3fs") == 0){
                            vFS = true; fileSystemInt = 3;
                        }else cout << "\033[;91;49m[Error]: El sistema de archivo \"" << parametroActual 
                        << "\" no se reconoce en el comando mkfs.\033[0m" << endl;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un sistema de archivo vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else fileSystemStr += parametros[i];
                break;
            }
            //* Error de sintaxis
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para mkfs. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }
                else{
                    parametroActual += parametros[i];
                    break;
                }
            }
            //* Comentario
            case -2: {
                comentario += parametros[i];
                break;
            }
        }//* End switch case
    }//* End for loop
    if (comentario.length() > 0) cout << "\033[38;5;246m[Comentario]: " << comentario << "\033[0m" << endl;

    Mkfs mf;
    if(vID && vTipo){
        // Establecemos por defecto el valor del file system en 2fs
        if(!vFS || fileSystemInt == 0) fileSystemInt = 2;

        mf.partitionId = particionID;
        mf.formatType = tipoFormato;
        mf.fileSystem = fileSystemInt;
        mf.acceso = true;
        return mf;
    }
    if (!vID) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">id\" para poder formatear la partición.\033[0m" << endl;
    if (!vTipo) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">type\" para poder formatear la partición.\033[0m" << endl;
    mf.acceso = false;
    return mf;
}   