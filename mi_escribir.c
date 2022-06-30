//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"

/* Permite escribir texto en una posición de un fichero (offset).

SINTAXIS: ./mi_escribir <disco> </ruta_fichero> <texto> <offset> */

int main(int argc, char **argv)
{
    // revisamos que la sintaxis sea correcta
    if (argc > 4)
    {
        bmount(argv[1]);
        // CALCULAMOS EL TAMAÑO QUE TENDRA QUE TENER EL BUFFER
        int tamanyo = strlen(argv[3]);
        char buffer[tamanyo];
        strcpy(buffer, argv[3]);
        printf("longitud texto: %d\n", tamanyo);
        int offset = atoi(argv[4]);

        int bytesEscritos = mi_write(argv[2], buffer, offset, tamanyo);
        char string[128];
        if (bytesEscritos < 0)
        {
            sprintf(string, "Bytes escritos: %d\n", 0);
            write(2, string, strlen(string));
        }
        else
        {
            sprintf(string, "Bytes escritos: %d\n", bytesEscritos);
            write(2, string, strlen(string));
        }

        // comprobamos
        // unsigned int p_inodo_dir = 0;
        // unsigned int p_inodo = 0;
        // unsigned int p_entrada = 0;
        // int error = buscar_entrada(argv[2], &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);

        // struct STAT estadisticasInodo;
        // mi_stat_f(p_inodo, &estadisticasInodo); // rellenamos la variable con las estadisticas
        // sprintf(string, "tamEnBytesLog %i\n", estadisticasInodo.tamEnBytesLog);
        bumount();
    }
    else
    {
        fprintf(stderr, "Sintaxis: ./mi_escribir <disco> </ruta_fichero> <texto> <offset>\n");
    }
}
