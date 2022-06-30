//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"

int main(int argc, char **argv)
{
    if(argc == 1){
        printf("Sintaxis: ./mi_mkdir <nombre_dispositivo> <permisos> </ruta_directorio/>\n");
    }else if(argc == 4){
        bmount(argv[1]);
        int permisos = atoi(argv[2]);
        if(permisos < 0 || permisos > 7){
            fprintf(stderr, "Error: modo inválido: <<%d>>\n",permisos); 
            
        }else{
            mi_creat(argv[3],*argv[2]);
        }
        bumount(argv[1]);
    }else{
        fprintf(stderr, "Numero de parámetros incorrectos\n"); 
    }

    return 0;
}
