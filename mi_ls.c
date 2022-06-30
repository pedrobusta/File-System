//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"
#define NUM_MAXIMO_DE_CARACTERES_A_MOSTRAR_EN_MI_LS 5000
int main(int argc, char **argv)
{
    if(argc == 1){
        printf("Sintaxis: ./mi_ls <disco> </ruta_directorio>\n");
    }else if(argc == 3){
        bmount(argv[1]);
        char bufferLectura[NUM_MAXIMO_DE_CARACTERES_A_MOSTRAR_EN_MI_LS];
        mi_dir(argv[2],bufferLectura,'f');
        printf("%s",bufferLectura);
        bumount(argv[1]);
    }else{
        fprintf(stderr, "Numero de parámetros incorrectos\n"); 
    }

    return 0;
}
