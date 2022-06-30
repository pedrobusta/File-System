// Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"

/**********************************************************************************
 * FUNCTION: Dada una cadena de caracteres camino (que comience por '/), separa su
 * contenido en dos:
 *  - Guarda en *inicial la porcion de *camino comprendida entre los dos primeros '/'
 *    (en tal caso *inicial contendrá el nombre de un directorio)
 *  - Cuando no hay segundo '/', copia *camino en *inicial sin el primer '/' (en tal
 *    caso *inicial contendrá el nombre de un fichero).
 * INPUT: const char *camino, char *inical, char *final, char *tipo
 * OUTPUT: int devolver, 1 si es un directorio, 2 si es un fichero
 * ********************************************************************************/
int extraer_camino(const char* camino, char* inicial, char* final, char* tipo) {
    // printf("\n%s\n",camino);
    int devolver = -1;
    char c;

    if (*camino != '/') {
        return devolver;
    }
    camino++;
    c = *camino;

    while (c != '/' && c != '\0') {
        *inicial = c;
        inicial++;
        camino++;
        c = *camino;
    }
    *inicial = '\0';

    if (c == '/') {
        devolver = 1;
        strcpy(final, camino);
        *tipo = 'd';
    } else {
        devolver = 2;
        *final = '\0';
        *tipo = 'f';
    }
    return devolver;
}

/**********************************************************************************
 * FUNCTION:
 * INPUT:
 * OUTPUT:
 * ********************************************************************************/
int buscar_entrada(const char* camino_parcial, unsigned int* p_inodo_dir, unsigned int* p_inodo, unsigned int* p_entrada, char reservar, unsigned char permisos) {
    //____________________________/INPUTS/_____________________________
    // camino_parcial -> Ruta que nos dan en cada caso para explorar
    // p_inodo_dir -> Inodo correspondiente al directorio donde voy a hacer la busqueda de la entrada que necesito encontrar
    // p_inodo -> Inodo correspondiente al nombre que estoy buscando
    // p_entrada -> Numero de entrada en el que he localizado ese nombre
    //_________________________________________________________________
    struct entrada entrada;
    memset(entrada.nombre, 0, TAMNOMBRE);
    struct inodo inodo_dir;
    char inicial[sizeof(entrada.nombre)];
    char final[strlen(camino_parcial)];
    memset(final, 0, sizeof(final));
    char tipo;
    int cant_entradas_inodos, num_entrada_inodo;

    // Si es el directorio raiz

    if (*(camino_parcial + 1) == '\0') {
        *p_inodo = 0;
        *p_entrada = 0;
        return 0;
    }

    // Si ha habido un error al extraer el camino
    if (extraer_camino(camino_parcial, inicial, final, &tipo) == -1) {
        return ERROR_CAMINO_INCORRECTO;
    }
#if DEBUGN7
    printf("[buscar_entrada()→ inicial: %s, final: %s, reservar: %d]\n", inicial, final, reservar);
#endif
    // Comprobamos si tenemos permisos de lectura
    leer_inodo(*p_inodo_dir, &inodo_dir);
    if ((inodo_dir.permisos & 4) != 4) {
        return ERROR_PERMISO_LECTURA;
    }

    // Cantidad de entradas que caben en un bloque
    int nENTRADAS = BLOCKSIZE / sizeof(entrada);
    // Tamaño de una entrada
    // int tamENTRADA = sizeof(entrada);
    // Buffer de entradas que caben en un bloque
    struct entrada bufferLecturaEntradas[nENTRADAS];
    memset(bufferLecturaEntradas, 0, BLOCKSIZE);

    // 1era llamada recursiva buscamos en el directorio raiz
    cant_entradas_inodos = inodo_dir.tamEnBytesLog / sizeof(entrada);
    num_entrada_inodo = 0;

    if (cant_entradas_inodos > 0) {
        int offset = 0;
        mi_read_f(*p_inodo_dir, bufferLecturaEntradas, offset, BLOCKSIZE);
        int counterEntradas = 0;
        entrada = bufferLecturaEntradas[counterEntradas++];
        while ((strcmp(inicial, entrada.nombre) != 0) && (num_entrada_inodo < cant_entradas_inodos)) {
            num_entrada_inodo++;
            if (counterEntradas < nENTRADAS) { //¡¡¡¡¡"<" en lugar de "<="!!!!!
                entrada = bufferLecturaEntradas[counterEntradas++];
            } else {
                memset(bufferLecturaEntradas, 0, BLOCKSIZE);
                offset += BLOCKSIZE;
                mi_read_f(*p_inodo_dir, bufferLecturaEntradas, offset, BLOCKSIZE);
                counterEntradas = 0;
                entrada = bufferLecturaEntradas[counterEntradas++];
            }
        }
    }
    if (strcmp(inicial, entrada.nombre) != 0) { //no ha encontrado la entrada
        switch (reservar) {
        case 0:
            return ERROR_NO_EXISTE_ENTRADA_CONSULTA;
            // break;
        case 1:
            if (inodo_dir.tipo == 'f') {
                return ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO;
            }
            if ((inodo_dir.permisos & 2) != 2) {
                return ERROR_PERMISO_ESCRITURA;
            } else {
                strcpy(entrada.nombre, inicial); //--[WARNING] antes akes pointer from integer without a cast
                if (tipo == 'd') {
                    if (strcmp(final, "/") == 0) {
                        // mi_waitSem();
                        entrada.ninodo = reservar_inodo('d', permisos);
                        // mi_signalSem();
#if DEBUGN7
                        printf("[buscar_entrada()→ reservado inodo %d  tipo d con permisos 6 para %s]\n", entrada.ninodo, entrada.nombre);
#endif
                    } else {
                        return ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO;
                    }
                } else {
                    // mi_waitSem();
                    entrada.ninodo = reservar_inodo('f', permisos);
                    // mi_signalSem();
#if DEBUGN7
                    printf("[buscar_entrada()→ reservado inodo %d  tipo f con permisos 6 para %s]\n", entrada.ninodo, entrada.nombre);
#endif
                }
#if DEBUGN7
                printf("[buscar_entrada()→ creada entrada: %s, %d]\n", entrada.nombre, entrada.ninodo);
#endif
            }
            // escribir la entrada en el directorio padre
            //printf("Soy el proceso %i y voy a escribir la entrada %s\n",getpid(),entrada.nombre);
            int resEscribir = mi_write_f(*p_inodo_dir, &entrada, inodo_dir.tamEnBytesLog, sizeof(entrada));
            
            // printf("\nresEscribir = %d\n",resEscribir);
            if (resEscribir == -1) {
                if (entrada.ninodo != -1) {

                    // mi_waitSem();
                    liberar_inodo(entrada.ninodo);
                    // mi_signalSem();
                }
                return EXIT_FAILURE;
            }
            break;
        }
    }
    char c = *final;
    char* auxPtr = final;
    auxPtr++;

    if (c == '\0' || *auxPtr == '\0') {

        if ((num_entrada_inodo < cant_entradas_inodos) && (reservar == 1)) {
            return ERROR_ENTRADA_YA_EXISTENTE;
        }
        // printf("salir\n");
        *p_inodo = entrada.ninodo;
        *p_entrada = num_entrada_inodo;
        return EXIT_SUCCESS;
    } else {
        *p_inodo_dir = entrada.ninodo;
        return buscar_entrada(final, p_inodo_dir, p_inodo, p_entrada, reservar, permisos);
    }
    return 0;
}
/****   ******************************************************************************
 * FUNCTION: Función de la capa de directorios que crea un fichero/directorio y su entrada de directorio.
 * INPUT: Devuelve si hay error o no al crear
 * OUTPUT:
 * ********************************************************************************/
int mi_creat(const char* camino, unsigned char permisos) {

    unsigned int p_inodo_dir = 0; // Por simplicidad suponemos que p_inodo dir es 0, input
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    char reservar = 1;
    // mi_waitSem();
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, permisos);

    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        // mi_signalSem();
        return -1;
    } else {
        // mi_signalSem();
        return 0;
    }
}

int mi_chmod(const char* camino, unsigned char permisos) {
    // Buscamos el numero de inodo al que hay que cambiar los permisos,
    // si hay algún error, lo guardamos en la variable posiblesErrores
    // para devolverselo al usuario
    unsigned int p_inodo_dir = 0;
    unsigned int num_inodo;
    unsigned int p_entrada;
    char reservar = 0;
    int posiblesErrores = buscar_entrada(camino, &p_inodo_dir, &num_inodo, &p_entrada, reservar, permisos);

    // A partir del número, obtenemos el inodo
    struct inodo inodo;
    leer_inodo(num_inodo, &inodo);
    // Le cambiamos los permisos
    inodo.permisos = permisos;
    escribir_inodo(num_inodo, inodo);
    return posiblesErrores;
}

int mi_stat(const char* camino, struct STAT* p_stat) {
    // Buscamos el numero de inodo al que hay que ver las estadisticas,
    // si hay algún error, lo guardamos en la variable posiblesErrores
    // para devolverselo al usuario
    unsigned int p_inodo_dir = 0;
    unsigned int num_inodo = 0;
    unsigned int p_entrada = 0;
    char reservar = 0;
    int posiblesErrores = buscar_entrada(camino, &p_inodo_dir, &num_inodo, &p_entrada, reservar, 6);
    // A partir del número de inodo, obtenemos los datos
    mi_stat_f(num_inodo, p_stat);
    // Mostramos todos los datos
    printf("Nºinodo: %d\n", num_inodo);
    printf("tipo: %c\n", p_stat->tipo);
    printf("permisos %c\n", p_stat->permisos);
    printf("atime: %s", ctime(&p_stat->atime));
    printf("ctime: %s", ctime(&p_stat->ctime));
    printf("mtime: %s", ctime(&p_stat->mtime));
    printf("nlinks: %d\n", p_stat->nlinks);
    printf("tamEnBytesLog: %d\n", p_stat->tamEnBytesLog);
    printf("numBloquesOcupados: %d\n\n", p_stat->numBloquesOcupados);
    return posiblesErrores;
}

int mi_dir(const char* camino, char* buffer, char tipo) {
    // Buscamos el numero de inodo al que hay que hacerls,
    // si hay algún error, lo guardamos en la variable posiblesErrores
    // para devolverselo al usuario
    unsigned int p_inodo_dir = 0;
    unsigned int num_inodo;
    unsigned int p_entrada;
    char reservar = 0;
    char permisos = 6;
    int posiblesErrores = buscar_entrada(camino, &p_inodo_dir, &num_inodo, &p_entrada, reservar, permisos);

    if (posiblesErrores < 0) {
        mostrar_error_buscar_entrada(posiblesErrores);
        return posiblesErrores;
    }

    // A partir del número, obtenemos el inodo
    struct inodo inodo;
    leer_inodo(num_inodo, &inodo);

    if (inodo.tipo != 'd') {
        strcat(buffer, "\033[034m");
        strcat(buffer, "Tipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n-------------------------------------------------------------");
        strcat(buffer, "\033[0m");
        strcat(buffer, "\n");
        char* num;
        num = malloc(sizeof(int));
        sprintf(num, "%c", inodo.tipo);
        strcat(buffer, num);
        strcat(buffer, "\t");
        if (inodo.permisos & 4)
            strcat(buffer, "r");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 2)
            strcat(buffer, "w");
        else
            strcat(buffer, "-");
        if (inodo.permisos & 1)
            strcat(buffer, "x");
        else
            strcat(buffer, "-");
        strcat(buffer, "\t\t");
        struct tm* tm;
        char* tmp;
        tmp = malloc(20);
        tm = localtime(&inodo.mtime);
        sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
        strcat(buffer, tmp);
        strcat(buffer, "\t");
        //*ptr = inodo.tamEnBytesLog;
        // strcat(buffer, &ptr);
        sprintf(num, "%d", inodo.tamEnBytesLog);
        strcat(buffer, num);
        // obtengo la entrada para averiguar el nombre del inodo
        // p_inodo_dir es el inodo directorio del inodo fichero que estoy buscando en p_entrada es el numero de entrada dentro de ese directorio
        struct entrada entrada;
        mi_read_f(p_inodo_dir, &entrada, p_entrada * sizeof(entrada), sizeof(entrada));
        strcat(buffer, "\t");
        strcat(buffer, entrada.nombre);
        strcat(buffer, "\n");
        strcat(buffer, "\n");
    } else {
        int nENTRADAS = BLOCKSIZE / sizeof(struct entrada);

        // Calculamos el nombre de entradas que tiene el inodo
        int entradasInodo = inodo.tamEnBytesLog / sizeof(struct entrada);
        char* num;
        num = malloc(sizeof(int));

        struct entrada bufferEntradas[nENTRADAS];
        int offset = 0;
        struct entrada entradaAux;
        sprintf(num, "%d", entradasInodo);
        strcat(buffer, "\033[034m");
        strcat(buffer, "Total: ");
        strcat(buffer, num);
        strcat(buffer, "\033[0m\n");
        if (entradasInodo > 0) {
            strcat(buffer, "\033[034mTipo\tPermisos\tmTime\t\t\tTamaño\tNombre\n-------------------------------------------------------------");
            strcat(buffer, "\033[0m");
            strcat(buffer, "\n");

            int cBuffEntradas = 0;
            int cEntradas = 0;

            mi_read_f(num_inodo, bufferEntradas, offset, BLOCKSIZE);
            offset += BLOCKSIZE;
            entradaAux = bufferEntradas[cBuffEntradas++];
            cEntradas++;

            while (entradasInodo >= cEntradas) {
                // Leemos el inodo correspondiente a la primera entrada
                leer_inodo(entradaAux.ninodo, &inodo);
                // Copiamos la información de la entrada
                sprintf(num, "%c", inodo.tipo);
                strcat(buffer, num);
                // printf("hola\n");
                // printf("\n%c\n",inodo.permisos);
                strcat(buffer, "\t");
                if (inodo.permisos & 4)
                    strcat(buffer, "r");
                else
                    strcat(buffer, "-");
                if (inodo.permisos & 2)
                    strcat(buffer, "w");
                else
                    strcat(buffer, "-");
                if (inodo.permisos & 1)
                    strcat(buffer, "x");
                else
                    strcat(buffer, "-");
                strcat(buffer, "\t\t");
                struct tm* tm;
                char* tmp;
                tmp = malloc(20);
                tm = localtime(&inodo.mtime);
                sprintf(tmp, "%d-%02d-%02d %02d:%02d:%02d", tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday, tm->tm_hour, tm->tm_min, tm->tm_sec);
                strcat(buffer, tmp);
                strcat(buffer, "\t");
                //*ptr = inodo.tamEnBytesLog;
                // strcat(buffer, &ptr);
                sprintf(num, "%d", inodo.tamEnBytesLog);
                strcat(buffer, num);
                strcat(buffer, "\t");
                strcat(buffer, entradaAux.nombre);
                strcat(buffer, "\n");
                if (nENTRADAS > cBuffEntradas) {
                    cEntradas++;
                    entradaAux = bufferEntradas[cBuffEntradas++];
                } else {
                    // si toca leer un bloque
                    // leemos el primer bloque de entradas correspondiente al inodo, que corresponde al directorio buscado
                    mi_read_f(num_inodo, bufferEntradas, offset, BLOCKSIZE);
                    offset += BLOCKSIZE;
                    cBuffEntradas = 0;
                    cEntradas++;
                    entradaAux = bufferEntradas[cBuffEntradas++];
                }
            }
            strcat(buffer, "\n");
        }
    }
    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION: Función de directorios.c para escribir contenido en un fichero.
 * Buscaremos la entrada camino con buscar_entrada() para obtener el p_inodo.
 * Si la entrada existe llamamos a la función correspondiente de ficheros.c
 * pasándole el p_inodo
 * INPUT: const char *camino, const void *buf, unsigned int offset, unsigned int nbytes
 * OUTPUT: devuelve el numero de bytes escritos
 * ********************************************************************************/
int mi_write(const char* camino, const void* buf, unsigned int offset, unsigned int nbytes) {
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return 0;
    }
    int a = mi_write_f(p_inodo, buf, offset, nbytes);
    return a;
}

/**********************************************************************************
 * FUNCTION: Función de directorios.c para leer los nbytes del fichero indicado por
 * camino, a partir del offset pasado por parámetro y copiarlos en el buffer buf.
 * INPUT: const char *camino, const void *buf, unsigned int offset, unsigned int nbytes
 * OUTPUT: devuelve el numero de bytes leidos
 * ********************************************************************************/
int mi_read(const char* camino, void* buf, unsigned int offset, unsigned int nbytes) {

    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        return 0;
    }
    return mi_read_f(p_inodo, buf, offset, nbytes);
}

/**********************************************************************************
 * FUNCTION: Crea el enlace de una entrada de directorio camino2 al inodo especificado
 *  por otra entrada de directorio camino1.
 * INPUT: const char *camino1, const char *camino2
 * OUTPUT:
 * ********************************************************************************/
int mi_link(const char* camino1, const char* camino2) {
    mi_waitSem();
    int error;
    unsigned int p_inodo_dir1 = 0;
    unsigned int p_inodo1 = 0;
    unsigned int p_entrada1 = 0;
    if ((error = buscar_entrada(camino1, &p_inodo_dir1, &p_inodo1, &p_entrada1, 0,6)) < 0) {
        mi_signalSem();
        mostrar_error_buscar_entrada(error);
        return EXIT_FAILURE;
    }
    struct inodo inodo;
    leer_inodo(p_inodo1, &inodo);
    if ((inodo.permisos & 4) != 4) {
        printf("No hay permisos de lectura\n");
        mi_signalSem();
        return EXIT_FAILURE;
    }
    // printf("tamEnBytesLog = %d\n",inodo.tamEnBytesLog);
    unsigned int p_inodo_dir2 = 0;
    unsigned int p_inodo2 = 0;
    unsigned int p_entrada2 = 0;
    if ((error = buscar_entrada(camino2, &p_inodo_dir2, &p_inodo2, &p_entrada2, 1, 6)) < 0) {
        // si errro entrada ya existente
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return EXIT_FAILURE;
    }
    struct entrada entrada;
    mi_read_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(entrada), sizeof(entrada));
    // printf("entrada.nombre = %s\n",entrada.nombre);
    entrada.ninodo = p_inodo1;
    mi_write_f(p_inodo_dir2, &entrada, p_entrada2 * sizeof(entrada), sizeof(entrada));
    liberar_inodo(p_inodo2);
    inodo.nlinks++;
    inodo.ctime = time(NULL);
    escribir_inodo(p_inodo1, inodo);
    mi_signalSem();
    return EXIT_SUCCESS;
}

/**********************************************************************************
 * FUNCTION: Función de la capa de directorios que borra la entrada de directorio
 * especificada (no hay que olvidar actualizar la cantidad de enlaces en el inodo)
 * y, en caso de que fuera el último enlace existente, borrar el propio fichero/directorio.
 * INPUT: const char *camino
 * OUTPUT:
 * ********************************************************************************/
int mi_unlink(const char* camino) {
    mi_waitSem();
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, 0, 6);
    if (error < 0) {
        mostrar_error_buscar_entrada(error);
        mi_signalSem();
        return 0;
    }
    struct inodo inodo;
    leer_inodo(p_inodo, &inodo);
    if (inodo.tipo == 'd' && inodo.tamEnBytesLog > 0) { // Es un directorio en el que aun hay ficheros o otros directorios
        printf("Error: El directorio %s no está vacío\n", camino);
        mi_signalSem();
        return 0;
    }
    leer_inodo(p_inodo_dir, &inodo);
    int nEntradasDir = inodo.tamEnBytesLog / sizeof(struct entrada); // obtenemos el numero de entradas que tiene el directorio asociado al fichero o directorio que queremos elim
    if (p_entrada == nEntradasDir - 1) { // es la ultima entrada basta con truncarlo en tamBytesLog - 1 entrada
        mi_truncar_f(p_inodo_dir, inodo.tamEnBytesLog - sizeof(struct entrada));
        leer_inodo(p_inodo, &inodo);
    } else {
        struct entrada entrada;
        mi_read_f(p_inodo_dir, &entrada, inodo.tamEnBytesLog - sizeof(struct entrada), sizeof(struct entrada));
        mi_write_f(p_inodo_dir, &entrada, p_entrada * sizeof(struct entrada), sizeof(struct entrada));
        mi_truncar_f(p_inodo_dir, inodo.tamEnBytesLog - sizeof(struct entrada));
        leer_inodo(p_inodo, &inodo);
    }
    // YA SEA UNA ENTRADA INTERMEDIA O LA ENTRADA FINAL QUE HEMOS BORRADO HAY QUE ACTUALIZAR EL INODO
    inodo.nlinks--;
    if (inodo.nlinks == 0) { // eliminamos el inodo
        liberar_inodo(p_inodo);
    } else {
        inodo.ctime = time(NULL);
        escribir_inodo(p_inodo, inodo);
    }
    mi_signalSem();
    return 1;
}

// Funcion para mostrar errores
void mostrar_error_buscar_entrada(int error) {
    switch (error) {
    case -1:
        fprintf(stderr, "Error: Camino incorrecto.\n");
        break;
    case -2:
        fprintf(stderr, "Error: Permiso denegado de lectura.\n");
        break;
    case -3:
        fprintf(stderr, "Error: No existe el archivo o el directorio.\n");
        break;
    case -4:
        fprintf(stderr, "Error: No existe algún directorio intermedio.\n");
        break;
    case -5:
        fprintf(stderr, "Error: Permiso denegado de escritura.\n");
        break;
    case -6:
        fprintf(stderr, "Error: El archivo ya existe.\n");
        break;
    case -7:
        fprintf(stderr, "Error: No es un directorio.\n");
        break;
    }
}
