#if !defined(size_t)
#define size_t unsigned long
#endif

/**
 * @function os_connect
 * @brief creazione del socket
 *        richiesta di registrazione e attesa di risposta
 * @param name = nome del client che vuole effettuare la registrazione
 * @results restituisce 1 se va tutto bene
 *          restituisce 0 se c'è stato un errore durante il processo o se non è stata accettata la registrazione
 */
int os_connect(char *name);

/**
 * @function os_store
 * @brief manda l'header di testo chiedendo il salvataggio di block
 *        legge l'esito del server e lo stampa
 * @param name = nome del dato da salvare
 *        block = valore del dato da salvare
 *        len = lunghezza in byte di block
 * @results restituisce 1 se va tutto bene
 *          restituisce 0 se c'è stato un errore durante il processo
 */
int os_store(char *name, void *block, size_t len);

/**
 * @function os_retrieve
 * @brief richiede un dato da ricevere
 *         legge l'esito del server e restituisce un puntatore al dato ricevuto
 * @param name = nome del dato che si vuole ricevere
 * @results restituisce il puntatore al dato se è andato tutto bene
 *          restituisce NULL se c'è stato un errore durante il processo o se non è stato trovato il dato
 */
void  *os_retrieve(char  *name);

/**
 * @function os_delete
 * @brief richiede la cancellazione di un dato dallo store
 * @param name = nome del dato da cancellare
 * @results restituisce 1 se va tutto bene
 *          restituisce 0 se c'è stato un errore durante il processo o se non è stata accettata la cancellazione del dato
 */
int  os_delete(char  *name);

/**
 * @function os_disconnect
 * @brief richiede la disconnessione dal server
 * @results restituisce 1 se va tutto bene
 *          restituisce 0 se c'è stato un errore durante il processo
 */
int os_disconnect();
