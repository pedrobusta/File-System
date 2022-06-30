// Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "bloques.h"
#include "ficheros_basico.h"

/**********************************************************************************
 * FUNCTION:se encargará de inicializar la lista de inodos libres
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int tamMB(unsigned int nbloques)
{

    int nBLCK_MB = (nbloques / 8) / BLOCKSIZE;
    int resto = (nbloques / 8) % BLOCKSIZE;
    if (resto == 0)
    {
        return nBLCK_MB;
    }
    else
    {
        return nBLCK_MB + 1;
    }
}

/**********************************************************************************
 * FUNCTION:se encargará de inicializar la lista de inodos libres
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int tamAI(unsigned int ninodos)
{
    // Multiplicamos el número de inodos por el tamaño en Bytes de un inodo para obtener
    // los bytes totales en inodos, lo dividimos por BLOCKSIZE para saber cuantos bloques ocupa el
    // array de inodos
    return (ninodos * INODOSIZE) / BLOCKSIZE;
}

/**********************************************************************************
 * FUNCTION:se encargará de inicializar el Super Bloque
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int initSB(unsigned int nbloques, unsigned int ninodos)
{
    struct superbloque SB;
    SB.posPrimerBloqueMB = posSB + tamSB;
    SB.posUltimoBloqueMB = SB.posPrimerBloqueMB + tamMB(nbloques) - 1;
    SB.posPrimerBloqueAI = SB.posUltimoBloqueMB + 1;
    SB.posUltimoBloqueAI = SB.posPrimerBloqueAI + tamAI(ninodos) - 1;
    SB.posPrimerBloqueDatos = SB.posUltimoBloqueAI + 1;
    SB.posUltimoBloqueDatos = nbloques - 1;
    SB.posInodoRaiz = 0;
    SB.posPrimerInodoLibre = 0;
    SB.cantBloquesLibres = nbloques;
    SB.cantInodosLibres = ninodos;
    SB.totBloques = nbloques;
    SB.totInodos = ninodos;
    bwrite(posSB, &SB);

    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION:se encargará de inicializar la lista de inodos libres
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
// Inicializa el mapa de bits poniendo a 1 los bits que representan los metadatos
// En este nivel, de momento, simplemente pondremos a 0 todos los bits del mapa de bits
int initMB()
{

    struct superbloque SB;
    bread(0, &SB);
    int nB = SB.totBloques;
    int nIn = SB.totInodos;
    int primerBLCKMB = SB.posPrimerBloqueMB;
    int nBLCK_MB = tamMB(nB);
    int nBLCK_MD = tamSB + nBLCK_MB + tamAI(nIn);
    int nBLCK_MB_MetaDatos = nBLCK_MD / 8 / BLOCKSIZE;
    int nBytes_MB_Metadatos = nBLCK_MD / 8;
    int nBytes_MB_MetadatosResto = nBLCK_MD % 8;
    unsigned char bufferMB[BLOCKSIZE];
    // EJEMPLO
    //  nBLCK_MD = 3
    //  nBLCK_MB_MetaDatos = 0
    //  nBytes_MB_Metadatos = 0
    //  nBytes_MB_MetadatosResto = 3
    // EJEMPLO2
    //  nB = 1.000.000
    //  nBCK_MD = 31374
    //  nBLCK_MB_MetaDatos = 3
    //  nBytes_MB_Metadatos = 3921
    //  nBytes_MB_MetadatosResto = 6;

    if (nBLCK_MB_MetaDatos >= 1)
    {
        memset(bufferMB, 255, BLOCKSIZE);
        for (int i = 0; i < nBLCK_MB_MetaDatos; i++)
        {
            bwrite(primerBLCKMB++, bufferMB);
        }
        // 3 x 1024 = 3072
        // Recalculamos de forma correcta
        // nBytes_MB_Metadatos = 3921 - (3*1024) = 849
        nBytes_MB_Metadatos = nBytes_MB_Metadatos - (nBLCK_MB_MetaDatos * BLOCKSIZE);
    }

    memset(bufferMB, 0, BLOCKSIZE);
    for (int i = 0; i < nBytes_MB_Metadatos; i++)
    {
        bufferMB[i] = 255;
    }

    bufferMB[nBytes_MB_Metadatos] = obtenerByteResto(nBytes_MB_MetadatosResto);
    for (int i = nBytes_MB_Metadatos + 1; i < BLOCKSIZE; i++)
    {
        bufferMB[i] = 0;
    }

    bwrite(primerBLCKMB, bufferMB);
    SB.cantBloquesLibres = SB.cantBloquesLibres - nBLCK_MD;
    bwrite(0, &SB);
    return EXIT_SUCCESS;
}

int obtenerByteResto(int resto)
{
    switch (resto)
    {
    case 1: // 1000 0000
        return 128;
        break;
    case 2: // 1100 0000
        return 192;
        break;
    case 3: // 1110 0000
        return 224;
        break;
    case 4: // 1111 0000
        return 240;
        break;
    case 5: // 1111 1000
        return 248;
        break;
    case 6: // 1111 1100
        return 252;
        break;
    case 7: // 1111 1110
        return 254;
        break;
    }
    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION:se encargará de inicializar la lista de inodos libres
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int initAI()
{
    struct superbloque SB;
    bread(0, &SB);
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    int contInodos = SB.posPrimerInodoLibre + 1;                       // si hemos inicializado SB.posPrimerInodoLibre = 0
    for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++) // para cada bloque del AI
    {
        // leer el bloque de inodos i del dispositivo virtual
        for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++) // para cada inodo del AI
        {
            inodos[j].tipo = 'l'; // libre
            if (contInodos < SB.totInodos)
            {                                               // si no hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = contInodos; // enlazamos con el siguiente
                contInodos++;
            }
            else
            {                                             // hemos llegado al último inodo
                inodos[j].punterosDirectos[0] = UINT_MAX; // UNSIGNED INTEGER MAX
                // hay que salir del bucle, el último bloque no tiene por qué estar completo !!!
                if (contInodos - 1 == SB.totInodos)
                {
                    break;
                }
            }
        }
        // escribir el bloque de inodos i  en el dispositivo virtual
        bwrite(i, &inodos);
    }
    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION: escribe el valor indicado por el parámetro bit: 0 (libre) ó 1 (ocupado)
 *           en un determinado bit del MB que representa el bloque nbloque.
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int escribir_bit(unsigned int nbloque, unsigned int bit)
{
    // Para saber a que byte relativo al mapa de bits pertenece el bloque, cada bit del MB es un bloque
    int posByte = nbloque / 8;
    // Para saber que bit dentro de posByte;
    int posBit = nbloque % 8;

    // Ahora en que bloque del MB relativo esta este byte
    int nbloqueMB = posByte / BLOCKSIZE;
    // Ahora la posicion byte relativa al bloque
    posByte = posByte % BLOCKSIZE;
    struct superbloque SB;
    bread(0, &SB);
    unsigned char bufferMB[BLOCKSIZE];
    // Leemos el bloque donde se situa el bit
    int nbloqueABS = SB.posPrimerBloqueMB + nbloqueMB;
    bread(nbloqueABS, bufferMB);

    unsigned char mascara = 128; // 1000 0000
    mascara >>= posBit;

    if (bit == 1)
    {
        bufferMB[posByte] |= mascara;
    }
    else
    {
        bufferMB[posByte] &= ~mascara;
    }
    // Escrbimos el bit
    bwrite(nbloqueABS, bufferMB);
    return 0;
}

/**********************************************************************************
 * FUNCTION: Lee un determinado bit del MB y devuelve el valor del bit leído
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
char leer_bit(unsigned int nbloque)
{
    // Calculamos el byte relativo al MB
    int posByte = nbloque / 8;
    // Calculamos el bit relativo a posByte
    int posBit = nbloque % 8;

    struct superbloque SB;
    bread(0, &SB);
    int nbloqueMB = posByte / BLOCKSIZE;
    int nbloqueAbs = SB.posPrimerBloqueMB + nbloqueMB;
    unsigned char bufferMB[BLOCKSIZE];
    bread(nbloqueAbs, bufferMB);

    unsigned char mascara = 128;
    mascara >>= posBit;
#if DEBUGN3 == 1
    fprintf(stderr, "[(leer_bit(%d) --> posbyte:%d, posbit:%d, nbloqueMB:%d, nbloqueabs:%d)]\n", nbloque, posByte, posBit, nbloqueMB, nbloqueAbs);
#endif
    posByte = posByte % BLOCKSIZE;
    mascara &= bufferMB[posByte];
    mascara >>= 7 - posBit;
    return mascara;
}

/**********************************************************************************
 * FUNCTION:Encuentra el primer bloque libre, consultando el MB (primer bit a 0),
 *          lo ocupa (poniendo el correspondiente bit a 1 con la ayuda de la función
 *          escribir_bit()) y devuelve su posición.
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int reservar_bloque()
{
    struct superbloque SB;
    bread(0, &SB);
    if (SB.cantBloquesLibres > 0)
    {
        // Para iterar en el MB
        int nbloquesAbs = SB.posPrimerBloqueMB;
        // Donde vamos a guardar el contenido de las iteraciones
        unsigned char bufferMB[BLOCKSIZE];
        // Lo vamos a usar para comparar con el buffer de contenido para saber si hay un 0
        unsigned char bufferAux[BLOCKSIZE];
        // Rellenamos de 1s
        memset(bufferAux, 255, BLOCKSIZE);
        // Leemos el primer bloque del MB
        bread(nbloquesAbs, bufferMB);
        // Mientras los dos buffers sean iguales lee un bloque
        bread(nbloquesAbs++, bufferMB);
        // Mientras los dos buffers sean iguales lee un bloque
        int n = memcmp(bufferMB, bufferAux, BLOCKSIZE);
        while (n == 0)
        {
            bread(nbloquesAbs++, bufferMB);
            n = memcmp(bufferMB, bufferAux, BLOCKSIZE);
        }
        nbloquesAbs--;
        int posByte = 0;
        int exit = 0; // 0 false, 1 true
        // Para encontrar el byte exacto donde se ubica el 0
        for (int i = 0; i < BLOCKSIZE && !exit; i++)
        {
            if (bufferMB[i] != 255)
            {
                posByte = i;
                exit = 1;
            }
        }

        // Para encontrar el bit exacto donde se ubica el 0
        unsigned char mascara = 128;
        int posBit = 0;
        while (bufferMB[posByte] & mascara)
        {
            bufferMB[posByte] <<= 1;
            posBit++;
        }

        // Bloque en relacion a todo el dispositivo
        int nbloque = ((nbloquesAbs - SB.posPrimerBloqueMB) * BLOCKSIZE + posByte) * 8 + posBit;
        escribir_bit(nbloque, 1);
        SB.cantBloquesLibres--;
        bwrite(0, &SB);

        // Vamos a limpiar (poner todo a 0s) el bloque que reservamos
        unsigned char buffer0s[BLOCKSIZE];
        memset(buffer0s, 0, BLOCKSIZE);
        bwrite(nbloque, buffer0s);

        // Devolvemos el bloque que hemos reservado
        return nbloque;
    }
    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION:Libera un bloque determinado (con la ayuda de la función escribir_bit())
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int liberar_bloque(unsigned int nbloque)
{
    escribir_bit(nbloque, 0);
    struct superbloque SB;
    bread(0, &SB);
    SB.cantBloquesLibres++;
    bwrite(0, &SB);
    return nbloque;
}

/**********************************************************************************
 * FUNCTION:Escribe el contenido de una variable de tipo struct inodo en un
 *          determinado inodo del array de inodos, inodos.
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int escribir_inodo(unsigned int ninodo, struct inodo inodo)
{
    // Obtenemos la posición del primer bloque del Array de Inodos
    struct superbloque SB;
    bread(0, &SB);
    int posPrimerBloqueAI = SB.posPrimerBloqueAI;

    // Array de struct de inodos donde vamos a colocar todo un bloque de Inodos
    struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // Posicion del inodo dentro del bloque que vamos a leer
    int posInodo = ninodo % (BLOCKSIZE / INODOSIZE);
    // Posicion de bloque relativa al AI
    int posBloque = (ninodo * INODOSIZE) / BLOCKSIZE;
    // Leemos el bloque, contiene 8 inodos
    bread(posBloque + posPrimerBloqueAI, inodos);
    // Modificamos el inodo que vamos a escribir
    inodos[posInodo] = inodo;
    bwrite(posBloque + posPrimerBloqueAI, inodos);

    return 0;
}

/**********************************************************************************
 * FUNCTION:Lee un determinado inodo del array de inodos para volcarlo en una
 *          variable de tipo struct inodo pasada por referencia
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int leer_inodo(unsigned int ninodo, struct inodo *inodo)
{
    struct superbloque SB;
    bread(0, &SB);

    int posPrimerBloqueAI = SB.posPrimerBloqueAI;
    int bloqueAI = (ninodo * INODOSIZE) / BLOCKSIZE;
    int bloqueABS = posPrimerBloqueAI + bloqueAI;
    struct inodo inodos[BLOCKSIZE / INODOSIZE];

    bread(bloqueABS, inodos);
    int posInodo = ninodo % (BLOCKSIZE / INODOSIZE);
    *inodo = inodos[posInodo];

    return 0;
}

/**********************************************************************************
 * FUNCTION:Encuentra el primer inodo libre (dato almacenado en el superbloque),
 *          lo reserva (con la ayuda de la función escribir_inodo()), devuelve su
 *          número y actualiza la lista enlazada de inodos libres
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int reservar_inodo(unsigned char tipo, unsigned char permisos)
{
    struct superbloque SB;
    bread(0, &SB);
    if (SB.cantInodosLibres > 0)
    {
        int posInodoReservado = SB.posPrimerInodoLibre;
        struct inodo inodo;
        leer_inodo(posInodoReservado, &inodo);
        SB.posPrimerInodoLibre = inodo.punterosDirectos[0];

        inodo.tipo = tipo;
        inodo.permisos = permisos;
        inodo.nlinks = 1;
        inodo.tamEnBytesLog = 0;
        inodo.atime = time(NULL);
        inodo.ctime = time(NULL);
        inodo.mtime = time(NULL);
        inodo.numBloquesOcupados = 0;
        for (int i = 0; i < 12; i++)
        {
            inodo.punterosDirectos[i] = 0;
            if (i < 3)
            {
                inodo.punterosIndirectos[i] = 0;
            }
        }
        escribir_inodo(posInodoReservado, inodo);

        SB.cantInodosLibres--;
        bwrite(0, &SB);
        return posInodoReservado;
    }
    else
    {
        // error no hay inodos libres
        perror("Error: no hay inodos libres");
        return -1;
    }
}

/**********************************************************************************
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int obtener_nRangoBL(struct inodo *inodo, unsigned int nblogico, unsigned int *ptr)
{
    if (nblogico < DIRECTOS)
    {
        *ptr = inodo->punterosDirectos[nblogico];
        return 0;
    }
    else if (nblogico < INDIRECTOS0)
    {
        *ptr = inodo->punterosIndirectos[0];
        return 1;
    }
    else if (nblogico < INDIRECTOS1)
    {
        *ptr = inodo->punterosIndirectos[1];
        return 2;
    }
    else if (nblogico < INDIRECTOS2)
    {
        *ptr = inodo->punterosIndirectos[2];
        return 3;
    }
    else
    {
        *ptr = 0;
        perror("Error: Bloque lógico fuera de rango");
        return -1;
    }
}

/**********************************************************************************
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int obtener_indice(unsigned int nblogico, int nivel_punteros)
{
    if (nblogico < DIRECTOS)
    {
        return nblogico; // ej nblogico=8
    }
    else if (nblogico < INDIRECTOS0)
    {
        return (nblogico - DIRECTOS);
    }
    else if (nblogico < INDIRECTOS1)
    { // ej nblogico=30.004
        if (nivel_punteros == 2)
        {
            return (nblogico - INDIRECTOS0) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return (nblogico - INDIRECTOS0) % NPUNTEROS;
        }
    }
    else if (nblogico < INDIRECTOS2)
    { // ej nblogico=400.004
        if (nivel_punteros == 3)
        {
            return (nblogico - INDIRECTOS1) / (NPUNTEROS * NPUNTEROS);
        }
        else if (nivel_punteros == 2)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) / NPUNTEROS;
        }
        else if (nivel_punteros == 1)
        {
            return ((nblogico - INDIRECTOS1) % (NPUNTEROS * NPUNTEROS)) % NPUNTEROS;
        }
    }
    return -1;
}

/**********************************************************************************
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int traducir_bloque_inodo(unsigned int ninodo, unsigned int nblogico, unsigned char reservar)
{

    struct inodo inodo;
    unsigned int ptr, ptr_ant;
    int salvar_inodo, nRangoBL, nivel_punteros, indice;
    unsigned int buffer[NPUNTEROS];
    leer_inodo(ninodo, &inodo);
    ptr = 0, ptr_ant = 0, salvar_inodo = 0;

    nRangoBL = obtener_nRangoBL(&inodo, nblogico, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
    // ERROR 23 ABRIL 2022: ptr no cambia su valor debe cambiar cuando tienen contenido
    nivel_punteros = nRangoBL; // el nivel_punteros +alto es el que cuelga del inodo
    while (nivel_punteros > 0)
    { // iterar para cada nivel de punteros indirectos
        // printf("\n\nNivel punteros:  %d\n", nivel_punteros);
        // printf("PTR = %d\n", ptr);
        if (ptr == 0)
        { // no cuelgan bloques de punteros
            if (reservar == 48)
            {

                return -1; // bloque inexistente -> no imprimir nada por pantalla!!!
            }
            else
            { // reservar bloques de punteros y crear enlaces desde el  inodo hasta el bloque de datos
                salvar_inodo = 1;
                ptr = reservar_bloque(); // de punteros
                inodo.numBloquesOcupados++;
                inodo.ctime = time(NULL); // fecha actual
                if (nivel_punteros == nRangoBL)
                { // Estamos situados en el inodo en algun nivel de punteros indirectos le damos valor IO,I1,I2 = (nRangoBL -1)
                    // Seria la primera iteracion si I0, I1, I2 = 0, y hay que darle un valor que apunte a un bloque de indices
                    inodo.punterosIndirectos[nRangoBL - 1] = ptr;
#if DEBUGN4 == 1
                    fprintf(stderr, "[traducir_bloque_inodo() --> inodo.punterosIndirectos[%i] = %i (reservado BF %i para punteros_nivel%i)]\n", nRangoBL - 1, ptr, ptr, nRangoBL);
#endif
                }
                else
                { // el bloque cuelga de otro bloque de punteros
                    buffer[indice] = ptr;
#if DEBUGN4 == 1
                    fprintf(stderr, "[traducir_bloque_inodo() --> punteros_nivel%i[%i] = %i(Reservado BF %i para punteros_nivel%i)]\n", nivel_punteros + 1, indice, ptr, ptr, nivel_punteros);

#endif
                    bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
                }
                memset(buffer, 0, BLOCKSIZE); // ponemos a 0 todos los punteros del buffer
            }
        }
        else // cuelgan bloques de punteros
        {
            bread(ptr, buffer); // leemos del dispositivo el bloque de punteros ya existente
        }
        // Obtenemos el indice correspondiente a un bloque lógico del bloque de indices
        indice = obtener_indice(nblogico, nivel_punteros);
        ptr_ant = ptr;        // guardamos el puntero actual
        ptr = buffer[indice]; // y lo desplazamos al siguiente nivel
        nivel_punteros--;
    } // al salir de este bucle ya estamos al nivel de datos

    if (ptr == 0)
    { // El valor de ese indice es igual a 0 por lo tanto no cuelga ningun bloque de datos
        if (reservar == 48)
        {

            return -1; // error lectura ∄ bloque
        }
        else
        {
            salvar_inodo = 1;
            ptr = reservar_bloque(); // de datos
            // printf("\nPTR despues de reservar bloque %d",ptr);
            inodo.numBloquesOcupados++;
            // printf("\nBLoque ocupados por el inodo %d\n",inodo.numBloquesOcupados);
            inodo.ctime = time(NULL);
            if (nRangoBL == 0) // Si nRangoBL estamos trabajando en punteros directos
            {
                inodo.punterosDirectos[nblogico] = ptr;
#if DEBUGN4 == 1
                fprintf(stderr, "[traducir_bloque_inodo() --> inodo.punterosDirectos[%i] = %i (reservado BF %i para BL %i)]\n", nblogico, ptr, ptr, nblogico);
#endif
            }
            else
            { // Si no se trata de un bloque indice y debemos escribirlo en el dispositivo
#if DEBUGN4 == 1
                fprintf(stderr, "[traducir_bloque_inodo() --> punteros_nivel%i[%i] = %i(reservado BF %i, para BL %i)\n", nivel_punteros + 1, indice, ptr, ptr, nblogico);
#endif
                buffer[indice] = ptr;    // asignamos la dirección del bloque de datos (imprimirlo para test)
                bwrite(ptr_ant, buffer); // salvamos en el dispositivo el buffer de punteros modificado
            }
        }
    }

    if (salvar_inodo == 1)
    {
        escribir_inodo(ninodo, inodo); // sólo si lo hemos actualizado
    }
    return ptr;
}

/**********************************************************************************
 * FUNCTION:Liberar un inodo implica por un lado, que tal inodo pasará a la cabeza
 *          de la lista de inodos libres (actualizando el campo SB.posPrimerInodoLibre)
 *          y tendremos un inodo más libre en el sistema, SB.cantInodosLibres, y por otro
 *          lado, habrá que recorrer la estructura de enlaces del inodo para liberar
 *          todos aquellos bloques de datos que estaba ocupando, más todos aquellos bloques
 *          índice que se hubieran creado para apuntar a esos bloques.
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int liberar_inodo(unsigned int ninodo)
{
    // Leer el inodo.
    struct inodo inodo;
    leer_inodo(ninodo, &inodo);
    // Llamar a la función auxiliar liberar_bloques_inodo() para liberar todos los bloques del inodo.
    int bloquesLiberados = liberar_bloques_inodo(0, &inodo);
    // A la cantidad de bloques ocupados del inodo, inodo.numBloquesOcupados, se le restará la cantidad
    //       de bloques liberados por la función anterior (y debería quedar a 0).
    inodo.numBloquesOcupados -= bloquesLiberados;
    // Marcar el inodo como tipo libre y tamEnBytesLog=0
    inodo.tipo = 'l';
    inodo.tamEnBytesLog = 0;

    // ACTUALIZAR LA LISTA ENLAZADA DE INODOS LIBRES
    // Leer el superbloque  para saber cuál es el primer inodo libre de la lista enlazada, SB.posPrimerInodoLibre.
    struct superbloque SB;
    bread(0, &SB);
    // Incluir el inodo que queremos liberar en la lista de inodos libres (por el principio), actualizando el
    // superbloque para que éste sea ahora el primero de la lista. El inodo liberado apuntará donde antes apuntaba
    // el campo del superbloque, SB.posPrimerInodoLibre.
    inodo.punterosDirectos[0] = SB.posPrimerInodoLibre;
    SB.posPrimerInodoLibre = ninodo;
    // En el superbloque, incrementar la cantidad de inodos libres, SB.cantInodosLibres
    SB.cantInodosLibres++;
    // Escribir el inodo
    escribir_inodo(ninodo, inodo);
    // Escribir el superbloque
    bwrite(posSB, &SB);
    return ninodo;
}

/**********************************************************************************
 * FUNCTION:La función liberar_bloques_inodo() libera todos los bloques ocupados
 *          (con la ayuda de la función liberar_bloque()) a partir del bloque lógico
 *          indicado por el argumento primerBL (inclusive).
 * INPUT:
 * OUTPUT: La caantidad de bloques liberados
 * ********************************************************************************/
int liberar_bloques_inodo(unsigned int primerBL, struct inodo *inodo)
{
    // libera los bloques de datos e índices iterando desde el primer bloque lógico a liberar hasta el último
    // por tanto explora las ramificaciones de punteros desde las hojas hacia las raíces en el inodo

    // Declaración variables
    unsigned int nivel_punteros, indice, ptr = 0;
    unsigned int nBL, ultimoBL;
    int nRangoBL;
    unsigned int bloques_punteros[3][NPUNTEROS]; // array de bloques de punteros
    unsigned char bufAux_punteros[BLOCKSIZE];    // para llenar de 0s y comparar
    int ptr_nivel[3];                            // punteros a bloques de punteros de cada nivel
    int indices[3];                              // indices de cada nivel
    int liberados;                               // nº de bloques liberados
    int bwrites = 0;
    int breads = 0;
    liberados = 0;
    // Si el fichero está vacío
    if (inodo->tamEnBytesLog == 0)
    {
        return 0;
    }
    // Obtenemos el último bloque lógico del inodo
    if (inodo->tamEnBytesLog % BLOCKSIZE == 0)
    {
        ultimoBL = (inodo->tamEnBytesLog / BLOCKSIZE) - 1;
    }
    else
    {
        ultimoBL = inodo->tamEnBytesLog / BLOCKSIZE;
    }

    // printf("\n[liberar_bloques_inodo()→ primer BL: %d, último BL: %d]\n", primerBL, ultimoBL);

    memset(bufAux_punteros, 0, BLOCKSIZE);

    for (nBL = primerBL; nBL <= ultimoBL; nBL++)
    {
        nRangoBL = obtener_nRangoBL(inodo, nBL, &ptr); // 0:D, 1:I0, 2:I1, 3:I2
        if (nRangoBL < 0)
        {
            return -1;
        }
        nivel_punteros = nRangoBL;

        while (ptr > 0 && nivel_punteros > 0) // Aqui obtenemos los bloques indices, sus indices y los punteros
        {
            indice = obtener_indice(nBL, nivel_punteros);
            if (indice == 0 || nBL == primerBL) // solo hay que leer del dispositivo si no está ya cargado previamente en un buffer
            {

                bread(ptr, bloques_punteros[nivel_punteros - 1]);
                breads++;
            }

            ptr_nivel[nivel_punteros - 1] = ptr;
            indices[nivel_punteros - 1] = indice;               // Contiene el indice del bloque de indices
            ptr = bloques_punteros[nivel_punteros - 1][indice]; // Contiene el puntero hacia un bloque fisico o hacia el siguiente bloque de indices
            nivel_punteros--;
        }

        if (ptr > 0) // Aqui entramos con un bloque de punteros indice o en el caso de directos
        {
            // liberamos bloque de capa mas superior
            liberar_bloque(ptr);
            liberados++;
            if (nRangoBL == 0) // Si estamos en directos
            {
                // printf("[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", ptr, nBL);
                inodo->punterosDirectos[nBL] = 0;
            }
            else
            {
                nivel_punteros = 1;
                // printf("[liberar_bloques_inodo()→ liberado BF %d de datos para BL %d]\n", ptr, nBL);
                while (nivel_punteros <= nRangoBL)
                {
                    indice = indices[nivel_punteros - 1];             // Obtenemos el indice de la capa mas alta
                    bloques_punteros[nivel_punteros - 1][indice] = 0; // Ponemos a 0 el bloque liberado
                    ptr = ptr_nivel[nivel_punteros - 1];
                    if (memcmp(bloques_punteros[nivel_punteros - 1], bufAux_punteros, BLOCKSIZE) == 0) // No cuelgan más bloques ocupados, hay que liberar el bloque de punteros
                    {
                        // liberamos bloque de capa inferior ya que al bloque de indices que apunta es todo 0s
                        liberar_bloque(ptr);
                        liberados++;
                        // printf("[liberar_bloques_inodo()→ liberado BF %d de punteros_nivel%d para BL %d]\n", ptr, nivel_punteros, nBL);
                        // Incluir mejora saltando los bloques que no sea necesario explorar
                        // al eliminar bloque de punteros    -> hay que actualizar nBL

                        if (nivel_punteros == nRangoBL)
                        {
                            inodo->punterosIndirectos[nRangoBL - 1] = 0;
                            // MEJORA
                            // Si hemos liberado el bloque de indices de profundidad 1 por lo tanto hemos puesto el puntero de indirectos a 0 no hara falta seguir leyendo los
                            // demas bloques logicos de ese/esos bloques de indices (descomentar para ver de que bloque a que bloque saltamos)
                            if (nRangoBL == 1)
                            {
                                // printf("\n\nnBL %d\n", nBL);
                                nBL = INDIRECTOS0;
                                // printf("BOOST\n");
                                // printf("nBL %d\n\n", nBL);
                            }
                            else if (nRangoBL == 2)
                            {
                                // printf("\n\nnBL %d\n", nBL);
                                nBL = INDIRECTOS1;
                                // printf("BOOST\n");
                                // printf("nBL %d\n\n", nBL);
                            }
                            else if (nRangoBL == 3)
                            {
                                // printf("\n\nnBL %d\n", nBL);
                                nBL = INDIRECTOS2;
                                // printf("BOOST\n");
                                // printf("nBL %d\n\n", nBL);
                            }
                        }

                        nivel_punteros++;
                    }
                    else
                    {
                        bwrite(ptr, bloques_punteros[nivel_punteros - 1]);
                        nivel_punteros = nRangoBL + 1;
                        bwrites++;
                    }
                }
            }
        }
        else
        {
            // SUPER BOOST
            //  //Incluir mejora saltando los bloques que no sea necesario explorar  al valer 0 un puntero
            if (nRangoBL != 0) // Si no estamos en punteros directos entramos si lo estamos salimos y vamos al siguiente puntero directo
            {
                if (nRangoBL == 1) // Estamos en rango 1
                {
                    // printf("\nnbL %d", nBL);
                    // printf("\n\nBOOST\n");
                    // Si el puntero indirectos1 -> I0  es igual a cero significa que todo el bloque de indices esta vacio por lo
                    // que vamos a I1 ya que I0 solo tiene un bloque de indices
                    nivel_punteros = 1;
                    ptr = ptr_nivel[nivel_punteros - 1];
                    // printf("\nPTR = %d\n", ptr);
                    if (ptr_nivel[nRangoBL - 1] == 0)
                    {
                        // printf("\nhola\n");
                        nBL = INDIRECTOS0;
                    }
                    // printf("\nnbL %d\n\n\n",nBL);
                }

                else if (nRangoBL == 2) // Estamos en rango 2
                {
                    // Si el puntero de nivel 2 vale 0 el bloque de punteros de nivel 1 estara vacio por lo tanto saltamos 256 posiciones
                    if (ptr_nivel[nRangoBL - 2] == 0)
                    {
                        nBL += BLOCKSIZE / sizeof(int);
                        // printf("\nhola\n");
                        //  Si el puntero de Indirectos 2 -> I1 es igual acero significa que todos los bloques de indices estan vacios podemos saltar a I2
                        if (ptr_nivel[nRangoBL - 1] == 0)
                        {
                            nBL = INDIRECTOS1;
                            // printf("\nhola2\n");
                        }
                    }
                    else if (nRangoBL == 3) // Estamos en rango 3
                    {
                        if (ptr_nivel[nRangoBL - 3] == 0)
                        {
                            // printf("\nhola\n");
                            int nBL_ant = nBL;
                            nBL += BLOCKSIZE / sizeof(int);
                            if (ptr_nivel[nRangoBL - 2] == 0)
                            {
                                // printf("\nhola2\n");
                                nBL_ant += BLOCKSIZE / sizeof(int) * BLOCKSIZE / sizeof(int);
                                nBL = nBL_ant;
                                if (ptr_nivel[nRangoBL - 1] == 0)
                                {
                                    // printf("\nhola3\n");
                                    nBL = INDIRECTOS2;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // printf("[liberar_bloques_inodo()→ total bloques liberados: %d, total breads: %d, total_bwrites:%d]", liberados, breads, bwrites);
    return liberados;
}
