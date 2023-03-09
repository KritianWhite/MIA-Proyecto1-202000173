#include <iostream>
#include <string.h>
#include <fstream>

#include "Rep.h"
#include "./Reportes/MbrReport.cpp"
#include "./Reportes/DiskReport.cpp"

using namespace std;

bool CrearReporte(string graphvizString, string reportPath){
    //* Variables de control
    string comandoUbuntu;
    int ansUbuntu;
    //* Validando extension de reporte
    string path = "", nombreArchivo = "", extension = "", pathArchivo = "";
    for(int i = reportPath.length() - 1; i >= 0; i--){
        if(reportPath[i] == '.') extension = reportPath.substr(i + 1, reportPath.length());
        else if(reportPath[i] == '/'){
            nombreArchivo = reportPath.substr(i + 1, reportPath.length() - i - extension.length() - 2);
            path = reportPath.substr(0, i);
            break;
        }
    }

    if(extension.length() == 0){
        cout << "\033[;91;49m[Error]: \"" << reportPath << "\" no posee una extensión. \033[0m" << endl;
        return false;
    }
    if(extension != "jpg" && extension != "jpeg" && extension != "png" && extension != "pdf"){
        cout << "\033[;91;49m[Error]: \"" << reportPath << "\" no posee una extension válida (.jpg, .jpeg, .png, .pdf). \033[0m" << endl;
        return false;
    }
    pathArchivo = path;
    pathArchivo += "/";
    pathArchivo += nombreArchivo;
    pathArchivo += ".dot";

    //* Try catch por si le da ansiedad :V 
    try{
        cout << "--> Creando path " << path << " para el reporte " << nombreArchivo << "." << extension << "..." << endl;
        comandoUbuntu = "mkdir -p \"";
        comandoUbuntu += path;
        comandoUbuntu += "\"";
        cout << comandoUbuntu << endl;
        ansUbuntu = system(comandoUbuntu.c_str());

        if(ansUbuntu != 0 && ansUbuntu != 1){
            cout << "\033[;91;49m[Error]: Ocurrio un error al intentar crear el directorio. Verificar permisos de la aplicacion. \033[0m" << endl;
            return false;
        }

        cout << "--> Creando archivo " << nombreArchivo << "-dot en la carpeta del reporte..." << endl;
        ofstream archivoDot;
        archivoDot.open(pathArchivo, ios::out);
        if(archivoDot.fail()){
            cout << "\033[;91;49m[Error]: Ocurrio un error al intentar crear el archivo " << nombreArchivo << ".dot \033[0m" << endl;
            return false;
        }
        archivoDot << graphvizString;
        archivoDot.close();

        cout << "--> Construyendo el archivo " << nombreArchivo << ".dot en el formato especificado (" << extension << ")..." << endl;
        comandoUbuntu = "dot \"";
        comandoUbuntu += pathArchivo;
        comandoUbuntu += "\" -T";
        comandoUbuntu += extension;
        comandoUbuntu += " -o \"";
        comandoUbuntu += reportPath;
        comandoUbuntu += "\"";
        cout << comandoUbuntu << endl;
        ansUbuntu += system(comandoUbuntu.c_str());

        if(ansUbuntu == 0) return true;

        cout << "\033[;91;49m[Error]: Ocurrió un error a la hora de convertir el archivo dot al formato especificado (" << extension << "). \033[0m" << endl;
        return false;
    }catch(const exception& e){
        cout << e.what() << endl;
        return false;
    }catch(...){
        return false;
    }

}

void generacionImg(string nombreEstructura, string cadena){
    try{
        ofstream file;
        file.open(nombreEstructura + ".dot", std::ios::out);

        if(file.fail()){
            exit(1);
        }
        file << cadena;
        file.close();
        //*dot -Tsvg ListUsers.dot -o ListUsers.svg
        string command = "dot -Tsvg " + nombreEstructura + ".dot -o " + nombreEstructura + ".svg";
        system(command.c_str());
    }catch(const std::exception& e){
        cout<<"No se pudo crear la imagen :("<<endl;
    }
}

bool CrearReporte_Texto(string fileText, string reportPath){
    //* Variables de control
    string comandoUbuntu;
    int ansUbuntu;
    //* Validamos extension del reporte
    string path = "", nombreArchivo = "", extension = "";
    for(int i = reportPath.length() - 1; i >= 0; i--){
        if(reportPath[i] == '.') extension = reportPath.substr(i + 1, reportPath.length());
        else {
            nombreArchivo = reportPath.substr(i + 1, reportPath.length() - i - extension.length() - 2);
            path = reportPath.substr(0, i);
            break;
        }
    }

    if(extension.length() == 0){
        cout << "\033[;91;49m[Error]: " << reportPath << " debe de poseer una extensión. \033[0m" << endl;
        return false;
    }

    try{
        cout << "--> Creando path " << path << " para el reporte " << nombreArchivo << "." << extension << "..." << endl;
        comandoUbuntu = "mkdir -p \"";
        comandoUbuntu += path;
        comandoUbuntu += "\"";
        ansUbuntu = system(comandoUbuntu.c_str());

        if(ansUbuntu != 0 && ansUbuntu != 1){
            cout << "\033[;91;49m[Error]: Ocurrió un error en el directorio. Verificar permisos de la aplicación.\033[0m" << endl;
            return false;
        }

        cout << "--> Creando archivo " << nombreArchivo << "." << extension << " en la carpeta... " << endl;
        ofstream reporte;
        path = path + "/" + nombreArchivo + "." + extension;
        reporte.open(path, ios::out);
        if(reporte.fail()){
            cout << "\033[;91;49m[Error]: no se pudo crear el archivo de texto. \033[0m" << endl;
            return false;
        }
        reporte << fileText;
        reporte.close();
        return true;
    }catch(...){
        return false;
    }
}

Rep _Rep(char *parametros){
    //* Variables de control
    int estado = 0;
    string parametroActual = "", comentario = "";
    //* name
    string nombre = "";
    bool vNombre = false;
    //* path
    string path = "";
    bool vPath = false;
    //* particion id
    string particionID = "";
    bool vID = false;
    //* ruta
    string directorio = "";
    bool vDirectorio = false;

    for (int i = 0; i <= (int)strlen(parametros); i++){
        switch(estado){
            //* Reconocimiento del caracter >
            case 0: {
                parametroActual += parametros[i];
                if(parametros[i] == '>') estado = 1;
                else if(parametros[i] == 9 || parametros[i] == 32);
                else if(parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter p
            //* Reconocimiento del caracter n
            //* Reconocimiento del caracter i
            //* Reconocimiento del caracter r
            case 1: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'p') estado = 2;
                else if ((char)tolower(parametros[i]) == 'n') estado = 9;
                else if ((char)tolower(parametros[i]) == 'i') estado = 16;
                else if ((char)tolower(parametros[i]) == 'r') estado = 21;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //TODO--> path
            //* Reconocimiento del caracter a
            case 2: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'a') estado = 3;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 3: {
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 't') estado = 4;
                else if (parametros[i] == '#'){comentario = "", comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter h
            case 4:{
                parametroActual += parametros[i];
                if ((char)tolower(parametros[i]) == 'h') estado = 5;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 5: {
                parametroActual += parametros[i];
                if (parametros[i] == '=') estado = 6;
                else if (parametros[i] == '#'){comentario = ""; comentario = parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de la path con comillas
            case 6: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vPath = false; path = ""; estado = 7;}
                //else if (parametros[i] == '/'){vPath = false; path = ""; path += parametros[i]; estado = 8;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vPath = false; path = ""; path = parametros[i]; estado = 8;}
                break;
            }
            case 7: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(path.length() > 0) vPath = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee una ruta vacia. \033[0m" << endl;
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
                    vPath = true;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else path += parametros[i];
                break;
            }
            //TODO--> Name
            //* Reconocimiento del caracter a
            case 9: {
                //cout << "Llegue al 16" << endl;
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 10;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter m
            case 10: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'm') estado = 11;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter e
            case 11: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'e') estado = 12;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {estado = -1;}
                break;
            }
            //* Reconocimiento del caracter =
            case 12: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 13;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de doble comillas
            case 13: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vNombre = false; nombre = ""; estado = 14;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vNombre = false; nombre = ""; nombre += parametros[i]; estado = 15;}
                break;
            }
            case 14: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(nombre.length() > 0){
                        for (int i = 0; i < (int)nombre.length(); i++){
                            nombre[i] = tolower(nombre[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vacío. \033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombre += parametros[i];
                break;
            }
            //* Reconocimiento del nombre sin comillas
            case 15: {
                parametroActual += parametros[i];
                if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(nombre.length() > 0){
                        for(int i = 0; i < (int)nombre.length(); i++){
                            nombre[i] = tolower(nombre[i]);
                        }
                        vNombre = true;
                    }else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un nombre vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else nombre += parametros[i];
                break;
            }
            //TODO--> Id
            //* Reconocimiento del caracter d
            case 16: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'd') estado = 17;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 17: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 18;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de comillas dobles
            case 18: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){vID = false; particionID = ""; estado = 19;}
                else if (parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vID = false; particionID = ""; particionID += parametros[i]; estado = 20;}
                break;
            }
            case 19: {
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
            case 20: {
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
            //TODO--> ruta
            //* Reconocimiento del caracter u
            case 21: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'u') estado = 22;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter t
            case 22: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 't') estado = 23;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter a
            case 23: {
                parametroActual += parametros[i];
                if((char)tolower(parametros[i]) == 'a') estado = 24;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento del caracter =
            case 24: {
                parametroActual += parametros[i];
                if(parametros[i] == '=') estado = 25;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else estado = -1;
                break;
            }
            //* Reconocimiento de las comillas dobles
            case 25: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"') {vDirectorio = false; directorio = ""; estado = 26;}
                else if(parametros[i] == 9 || parametros[i] == 32) ;
                else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else {vDirectorio = false; directorio = ""; directorio += parametros[i]; estado = 27;}
                break;
            }
            case 26: {
                parametroActual += parametros[i];
                if(parametros[i] == '\"'){
                    if(directorio.length() > 0) vDirectorio = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un directorio vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else directorio += parametros[i];
                break;
            }
            //* Reconocimiento sin comillas dobles
            case 27: {
                parametroActual += parametros[i];
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    if(directorio.length() > 0) vDirectorio = true;
                    else cout << "\033[;91;49m[Error]: \"" << parametroActual << "\" posee un directorio vació.\033[0m" << endl;
                    parametroActual = "";
                    estado = 0;
                }else if (parametros[i] == '#'){comentario = ""; comentario += parametros[i]; estado = -2;}
                else directorio += parametros[i];
                break;
            }
            //* Error de sintaxis
            case -1: {
                if (parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
                    cout << "\033[0;91;49m[Error]: El parametro \"" << parametroActual << "\" es inválido para rep. \033[0m" << endl;
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

    Rep rp;
    if(vPath && vNombre && vID){
        rp.reportName = nombre;
        rp.reportPath = path;
        rp.partitionId = particionID;
        if(vDirectorio) rp.directoryPath = directorio;
        else rp.directoryPath = "";
        rp.acceso = true;
        return rp;
    }

    if (!vNombre) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">nombre\" para poder generar el reporte.\033[0m" << endl;
    if (!vPath) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">path\" para poder generar el reporte.\033[0m" << endl;
    if (!vID) cout << "\033[0;91;49m[Error]: Faltan el parametro obligatorio \">id\" para poder para poder generar el reporte.\033[0m" << endl;
    rp.acceso = false;
    return rp;
}
