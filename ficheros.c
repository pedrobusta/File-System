// Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "ficheros.h"

/**********************************************************************************
 * FUNCTION: Escribe el contenido procedente de un buffer de memoria, buf_original,
 * de tamaño nbytes, en un fichero/directorio (correspondiente al inodo pasado
 * como argumento, ninodo): le indicamos la posición de escritura inicial en
 * bytes lógicos, offset, con respecto al inodo, y el número de bytes, nbytes,
 * que hay que escribir.
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int mi_write_f(unsigned int ninodo, const void *buf_original, unsigned int offset, unsigned int nbytes)
{
    mi_waitSem();
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);

    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "No hay permisos de escritura\n");
        return -1;
    }

    int bytesEscritos = 0;
    int primerBL = offset / BLOCKSIZE;                // Primer blogico en el que escribiremos
    int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; // Ultimo blogico en el que escribiremos
    int desp1 = offset % BLOCKSIZE;                   // Byte donde empezaremos a escribir
    int desp2 = (offset + nbytes - 1) % BLOCKSIZE;    // Byte hasta donde escribiremos

    // HAY QUE ESCRIBIR UN BLOQUE O MENOS
   
    if (primerBL == ultimoBL)
    {
     
        int nBFisico = traducir_bloque_inodo(ninodo, primerBL, '1');
        // mi_signalSem();
        if (nbytes == BLOCKSIZE)
        {
            // SE ESCRIBE UN BLOQUE LOGICO DE FICHERO ENTERO
            bwrite(nBFisico, buf_original);
            bytesEscritos = BLOCKSIZE;
        }
        else
        {
            // SE ESCRIBE MENOS DE UN BLOQUE LOGICO DE FICHERO
            unsigned char buf_bloque[BLOCKSIZE];
            bread(nBFisico, buf_bloque);
            memcpy(buf_bloque + desp1, buf_original, nbytes);
            bwrite(nBFisico, buf_bloque);
            bytesEscritos = nbytes;
        }
      
    }
    // HAY QUE ESCRIBIR MAS DE UN BLOQUE
    else
    {
        // PRIMER BLOQUE FICHERO LÓGICO

        // 23.ABRIL.2022 : EN ESTA LINEA LA FUNCION TRADUCIR_BLOQUE INODO NO RELLENA LOS
        // METADATOS DE PUNTEROS DIRECTOS NI LOS DE PUNTEROS INDIRECTOS
        // ARREGLADO 24.ABRIL.2022
      
        int nBFisico = traducir_bloque_inodo(ninodo, primerBL, '1');

        // printf("[traducir_bloque_inodo()→ inodo.punterosDirectos[%d] = %d (reservado BF %d para BL %d)]\n",primerBL,nBFisico,primerBL,nBFisico);
        unsigned char buf_bloque[BLOCKSIZE];
        bread(nBFisico, buf_bloque);
        memcpy(buf_bloque + desp1, buf_original, BLOCKSIZE - desp1);

        bwrite(nBFisico, buf_bloque);

        bytesEscritos += BLOCKSIZE - desp1;
        // BOQUES LOGICOS INTERMEDIOS FICHERO
        for (int i = primerBL + 1; i < ultimoBL; i++)
        {

            nBFisico = traducir_bloque_inodo(ninodo, i, '1');

            bwrite(nBFisico, buf_original + BLOCKSIZE - desp1 + (i - primerBL - 1) * BLOCKSIZE);

            bytesEscritos += BLOCKSIZE;
        }

        // ÚLTIMO BLOQUE LOGICO DEL FICHERO

        nBFisico = traducir_bloque_inodo(ninodo, ultimoBL, '1');

        //  printf("Ultimo BLOQUE LOGICO E= %d", nBFisico);
        bread(nBFisico, buf_bloque);
        memcpy(buf_bloque, buf_original + (nbytes - desp2 - 1), desp2 + 1);
        bwrite(nBFisico, buf_bloque);
       
        bytesEscritos += desp2 + 1;
    }
  
    leer_inodo(ninodo, &inodo);
    // SALVO METAINFORMACIÓN DEL INODO
    if (offset + nbytes > inodo.tamEnBytesLog)
    {
        inodo.tamEnBytesLog = offset + nbytes;
        inodo.ctime = time(NULL);
    }
    inodo.mtime = time(NULL);

    escribir_inodo(ninodo, inodo);
    mi_signalSem();

    if (bytesEscritos != nbytes)
    {
        perror("Error: No se han escrito todos los bytes en la llamada a mi_write_f");
    }

    return bytesEscritos;
}

/**********************************************************************************
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int mi_read_f(unsigned int ninodo, void *buf_original, unsigned int offset, unsigned int nbytes)
{

    //                  DECLARACIÓN DE VARIABLES
    // Buffer donde vamos a almacenar los bloques leidos del dispositivo
    unsigned char buf_bloque[BLOCKSIZE];
    // Variable donde se van a almacenar los bytes leidos
    int leidos = 0;
    // Variable con la que vamos a manipular el inodo leido
    struct inodo inodo;

    //             FUNCIONALIDAD
    // Leemos el inodo en cuestión
    mi_waitSem();
    leer_inodo(ninodo, &inodo);
    inodo.atime = time(NULL);
    escribir_inodo(ninodo, inodo);
    mi_signalSem();

    // Si el inodo tiene permisos de lectura entramos
    if ((inodo.permisos & 4) == 4)
    {
        if (offset >= inodo.tamEnBytesLog) // El offset esta situado por encima del tamaño en bytes del fichero
        {
            return leidos;
        }

        if (offset + nbytes >= inodo.tamEnBytesLog) // El offset esta bien situado pero pretende leer mas alla del fichero
        {
            nbytes = inodo.tamEnBytesLog - offset;
        }

        int primerBL = offset / BLOCKSIZE;                // primer BL del que leeremos
        int ultimoBL = (offset + nbytes - 1) / BLOCKSIZE; // ultimo bloque del que leeremos

        // HAY QUE LEER UN BLOQUE O MENOS
        if (primerBL == ultimoBL)
        {
            int nBFisico = traducir_bloque_inodo(ninodo, primerBL, '0');
            if (nbytes == BLOCKSIZE) // Es un bloque entero
            {
                if (nBFisico > -1)
                {
                    bread(nBFisico, buf_original);
                    leidos = BLOCKSIZE;
                }
                else
                {
                    leidos = BLOCKSIZE;
                }
            }
            else // Es menos de un bloque
            {
                if (nBFisico > -1)
                {
                    bread(nBFisico, buf_bloque);
                    int desp1 = offset % BLOCKSIZE;                   // Cálculo (0-1024) Primer Byte logico
                    memcpy(buf_original, buf_bloque + desp1, nbytes); // Copiamos los bytes que habian que leer en buf_original
                    leidos = nbytes;
                }
                else
                {
                    leidos = nbytes;
                }
            }
        }
        else // LO QUE VAMOS A LEER OCUPA MAS DE UN BLOQUE
        {
            int nBFisico = traducir_bloque_inodo(ninodo, primerBL, '0');
            int desp1 = offset % BLOCKSIZE;
            int desp2 = (offset + nbytes - 1) % BLOCKSIZE;
            // PRIMER BLOQUE
            if (nBFisico > -1)
            {
                // FIXED
                bread(nBFisico, buf_bloque);
                memcpy(buf_original, buf_bloque + desp1, BLOCKSIZE - desp1);
                leidos = BLOCKSIZE - desp1;
            }
            else
            {
                leidos = BLOCKSIZE - desp1;
            }
            // BLOQUES INTERMEDIOS
            for (int i = primerBL + 1; i < ultimoBL; i++)
            {
                nBFisico = traducir_bloque_inodo(ninodo, i, '0');
                if (nBFisico > -1)
                {
                    bread(nBFisico, buf_original + (BLOCKSIZE - desp1) + (i - primerBL - 1) * BLOCKSIZE);
                    leidos += BLOCKSIZE;
                }
                else
                {
                    leidos += BLOCKSIZE;
                }
            }

            // ULTIMO BLOQUE
            nBFisico = traducir_bloque_inodo(ninodo, ultimoBL, '0');
            if (nBFisico > -1)
            {
                bread(nBFisico, buf_bloque);
                memcpy(buf_original + (nbytes - desp2 - 1), buf_bloque, desp2 - 1);
                leidos += desp2 + 1;
            }
            else
            {
                leidos += desp2 + 1;
            }
        }
    }
    else
    {
        printf("No hay permisos de lectura\n");
    }

    return leidos;
}

/**********************************************************************************
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int mi_stat_f(unsigned int ninodo, struct STAT *p_stat)
{
    // Leemos el inodo
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);

    p_stat->atime = inodo.atime;
    p_stat->ctime = inodo.ctime;
    p_stat->mtime = inodo.mtime;
    p_stat->nlinks = inodo.nlinks;
    p_stat->numBloquesOcupados = inodo.numBloquesOcupados;
    p_stat->permisos = inodo.permisos;
    p_stat->tamEnBytesLog = inodo.tamEnBytesLog;
    p_stat->tipo = inodo.tipo;

    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int mi_chmod_f(unsigned int ninodo, unsigned char permisos)
{
    struct inodo inodo;
    mi_waitSem();
    leer_inodo(ninodo, &inodo);
    inodo.permisos = permisos;
    inodo.ctime = time(NULL);
    escribir_inodo(ninodo, inodo);
    mi_signalSem();
    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION:Trunca un fichero/directorio (correspondiente al nº de inodo, ninodo,
 *          pasado como argumento) a los bytes indicados como nbytes, liberando los
 *          bloques necesarios.
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int mi_truncar_f(unsigned int ninodo, unsigned int nbytes)
{
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);

    if ((inodo.permisos & 2) != 2)
    {
        fprintf(stderr, "No hay permisos de escritura\n");
        return -1;
    }
    if (nbytes > inodo.tamEnBytesLog)
    {
        fprintf(stderr, "Se ha intentado liberar un inodo mas alla de su tamaño en bytes lógicos\n");
        return -1;
    }

    // AVERIGUAMOS CUAL ES EL PRIMER BLOQUE LOGICO A TRUNCAR
    int primerBL = 0;
    if (nbytes % BLOCKSIZE == 0)
    {
        primerBL = nbytes / BLOCKSIZE;
    }
    else
    {
        primerBL = nbytes / BLOCKSIZE + 1;
    }

    int bloquesLiberados = liberar_bloques_inodo(primerBL, &inodo);
    inodo.ctime = time(NULL);
    inodo.mtime = time(NULL);
    inodo.tamEnBytesLog = nbytes;
    inodo.numBloquesOcupados -= bloquesLiberados;

    escribir_inodo(ninodo, inodo);
    return bloquesLiberados;
}
