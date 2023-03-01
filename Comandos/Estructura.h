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

#endif // ESTRUCTURAS_H
