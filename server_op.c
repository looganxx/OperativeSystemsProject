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
#include <fcntl.h>
#include <pthread.h>

//libreria per define e system call
#include "util.h"

//mutex e variabile globale per il conteggio della grandezza dei dati presenti nello store
unsigned int size_store = 0;
static pthread_mutex_t mux_size = PTHREAD_MUTEX_INITIALIZER;

int sep_name(char* buf, char* name, char* request){
  //tail usata per la coda della strtok_r
  char *tail;
  //appo usata per catturare il risultato della strtok_r ad ogni passaggio
  char *appo = strtok_r(buf, " \n", &tail);
  strcpy(request, appo);
  if(strcmp(request, "REGISTER")!=0){return 0;}
  appo = strtok_r(NULL, " \n", &tail);
  strcpy(name, appo);
  //potrebbe esserci un mem leak
  return 1;
}

int check_user(char* name, int fd_c){
  DIR *mainfolder;
  struct dirent *dir;
  int found = 0, r;
  
  if ((mainfolder = opendir(".")) == NULL) {
    perror("./data not present");
    DISCONNECTED("KO opening dir \n");
  }
  while ((errno = 0, dir = readdir(mainfolder)) != NULL && !found){
    if (strcmp(dir->d_name, name) == 0)
      found = 1;
  }
  if (errno != 0){
    DISCONNECTED("KO read dir in data\n");
  }
  if (!found){
    RETCALL(r, mkdir(name, 0700), "KO creating usr dir \n");
  }
  if ((closedir(mainfolder) == -1)){
    perror("closing dir");
    DISCONNECTED("KO close dir usr \n");
  }
  if ((r = write(fd_c, "OK \n", 5)) < 0){
    perror("write answer");
    return -1;
  }
  if (r == 0)
    return 0;
  return 1;
}

int take_request(char* buf, char* request, char* rest){
  char *appo;
  char *help;
  appo = strtok_r(buf, " \n", &help);
  strcpy(request, appo);
  if (strcmp(request, "REGISTER") == 0)
    return 0;
  strcpy(rest, help);
  return 1;
}

int serv_store(char *usr, char *rest, int fd_c){
  //rest = name len \n
  char *name;
  char *tail;
  int len, r=0, overwrite = 0;
  void *data;
  name = strtok_r(rest, " \n", &tail);
  len = atoi(strtok_r(NULL, " \n", &tail));
  data = Malloc(len);
  int left = len;
  //read dal buffer contenente il dato
  while(left>0){
    CALL(r, read(fd_c, data, left), "reading data", data);
    if(r == 0){
      free(data);
      return 0;
    }
    left = left-r;
  }

  char path[CWD];
  strcpy(path, usr);
  strcat(path, "/");
  strcat(path, name);
  int fd;
  RETCALL(fd, open(path, O_RDWR | O_CREAT, 0666), "KO creating file \n");
  //controllo se in sovrascrittura
  int len_file = lseek(fd, 0, SEEK_END);
  if(len_file!=0){
    overwrite = 1;
    RETCALL(r, unlink(path), "KO clearing \n");
    RETCALL(r, close(fd), "KO closing file \n");
    RETCALL(fd, open(path, O_RDWR | O_CREAT, 0666), "KO creating file \n");
  }
  //se in sovrascrittura si controlla se il numero di byte totale dello store diminuisce o incrementa
  len_file = len-len_file;
  if((r=write(fd, data, len))<0){
    free(data);
    DISCONNECTED("KO writig on file \n")
  }  
  free(data);
  pthread_mutex_lock(&mux_size);
  size_store += len_file;
  pthread_mutex_unlock(&mux_size);
  RETCALL(r, close(fd), "KO closing file \n");
  /*if ((r = write(fd_c, "KO sei scemo \n", 14)) < 0)
  {
    perror("answer");
    return -1;
  }*/
  if ((r = write(fd_c, "OK \n", 4)) < 0){
    perror("answer");
    return -1;
  }
  if (r == 0)
    return 0;
  if(overwrite) return 2;
  return 1;
}

int serv_retrieve(char* usr, char* name, int fd_c){
  //name = nome del file da tornare
  char* tail;
  char path[CWD];
  int r, found = 0, len;
  int ifp;
  name = strtok_r(name, " \n", &tail);
  void *res; //res = dato da restituire
  char *answer;
  strcpy(path, usr);
  strcat(path, "/");
  strcat(path, name);
  //solo con la open controllando l'errore sulla open
  RETCALL(ifp, open(path, O_RDONLY), "KO file not present \n");
  len = lseek(ifp, 0, SEEK_END);
  int left = len;
  lseek(ifp, 0, SEEK_SET);
  res = Malloc(len);
  memset(res, 0, len);
  while (left > 0){
    if ((r = read(ifp, res, left)) < 0){
      free(res);
      DISCONNECTED("KO reading file \n")
    }
    left = left - r;
  }
  char *len_conv = Malloc(sizeof(char) * 6);
  sprintf(len_conv, "%d", len);
  answer = Malloc(sizeof(char)*(9+strlen(len_conv)));
  strcpy(answer, "DATA ");
  strcat(answer, len_conv);
  strcat(answer, " \n ");
  left = (8 + strlen(len_conv));
  free(len_conv);
  while(left>0){
    CALL(r, write(fd_c, answer,left), "answer", answer);
    if(r==0)return 0;
    left=left-r;
  }
  left=len;
  while(left>0){
    CALL(r, write(fd_c, res,left), "answer", res);
    if(r==0)return 0;
    left=left-r;
  }
  free(res);
  free(answer);
  RETCALL(r, close(ifp), "KO closing file \n");
  return 1;
}

int serv_delete(char *usr, char* name, int fd_c){
  //name = nome file da eliminare
  DIR *mainfolder;
  char* tail;
  struct dirent *dir;
  int r, found = 0;
  int len, ifp;
  char path[CWD];
  name = strtok_r(name, " \n", &tail);
  strcpy(path, usr);
  strcat(path, "/");
  strcat(path, name);
  RETCALL(ifp, open(path, O_RDONLY), "KO file not present \n");
  len = lseek(ifp, 0, SEEK_END);
  RETCALL(r, unlink(path), "KO clearing \n");
  //fprintf(stderr, "path: %s\n", path);
  RETCALL(r, close(ifp), "KO closing file \n");
  pthread_mutex_lock(&mux_size);
  size_store -= len;
  pthread_mutex_unlock(&mux_size);
  if ((r = write(fd_c, "OK \n", 5)) < 0){
    perror("answer");
    return -1;
  }
  if (r == 0)
    return 0;
  return 1;
}