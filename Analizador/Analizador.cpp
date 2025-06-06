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
#include "./../Comandos/Login/Login.cpp"
#include "./../Comandos/Logout/Logout.cpp"
#include "./../Comandos/Mkgrp/Mkgrp.cpp"
#include "./../Comandos/Rmgrp/Rmgrp.cpp"
#include "./../Comandos/Mkusr/Mkusr.cpp"
#include "./../Comandos/Rmusr/Rmusr.cpp"
#include "./../Comandos/Mkfile/Mkfile.cpp"
#include "./../Comandos/Cat/Cat.cpp"
#include "./../Comandos/Edit/Edit.cpp"
#include "./../Comandos/Rename/Rename.cpp"
#include "./../Comandos/Mkdir/Mkdir.cpp"
#include "./../Comandos/Find/Find.cpp"
#include "./../Comandos/Chgrp/Chgrp.cpp"

#include "./../Comandos/Rep/Rep.cpp"

//* Reportes
//#include "./../Comandos/Rep/Reportes/MbrReport.cpp"


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
            else if (strcasecmp(parte, "mkfs") == 0){produccion = 2; cmmd = mkfs_command;}
            //* Comando login
            else if (strcasecmp(parte, "login") == 0){produccion = 2; cmmd = login_command;}
            //* Comando logout
            else if (strcasecmp(parte, "logout") == 0){produccion = 2; cmmd = logout_command;}
            //* Comando mkgrp
            else if (strcasecmp(parte, "mkgrp") == 0){produccion = 2; cmmd = mkgrp_command;}
            //* Comando rmgrp
            else if (strcasecmp(parte, "rmgrp") == 0){produccion = 2; cmmd = rmgrp_command;}
            //* Comando mkusr
            else if (strcasecmp(parte, "mkusr") == 0){produccion = 2; cmmd = mkusr_command;}
            //* Comando rmusr
            else if (strcasecmp(parte, "rmusr") == 0){produccion = 2; cmmd = rmusr_command;}
            //* Comando mkfile
            else if (strcasecmp(parte, "mkfile") == 0){produccion = 2; cmmd = mkfile_command;}
            //* Comando cat
            else if (strcasecmp(parte, "cat") == 0){produccion = 2; cmmd = cat_command;}
            //* Comando edit
            else if (strcasecmp(parte, "edit") == 0){produccion = 2; cmmd = edit_command;}
            //* Comando rename
            else if (strcasecmp(parte, "rename") == 0){produccion = 2; cmmd = rename_command;}
            //* Comando mkdir
            else if (strcasecmp(parte, "mkdir") == 0){produccion = 2; cmmd = mkdir_command;}
            //* Comando Find
            else if (strcasecmp(parte, "find") == 0){produccion = 2; cmmd = find_command;}
            //* Comando Chgrp
            else if (strcasecmp(parte, "chgrp") == 0){produccion = 2; cmmd = chgrp_command;}
            //* Comando rep
            else if (strcasecmp(parte, "rep") == 0){produccion = 2; cmmd = rep_command;}
            //* Reconocimiento de comentarios.
            else if (parte[0] == '#'){cout << "\033[38;5;246m[comentario]: " << parte << "\033[0m" << endl; produccion = 4;}

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
                    case mkfs_command: {
                        Mkfs mf;
                        mf = _Mkfs(parte);
                        if(mf.acceso){
                            estado = FormatoParticion(mf, listMountedPartitions);
                            if(estado) cout << "\033[0;92;49m[Correcto]: Se ha formateado la particion " << mf.partitionId
                            << " con el sistema de archivo ext" << mf.fileSystem << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar formatear la particion " <<
                            mf.partitionId << " con el sistema de archivos ext" << mf.fileSystem << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case login_command: {
                        Login lg;
                        lg = _Login(parte);
                        if(lg.acceso){
                            estado = LoginUser(sesionAbierta, lg.partitionId, lg.username, lg.password, listMountedPartitions);
                            if(estado){
                                cout << "\033[0;92;49m[Correcto]: Inicio de sesion exitoso. Bienvenido " << lg.username << ".  \033[0m" << endl;
                                sesionAbierta = true;
                                nombreUsuario = lg.username;
                                pariticionID = lg.partitionId;
                            }else cout << "\033[0;91;49m[Error]: Inicio de sesion fallido como " << lg.username << ".  \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case logout_command: {
                        if(LogoutUser(sesionAbierta)){
                            sesionAbierta = false;
                            nombreUsuario = "";
                            pariticionID = "";
                        }
                        incompleto = false;
                        break;
                    }
                    case mkgrp_command: {
                        Mkgrp mg;
                        mg = _Mkgrp(parte);
                        if(mg.acceso){
                            estado = CrearGrupo(sesionAbierta, nombreUsuario, pariticionID, mg.groupName, listMountedPartitions);
                            if(estado) cout << "\033[0;92;49m[Correcto]: Se ha creado el grupo " << mg.groupName
                            << " en la particion " << pariticionID << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar crear el grupo " << mg.groupName << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case rmgrp_command: {
                        Rmgrp rg;
                        rg = _Rmgrp(parte);
                        if(rg.acesso){
                            estado = EliminarGrupo(sesionAbierta, nombreUsuario, pariticionID, rg.groupname, listMountedPartitions);
                            if(estado) cout << "\033[0;92;49m[Correcto]: Se ha eliminado el grupo " << rg.groupname
                            << " en la particion " << pariticionID << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar eliminar el grupo " << rg.groupname << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case mkusr_command: {
                        Mkusr mu;
                        mu = _Mkusr(parte);
                        if(mu.acceso){
                            estado = createUser(sesionAbierta, nombreUsuario, mu.username, mu.password, mu.groupname, pariticionID, listMountedPartitions);
                            if(estado) cout << "\033[0;92;49m[Correcto]: Se ha creado el usuario " << mu.username << " del grupo " 
                            << mu.groupname << " en la particion " << pariticionID << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar crear el usuario " << mu.username << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case rmusr_command: {
                        Rmusr ru;
                        ru = _Rmusr(parte);
                        if(ru.acceso){
                            estado = EliminarUsuario(sesionAbierta, nombreUsuario, pariticionID, ru.username, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha eliminado el usuario " << ru.username 
                            << " en la particion " << pariticionID << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar eliminar el usuario " << ru.username << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case mkfile_command: {
                        Mkfile mf;
                        mf = _Mkfile(parte);
                        if(mf.acceso){
                            estado = CrearArchivo(sesionAbierta, nombreUsuario, mf, pariticionID, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha creado el archivo con ruta " << mf.filePath
                            << " en la particion " << pariticionID << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar crear el archivo en la ruta " << 
                            mf.filePath << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case cat_command: {
                        Cat cat;
                        cat = _Cat(parte);
                        if(cat.acceso){
                            estado = MostrarContenido_Archivo(sesionAbierta, pariticionID, cat.filePathList, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha mostrado el contenido de " << cat.filePathList.size()
                            << " archivos correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar mostrar los archivos. \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case edit_command: {
                        Edit e;
                        e = _Edit(parte);
                        if(e.acceso){
                            estado = EditarArchivo(e.filePath, e.contentPath, sesionAbierta, nombreUsuario, pariticionID, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha editado el contenido del archivo " << e.filePath
                            << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar editar el archivo " << e.filePath << ". \033[0m" << endl;
                        }
                        incompleto = false; 
                        break;
                    }
                    case rename_command: {
                        Rename re;
                        re = _Rename(parte);
                        if(re.acceso){
                            estado = CambiarNombre_Archivo(sesionAbierta, pariticionID, re.path, re.oldName, re.newName, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha cambiado el nombre del archivo " << re.oldName
                            << " a " << re.newName << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar cambiar el nombre del archivo " << re.oldName << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case mkdir_command: {
                        Mkdir md;
                        md = _Mkdir(parte);
                        if(md.acceso){
                            estado = CrearCarpeta(sesionAbierta, nombreUsuario, md.directoryPath, md.createParentFolderS, pariticionID, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha creado la carpeta " << md.directoryPath<< " en la particion " 
                            << pariticionID << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar crear la carpeta " << md.directoryPath << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case find_command: {
                        Find f;
                        f = _Find(parte);
                        if (f.acceso){
                            estado = BuscarArchivos(sesionAbierta, pariticionID, f.originPath, f.regularExpression, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha mostrado los archivos encontrados que concuerdan con la expresion regular "<< f.regularExpression
                            << " en la particion " << pariticionID <<" correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al intentar mostrar los archivos. \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case chgrp_command: {
                        Chgrp cg;
                        cg = _Chgrp(parte);
                        if(cg.acceso){
                            estado = CambiarGrupo(sesionAbierta, nombreUsuario, pariticionID, cg.username, cg.newGroup, listMountedPartitions);
                            if(estado)  cout << "\033[0;92;49m[Correcto]: Se ha cambiado el usuario " << cg.username << " al grupo " << 
                            cg.newGroup << " correctamente.\033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido al intentar cambiar de grupo al usuario " << cg.username << ". \033[0m" << endl;
                        }
                        incompleto = false;
                        break;
                    }
                    case rep_command: {
                        Rep rp;
                        rp = _Rep(parte);
                        if(rp.acceso){
                            //* Reporte MBR
                            if(strcasecmp(rp.reportName.c_str(), "mbr") == 0)estado = Reporte_MBR(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "disk") == 0) estado = Reporte_Disk(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "inode") == 0) estado = Reporte_inode(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "block") == 0) estado = Reporte_Bloque(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "bm_inode") == 0) estado = Reporte_bm_inode(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "bm_block") == 0) estado = Reporte_bm_block(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "tree") == 0) estado = Reporte_tree(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "sb") == 0) estado = Reporte_super_bloque(rp.partitionId, rp.reportPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "file") == 0) estado = Reporte_File(rp.partitionId, rp.reportPath, rp.directoryPath, listMountedPartitions);
                            else if(strcasecmp(rp.reportName.c_str(), "ls") == 0) estado = Reporte_ls(rp.partitionId, rp.reportPath, rp.directoryPath, listMountedPartitions);
                            else cout << "\033[0;91;49m[Error]: No se reconoció el nombre del reporte " << rp.reportName << " en el comando rep \033[0m" << endl;

                            if(estado) cout << "\033[0;92;49m[Correcto]: Se ha generado el reporte " << rp.reportName << " en " << rp.reportPath
                            << " correctamente. \033[0m" << endl;
                            else cout << "\033[0;91;49m[Error]: Ha ocurrido un error al generar el reporte " << rp.reportName << " en " << rp.reportPath << "\033[0m" << endl;
                        }
                        incompleto = false;
                        break;
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

    if(incompleto && produccion != 4){
        if(cmmd == mount_command){
            showMountedPartitions(listMountedPartitions);
        }else if (cmmd == logout_command){
            if(LogoutUser(sesionAbierta)){
                sesionAbierta = false;
                nombreUsuario = "";
                pariticionID = "";
            }
        }else cout << "\033[0;91;49m[Error]: ingrese los parametros obligatorios del comando. \033[0m" << endl;
    }

    return 0;
}