#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

//struct per la gestione degli utenti connessi nella coda
typedef struct user_{
  char name[20];
  struct user_ *next;
} user;

//variabile che tiene il conto dei client connessi
static int connessi = 0;

/**
 * @function lockL
 * @brief acquisizione del lucchetto per garantire la mutua esclusione
 * @results lucchetto ottenuto
 */
void lockL();

/**
 * @function unlockL
 * @brief rilascio del lucchetto
 * @results lucchetto rilasciato
 */
void unlockL();

/**
 * @function createL
 * @brief creazione del puntatore alla lista degli utenti attivi
 * @results restituisce un puntatore alla testa della lista
 */
user *createL();

/**
 * @function checkinL
 * @brief controlla se il client appartiene alla lista o no
 * @param head = puntatore alla testa della lista
 *        name = nome del client da controllare
 * @results restituisce 1 se l'utente è già in lista
 *          restituisce 0 se non è connesso
 */
int checkinL(user *head, char *name);

/**
 * @function insertL
 * @brief inserisce in testa il client che si è connesso
 * @param head = puntatore alla testa della lista da modificare
 *        name = nome del client da inserire
 * @results utente inserito in testa
 */
void insertL(user **head, char *name);

/**
 * @function removeL
 * @brief rimuove dalla lista l'utente
 * @param head = puntatore alla testa della lista da modificare
 *        name = nome del client da rimuovere
 * @results utente rimosso dalla lista
 */
void removeL(user **head, char *name);

/**
 * @function destroyL
 * @brief elimina la lista liberando la memoria
 * @param head = puntatore alla testa della lista da modificare
 * @results lista eliminata
 */
void destroyL(user **head);