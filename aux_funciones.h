#ifndef AUXFUNCIONES_H
#define AUXFUNCIONES_H

/* AUX VARIABLES */
// Ids para identificar structs en la lectura del disco
//char idMBR = 'a';
// Zero -> Never used | Nullchar -> Used and deleted
//char cero = '0', one = '1';
//char nullChar = '\0';

char cero = '0', uno = '1', nullChar = '\0', idMBR = 'a';


/* AUX FUNCTIONS */
bool isNumber(char c){
    if(c >= 48 && c <= 57) return true;
    return false;
}

bool isLetter(char c){
    if((c >= 65 && c <= 90) || (c >= 97 && c <= 122)) return true;
    return false;
}

#endif // AUXFUNCIONES_H
