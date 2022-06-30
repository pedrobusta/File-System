// Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "bloques.h"
#include "semaforo_mutex_posix.h"

static int descriptor=0; //  Identificador del fichero (dispositivo virtual)
static sem_t *mutex;              //  Variable global para el semáforo
static unsigned int inside_sc = 0;

/***********************************************************************************/
/*  Función que monta el dispositivo virtual (disco, memoria secundaria), dado que */
/*  se trata de un fichero esa acción consistira en abrirlo si existe o crearlo en */
/*  caso de que no exista                                                          */
/*                                                                                 */
/*  INPUTS:                                                                        */
/*  - camino = Nombre o path del archivo que usaremos como dispositivo virtual     */
/*  OUTPUTS:                                                                       */
/*  - descriptorFichero = Identificador del fichero que usaremos como memoria sec. */
/*  - (-1) = si ha habido error                                                    */
/***********************************************************************************/

int bmount(const char *camino)
{
    if (descriptor > 0)
    {
        close(descriptor);
    }
    

    if (!mutex)
    { // el semáforo es único en el sistema y sólo se ha de inicializar 1 vez (padre)
        mutex = initSem();
        if (mutex == SEM_FAILED)
        {
            return -1;
        }
    }
    //umask(000);
    
    descriptor = open(camino, O_RDWR | O_CREAT, 0666);

    if(descriptor== -1){
        fprintf(stderr,"Error:bloques.c -> bmount() -> open()\n");
    }

    return descriptor;
}

/***********************************************************************************/
/*  Función que desmonta el dispositivo virtual y libera el descriptorFichero      */
/*                                                                                 */
/*  INPUTS:                                                                        */
/*  OUTPUTS:                                                                       */
/*  - (0) = Si se ha conseguido cerrar el dispositivo virtual                      */
/*  - (-1) = Si ha habido errro al cerrar el dispositivo virtual                   */
/***********************************************************************************/
int bumount()
{
    
    descriptor = close(descriptor);
    if (descriptor == -1)
    {
        fprintf(stderr,"Error:bloques.c -> bumount() -> close(): %d: %s\n",errno,strerror(errno));
        return -1;
    }
    deleteSem(); //borramos Semaforo
    return 0;
}

/***********************************************************************************/
/*  Función que escribe un bloque en el dispositivo virtual                        */
/*                                                                                 */
/*  INPUTS:                                                                        */
/*  - nbloque = Numero posicion de bloque fisico donde vamos a escribir            */
/*  - buf = Buffer de memoria que contiene los datos que vamos a escribir          */
/*  OUTPUTS:                                                                       */
/*  - (-1) = Si ha habido error                                                    */
/*  - (0) = Si no ha habido error                                                  */
/***********************************************************************************/
int bwrite(unsigned int nbloque, const void *buf)
{
    int bytesEscritos;

    off_t offset = nbloque * BLOCKSIZE;
    lseek(descriptor, offset, SEEK_SET);
    bytesEscritos = write(descriptor, buf, BLOCKSIZE);
    if (bytesEscritos == -1)
    {
        perror("Error");
    }

    return bytesEscritos;
}

/***********************************************************************************/
/*  Función que lee un bloque en el dispositivo virtual                            */
/*                                                                                 */
/*  INPUTS:                                                                        */
/*  - nbloque = Numero posicion de bloque fisico del que vamos a leer              */
/*  - buf = Buffer de memoria donde vamos a volcar los datos leidos                */
/*  OUTPUTS:                                                                       */
/*  - (1) = Si ha habido un error                                                 */
/*  - (nytes leidos) = Si no ha habido error                                        */
/***********************************************************************************/
int bread(unsigned int nbloque, void *buf)
{
    int bytesLeidos;

    off_t offset = nbloque * BLOCKSIZE;
    lseek(descriptor, offset, SEEK_SET);
    bytesLeidos = read(descriptor, buf, BLOCKSIZE);
    if (bytesLeidos == -1)
    {
        perror("Error");
        return EXIT_FAILURE;
    }

    return bytesLeidos;
}

void mi_waitSem()
{
    if (!inside_sc)
    { // entra si inside es 0 entra
        waitSem(mutex);
    }
    inside_sc++;
}

void mi_signalSem()
{
    inside_sc--;
    if (!inside_sc)
    {
        signalSem(mutex);
    }
}
