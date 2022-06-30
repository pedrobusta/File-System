//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"
const int TAM_BUFFER = BLOCKSIZE;
/* Permite escribir texto en una posición de un fichero (offset).

SINTAXIS: ./mi_cat <disco> </ruta_fichero> */

int main(int argc, char **argv)
{
    // revisamos que la sintaxis sea correcta
    if (argc > 2)
    {
        bmount(argv[1]);
        unsigned int p_inodo_dir = 0;
        unsigned int p_inodo = 0;
        unsigned int p_entrada = 0;
        int error = buscar_entrada(argv[2], &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
        if (error < 0)
        {
            mostrar_error_buscar_entrada(error);
        }
        else
        {
            struct STAT estadisticasInodo;
            mi_stat_f(p_inodo, &estadisticasInodo); // rellenamos la variable con las estadisticas
            if (estadisticasInodo.tipo == 'f')
            {

                int numBytesLeidosTotal = 0;
                int numBytesLeidos = 0;
                char buffer[TAM_BUFFER];
                int offset = 0;
                int i = 0;

                do // leemos el fichero
                {
                    offset = TAM_BUFFER * i;
                    memset(buffer, 0, sizeof(buffer));
                    numBytesLeidos = mi_read(argv[2], buffer, offset, TAM_BUFFER);
                    write(1, buffer, numBytesLeidos);
                    numBytesLeidosTotal += numBytesLeidos;
                    i++;

                } while (numBytesLeidos > 0);

                char string[128];
                sprintf(string, "\n\nTotal_leidos %i\n", numBytesLeidosTotal);
                write(2, string, strlen(string));

                
            }
            else
            {
                // es un directorio
            }
        }
        bumount();
    }
    else
    {
        fprintf(stderr, "Sintaxis: ./mi_cat <disco> </ruta_fichero>\n");
    }
}
