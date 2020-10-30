#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>

//libreria per define e chiamate di sistema
#include "util.h"

int fd_c;

//serve per tokenizzare il messaggio ricevuto dal server
//torna 0 se c'è un KO, 1 altrimenti
int check_result(char *appo){
  char *token = strtok(appo, " ");
  if (strcmp(token, "KO") == 0){
    return 0;
  }
  return 1;
}

int os_connect(char *name){
  int r;

  struct sockaddr_un sa;
  strncpy(sa.sun_path, SOCKNAME,UNIX_PATH_MAX);
  sa.sun_family = DOMAIN;
  fd_c = socket(DOMAIN, SOCK_STREAM, 0);
  while (connect(fd_c,(struct sockaddr*)&sa,sizeof(sa)) == -1 ) {
      if ( errno != ENOENT ){
        perror("no connect");
        exit(EXIT_FAILURE);
      }
  }

  char *request = Malloc(sizeof(char) * (13 + strlen(name)));
  strcpy(request, "REGISTER ");
  strcat(request, name);
  strcat(request, " \n");
  CCALL(r, write(fd_c, request, strlen(request)), "header REGISTER", request);
  if(r==0){
    close(fd_c);
    perror("server disconnected");
    return 0;
  }
  free(request);
  char *res = malloc(sizeof(char)*N);
  memset(res, 0, N);
  CCALL(r, read(fd_c, res, N), "read response", res);
  if(r==0){
    free(res);
    close(fd_c);
    perror("server disconnected");
    return 0;
  }
  if(strcmp(res, "OK \n") == 0){
    free(res);
    return 1;
  }
  CCALL(r, write(1, res, strlen(res)), "read response", res);
  free(res);
  return 0;
}

int os_store(char *name, void *block, size_t len){
  int r;
  char* len_conv = Malloc(sizeof(char)*7);
  sprintf(len_conv, "%ld", len);
  char* request = Malloc(sizeof(char)*(11+strlen(name)+strlen(len_conv)));
  strcpy(request, "STORE ");
  strcat(request, name);
  strcat(request, " ");
  strcat(request, len_conv);
  strcat(request, " \n ");
  int left = (10+strlen(name)+strlen(len_conv)); 
  free(len_conv);
  while(left>0){
    CCALL(r, write(fd_c, request, left), "header STORE", request);
    if(r==0){
      free(request);
      close(fd_c);
      perror("server disconnected");
      return 0;
    }
    left = left-r;
  }

  free(request);
  left = len;
  while(left>0){
    if ((r = write(fd_c, block, left)) < 0){
      perror("header STORE");
      return 0;
    }
    if (r == 0){
      close(fd_c);
      perror("server disconnected");
      return 0;
    }
    left = left - r;
  }
  char *res = malloc(sizeof(char)*N);
  memset(res, 0, N);
  CCALL(r, read(fd_c, res, N), "read response", res);
  if(r==0){
    free(res);
    close(fd_c);
    perror("server disconnected");
    return 0;
  }
  if(strcmp(res, "OK \n") == 0){
    free(res);
    return 1;
  }
  CCALL(r, write(1, res, strlen(res)), "read response", res);
  free(res);
  return 0;
}

void *os_retrieve(char *name){
  int r, len;
  char* request = Malloc(sizeof(char)*(strlen(name)+ 12));
  strcpy(request, "RETRIEVE ");
  strcat(request, name);
  strcat(request, " \n");
  RCALL(r, write(fd_c, request, (strlen(name) + 11)), "header RETRIEVE", request);
  free(request);
  if (r == 0){
    close(fd_c);
    perror("server disconnected");
    return NULL;
  }
  int i = 0, stop = 0;
  char* res = malloc(sizeof(char)*N);
  memset(res, 0, N);
  while (i < N && !stop){
    RCALL(r, read(fd_c, &res[i], 1), "reading answer", res);
    if (r == 0){
      free(res);
      close(fd_c);
      perror("server disconnected");
      return NULL;
    }
    if (res[i] == '\n')
      stop = 1;
    i++;
  }
  char *appo = Malloc(sizeof(char) * N);
  memset(appo, 0, N);
  memcpy(appo, res, N);
  r = check_result(appo);
  //se la funzione check_result torna zero ritorna NULL
  if (r == 0){
    free(appo);
    RCALL(r, write(1, res, strlen(res)), "read response", res);
    free(res);
    return NULL;
  }
  //leggo a vuoto per lo spazio tra \n e data
  char c;
  if ((r = read(fd_c, &c, 1)) < 0){
    free(res);
    perror("reading request to connect");
    return NULL;
  }
  if(r == 0){
    free(res);
    close(fd_c);
    perror("server disconnected");
    return NULL;
  }

  char *token = strtok(res, " \n");
  len = atoi(strtok(NULL, " \n"));
  free(res);
  void *data = Malloc(len + 1);
  memset(data, 0 , len+1);
  int left = len;
  //read dal buffer contenente il dato
  while(left>0){
    RCALL(r, read(fd_c, data, left), "reading data", data);
    if(r==0){
      close(fd_c);
      perror("server disconnected");
      return NULL;
    }
    left = left-r;
  }
  return data;
}

int os_delete(char *name){
  int r;
  char* request = Malloc(sizeof(char)*(strlen(name)+10));
  strcpy(request,"DELETE ");
  strcat(request, name);
  strcat(request, " \n");
  CCALL(r, write(fd_c, request, (strlen(name) + 9)), "header DELETE", request);
  free(request);
  if(r==0){
    close(fd_c);
    perror("server disconnected");      
    return 0;
  }
  char *res = malloc(sizeof(char)*N);
  memset(res, 0 , N);
  CCALL(r, read(fd_c, res, N), "read response", res);
  if (r == 0){
    free(res);
    close(fd_c);
    perror("server disconnected");
    return 0;
  }
  if (strcmp(res, "OK \n") == 0){
    free(res);
    return 1;
  }
  CCALL(r, write(1, res, strlen(res)), "read response", res);
  free(res);
  return 0;
}

int os_disconnect(){
  int r;
  char* request = Malloc(sizeof(char)*8);
  strcpy(request, "LEAVE \n");
  CCALL(r, write(fd_c, request, 7), "header LEAVE", request);
  free(request);
  if(r==0){
    close(fd_c);
    perror("server disconnected");
    return 0;
  }
  char *res = malloc(sizeof(char)*N);
  memset(res, 0, N);
  CCALL(r, read(fd_c, res, N), "read response", res);
  if(r==0){
    free(res);
    close(fd_c);
    perror("server disconnected");
    return 0;
  }
  if (strcmp(res, "OK \n") == 0){
    free(res);
    close(fd_c);
    return 1;
  }
  CCALL(r, write(1, res, strlen(res)), "read response", res);
  free(res);
  close(fd_c);
  return 1;
}
