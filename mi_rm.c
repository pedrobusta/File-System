//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"

// Sintaxis: ./mi_rm disco /ruta

int main(int argc, char **argv)
{
    if (argc > 2)
    {

        if (argv[2][1] == '\0')
        {
            printf("No se puede borrar el directorio raiz");
        }
        else
        {
            //printf("helo\n");
            bmount(argv[1]);
            mi_unlink(argv[2]);
            bumount();
        }
    }
    else
    {
        printf("Sintaxis: ./mi_rm disco /ruta\n");
    }
    return 0;
}
