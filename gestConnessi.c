#include "gestConnessi.h"

//variabile di mutua esclusione per l'accesso alla lista
static pthread_mutex_t mux_lista = PTHREAD_MUTEX_INITIALIZER;

void lockL()
{
  pthread_mutex_lock(&mux_lista);
}

void unlockL()
{
  pthread_mutex_unlock(&mux_lista);
}

user *createL(){
  user *head = NULL;
  return head;
}

void insertL(user **head, char *name){
  user *new = (user*) malloc(sizeof(user));
  strcpy(new->name, name);
  new->next = NULL;

  if (*head == NULL){
    *head = new;
  }else{
    new->next = *head;
    *head = new;
  }
  connessi++;
}

void removeL(user **head, char *name){
  user *prev;
  user *curr;
  prev = curr = *head;
  while(curr!=NULL && (strcmp(name, curr->name)!=0)){
    prev = curr;
    curr = curr->next;
  }
  if(curr!=NULL){
    if(*head == curr)*head = (*head)->next;
    else prev->next = curr->next;
    free(curr);
  }
  connessi--;
}

void destroyL(user **head){
  if(*head!=NULL){
    user *tmp = *head;
    *head = (*head)->next;
    free(tmp);
    destroyL(head);
  }
  connessi = 0;
}

int checkinL(user *head, char *name){
  while (head != NULL && (strcmp(name, head->name) != 0)){
    head = head->next;
  }
  if(head == NULL) return 0;
  else return 1;
}