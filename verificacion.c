//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "verificacion.h"

void crearStringDelStructInfo(struct informacion info, char* bufferInfo, int pid);

int main(int argc, char **argv){
    if( argc == 1){ //Si hay un unico parámetro le indicamos como se usa el comando
        printf("Uso: verificacion <nombre_dispositivo> <directorio_simulación>\n");
    }
    else if(argc == 3){ //3 parámetros es el número correcto de parametros
        //Montamos el dispositivo        
        bmount(argv[1]);
        //Quitamos la barra del final del directorio
        argv[2][strlen(argv[2])-1] = '\0';
        //CALCULAR EL NUMERO DE ENTRADAS DEL DIRECTORIO SIM A PARTIR DEL STAT DEL INOD
        //leemos el inod
        unsigned int num_inodo_dir= 0; //directorio padre
        unsigned int num_inodo_simul; //simul_num
        unsigned int p_entrada;
        char reservar = 0;
        char permisos = 6;
        int posiblesErrores = buscar_entrada(argv[2], &num_inodo_dir, &num_inodo_simul, &p_entrada, reservar, permisos);
        if(posiblesErrores < 0){
            mostrar_error_buscar_entrada(posiblesErrores);
        }
        struct inodo inodoSimul;
        leer_inodo(num_inodo_simul,&inodoSimul);
        //Oobtenemos el numero de entradas del inod
        int numEntradas = ((int) inodoSimul.tamEnBytesLog) / sizeof(struct entrada);
        //printf("Este directorio tiene %i entradas\n", numEntradas);
        if(numEntradas != NUMPROCESOS){
            fprintf(stderr,"Error: El número de entradas no coincide con el numero de procesos");
            exit(0);
        }

        printf("dir_sim: %s\n",argv[2]);
        printf("numentradas: %i NUMPROCESOS %i\n",numEntradas, NUMPROCESOS);
        
        //CREAMOS EL FICHERO "informe.txt" y la variable informe
        char caminoInformeTxt[strlen(argv[2]) + 11]; //"informe.txt" tiene 11 caracteres
        strcpy(caminoInformeTxt,argv[2]);
        strcat(caminoInformeTxt,"/informe.txt");
        mi_creat(caminoInformeTxt,6);

        //LEEMOS TODAS LAS ENTRADAS DEL DIRECTORIO
        struct entrada entrada;
        int nENTRADAS = BLOCKSIZE / sizeof(struct entrada);
        struct entrada bufferLecturaEntradas[nENTRADAS];
        memset(bufferLecturaEntradas, 0, BLOCKSIZE);
        int offset = 0;
        mi_read_f(num_inodo_simul, bufferLecturaEntradas, offset, BLOCKSIZE);
        int counterEntradas = 0;
        for(int i = 0; i<numEntradas ; i++){
            struct informacion info;
            //Leemos la entrada del directorio
            if (counterEntradas < nENTRADAS) { //¡¡¡¡¡"<" en lugar de "<="!!!!!
                entrada = bufferLecturaEntradas[counterEntradas++];
            } else {
                memset(bufferLecturaEntradas, 0, BLOCKSIZE);
                offset += BLOCKSIZE;
                mi_read_f(num_inodo_simul, bufferLecturaEntradas, offset, BLOCKSIZE);
                counterEntradas = 0;
                entrada = bufferLecturaEntradas[counterEntradas++];
            }
            //Extraemos el PID a partir del nombre de la entrada
            char *procesoNumChar = strchr(entrada.nombre,'_');
            procesoNumChar++;
            int pid = atoi(procesoNumChar);
            
            //Obtenemos el num_inodo_pruebaDat del directorio actual
            //RECORREMOS SECUENCIALMENTE EL FICHERO prueba.dat
            char caminoHaciaPruebaDat[strlen(argv[2]) + strlen(entrada.nombre)]; //"prueba.dat" tiene 11 caracteres
            strcpy(caminoHaciaPruebaDat,argv[2]);
            strcat(caminoHaciaPruebaDat,"/");
            strcat(caminoHaciaPruebaDat,entrada.nombre);
            strcat(caminoHaciaPruebaDat,"/prueba.dat");
            int cant_registros_buffer_escrituras = 256; 
            struct REGISTRO buffer_registros [cant_registros_buffer_escrituras];
            int offsetProceso = 0;
            memset(buffer_registros, 0, sizeof(buffer_registros));
            //mi_read_f(num_inodo_pruebaDat, buffer_registros, offsetProceso, sizeof(buffer_registros));

            struct REGISTRO registroEscrituraMaxima;
            struct REGISTRO registroEscrituraMinima;
            struct REGISTRO registroPosicionMinima;
            struct REGISTRO registroPosicionMaxima;         

            int numEscriturasValidadas = 0;            
            while (mi_read(caminoHaciaPruebaDat, buffer_registros, offsetProceso, sizeof(buffer_registros)) > 0) {
                offsetProceso += sizeof(struct REGISTRO) * cant_registros_buffer_escrituras;
                for(int j = 0; j<cant_registros_buffer_escrituras;j++){
                    if(buffer_registros[j].pid == pid){ //Si el registro es valido
                        if(numEscriturasValidadas == 0){ //si es la primiera
                            registroEscrituraMaxima = buffer_registros[j];
                            registroEscrituraMinima = buffer_registros[j];
                            registroPosicionMinima = buffer_registros[j];
                        }else{
                            //miramos si es mayor que la maxima,si es cierto, será la nueva maxima
                            if(buffer_registros[j].nEscritura > registroEscrituraMaxima.nEscritura){
                                registroEscrituraMaxima = buffer_registros[j];
                            }
                            //miramos si es menor que la minima,si es cierto, será la nueva minima
                            if(buffer_registros[j].nEscritura < registroEscrituraMinima.nEscritura){
                                registroEscrituraMinima = buffer_registros[j];
                            }
                        }
                        numEscriturasValidadas++;
                        //printf("contenido: pid %i,registro %i,escritura %i\n",buffer_registros[j].pid,buffer_registros[j].nRegistro,buffer_registros[j].nEscritura);
                        //Actualizamos la ultima escritura leida y así la ultima vez que ejecutemos esta línea
                        //sera la escritura justo antes del end of file 
                        registroPosicionMaxima = buffer_registros[j]; 
                    }
                } //fin del for
                memset(buffer_registros, 0, sizeof(buffer_registros));  
            } //fin del while
            #if DEBUGN13
                printf("[%i) %i escrituras validas en %s]\n",i,numEscriturasValidadas,caminoHaciaPruebaDat);
            #endif
            
            info.MayorPosicion = registroPosicionMaxima;
            info.MenorPosicion = registroPosicionMinima;
            info.nEscrituras = numEscriturasValidadas;
            info.PrimeraEscritura = registroEscrituraMinima;
            info.UltimaEscritura = registroEscrituraMaxima;
            char bufferInfo [300];
            memset(bufferInfo,0,sizeof(bufferInfo));
            crearStringDelStructInfo(info,bufferInfo,pid);
            mi_write(caminoInformeTxt, bufferInfo ,i *sizeof(bufferInfo),sizeof(bufferInfo));
        }//fin del bucle for each proceso
        bumount(argv[1]);
    }else{ //Cualquier número de parametros que no sea ni 3 ni 1 es incorrecto
        fprintf(stderr,"Error: numero de parametros incorrecto");
    }
}

void crearStringDelStructInfo(struct informacion info, char* bufferInfo, int pid) {
    strcat(bufferInfo, "PID:");
    char str[15];
    sprintf(str, "%d", pid); // integer to string
    strcat(bufferInfo, str);
    strcat(bufferInfo, "\n");
    strcat(bufferInfo, "Numero de escrituras: ");
    char str2[15];
    sprintf(str2, "%d", info.nEscrituras); // integer to string
    strcat(bufferInfo, str2);
    strcat(bufferInfo, "\n");
    strcat(bufferInfo, "Primera escritura\t");
    char str3[15];
    sprintf(str3, "%d", info.PrimeraEscritura.nEscritura); // integer to string
    strcat(bufferInfo, str3);
    strcat(bufferInfo, "\t");
    char str4[15];
    sprintf(str4, "%d", info.PrimeraEscritura.nRegistro); // integer to string
    strcat(bufferInfo, str4);
    strcat(bufferInfo, "\t");
    strcat(bufferInfo, asctime(localtime(&info.PrimeraEscritura.fecha)));

    strcat(bufferInfo, "Ultima escritura \t");
    char str6[15];
    sprintf(str6, "%d", info.UltimaEscritura.nEscritura); // integer to string
    strcat(bufferInfo, str6);
    strcat(bufferInfo, "\t");
    char str7[15];
    sprintf(str7, "%d", info.UltimaEscritura.nRegistro); // integer to string
    strcat(bufferInfo, str7);
    strcat(bufferInfo, "\t");
    strcat(bufferInfo, asctime(localtime(&info.UltimaEscritura.fecha)));

    strcat(bufferInfo, "Menor posicion  \t");
    char str9[15];
    sprintf(str9, "%d", info.MenorPosicion.nEscritura); // integer to string
    strcat(bufferInfo, str9);
    strcat(bufferInfo, "\t");
    char str10[15];
    sprintf(str10, "%d", info.MenorPosicion.nRegistro); // integer to string
    strcat(bufferInfo, str10);
    strcat(bufferInfo, "\t");
    strcat(bufferInfo, asctime(localtime(&info.MenorPosicion.fecha)));

    strcat(bufferInfo, "Mayor posicion  \t");
    char str11[15];
    sprintf(str11, "%d", info.MayorPosicion.nEscritura); // integer to string
    strcat(bufferInfo, str11);
    strcat(bufferInfo, "\t");
    char str12[15];
    sprintf(str12, "%d", info.MayorPosicion.nRegistro); // integer to string
    strcat(bufferInfo, str12);
    strcat(bufferInfo, "\t");
    strcat(bufferInfo, asctime(localtime(&info.MayorPosicion.fecha)));
    strcat(bufferInfo, "\n\n\n");
}
