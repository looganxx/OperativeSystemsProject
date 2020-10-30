/*
le funzioni tornano al thread chiamante:
0 se c'è stata una read o write = 0, quindi client disconnesso
-1 se c'è stato un errore in uno dei vari passaggi della funzione
1 se è andato tutto a buon fine
*/

/**
 * @function sep_name
 * @brief dal messaggio buf separa la richiesta dal nome
 *        viene controllata se request==REGISTER
 * @param buf = valore letto dalla read
 *        name = stringa sulla quale copiare il nome del client
 *        request = stringa sulla quale copiare il tipo di richiesta 
 *        fatta (in questo caso si controlla sia una register)
 * 
 * @returns se request == REGISTER torna true(1)
 *          altrimenti torna false(0)
 */
int sep_name(char* buf, char* name, char* request);

/**
 * @function check_user
 * @brief nella directory ./data cerco il client con nome 'name'
 *        se non lo trovo creo la directory accedo alla directory del client
 * 
 * @param name = nome del client
 *        fd_c = socket per la comunicazione tra thread e client
 * 
 * @returns se tutto ok return 1 (registrazione effettuata con successo) 
 *          altrimenti return -1 se c'è stato un errore durante il processo 
 *          o 0 se il client si è disconnesso
 */
int check_user(char* name, int fd_c);

/**
 * @function take_request
 * @brief viene separata dalla richiesta il resto dell'header
 *        viene controllato che la richiesta sia diversa da REGISTER (arrivati a questa funzione
 *        il client sarà gia connesso)
 * 
 * @param buf = stringa contenente l'header completo
 *        request = stringa sulla quale copiare il tipo di richiesta
 *        rest = stringa sulla quale allocare il resto dell'header (varia da richiesta a richiesta)
 * 
 * @returns se request==REGISTER ritorna false(0) 
 *          altrimenti torna vero (1)
 */
int take_request(char* buf, char* request, char* rest);

/**
  * @function server_store
  * @brief rest è il risultato della precedente chiamata di funzione "take request"
  *         da rest vengono acquisiti tutti i vari parametri tra cui il nome del dato e la sua lunghezza
  *         successivamente vengono letti dal buffer il valore del dato da salvare 
  *         viene creato o aperto un file se già esistente controllo se in sovrascrittura 
  *         salvataggio del valore viene mandato il messaggio di uscita al client
  * 
  * @param usr = nome del client che richiede l'operazione di store
  *        rest = in questo caso contiene: "name len \n"
  *        fd_c = socket per la comunicazione tra thread e client
  * 
  * @returns restituisce 1 se va tutto bene 
  *          restituisce 2 se il file già esisteva e viene sovrascritto, 
  *                        in modo di non incrementare il numero di oggetti nel server
  *          altrimenti -1 se c'è stato un errore durante il processo
  *          o 0 se il client si è disconnesso (viene letto 0 dalla read)
 */
int serv_store(char* usr, char* rest, int fd_c);

/**
 * @function serv_retrieve
 * @brief nel caso retrieve la stringa rest conterrà solo il nome del file 
 *        viene trovato il dato, e creato il messaggio da mandare al client
 * 
 * @param usr = nome del client che richiede l'operazione di retrieve
 *        name = nel caso della retrieve rest sarà solo il nome del file da restituire
 *        fd_c = socket per la comunicazione tra thread e client
 * 
 * @returns restituisce 1 se va tutto bene 
 *          restituisce -1 se c'è stato un errore durante il processo
 *          o 0 se il client si è disconnesso (viene letto 0 dalla read)
 */
int serv_retrieve(char *usr, char *name, int fd_c);

/**
 * @function serv_delete
 * @brief nel caso delete la stringa rest conterrà solo il nome del file da eliminare 
 *        viene eliminato il file e mandato il messaggio di uscita al client
 * 
 * @param usr = nome del client che richiede l'operazione di delete
 *        name = nel caso della delete rest sarà solo il nome del file da eliminare
 *        fd_c = socket per la comunicazione tra thread e client
 * 
 * @returns restituisce 1 se va tutto bene 
 *          altrimenti -1 se c'è stato un errore durante il processo 
 *          o 0 se il client si è disconnesso (viene letto 0 dalla read)
 */
int serv_delete(char *usr, char *name, int fd_c);