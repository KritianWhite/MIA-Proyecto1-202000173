#include <iostream>
#include <cstdlib>
#include <string.h>
#include <fstream>
#include <limits>

// Encabezado
#include "Analizador.h"

// Comandos
#include "./../Comandos/Execute/Execute.cpp"
#include "./../Comandos/Mkdisk/Mkdisk.cpp"
#include "./../Comandos/Rmdisk/Rmdisk.cpp"
#include "./../Comandos/Fdisk/Fdisk.cpp"
#include "./../Comandos/Mount/Mount.cpp"
#include "./../Comandos/Unmount/Unmount.cpp"
#include "./../Comandos/Mkfs/Mkfs.cpp"

using namespace std;

PartitionNode* listMountedPartitions = NULL;

bool sesionAbierta = false;
string nombreUsuario = "", pariticionID = "";

int Analizador(char *Comando, bool esScript){

    /**
    * * Creamos unas variables de control Donde:
    * TODO --> Producción: Es el numero de proyducciones de nuestra gramatica como tal.
    * TODO --> cmmd: será la variable que tomara como tal el comando encontrado dentro de la gramatica.
    */
    int produccion = 1, cmmd = 0;
    bool incompleto = true, estado = false;

    //* Vamos a separar el comando por partes por medio del espacio
    string nuevoComando = Comando;
    char *parte;
    parte = strtok(Comando, " ");

    while (parte != NULL){
        //? Primeramente vamos a reconocer el tipo de comando?
        if (produccion == 1){
            //* Comando execute
            if (strcasecmp(parte, "execute") == 0){produccion = 2; cmmd = execute_command;}
            //* Comando Pause
            else if (strcasecmp(parte, "pause") == 0) produccion = 3;
            //* Comando mkdisk
            else if (strcasecmp(parte, "mkdisk") == 0){produccion = 2; cmmd = mkdisk_command;}
            //* Comando rmdisk
            else if (strcasecmp(parte, "rmdisk") == 0){produccion = 2; cmmd = rmdisk_command;}
            //* Comando fdisk
            else if (strcasecmp(parte, "fdisk") == 0) {produccion = 2; cmmd = fdisk_command;}
            //* Comando mount
            else if (strcasecmp(parte, "mount") == 0){produccion = 2; cmmd = mount_command;}
            //* Comando unmount
            else if (strcasecmp(parte, "unmount") == 0){produccion = 2; cmmd = unmount_command;}
            //* Comando fsdisk
            else if (strcasecmp(parte, "mkfs") == 0){produccion = 2; cmmd = fsdisk_command;}
            //* Reconocimiento de comentarios.
            else if (parte[0] == '#'){cout << "\033[38;5;246m[comentario] > " << parte << "\033[0m" << endl; produccion = 4;}

            //TODO --> Cuando los usuarios existen verificamos la particion donde se encuentra el usuario
            if (produccion == 2 || produccion == 3){
                if (produccion == 2 && sesionAbierta){
                    cout << "\033[0;1;49m(Sesion abierta " << nombreUsuario << " | Particion: " << pariticionID << ")\033[0m\n";
                }
                if(esScript){
                    cout << "--> " << nuevoComando << endl;
                }
            }else if (produccion != 4){
                cout << "\033[0;91;49m> Error: no se reconoce el comando " << parte << " \033[0m\n\n"; return 0;
            }

            //! Programando el comando pause
            if (produccion == 3) return 2;
        
            // TODO --> Empezamos a analizar cada uno de los parametros
            // TODO --> con los que puede venir cada uno de los comandos.
        }else if (produccion == 2){
            //cout << parte << " A dentro de produccion 2" <<  endl;
            try {
                switch(cmmd){
                    case execute_command:{
                        //cout << "Se reconoció " << cmmd << endl;
                        //* LLamamos a nuestro struct para pasarle la path
                        Execute e;
                        e = _Execute(parte);
                        if (e.acceso){
                            //* Buscamos el archivo para abrirlo
                            ifstream script;
                            string comandoScript;
                            script.open(e.path, ios::in);
                            if(script.fail()) cout << "\033[0;91;49m[Error]: No pudo abrirse el archivo " << e.path << "\033[0m" << endl;
                            else{
                                cout << "Iniciando ejecución del script..."<< e.path << endl;
                                int estado;
                                while (!script.eof()){
                                    comandoScript = "";
                                    getline(script, comandoScript);
                                    if(comandoScript != "\0"){
                                        estado = Analizador((char *)comandoScript.c_str(), true);
                                        if(estado == 1) break;
                                        else if (estado == 2){
                                            string enter = "";
                                            cout << "--> Presione ENTER para continuar.." << endl;
                                            getline(cin, enter);
                                        }
                                    }
                                } //* Fin de while
                                cout << "Fin de la ejecución del script " << e.path << endl;
                            }
                        }
                        incompleto = false;
                        break;
                    }
                    case mkdisk_command: {
                        Mkdisk mk;
                        mk = _Mkdisk(parte);
                        if (mk.acceso){
                            estado = CrearDisco(mk);
                            if(estado){
                                cout << "\033[0;92;49m[Correcto]: Se ha creado el disco correctamente. Nombre: "<< mk.diskName 
                                << ". De tamaño: " << mk.diskSize << ". En la dirección: \"" << mk.diskPath << "\". \033[0m" << endl;
                            }else {
                                cout << "\033[0;91;49m[Error]: Ocurrió un error en la creación del disco. \033[0m" << endl;
                            }
                        }
                        incompleto = false;
                        break;
                    }
                    case rmdisk_command: {
                        Rmdisk rm;
                        rm = _Rmdisk(parte);
                        if(rm.acceso){
                            cout << "--> ¿Desea eliminar el disco " << rm.path << " permanentemente? Si (1)/No (2): ";
                            int opcion;
                            cin >> opcion;
                            if (!cin) throw "\033[0;91;49m[Error]: Ingrese un número entero. \033[0m";
                            cin.ignore(numeric_limits<streamsize>::max(), '\n');
                            if(opcion == 1){
                                estado = EliminarDisco(rm.path);
                                if(estado) cout << "\033[0;92;49m[Correcto]: Se ha eliminado el disco \"" << rm.path << "\" correctamente. \033[0m" << endl;
                                else cout << "\033[0;91;49m[Error]: No se pudo eliminar el disco. Error interno. \033[0m" << endl;
                            }else if (opcion == 2) cout << "[Mensaje]: El disco no se será eliminado.." << endl;
                            else cout << "\033[0;91;49m[Error]: Opción ingresada incorrecta.\033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case fdisk_command: {
                        Fdisk fd;
                        fd = _Fdisk(parte);
                        //* Se verifica primeramente que exista alguna particion con el acceso
                        if (fd.acceso){
                            if(fd.deletionType != ""){
                                //* Eliminación de la partición
                                estado = EliminarParticion(fd.diskPath, fd.partitionName, "full");
                                if(estado) cout << "\033[0;92;49m[Correcto]: Se ha eliminado la particion \"" << fd.partitionName << "\" correctamente. \033[0m" << endl;
                                else cout << "\033[0;91;49m[Error]: Ocurrió algún error al tratar de eliminar la partición. \033[0m" << endl;
                            }else if (fd.resizePartition){
                                cout << "[Mensaje]: Modificando el tamaño de la partición." << endl;
                                estado = false;
                            }else{
                                //* Creación de la partición
                                estado = CrearParticion(fd.diskPath, fd.partitionName, fd.partitionSize, fd.sizeUnit, fd.partitionType, fd.partitionFit);
                                string typeParticion;
                                
                                if(fd.partitionType == 'P') typeParticion = "primaria";
                                else if(fd.partitionType == 'E') typeParticion = "extendida";
                                else typeParticion = "logica";

                                if(estado){
                                    switch(fd.sizeUnit){
                                        case 'B': {
                                            cout << "\033[0;92;49m[Correcto]: Se ha creado la partición " << typeParticion << 
                                            " \"" << fd.partitionName <<"\" de tamaño " << to_string(fd.partitionSize) << " B " << 
                                            " en "<< fd.diskPath <<" correctamente. \033[0m" << endl;
                                            break;
                                        }
                                        case 'K': {
                                            cout << "\033[0;92;49m[Correcto]: Se ha creado la partición " << typeParticion << 
                                            " \"" << fd.partitionName <<"\" de tamaño " << to_string(fd.partitionSize) << " KB " << 
                                            " en "<< fd.diskPath <<" correctamente. \033[0m" << endl;
                                            break;
                                        }
                                        case 'M': {
                                            cout << "\033[0;92;49m[Correcto]: Se ha creado la partición " << typeParticion << 
                                            " \"" << fd.partitionName <<"\" de tamaño " << to_string(fd.partitionSize) << " MB " << 
                                            " en "<< fd.diskPath <<" correctamente. \033[0m" << endl;
                                            break;
                                        }
                                    }
                                }else {
                                    switch(fd.sizeUnit){
                                        case 'B': {
                                            cout << "\033[0;91;49m[Error]: Ocurrió un error durante la creación de la partición " << 
                                            typeParticion << " \"" << fd.partitionName <<"\" de tamaño " << to_string(fd.partitionSize) << 
                                            " B " << " en "<< fd.diskPath <<". \033[0m" << endl;
                                            break;
                                        }
                                        case 'K': {
                                            cout << "\033[0;91;49m[Error]: Ocurrió un error durante la creación de la partición " << 
                                            typeParticion << " \"" << fd.partitionName <<"\" de tamaño " << to_string(fd.partitionSize) << 
                                            " KB " << " en "<< fd.diskPath <<". \033[0m" << endl;
                                            break;
                                        }
                                        case 'M': {
                                            cout << "\033[0;91;49m[Error]: Ocurrió un error durante la creación de la partición " << 
                                            typeParticion << " \"" << fd.partitionName <<"\" de tamaño " << to_string(fd.partitionSize) << 
                                            " MB " << " en "<< fd.diskPath <<". \033[0m" << endl;
                                            break;
                                        }
                                    }
                                }
                            }
                        }//* End if acceso
                        incompleto = false;
                        break;
                    }
                    case mount_command: {
                        Mount mnt;
                        mnt = _Mount(parte);
                        if(mnt.acceso){
                            if(mnt.diskPath != "" && mnt.diskName != "" && mnt.partitionName != ""){
                                estado = ParticionMount(mnt, listMountedPartitions);
                                if(estado) cout << "\033[0;92;49m[Correcto]: Se ha montado la particion " << mnt.partitionName << 
                                " del disco " << mnt.diskPath << mnt.diskName << " correctamente. \033[0m" << endl;
                                else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar montar la particion " << mnt.partitionName << 
                                " del disco " << mnt.diskPath << mnt.diskName << ". Error interno, \033[0m" << endl;
                            }else showMountedPartitions(listMountedPartitions);
                        }
                        incompleto = false;
                        break;
                    }
                    case unmount_command: {
                        Unmount umnt;
                        umnt = _Unmount(parte);
                        if(umnt.acceso){
                            estado = ParticionUnmount(umnt.partitionId, listMountedPartitions);
                            if(estado) cout << "\033[0;92;49m[Correcto]: Se ha desmontado la particion " 
                            << umnt.partitionId << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar desmontar la particion " 
                            << umnt.partitionId << ". Error interno, \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case fsdisk_command: {
                        Mkfs mf;
                        mf = _Mkfs(parte);
                    }
                }
            }
            catch (const exception& e)
            {
                incompleto = false;
                cout << e.what() << endl;
                cout << "\033[0;91;49m[Error]: Ha ocurrido un error en el sistema al intentar analizar los parametros.\033[0m" << endl;
            }catch(const char *err){
                cout << "\033[0;91;49m[Error]: Algo ocurrió: [" <<  err << "]. \033[0m" << endl;
            }catch(...){
                incompleto = false;
                cout << "\033[0;91;49m[Error]: Ha ocurrido un error en el sistema al intentar analizar los parametros. \033[0m" << endl;
            }
        }
        //* Comentarios
        else if (estado == 4){
            if(esScript) cout << "\033[38;5;246m" << parte << "\033[0m\n";
            else cout << "\033[38;5;246m" << parte << "\033[0m\n\n";
        }
        
        parte = strtok(NULL, "");
    } //* End while
    return 0;
}