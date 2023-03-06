#include <iostream>

using namespace std;

bool LogoutUser(bool SesionAbierta){
    if(!SesionAbierta){
        cout << "\033[0;91;49m[Error]: no se encontró sesión activa\033[0m" << endl;
        return false;
    }
    cout << "\033[0;92;49m[Correcto]: Se ha cerrado sesión correctamente\033[0m" << endl;
    return true;
}