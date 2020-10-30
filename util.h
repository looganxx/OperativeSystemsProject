#include <stdio.h>
#include <stdlib.h>


//***define comuni a server e client***
#define UNIX_PATH_MAX 108
#define DOMAIN AF_UNIX
#define SOCKNAME "./objstore.sock"

//define per le varie stringhe di supporto create
#define N 60

//***define server***

//define per le chiamate di sistema SERVER
#define SCALL(r, c, e, f)  \
  if ((r = c) < 0)         \
  {                        \
    perror(e);             \
    safe_exit(fd_c, name); \
    free(f);               \
    return (void *)-1;     \
  }

//chiamate di sistema per il main per differenziare il return
#define MCALL(r, c, e, f) \
  if ((r = c) < 0)        \
  {                       \
    perror(e);            \
    free(f);              \
    return EXIT_FAILURE;  \
  }

//grandezza della stringa creata per memorizzare il path della cartella data
#define CWD 200
//define per la creazione dei thread
#define CREATE_TH(r, c, e, f) \
  if ((r = c) != 0)           \
  {                           \
    perror(e);                \
    free(f);                  \
    exit(errno);              \
  }

//define di disconnessione nei casi di KO con 'e' che rappresenta la descrizione del messaggio di errore (SERVER)
#define SDISCONNECTED(e)                        \
  if ((r = write(*fd_c, e, strlen(e))) < 0) \
  {                                             \
    perror("error in DISCONNECTED");            \
    safe_exit(fd_c, name);                      \
    return (void *)-1;                          \
  }

//define chiamata nel caso r=0 che segnala la chiusura del socket da parte dell'utene
//prevede la morte del thread
#define CLIENTDOWN               \
  perror("client disconnected"); \
  safe_exit(fd_c, name);        \
  return (void *)-1;

//define per una chiamata che in caso di errore usa la disconnect che manda un KO message al client (libreria server)
#define RETCALL(r, c, e) \
  if ((r = c) < 0)       \
  {                      \
    DISCONNECTED(e);    \
    return -1;           \
  }
//define per una chiamata di sistema che non manda un messaggio di KO al client (libreria server)
#define CALL(r, c, e, f) \
  if ((r = c) < 0)       \
  {                      \
    perror(e);           \
    free(f);             \
    return -1;           \
  }
//define di disconnessione manda un KO e ritorna -1 (libreria server)
#define DISCONNECTED(e)                        \
  if ((r = write(fd_c, e, strlen(e))) < 0) \
  {                                            \
    perror("error in DISCONNECTED");           \
    return -1;                                 \
  }

//***define client***

//RETRIEVE CALL - la funzione torna un void* quindi serve tornare NULL
#define RCALL(r, c, e, f) \
  if ((r = c) < 0)        \
  {                       \
    perror(e);            \
    free(f);              \
    return NULL;          \
  }

//define per una chiamata di sistema da parte del client per return -1
#define CCALL(r, c, e, f) \
  if ((r = c) < 0)        \
  {                       \
    perror(e);            \
    free(f);              \
    return 0;            \
  }

/**
 * @function Malloc
 * @brief malloc con controllo dell'errore
 *
 * @param size = byte di memoria da allocare
 *
 * @returns puntatore al blocco di memoria allocato
 */
void* Malloc (size_t size);