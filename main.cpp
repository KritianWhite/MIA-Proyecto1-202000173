#include <iostream>
#include <cstring>
#include "./Analizador/Analizador.cpp"

using namespace std;


char comando[250];

bool Consola(){
    // ? --> Inicializamos cada elemento de comando con el valor cero
    // ?     y con el tamaño del mismo de 250 bytes.
    memset(comando, 0, 250);
    cout << "--> ";
    cin.getline(comando, 250, '\n');
    if (!cin) throw "ERROR: Desbordamiento en la cantidad de carácteres. \n";
    

    int estado;
    estado = Analizador(comando, false);

    switch (estado){
        case 0:
            //? Continuamos con el programa.
            return true;
        case 1:
            //! El programa finaliza ó ocurre algún error.
            return false;
        case 2:{
            string enter = "";
            cout << "Presione ENTER para continuar..";
            getline(cin, enter);
            cout << "" << endl;
            return true;
        }
        default:
            return false;
    }
}

int main()
{
    srand(time(NULL));
    bool menu = true;

    while (menu){
        try {
            menu = Consola();
        }catch(const char* messageError){
            cout << messageError << endl;
            break;
        }catch (...){
            cout << "Me dio ansiedad. Error en el sistema" << endl;
            break;
        }
    }

    //cout << "Hola mundo!" << endl;

    return 0;
}
