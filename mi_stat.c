//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"
int main(int argc, char **argv)
{
    //printf("\nargv2 = %s\n",argv[2]);
    if(argc == 1){
        printf("Sintaxis: ./mi_stat <disco> </ruta>");
    }else if(argc == 3){
        bmount(argv[1]);
        struct STAT p_stat;
        //printf("\nargv2 = %s\n",argv[2]);
        mi_stat(argv[2],&p_stat);
        bumount(argv[1]);
    }else{
        fprintf(stderr, "Numero de parámetros incorrectos\n"); 
    }

    return 0;
}
