//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"

int main(int argc, char **argv)
{

    if (argc == 3)
    {
        int strcVal = strcmp(argv[0], "./mi_mkfs");
        if (strcVal == 0)
        {
            int nbloques = atoi(argv[2]);
            int ninodos = nbloques / 4;
            // Hacemos el array de 1024 cajas porque cada caja es un byte (char 8 bits)
            // y nuestro tamaño de bloque es de 1024 Bytes
            unsigned char bloqueVacio[BLOCKSIZE];
            // Rellenamos las 1024 bytes a 0
            memset(bloqueVacio, 0, BLOCKSIZE);
            // Montamos el dispositivo virtual con el nombre especificado
            bmount(argv[1]);
            // Escribimos todos los bloques
            for (int i = 0; i < nbloques; i++)
            {
                bwrite(i, bloqueVacio);
            }

            initSB(nbloques, ninodos);
            initMB();
            initAI();
            //Creamos el directorio raíz
            reservar_inodo('d', 7);
            // Desmontamos el dispositivo virtual
            bumount();
        }
    }
    return 0;
}

