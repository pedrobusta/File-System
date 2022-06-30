//Antoni Font, Pedro Bustamante, Pavel Ernesto Garcia
#include "simulacion.h"
#include "directorios.h"
#include "semaforo_mutex_posix.h"
#include <sys/wait.h>
#include <signal.h>


int acabados = 0;

int main(int argc, char **argv)
{
    if (argc > 1)
    {
        //montamos el dispositivo padre
        bmount(argv[1]); 
        
        //Creamos el formato del direcorio de simulación: /simul_aaaammddhhmmss/ 
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        char camino[1000];
        char *formato = "/simul_%Y%m%d%H%M%S/";
        strftime(camino, sizeof(camino), formato, tm);
        strcat(camino,"\0");
        printf("\n");

        if (mi_creat(camino, 6) != -1)
        {
            pid_t pid;
            signal(SIGCHLD, reaper);
            for (int proceso = 1; proceso <= NUMPROCESOS; proceso++)
            {
                pid = fork();
                // si es el hijo
                if (pid == 0)
                {
                    bmount(argv[1]);

                    // Crear el directorio del proceso añadiendo el PID al nombre
                    char directorioProceso[2000];
                    memset(directorioProceso, 0, sizeof(directorioProceso));
                    sprintf(directorioProceso, "%sproceso_%d/", camino, getpid());
                    strcat(directorioProceso,"\0");
                    mi_creat(directorioProceso, 6); 
                    char ficheroProceso[3000];
                    memset(ficheroProceso, 0, sizeof(ficheroProceso));
                    sprintf(ficheroProceso, "%sprueba.dat", directorioProceso);
                    strcat(ficheroProceso,"\0");
                    mi_creat(ficheroProceso, 6);

                    //inicializamos la semilla
                    srand(time(NULL) + getpid());
                    for (int nEscritura = 1; nEscritura <= NUMESCRITURAS; nEscritura++)
                    {
                        struct REGISTRO registro;
                        registro.fecha = time(NULL);
                        registro.pid = getpid();
                        registro.nEscritura = nEscritura;
                        registro.nRegistro = rand() % REGMAX;
                        mi_write(ficheroProceso, &registro, (registro.nRegistro) * sizeof(registro), sizeof(registro));
                        
                        my_sleep(50);
                        if(proceso==1){
                            printf("\x1b[31m[simulación.c → Escritura %d en %s]\x1b[41m\x1b[0m\n", nEscritura, ficheroProceso);
                        }else if(proceso==2){
                            printf("\x1b[36m[simulación.c → Escritura %d en %s]\x1b[46m\x1b[0m\n", nEscritura, ficheroProceso);
                        }else{
                            printf("[simulación.c → Escritura %d en %s]\n", nEscritura, ficheroProceso);
                        }
                        
                    }
                    if(proceso==1){
                        printf("\x1b[31m[Proceso %d: Completadas %d escrituras en %s]\x1b[41m\x1b[0m\n",proceso,NUMESCRITURAS,ficheroProceso);
                    }else if(proceso==2){
                        printf("\x1b[36m[Proceso %d: Completadas %d escrituras en %s]\x1b[46m\x1b[0m\n",proceso,NUMESCRITURAS,ficheroProceso);
                    }else if(proceso==3){
                        printf("\x1b[37m[Proceso %d: Completadas %d escrituras en %s]\x1b[47m\x1b[0m\n",proceso,NUMESCRITURAS,ficheroProceso);
                    }else if(proceso==4){
                        printf("\x1b[39m[Proceso %d: Completadas %d escrituras en %s]\x1b[49m\x1b[0m\n",proceso,NUMESCRITURAS,ficheroProceso);
                    }else{
                        printf("[Proceso %d: Completadas %d escrituras en %s]\n",proceso,NUMESCRITURAS,ficheroProceso);
                    }
                    printf("\n[Proceso %d: Completadas %d escrituras en %s]\n",proceso,NUMESCRITURAS,ficheroProceso);
                    bumount();
                    exit(0);
                }
                // Esperar 0,15 seg para lanzar siguiente proceso
                my_sleep(150);
            }
            // Permitir que el padre espere por todos los hijos:
            // Mientras acabados < NUMPROCESOS hacer
            //   pause();
            // fmientras
            while (acabados < NUMPROCESOS)
            {
                pause();
            }
        }
        bumount();
        exit(0);
    }
    else
    {
        printf("Sintaxis: ./simulacion <disco>\n");
    }
}

void my_sleep(unsigned msec)
{ // recibe tiempo en milisegundos
    struct timespec req, rem;
    int err;
    req.tv_sec = msec / 1000;              // conversión a segundos
    req.tv_nsec = (msec % 1000) * 1000000; // conversión a nanosegundos
    while ((req.tv_sec != 0) || (req.tv_nsec != 0))
    {
        if (nanosleep(&req, &rem) == 0)
            // rem almacena el tiempo restante si una llamada al sistema
            // ha sido interrumpida por una señal
            break;
        err = errno;
        // Interrupted; continue
        if (err == EINTR)
        {
            req.tv_sec = rem.tv_sec;
            req.tv_nsec = rem.tv_nsec;
        }
    }
}

void reaper()
{
    pid_t ended;
    signal(SIGCHLD, reaper);
    while ((ended = waitpid(-1, NULL, WNOHANG)) > 0)
    {
        acabados++;
    }
}
