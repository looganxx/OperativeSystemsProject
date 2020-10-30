#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <sys/un.h>

//libreria che raccoglie tutte le funzioni del client
#include "access.h"
//libreria per define e chiamate di sistema
#include "util.h"

//lunghezza min data
#define LEN 100
//lunghezza da aggiungere ogni volta
#define SUM 4995

int main(int argc, char const *argv[]) {
  //controlli sulle funzioni passate da bash
  if(argc != 3){
    fprintf(stderr, "use %s name_user n_battery_test[1-3] \n", argv[0]);
    return EXIT_FAILURE;
  }
  int battery = atoi(argv[2]);
  char *name = (char *) argv[1];
  if(battery<1 || battery>3){
    fprintf(stderr, "use n_battery_test[1-3] \n");
    return EXIT_FAILURE;
  }

  char *nome_file = Malloc(sizeof(char)*7);

  int op_richieste = 1, op_success = 0; 
  if(os_connect(name)<1){
    fprintf(stdout, "op richieste:%d\nop con successo:%d\nop fallite:%d\nclient:%s-%d\n\n", op_richieste, op_success, (op_richieste - op_success), name, battery);
    return(EXIT_FAILURE);
  }
  op_success++;
  char c;
  int i = 0, r;
  void *obj;

  i = 0, r=1;
  switch (battery){
    case 1: while(i<20){
              char *s = Malloc(sizeof(char)*(LEN+(SUM*i)+1));
              memset(s, 0, (LEN + (SUM * i) + 1));
              sprintf(nome_file, "%d", LEN + (SUM * i));
              c = '0' + i;
              for (int j = 0; j < (LEN + (SUM * i)); j++){
                s[j] = c;
              }
              r = os_store(nome_file, (void*) s, (LEN + (SUM * i)));
              free(s);
              op_richieste++;
              if(r==1) op_success++;
              i++;
            }
            break;
    case 2: if ((obj = os_retrieve("15085")) != NULL){
              op_success++;
              r = 1;
            }else{
              r=0;
            }
            op_richieste++;
            free(obj);
            break;
    case 3: r = os_delete("100");
            op_richieste++;
            if(r==1) op_success++;
            break;
  }
  free(nome_file);
  if(os_disconnect()<=0) {
    op_richieste++;
    fprintf(stdout, "op richieste:%d\nop con successo:%d\nop fallite:%d\nclient:%s-%d\n\n", op_richieste, op_success, (op_richieste - op_success), name, battery);
    return(EXIT_FAILURE);
  }
  op_richieste++;
  op_success++;
  if(op_richieste-op_success == 0){
    fprintf(stdout, "op richieste:%d\nop con successo:%d\nop fallite:%d\n\n", op_richieste, op_success, (op_richieste-op_success));
  }else{
    fprintf(stdout, "op richieste:%d\nop con successo:%d\nop fallite:%d\nclient:%s-%d\n\n", op_richieste, op_success, (op_richieste - op_success), name, battery);
  }
  return 0;
}
