#include <iostream>
#include <string.h> // memset()...
#include <cmath>    // floor() para n inodos
#include <fstream>

#include "Mkfs.h"
#include "../Estructura.h"
//#include "../../aux_funciones.h"

using namespace std;


int Calc_NumeroInodos(int partitionSize, int fileSystem, bool isLogicalPartition){
    /*
    FORMULA
    partition_size = Superblock + InodeBitmap + BlockBitmap + Inodes + Blocks
    * partition.part_s = sizeof(Superblock) + n + 3n + n*sizeof(Inode) + 3n*sizeof(Block)
    partition.part_s - sizeof(Superblock) = 4n + n*(sizeof(Inode) + 3*sizeof(Block))
    n*(4 + sizeof(Inode) + 3*sizeof(Block)) = partition.part_s - sizeof(Superblock)

    numerator = partition.part_s - sizeof(Superblock)   [- sizeof(EBR) for logical partitions]
    denominator = 4 + sizeof(Inode) + 3*sizeof(Block)
    n = numerator / denominator

    -> Available inodes: n
    -> Available blocks: 3n
    */
    float numerator, denominator, result;
    int inodes = 0;
    if(!isLogicalPartition){
        /* EXT2 */
        numerator = partitionSize - sizeof(Superblock);
        denominator = 4*sizeof(char) + sizeof(Inode) + 3*sizeof(FileBlock); // 4 char para bitmaps, 1 inodo, 3 bloques
        result = numerator / denominator;
        int inodes = floor(result);
        return inodes;
        // TODO EXT3
    }
    // TODO INODES NUMBER LOGICAL PARTITION

    return inodes;
}

bool FormatoParticion(Mkfs mf, PartitionNode *&firstNode){
    /* Verificando que el id represente una partición montada */
    if (!isIdInList(firstNode, mf.partitionId)){
        cout << "\033[0;91;49m[Error]: La particion con id " << mf.partitionId <<
        " no fue encontrada en la lista de particiones montadas (use mount para ver esta lista) \033[0m" << endl;
        return false;
    }

    cout << "--> Obteniendo datos del disco..." << endl;
    fstream disk;
    string diskPath = getDiskPath(firstNode, mf.partitionId);
    disk.open(diskPath, ios::in|ios::out|ios::binary);
    if(disk.fail()){
        cout << "\033[0;91;49m[Error]: No se ha podido abrir el disco asociado a la partición " << mf.partitionId <<
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

    string partitionName = getPartitionName(firstNode, mf.partitionId);
    int numberPrimaryPartition = 0, partitionStart = 0, partitionSize = 0;
    char partStatus = '0';
    int numberExtendedPartition = 0;

    // while para poder finalizar las validaciones cuando se encuentre la partición
    // solo se ejecuta una vez
    while(true){
        if(mbrDisk.mbr_partition_1.part_status != 'E'){
            if(mbrDisk.mbr_partition_1.part_name == partitionName){
                if(mbrDisk.mbr_partition_1.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 1;
                partitionStart = mbrDisk.mbr_partition_1.part_start;
                partitionSize = mbrDisk.mbr_partition_1.part_s;
                partStatus = mbrDisk.mbr_partition_1.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_1.part_type == 'E') numberExtendedPartition = 1;
        }
        if(mbrDisk.mbr_partition_2.part_status != 'E'){
            if(mbrDisk.mbr_partition_2.part_name == partitionName){
                if(mbrDisk.mbr_partition_2.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 2;
                partitionStart = mbrDisk.mbr_partition_2.part_start;
                partitionSize = mbrDisk.mbr_partition_2.part_s;
                partStatus = mbrDisk.mbr_partition_2.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_2.part_type == 'E') numberExtendedPartition = 2;
        }
        if(mbrDisk.mbr_partition_3.part_status != 'E'){
            if(mbrDisk.mbr_partition_3.part_name == partitionName){
                if(mbrDisk.mbr_partition_3.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 3;
                partitionStart = mbrDisk.mbr_partition_3.part_start;
                partitionSize = mbrDisk.mbr_partition_3.part_s;
                partStatus = mbrDisk.mbr_partition_3.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_3.part_type == 'E') numberExtendedPartition = 3;
        }
        if(mbrDisk.mbr_partition_4.part_status != 'E'){
            if(mbrDisk.mbr_partition_4.part_name == partitionName){
                if(mbrDisk.mbr_partition_4.part_type == 'E'){
                    cout << "\033[0;91;49m[Error]: La partición montada coincide con una extendida, solo pueden montarse primarias o extendidas. \033[0m" << endl;
                    disk.close();
                    return false;
                }
                numberPrimaryPartition = 4;
                partitionStart = mbrDisk.mbr_partition_4.part_start;
                partitionSize = mbrDisk.mbr_partition_4.part_s;
                partStatus = mbrDisk.mbr_partition_4.part_status;
                break;
            }
            if(mbrDisk.mbr_partition_4.part_type == 'E') numberExtendedPartition = 4;
        }
        break;
    }

    /* MKFS PRIMARY PARTITION */
    if(numberPrimaryPartition != 0){
        if(partStatus == 'F'){
            cout << "--> La particion " << partitionName << " ya cuenta con un sistema de archivos, ¿desea formatearla? (sus datos se perderan)" <<
                " Preione 1(si)/2(No): ";
            int opcion;
            cin >> opcion; if(!cin) throw "\033[0;91;49m[Error]: ingrese un numero entero. \033[0m\n";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');

            if(opcion == 2){
                cout << "[Mensaje]: La partición no será formateando." << endl;
                disk.close();
                return false;
            }
            else if(opcion != 1){
                cout << "\033[0;91;49m[Error]: Ingrese un numero entero valido. \033[0m" << endl;
                disk.close();
                return false;
            }
        }

        cout << "--> Realizando formateo " << mf.formatType << " en la partición " << mf.partitionId << "..." << endl;
        disk.seekp(partitionStart);
        for(int i = 0; i < partitionSize; i++)
            {disk.write((char*)&cero, sizeof(char));}

        /* EXT2 */
        cout << "--> Configurando superbloque..." << endl;
        int inodes = Calc_NumeroInodos(partitionSize, mf.fileSystem, false);
        Superblock sb;
        sb.s_filesystem_type = mf.fileSystem;
        sb.s_inodes_count = inodes;
        sb.s_blocks_count = 3*inodes;
        sb.s_free_blocks_count = 3*inodes; // cambiarán conforme se agreguen bloques
        sb.s_free_inodes_count = inodes; // cambiarán conforme se agreguen inodos
        time(&sb.s_mtime);
        sb.s_umtime = NULL;
        sb.s_mnt_count = 1;
        sb.s_magic = 61267; // 0xEF53
        sb.s_inode_s = sizeof(Inode);
        sb.s_block_s = sizeof(FolderBlock);
        sb.s_first_ino = partitionStart + sizeof(Superblock) + inodes + 3*inodes; // cambiarán conforme se agreguen bloques
        sb.s_first_blo = partitionStart + sizeof(Superblock) + inodes + 3*inodes + inodes*sizeof(Inode);  // cambiarán conforme se agreguen bloques
        sb.s_bm_inode_start = partitionStart + sizeof(Superblock);
        sb.s_bm_block_start = partitionStart + sizeof(Superblock) + inodes;
        sb.s_inode_start = partitionStart + sizeof(Superblock) + inodes + 3*inodes;
        sb.s_block_start = partitionStart + sizeof(Superblock) + inodes + 3*inodes + inodes*sizeof(Inode);


        cout << "--> Creando directorio root (/)..." << endl;
        /* CREACIÓN DE INODO CARPETA "/" */
        Inode root;
        root.i_uid = 1; // id user: 1->root
        root.i_gid = 1; // id group: 1->root
        root.i_s = -1; // folder
        root.i_atime  = NULL;
        time(&root.i_ctime);
        time(&root.i_mtime);
        root.i_block[0] = sb.s_first_blo; // BLOQUE ARCHIVOS PARA users.txt
        for(int i = 1; i < 15; i++)
            {root.i_block[i] = -1;}
        root.i_type = '0'; // folder
        root.i_perm = 775; //rwx rwx r-x

        /* Escribiendo el inodo carpeta "/" */
        disk.seekp(sb.s_first_ino);
        disk.write((char*)&root, sizeof(Inode));
        sb.s_first_ino += sb.s_inode_s;
        sb.s_free_inodes_count -= 1;
        /* Escribiendo en el bitmap de inodos */
        disk.seekg(sb.s_bm_inode_start);
        while(!disk.eof()){
            disk.read((char *)&aux, sizeof(char));
            if(!disk.eof() && aux == cero){
                disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                disk.write((char*)&uno, sizeof(char));
                break;
            }
        }

        cout << "--> Creando archivo /users.txt..." << endl;
        /* CREACIÓN DE BLOQUE CARPETA PARA GUARDAR EL INODO ARCHIVO QUE VIENE */
        FolderBlock folderB;
        string users = "users.txt";
        // folderB.b_content[0] -> .
        memset(folderB.b_content[0].b_name, 0, 12);
        folderB.b_content[0].b_name[0] = '.';
        folderB.b_content[0].b_inodo = sb.s_inode_start;
        // folderB.b_content[1] -> ..
        memset(folderB.b_content[1].b_name, 0, 12);
        folderB.b_content[1].b_name[0] = '.';
        folderB.b_content[1].b_name[1] = '.';
        folderB.b_content[1].b_inodo = sb.s_inode_start;
        // folderB.b_content[2] -> users.txt
        memset(folderB.b_content[2].b_name, 0, 12);
        for(int j = 0; j < (int)users.length() && j < 12; j++)
            {folderB.b_content[2].b_name[j] = users[j];}
        folderB.b_content[2].b_inodo = sb.s_first_ino; // ya se tomó en cuenta el inodo raíz
        // folderB.b_content[3] -> vacío
        memset(folderB.b_content[3].b_name, 0, 12);
        folderB.b_content[3].b_inodo = -1;

        /* Escribiendo el bloque carpeta */
        disk.seekp(sb.s_first_blo);
        disk.write((char*)&folderB, sizeof(FolderBlock));
        sb.s_first_blo += sb.s_block_s;
        sb.s_free_blocks_count -= 1;
        /* Escribiendo en el bitmap de bloques */
        disk.seekg(sb.s_bm_block_start);
        while(!disk.eof()){
            disk.read((char *)&aux, sizeof(char));
            if(!disk.eof() && aux == cero){
                disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                disk.write((char*)&uno, sizeof(char));
                break;
            }
        }

        /* FORMATO DE users.txt:
        * GID, Tipo, Grupo -> 1,1,MAX10\n
        * UID, Tipo, Grupo, Usuario, Contraseña -> 1,1,MAX10,MAX10\n
        */
        string usersContent = "1,G,root\n";
        usersContent += "1,U,root,root,123\n";

        // Cálculo de bloques necesarios para escribir el archivo
        float blocksNum = (float)usersContent.length() / 64;
        int fileBlocks = (int)blocksNum;
        if(blocksNum > fileBlocks)
            fileBlocks += 1;

        /* CREACIÓN DE INODO ARCHIVO /users.txt */
        Inode usersInode;
        usersInode.i_uid = 1; // id user: 1->root
        usersInode.i_gid = 1; // id group: 1->root
        usersInode.i_s = (int)usersContent.length();
        usersInode.i_atime  = NULL;
        time(&usersInode.i_ctime);
        time(&usersInode.i_mtime);
        for(int i = 0; i < 15; i++){
            if(i < fileBlocks)
                usersInode.i_block[i] = sb.s_first_blo + (i)*sb.s_block_s;
            else
                usersInode.i_block[i] = -1;
        }
        usersInode.i_type = '1'; // file
        usersInode.i_perm = 774; //rwx rwx r--

        /* Escribiendo el inodo archivo "/users.txt" */
        disk.seekp(sb.s_first_ino);
        disk.write((char*)&usersInode, sizeof(Inode));
        sb.s_first_ino += sb.s_inode_s;
        sb.s_free_inodes_count -= 1;
        /* Escribiendo en el bitmap de inodos */
        disk.seekg(sb.s_bm_inode_start);
        while(!disk.eof()){
            disk.read((char *)&aux, sizeof(char));
            if(!disk.eof() && aux == cero){
                disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                disk.write((char*)&uno, sizeof(char));
                break;
            }
        }

        /* CREACIÓN DE BLOQUES ARCHIVO PARA EL ARCHIVO /users.txt */
        FileBlock fileB;
        memset(fileB.b_content, 0, 64);
        int maxContent = 0;
        for(int i = 0; i < (int)usersContent.length(); i++){
            fileB.b_content[maxContent] = usersContent[i];
            maxContent += 1;
            if(maxContent == 64){
                /* Escribiendo el bloque carpeta para pasar al otro */
                disk.seekp(sb.s_first_blo);
                disk.write((char*)&fileB, sizeof(FileBlock));
                sb.s_first_blo += sb.s_block_s;
                sb.s_free_blocks_count -= 1;
                /* Escribiendo en el bitmap de bloques */
                disk.seekg(sb.s_bm_block_start);
                while(!disk.eof()){
                    disk.read((char *)&aux, sizeof(char));
                    if(!disk.eof() && aux == cero){
                        disk.seekp(-1, ios::cur); // 1 antes del cero encontrado para escribir el 1
                        disk.write((char*)&uno, sizeof(char));
                        break;
                    }
                }
                memset(fileB.b_content, 0, 64); // limpiando el content para volver a llenarlo
                maxContent = 0;
            }
        }
        if(maxContent > 0){
            /* Escribiendo el úlitmo bloque carpeta que no llegó a ocupar los 64 bytes */
            disk.seekp(sb.s_first_blo);
            disk.write((char*)&fileB, sizeof(FileBlock));
            sb.s_first_blo += sb.s_block_s;
            sb.s_free_blocks_count -= 1;
            /* Escribiendo en el bitmap de bloques */
            disk.seekg(sb.s_bm_block_start);
            while(!disk.eof()){
                disk.read((char *)&aux, sizeof(char));
                if(!disk.eof() && aux == cero){
                    disk.seekp(-1, ios::cur);
                    disk.write((char*)&uno, sizeof(char));
                    break;
                }
            }
        }

        /* Escribiendo superbloque con la información actualizada */
        disk.seekp(partitionStart);
        disk.write((char*)&sb, sizeof(Superblock));

        /* Actualizando y escribiendo MBR */
        switch(numberPrimaryPartition){
            case 1: mbrDisk.mbr_partition_1.part_status = 'F'; break;
            case 2: mbrDisk.mbr_partition_2.part_status = 'F'; break;
            case 3: mbrDisk.mbr_partition_3.part_status = 'F'; break;
            case 4: mbrDisk.mbr_partition_4.part_status = 'F'; break;
            default: cout << "\033[0;91;49m[Error]:  se esperaba que la partición fuera 1-4, vino: "
                     << numberPrimaryPartition << ".\033[0m" << endl;
                     disk.close();
                     return false;
        }
        disk.seekp(1);
        disk.write((char*)&mbrDisk, sizeof(MBR));

        disk.close();
        return true;
    }
    /* TODO else: MKFS LOGICAL PARITION */

    disk.close();
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
            //TODO--> Id
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