#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include <ctime>

struct Partition{
    char part_status; // Indica si la partición está activa o no, E (EMPTY default), A (ACTIVE), F (FORMATTED)
    char part_type; // P | E (L no, se usa el EBR para lógicas)
    char part_fit; // B | F | W
    int part_start; // byte en donde inicia la partición
    int part_s; // tamaño total de la partición en bytes
    char part_name[16]; // nombre de la partición
};

struct MBR{
    int mbr_tamano; // tamaño total del disco en bytes
    time_t mbr_fecha_creacion;
    int mbr_dsk_signature; // id único del disco
    char dsk_fit; // B | F | W
    struct Partition mbr_partition_1;
    struct Partition mbr_partition_2;
    struct Partition mbr_partition_3;
    struct Partition mbr_partition_4;
    /* MANEJO DE FECHA */
    // time(&mbr_fecha_creacion) // Obtener la hora actual
    // printf("Today is %s", ctime(&mbr_fecha_creacion));
};

struct EBR{
    // part_status: E (EMPTY: Es el primer EBR y no hay lógicas), A (ACTIVE)
    char part_status;
    char part_fit; // F | B | W
    int part_start; // byte del disco donde inicia la partición lógica
    int part_s; // tamaño total de la partición lógica en bytes
    int part_next; // byte en el que está el próximo EBR, -1 si no hay siguiente (default)
    char part_name[16]; // nombre de la partición lógica
};

/*
    TODO--> ¿JOURNALING?

    * Filesystem *
    * | SUPERBLOCK | INODE BITMAP | BLOCK BITMAP | INODES | BLOCKS |
    * | SUPERBLOCK | JOURNALING -> EXT3 | INODE BITMAP | BLOCK BITMAP | INODES | BLOCKS |


    * Formula for inodes and blocks **
    * (ADD EBR FOR LOGICAL PARTITIONS)
    * partition_size = Superblock + InodeBitmap + BlockBitmap + Inodes + Blocks
    * partition.part_s = sizeof(Superblock) + n + 3n + n*sizeof(Inode) + 3n*sizeof(Block)
    * partition.part_s - sizeof(Superblock) = 4n + n*(sizeof(Inode) + 3*sizeof(Block))
    * n*(4 + sizeof(Inode) + 3*sizeof(Block)) = partition.part_s - sizeof(Superblock)

    * numerator = partition.part_s - sizeof(Superblock)   [- sizeof(EBR) for logical partitions]
    * denominator = 4 + sizeof(Inode) + 3*sizeof(Block)
    * n = numerator / denominator

    * -> Available inodes: n
    * -> Available blocks: 3n

*/

// TODO MOUNT Y UNMOUNT EN SUPERBLOQUE
struct Superblock{
    int s_filesystem_type; // 2 | 3 (identifica el sistema de archivos utilizado)
    int s_inodes_count; // total de inodos
    int s_blocks_count; // total de bloques
    int s_free_blocks_count;
    int s_free_inodes_count;
    time_t s_mtime; // última fecha en el que el sistema fue montado
    time_t s_umtime; // última fecha en que el sistema fue desmontado
    int s_mnt_count; // cantidad de veces que el sistema se ha montado
    int s_magic; // identifica al sistema de archivos, 0xEF53
    int s_inode_s; // tamaño del inodo
    int s_block_s; // tamaño del bloque
    int s_first_ino; // posición del primer inodo libre
    int s_first_blo; // posición del primer bloque libre
    int s_bm_inode_start; // posición de inicio del bitmap de inodos
    int s_bm_block_start; // posición de inicio del bitmap de bloques
    int s_inode_start; // posición de inicio de la tabla de inodos
    int s_block_start; // posición de inicio de la tabla de bloques
};

struct Inode{
    int i_uid; // UID del propietario del archivo o carpeta
    int i_gid; // GID del grupo al que pertence el archivo o carpeta
    int i_s; // tamaño del archivo en bytes, SOLUCIÓN PROPIA: -1 si es carpeta
    time_t i_atime; // última fecha en que se leyó el inodo sin modificarlo
    time_t i_ctime; // fecha en la que se creó el inodo
    time_t i_mtime; // última fecha en la que se modifica el inodo
    int i_block[15]; // -1 si no son utilizados
    /*
        * Primeros 12: bloques directos
        * 13: bloque simple indirecto (16 bloques directos más, cada pointerBlock apunta a directos)
        * 14: bloque doble indirecto (16² bloques directos más, cada pointertBlock apunta a otro pointerBlock, y estos apuntan a directos)
        * 15: bloque triple indirecto (16³ bloques directos más, cada pointerBlock apunta a otro pointerBlock, estos a su vez apuntan a otros pointerBlock, y estos últimos apuntan a directos)
        * bloques directos posibles por inodo -> 12 + 16^1 + 16^2 + 16^3 -> 12 + 16 + 256 + 4096 -> 4380 bloques */
    char i_type; // 0: carpeta, 1: archivo
    int i_perm; // permisos del archivo o carpeta (conjuntos de 3 bits: RWX)
    /* 
        * Primeros 3 bits: permisos de usuario i_uid
        * Siguientes 3 bits: permisos del grupo al que pertenece el usuario propietario
        * Últimos 3 bits: permisos de otros usuarios */
};

//* BLOQUES DIRECTOS

//* AUX STRUCT: se encontrarán 4 en cada FolderBlock
struct content{
    char b_name[12]; // nombre de carpeta o archivo
    int b_inodo; // apuntador a un inodo carpeta o archivo, -1 por defecto
};

//* asociados a un Inode con i_type = 0
struct FolderBlock{
    struct content b_content[4];
};

//* asociados a un Inode con i_type = 1, guarda el contenido de un archivo
struct FileBlock{
    char b_content[64];
    //* capaciddad máxima para un archivo -> 4380 FileBlock ḿáximo en el inodo * 64 -> 280320 bytes */
};

//* BLOQUES INDIRECTOS */

struct pointerBlock{
    int b_pointers[16];
    //* Array con pointers a bloques (folderBlock | fileBlock | pointerBlock (para dobles y triples))
};

#endif // ESTRUCTURAS_H
