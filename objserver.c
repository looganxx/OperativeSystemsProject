#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>
#include <dirent.h>
#include <signal.h>
#include <pthread.h>

//libreria che raccoglie tutte le funzioni del server
#include "server_op.h"
//libreria che gestisce chiamate ad una lista che raccoglie tutti gli utenti connessi al sistema
#include "gestConnessi.h"
//libreria per define e chiamate di sistema
#include "util.h"

//enum per le varie richieste che l'utente può fare dopo una REGISTER
//viene usato nell switch del thread worker
typedef enum {STORE=1, RETRIEVE, DELETE, LEAVE} request_;

//variabili globali
char path[CWD]; //path della directory data
user *userConnected; //puntatore all'inizio della lista dei clienti connessi, condiviso tra i thread

//var signal

//modificata dall'handler di terminazione, usata come flag per gestire la terminazione del main
volatile sig_atomic_t termination = 0;

//modificata dall'handler state, usata come flag per gestire la stampa dello stato del server
volatile sig_atomic_t state = 0;

//var condivisa tra thread
static int attivi = 0; //conta il numero di clienti attivi e presenti nella lista dei clienti connessi
static unsigned int obj = 0; //conta il numero di oggetti presenti
extern unsigned int size_store; //dichiarata in server_op per tenere conto della grandezza dello store

//gestione thread
static pthread_mutex_t mux = PTHREAD_MUTEX_INITIALIZER; //usata per la variabile condivisa 'attivi'
//usata per far aspettare il main fin quando non finiscono tutti i thread attivi dopo un segnale di terminazione
static pthread_cond_t th_main = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t mux_obj = PTHREAD_MUTEX_INITIALIZER; //per gestire il numero di oggetti presenti nello store

//quando riceve un segnale di terminazione mette il flag a 1
static void handler_termination(int signum){
  termination = 1;
}
//quando viene mandato SIGUSR1 mette il flag a 1
static void handler_state(int signum){
  state = 1;
}

/*
gestione di uscita sicura del thread
chiude il socket, decrementa il numero di thread attivi per i client
e se presente nella lista degli utenti connessi rimuove l'utente di client 'name'
*/
void safe_exit(int *fd_c, char *name){
  close(*fd_c);
  free(fd_c);
  pthread_mutex_lock(&mux);
  attivi--;
  pthread_cond_signal(&th_main);
  pthread_mutex_unlock(&mux);
  if (name != NULL){
    lockL();
    removeL(&userConnected, name);
    unlockL();
  }
}
//funzione per il riconoscimento della richiesta del client da usare nello switch
//check è la variabile usata nello switch che viene modificata all'interno della funzione
void checkRequest(request_ *check, char* request){
  if(strcmp("STORE", request)==0) *check = STORE;
  else 
    if (strcmp("RETRIEVE", request) == 0) *check = RETRIEVE;
    else 
      if (strcmp("DELETE", request) == 0) *check = DELETE;
        else 
          if (strcmp("LEAVE", request) == 0) *check = LEAVE;
          else *check = -1;
}

//thread worker
static void* worker(void* arg){
  int *fd_c = (int*) arg;
  int r;
  char *buf = Malloc(sizeof(char) * N);
  char request[N];
  char name[N];
  char rest[N];

  //aumento attivi
  pthread_mutex_lock(&mux);
  attivi++;
  pthread_mutex_unlock(&mux);

  //leggo la prima richista (REGISTER)
  int i = 0, stop = 0;
  while (i < N && !stop){
    SCALL(r, read(*fd_c, &buf[i], 1), "reading request to connect", buf);
    if (r == 0){CLIENTDOWN}
    if (buf[i] == '\n')
      stop = 1;
    i++;
  }

 
  //se la richiesta fatta == REGISTER torna true e modifica request e nome
  r = sep_name(buf, name, request);
  free(buf);
  //se il socket è chiuso avviene una disconnessione
  if(r == 0){
    memset(name, 0, N);
    SDISCONNECTED("KO no login \n");
  }
  //controllo se l'utente che ha chiesto la registrazione è già connesso
  if (checkinL(userConnected, name)){
    memset(name, 0, N);
    SDISCONNECTED("KO user already connected \n");
  }
  //inserisco l'utente nella lista
  lockL();
  insertL(&userConnected, name);
  unlockL();
  //controllo se esiste già la directory relativa a quell'utente, 
  //se non c'è viene creata e viene fatto l'accesso a quella directory
  if(check_user(name, *fd_c)<=0) {
    perror("creating dir");
    safe_exit(fd_c, name);
    return (void*) -1;
  }
  //operazioni
  while(!termination && r != 0){
    char *buf = Malloc(sizeof(char)*N);
    char request[N];
    char rest[N];
    char c; //serve per leggere un byte a vuoto per lo spazio dopo \n nel caso di una store
    memset(buf, 0,N);
    memset(request,0,N);
    memset(rest, 0, N);
    int i = 0, stop = 0;
    while (i < N && !stop){
      SCALL(r, read(*fd_c, &buf[i], 1), "reading request to connect", buf);
      if (r == 0){CLIENTDOWN}
      if (buf[i] == '\n')
        stop = 1;
      i++;
    }
    //buf = request rest \n

    //torna false se viene richiesta nuovamente la registrazione
    if((r = take_request(buf, request, rest)) == 0 ){SDISCONNECTED("KO login alredy done \n")}

    //switch con i vari case
    request_ check;
    //controllo quale richiesta viene fatta modificando la variabile check per entrare nel giusto case
    checkRequest(&check, request);
    switch(check){
      case 1: 
        //read vuota per lo spazio
        if ((r = read(*fd_c, &c, 1)) < 0) { 
          perror("reading request"); 
          return (void *)-1; 
        }
        r = serv_store(name, rest, *fd_c);
        if (r == 1){
          pthread_mutex_lock(&mux_obj);
          obj++;
          pthread_mutex_unlock(&mux_obj);
        }else if(r == 0){CLIENTDOWN}
        break;
      case 2:
        r = serv_retrieve(name, rest, *fd_c);
        break;
      case 3:
        r = serv_delete(name, rest, *fd_c);
        if (r == 1)
        {
          pthread_mutex_lock(&mux_obj);
          obj--;
          pthread_mutex_unlock(&mux_obj);
        }else if (r == 0){CLIENTDOWN}
          break;
      case 4: if ((r = write(*fd_c, "OK \n", 4)) < 0) { 
                perror("in leave"); 
                return (void *)-1; 
              }
              r = 0;
              break;
      default: SDISCONNECTED("KO operazione impossibile \n");
    }
    free(buf);
  }
  //dopo una richiesta di leave viene fatta una safe exit e poi la terminazione del thread
  safe_exit(fd_c, name);
  return (void*) 0;
}

int main(int argc, char const *argv[]){
    int fd_sock, fd_client, r;
    int err, status;
    char buf[N];
    struct sockaddr_un sa;

    //per la gestione della terminazione con i segnali SIGINT e SIGSTOP
    struct sigaction s_terminazione;
    memset(&s_terminazione, 0, sizeof(s_terminazione));
    s_terminazione.sa_handler = handler_termination;
    sigaction(SIGINT, &s_terminazione, NULL);
    sigaction(SIGTSTP, &s_terminazione, NULL);
    //per la gestione della stampa dei dati del serer quando riceve un SIGUSR1
    struct sigaction s_state;
    memset(&s_state, 0, sizeof(s_state));
    s_state.sa_handler = handler_state;
    sigaction(SIGUSR1, &s_state, NULL);

    //per la creazione del socket
    strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);
    sa.sun_family = DOMAIN;
    fd_sock = socket(DOMAIN, SOCK_STREAM, 0);
    bind(fd_sock, (struct sockaddr *) &sa, sizeof(sa));
    listen(fd_sock, SOMAXCONN);
    //creazione della lista degli utenti connessi
    userConnected = createL();
    if ((r = mkdir("data", 0700)) < 0){
      perror("creating directory data");
      return -1;
    }
    if (chdir("data") == -1){
      perror("chdir start path");
      return -1;
    }
    //salvataggio del path di data nella variabile globale path di lunghezza CWD
    char *path_data = getcwd(path, CWD);

    while(!termination){ 
      //se il flag state viene messo a 1 dall'handler vengono stampati i dati richiesti
      if(state){
        char *c_attivi = Malloc(sizeof(char) * N);
        sprintf(c_attivi, "%d", attivi);
        strcat(c_attivi, " client attivi\n");

        char *n_obj = Malloc(sizeof(char) * N);
        sprintf(n_obj, "%u", obj);
        strcat(n_obj, " oggetti presenti\n");

        char *data_size = Malloc(sizeof(char) * N);
        sprintf(data_size, "%u", size_store);
        strcat(data_size, " B nello store\n");

        strncat(c_attivi, n_obj, N);
        strncat(c_attivi, data_size, N);

        MCALL(r, write(1, c_attivi, strlen(c_attivi)), "write state", c_attivi);

        state = 0;
        free(c_attivi);
        free(n_obj);
        free(data_size);
      }
      //accettazione di un nuovo cliente con la consecutiva creazione del thread dedicato a gestire le richieste del cliente
      if((fd_client = accept(fd_sock, NULL, 0))<0){
        perror("accept");
      }else{
        int *new_sock = Malloc(sizeof(int)); //libreria di controllo
        *new_sock = fd_client;
        pthread_t th_worker;
        CREATE_TH(err, pthread_create(&th_worker, NULL, &worker, (void *)new_sock), "creating thread worker", new_sock);
        //viene usata la detach così da liberare subito memeria non appena il thread finisce la sua esecuzione
        pthread_detach(th_worker);
      }
    }
    //se alla ricezione del segnale di terminazione ci sono thread in esecuzione, si aspetta la loro terminazione 
    pthread_mutex_lock(&mux);
    while (attivi != 0)
      pthread_cond_wait(&th_main, &mux);
    pthread_mutex_unlock(&mux);
    //cancellazione della lista dei clienti attivi
    destroyL(&userConnected);
    return 0;
}