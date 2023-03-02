#ifndef MOUNT_H
#define MOUNT_H

#include <iostream>

using namespace std;

string lastDigits = "73"; //* 202000173

struct Mount{
    bool acceso;
    string diskPath;
    string diskName;
    string partitionName;
};

struct MountedPartition{
    string partitionId;
    string partitionName; //* para obtener el vínculo entre id y partición en MBR
    string diskPath; //* para acceder al disco en donde se encuentra la partición
};

struct PartitionNode{
    MountedPartition mountedP;
    PartitionNode* next;
};


/* función auxiliar interna, devuelve el id de partición que corresponde,
   para que sea id único, independiente del orden en que estas se encuentren
   distribuidas en el mbr (partitionNumber incremental, en base al nombre del disco)*/
string getNewId(PartitionNode *first, string diskName){
    PartitionNode* actual = NULL;
    string partitionId = "";
    int counter = 1;
    bool isUniqueId;

    /* counter <= 15 : 3 primary + 12 logical (see fdisk, creation of logical partitions) */
    while(counter <= 15){
        // partitionId : lastDigits + partitionNumber + diskName
        isUniqueId = true;
        partitionId = lastDigits;
        partitionId += to_string(counter);
        partitionId += diskName;

        actual = first;
        while(actual != NULL){
            if(actual->mountedP.partitionId == partitionId){
                isUniqueId = false;
                break;
            }
            actual = actual->next;
        }
        if(isUniqueId)
            return partitionId;
        counter += 1;
    }

    return "";
}

/*
    Pasando por referencia el puntero (&) y accediendo a su valor
    Pasando el valor del puntero a el objeto first por referencia
*/
bool insertMountedPartition(PartitionNode *&first, string partitionName, string diskNameOnly, string diskPath){
    string newId = getNewId(first, diskNameOnly);

    if(newId == "")
        return false;

    PartitionNode *newPartitionNode = new PartitionNode();
    newPartitionNode->mountedP.partitionId = newId;
    newPartitionNode->mountedP.partitionName = partitionName;
    newPartitionNode->mountedP.diskPath = diskPath;
    newPartitionNode->next = NULL;

    if(first == NULL)
        first = newPartitionNode;
    else{
        PartitionNode* actualNode = first;
        while(actualNode->next != NULL)
            actualNode = actualNode->next;
        actualNode->next = newPartitionNode;
    }

    return true;
}

/* para comprobar que una partición esté montada, sin saber su id
   (ocurre cuando se quiere montar una partición que ya ha sido montada)*/
bool isPartitionMounted(PartitionNode* first, string partitionName, string diskPath){
    PartitionNode *actual = first;

    while(actual != NULL){
        if((actual->mountedP.partitionName == partitionName) && (actual->mountedP.diskPath == diskPath))
            return true;
        actual = actual->next;
    }

    return false;
}

/* para acceder al id de la partición, una vez comprobado que está montada
   (se usa para reportar el error de montar una partićión ya montada)*/
string getPartitionId(PartitionNode* first, string partitionName){
    PartitionNode *actual = first;

    while(actual != NULL){
        if(actual->mountedP.partitionName == partitionName)
            return actual->mountedP.partitionId;
        actual = actual->next;
    }

    return "";
}

/* para comprobar que una partición esté montada, cuando se proporciona su id */
bool isIdInList(PartitionNode* first, string partitionId){
    PartitionNode *actual = first;

    while(actual != NULL){
        if(actual->mountedP.partitionId == partitionId)
            return true;
        actual = actual->next;
    }

    return false;
}

/* para acceder al disco, cuando se proporciona el id de una partición montada */
string getDiskPath(PartitionNode* first, string partitionId){
    PartitionNode *actual = first;

    while(actual != NULL){
        if(actual->mountedP.partitionId == partitionId)
            return actual->mountedP.diskPath;
        actual = actual->next;
    }

    return "";
}

/* para acceder a la partición, cuando se proporciona el id (luego de getDiskPath) */
string getPartitionName(PartitionNode* first, string partitionId){
    PartitionNode *actual = first;

    while(actual != NULL){
        if(actual->mountedP.partitionId == partitionId)
            return actual->mountedP.partitionName;
        actual = actual->next;
    }

    return "";
}

/* para desmontar una partición, una vez se verificó que esta existe en la lista */
bool deletePartitionFromList(PartitionNode *&first, string partitionId){
    PartitionNode *actual = first;
    PartitionNode *previous = NULL;

    while(actual != NULL){
        if(actual->mountedP.partitionId == partitionId){
            if(previous == NULL)
                {first = actual->next;}
            else
                {previous->next = actual->next;}

            delete actual;
            return true;
        }
        previous = actual;
        actual = actual->next;
    }

    return false;
}

/* función auxiliar interna, para mostrar las particiones montadas */
void showMountedPartitions(PartitionNode *first){
    cout<<"> Mostrando lista de particiones montadas en memoria:\n";

    PartitionNode *actual = first;
    int contador = 0;
    while(actual != NULL){
        cout << "[ Id: " << actual->mountedP.partitionId  << " | ";
        cout << "Partition name: " << actual->mountedP.partitionName  << " | ";
        cout << "Disk path: " << actual->mountedP.diskPath  << " ]\n";

        actual = actual->next;
        contador++;
    }
    cout<<"> Total particiones montadas: " << contador << "\n\n";
}



#endif // !MOUNT_H
