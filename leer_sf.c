//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "directorios.h"

void testNivel3(); // Declaramos estas funciones
void testNivel4();
void testNivel7();

void mostrar_buscar_entrada(char *camino, char reservar)
{
    unsigned int p_inodo_dir = 0;
    unsigned int p_inodo = 0;
    unsigned int p_entrada = 0;
    int error;
    printf("camino: %s, reservar: %d\n", camino, reservar);
    if ((error = buscar_entrada(camino, &p_inodo_dir, &p_inodo, &p_entrada, reservar, 6)) < 0)
    {
        mostrar_error_buscar_entrada(error); // HACERLO LUEGO ***********************************
    }
    printf("**********************************************************************\n");
    return;
}

int main(int argc, char **argv)
{
    int strcVal = strcmp(argv[0], "./leer_sf");
    if (strcVal == 0)
    {
        bmount("disco");
        struct superbloque SB;
        bread(0, &SB);
        printf("\nDATOS DEL SUPERBLOQUE\n");
        printf("posPrimerBloqueMB = %d\n", SB.posPrimerBloqueMB);
        printf("posUltimoBloqueMB = %d\n", SB.posUltimoBloqueMB);
        printf("posPrimerBloqueAI = %d\n", SB.posPrimerBloqueAI);
        printf("posUltimoBloqueAI = %d\n", SB.posUltimoBloqueAI);
        printf("posPrimerBloqueDatos = %d\n", SB.posPrimerBloqueDatos);
        printf("posUltimoBloqueDatos = %d\n", SB.posUltimoBloqueDatos);
        printf("posInodoRaiz = %d\n", SB.posInodoRaiz);
        printf("posPrimerInodoLibre = %d\n", SB.posPrimerInodoLibre);
        printf("cantBloquesLibres = %d\n", SB.cantBloquesLibres);
        printf("cantInodosLibres = %d\n", SB.cantInodosLibres);
        printf("totBloques = %d\n", SB.totBloques);
        printf("totInodos= %d\n\n\n", SB.totInodos);

        //testNivel3();
        //testNivel4();
        //testNivel7
        //  Mostrar creación directorios y errores
        // mostrar_buscar_entrada("pruebas/", 1);           // ERROR_CAMINO_INCORRECTO
        // mostrar_buscar_entrada("/pruebas/", 0);          // ERROR_NO_EXISTE_ENTRADA_CONSULTA
        // mostrar_buscar_entrada("/pruebas/docs/", 1);     // ERROR_NO_EXISTE_DIRECTORIO_INTERMEDIO
        // mostrar_buscar_entrada("/pruebas/", 1);          // creamos /pruebas/
        // mostrar_buscar_entrada("/pruebas/docs/", 1);     // creamos /pruebas/docs/
        // mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // creamos /pruebas/docs/doc1
        // mostrar_buscar_entrada("/pruebas/docs/doc1/doc11", 1);
        // // ERROR_NO_SE_PUEDE_CREAR_ENTRADA_EN_UN_FICHERO
        // mostrar_buscar_entrada("/pruebas/", 1);          // ERROR_ENTRADA_YA_EXISTENTE
        // mostrar_buscar_entrada("/pruebas/docs/doc1", 0); // consultamos /pruebas/docs/doc1
        // mostrar_buscar_entrada("/pruebas/docs/doc1", 1); // creamos /pruebas/docs/doc1
        // mostrar_buscar_entrada("/pruebas/casos/", 1);    // creamos /pruebas/casos/
        // mostrar_buscar_entrada("/pruebas/docs/doc2", 1); // creamos /pruebas/docs/doc2

        // Desmontamos el dispositivo virtual
        bumount();
    }
}

void testNivel4()
{
        //EST DEL NIVEL 4
        int inodoReservado = reservar_inodo('l', '6'); // Reservamos un inodo
        traducir_bloque_inodo(inodoReservado, 8, 1);
        traducir_bloque_inodo(inodoReservado, 204, 1);
        traducir_bloque_inodo(inodoReservado, 30004, 1);
        traducir_bloque_inodo(inodoReservado, 400004, 1);
        traducir_bloque_inodo(inodoReservado, 468750, 1);
}

void testNivel3()
{

    /* EN EL ENUNCIADO NIVEL 4 SE NOS PIDIÓ QUE COMENTARAMOS
    LO DE RESERVAR BLOQUE, LIBERAR BLOQUE, MOSTRAR EL MAPA DE BITS Y EL
    DIRECTORIO RAIZ

    printf("\nRESERVAMOS UN BLOQUE Y LUEGO LO LIBERAMOS");
    int bloqueReservado = reservar_bloque();
    //TODO: NO FUNCIONA RESERVAR BLOQUE
    //Respuesta 25-marzo 13:30 -> ¿Sigue siendo cierto este comentario?
    //No se cuando fue escrito
    printf("\nSe ha reservado el bloque físico nº%d que era el 1º libre indicado por el MB", bloqueReservado);
    bread(0, &SB);
    printf("\ncantBloquesLibres = %d\n", SB.cantBloquesLibres);
    liberar_bloque(bloqueReservado);
    bread(0, &SB);
    printf("Liberamos ese bloque y después SB.cantBloquesLibres = %d\n", SB.cantBloquesLibres);


    printf("\nMAPA DE BITS CON BLOQUES DE METADATOS OCUAPADOS\n");
    printf("leer_bit(0) = %d\n\n", leer_bit(0));
    printf("leer_bit(1) = %d\n\n", leer_bit(1));
    printf("leer_bit(%d) = %d\n\n",SB.posUltimoBloqueMB, leer_bit(SB.posUltimoBloqueMB));
    printf("leer_bit(%d) = %d\n\n",SB.posPrimerBloqueAI, leer_bit(SB.posPrimerBloqueAI));
    printf("leer_bit(%d) = %d\n\n",SB.posUltimoBloqueAI, leer_bit(SB.posUltimoBloqueAI));
    printf("leer_bit(%d) = %d\n\n",SB.posPrimerBloqueDatos, leer_bit(SB.posPrimerBloqueDatos));
    printf("leer_bit(%d) = %d\n\n",SB.posUltimoBloqueDatos, leer_bit(SB.posUltimoBloqueDatos));

    printf("DATOS DEL DIRECTORIO RAIZ\n");
    struct inodo inodo;
    leer_inodo(0,&inodo);
    printf("tipo: %c\n",inodo.tipo);
    printf("permisos: %d\n",inodo.permisos);
    printf("atime: %s", ctime(&inodo.atime));
    printf("ctime: %s", ctime(&inodo.ctime));
    printf("mtime: %s", ctime(&inodo.mtime));
    printf("nlinks: %d\n",inodo.nlinks);
    printf("tamEnBytesLog: %d\n",inodo.tamEnBytesLog);
    printf("numBloquesOcupados: %d\n",inodo.numBloquesOcupados);

    */
    // printf("\n\nDATOS DEL DIRECTORIO RAIZ\n");

    // struct inodo inodos[BLOCKSIZE / INODOSIZE];
    // for (int i = SB.posPrimerBloqueAI; i <= SB.posUltimoBloqueAI; i++)
    // {
    //     bread(i, inodos);
    //     for (int j = 0; j < BLOCKSIZE / INODOSIZE; j++)
    //     {
    //         printf("%d ", inodos[j].punterosDirectos[0]);
    //     }
    // }
}

void testNivel7()
{
}
