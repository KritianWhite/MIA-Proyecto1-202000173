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

using namespace std;

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
                            if(script.fail()) cout << "Error crítico: no pudo abrirse el archivo " << e.path << endl;
                            else{
                                cout << "EJECUTANDO COMANDO DEL eea" << endl;
                                int estado;
                                while (!script.eof()){
                                    comandoScript = "";
                                    getline(script, comandoScript);
                                    if(comandoScript != "\0"){
                                        estado = Analizador((char *)comandoScript.c_str(), true);
                                        if(estado == 1) break;
                                        else if (estado == 2){
                                            string enter = "";
                                            cout << "Presione ENTER para continuar.." << endl;
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
                                cout << "Se ha creado el disco." << endl;
                            }else {
                                cout << "Ocurrió un error en la creación del disco." << endl;
                            }
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
                cout << "--> Ha ocurrido un error en el sistema al intentar analizar los parametros." << endl;
            }catch(const char *err){
                cout << "Algo ocurrió: []" <<  err << "]." << endl;
            }catch(...){
                incompleto = false;
                cout << "--> Ha ocurrido un error en el sistema al intentar analizar los parametros." << endl;
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