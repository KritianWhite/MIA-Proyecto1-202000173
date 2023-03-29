// #include <iostream>
// #include <string.h>
// #include <string>

// using namespace std;


// Rmdisk automataRmdisk(char *parametros){
//     string path = "";
//     bool vPath = false;
//     int estado = 0;
//     string parametroActual = "";
//     string comentario = "";

//     for(int i = 0; i <= (int)strlen(parametros); i++){
//         switch(estado){
//             case 0: {
//                 parametroActual += parametros[i];
//                 if(parametros[i] == '>') estado = 1;
//                 else if(parametros[i] == 9 || parametros[i] == 32) ;
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else {estado = -1;}
//                 break;
//             }
//             case 1: {
//                 parametroActual += parametros[i];
//                 if((char)tolower(parametros[i]) == 'p') estado = 2;
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else {estado = -1;}
//                 break;
//             }
//             case 2: {
//                 parametroActual += parametros[i];
//                 if((char)tolower(parametros[i]) == 'a') estado = 3;
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else {estado = -1;}
//                 break;
//             }
//             case 3: {
//                 parametroActual += parametros[i];
//                 if((char)tolower(parametros[i]) == 't') estado = 4;
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else {estado = -1;}
//                 break;
//             }
//             case 4: {
//                 parametroActual += parametros[i];
//                 if((char)tolower(parametros[i]) == 'h') estado = 5;
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else {estado = -1;}
//                 break;
//             }
//             case 5: {
//                 parametroActual += parametros[i];
//                 if(parametros[i] == '=') estado = 7;
//                 else if(parametros[i] == 9 || parametros[i] == 32) ;
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else {estado = -1;}
//                 break;
//             }
//             case 7: {
//                 parametroActual += parametros[i];
//                 if(parametros[i] == '\"') {vPath = false; path = ""; estado = 8;}
//                 else if(parametros[i] == 9 || parametros[i] == 32) ;
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else {vPath = false; path = ""; path += parametros[i]; estado = 9;}
//                 break;
//             }
//             case 8: {
//                 parametroActual += parametros[i];
//                 if(parametros[i] == '\"'){
//                     if(path.length() > 0) vPath = true;
//                     else {cout << "Error: " << parametroActual << " posee una ruta vacia \n";}
//                     parametroActual = "";
//                     estado = 0;
//                 }
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else path += parametros[i];
//                 break;
//             }
//             case 9: {
//                 parametroActual += parametros[i];
//                 if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
//                     vPath = true;
//                     parametroActual = "";
//                     estado = 0;
//                 }
//                 else if(parametros[i] == '#') {comentario = ""; comentario += parametros[i]; estado = -2;}
//                 else path += parametros[i];
//                 break;
//             }
//             case -1: {
//                 if(parametros[i] == 0 || parametros[i] == 9 || parametros[i] == 32){
//                     cout << "Error: " << parametroActual << " es invalido para el comando rmdisk \n";
//                     parametroActual = "";
//                     estado = 0;
//                 }
//                 else parametroActual += parametros[i];
//                 break;
//             }
//             case -2: {
//                 comentario += parametros[i];
//                 break;
//             }
//         } // end: switch
//     } // end: for char in parametros

// }