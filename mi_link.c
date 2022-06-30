//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"

// Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace

int main(int argc, char **argv)
{
    if (argc > 3)
    {
        bmount(argv[1]);

        int longCamino1 = strlen(argv[2]);
        char *miniBuff1;
        miniBuff1 = malloc(longCamino1);
        for (int i = 0; i < longCamino1 - 1; i++)
        {
            miniBuff1++;
        }
        int longCamino2 = strlen(argv[3]);
        char *miniBuff2;
        miniBuff2 = malloc(longCamino2);
        for (int i = 0; i < longCamino2 - 1; i++)
        {
            miniBuff2++;
        }

        if (*miniBuff1 != '/' && *miniBuff2 != '/')
        { // son ficheros
            mi_link(argv[2], argv[3]);
        }
        else
        {
            // error no son ficheros
            printf("No son ficheros\n");
        }
        bumount();
    }
    else
    {
        printf("Sintaxis: ./mi_link disco /ruta_fichero_original /ruta_enlace\n");
    }
    return 0;
}
