
/* Client of a client-server application designed to check
 that Comm module works correctly between different machines

 Syntax = client hostname port
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <sys/time.h>

#include "comm.h"
#include "errorTrains.h"
#include "trains.h" // To have message typedef


#define CONNECT_TIMEOUT 2000 // milliseconds
#define CLIENTS_AMOUNT 10
#define ID_REQUEST_MESSAGE "IDREQUEST"
#define ID_REPLY_MESSAGE "IDREPLY:%d"
#define BROADCAST_MESSAGE "WHAT_IS_THE_ANSWER?"

#define TIMING_WARMUP 20
#define TIMING_MEASURE 180
#define TIMING_COOLDOWN 10

void* client_thread(void* args);
int sendMessage(trComm* comm, char* msg_payload);
message* readMessage(trComm* comm, int* nbRead);


struct client_config {
  char**  argv;
  int     threadId;
};

int main(int argc, char *argv[]){
  //On verifie les arguments
  if (argc != 3) {
    fprintf(stderr, "USAGE = %s hostname port\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  //On lance la fonction client_thread CLIENTS_AMOUNT fois
  //chaque fois dans un nouveau thread
  pthread_t thread[CLIENTS_AMOUNT];
  int i;
  for (i = 0; i < CLIENTS_AMOUNT; i++) {
    struct client_config *conf = malloc(sizeof (struct client_config));
    conf->argv = argv;
    conf->threadId = i;
    pthread_create(&thread[i], NULL, &client_thread, (void *) conf);
  }
  //On attend que tous les thread aient fini (jamais)
  pthread_exit(0);
}

void* client_thread(void* conf) {
  struct client_config* config = conf;
  char** argv = config->argv;
  int threadId = config->threadId;

  trComm *commForConnect;
  message *msg;

  int nbRead;
  int clientId;

  struct timeval tvStart, tvSent, tvRecieved, tvNow;
  long long elapsedMicroSeconds;
  bool printMeasure = false;

  gettimeofday(&tvStart, NULL);
  // Connection au serveur
  printf("[THREAD #%d] Connecting %s on port %s...\n", threadId, argv[1], argv[2]);
  commForConnect = commNewAndConnect(argv[1], argv[2], CONNECT_TIMEOUT);
  if (commForConnect == NULL )
    ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__,
        "comm_newAndConnect");
  printf("[THREAD #%d] Connected !\n", threadId);
  //Connecté

  //Envoie ID Request
  printf("\t[THREAD #%d] Sending ID Request to server\n", threadId);
  gettimeofday(&tvSent, NULL);
  sendMessage(commForConnect, ID_REQUEST_MESSAGE);
  //Attend & Lit la réponse du server
  msg = readMessage(commForConnect, &nbRead);
  sscanf(msg->payload, ID_REPLY_MESSAGE, &clientId);
  printf("\t[THREAD #%d][CLIENT #%d] ID Reply from server : %d\n", threadId, clientId, clientId);
  //On calcule et affiche la durée de l'échange d'id
  gettimeofday(&tvRecieved, NULL);
  elapsedMicroSeconds = (tvRecieved.tv_sec-tvSent.tv_sec)*1000000LL + tvRecieved.tv_usec-tvSent.tv_usec;
  printf("\t[THREAD #%d][CLIENT #%d] ID Exchange took : %lld microseconds\n", threadId, clientId, elapsedMicroSeconds);

  //boucle infinie
  while(1) {
    //Log la date
    gettimeofday(&tvSent, NULL);
    //envoie le broadcast
    printf("\t[THREAD #%d][CLIENT #%d] Sending broadcast message...\n", threadId, clientId);
    sendMessage(commForConnect, BROADCAST_MESSAGE);
    //Attend & lit la réponse du serveur
    msg = readMessage(commForConnect, &nbRead);
    //On stock la réponse (TODO gerer les erreurs sscanf)
    int answer;
    sscanf(msg->payload, "%d", &answer);
    printf("\t[THREAD #%d][CLIENT #%d] Answer from server : %d\n", threadId, clientId, answer);
    //On calcule et affiche la durée de l'échange d'id
    gettimeofday(&tvRecieved, NULL);
    elapsedMicroSeconds = (tvRecieved.tv_sec-tvSent.tv_sec)*1000000LL + tvRecieved.tv_usec-tvSent.tv_usec;

    gettimeofday(&tvNow, NULL);
    //cooldown fini, on meurt
    if (tvNow.tv_sec - tvStart.tv_sec > (TIMING_WARMUP + TIMING_MEASURE + TIMING_COOLDOWN)) {
      //On sort de la boucle et on laisse mourir le thread
      break;
    }
    //Si programme est lancé depuis 180s on passe en cooldown
    else if (tvNow.tv_sec - tvStart.tv_sec > (TIMING_WARMUP + TIMING_MEASURE)) {
      printMeasure = false;
    //Si programme est lancé depuis 20s on log les timings
    } else if (tvNow.tv_sec - tvStart.tv_sec > TIMING_WARMUP) {
      printMeasure = true;
    }
    if (printMeasure) {
      printf("\t[THREAD #%d][CLIENT #%d][TIMING] Broadcast took : %lld microseconds\n", threadId, clientId, elapsedMicroSeconds);
    }

    //on attend 10secondes avant le prochain message
    sleep(10);
  }

  usleep(1000000);
  printf("[THREAD #%d][CLIENT #%d] Closing connection...\n", threadId, clientId);
  freeComm(commForConnect);
  printf("[THREAD #%d][CLIENT #%d] Connection closed !\n", threadId, clientId);

  return EXIT_SUCCESS;
}


int sendMessage(trComm* comm, char* msg_payload) {
  int nbWritten;
  int len;
  message *msg;

  //On calcule la taille du message
  len = sizeof(messageHeader) + strlen(msg_payload) + sizeof('\0');
  msg = malloc(len); //On alloue de la mémoire en concéquence
  assert(msg != NULL); //On verifie que malloc n'a pas raté
  //On remplie le message (taille+contenu)
  msg->header.len = len;
  strcpy(msg->payload, msg_payload);
  nbWritten = commWrite(comm, msg, msg->header.len);

  //On verifie que le message a bien été envoyé
  if (nbWritten != len) {
    ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__,
        "sent only %d/%d bytes", nbWritten, len);
  }
  free(msg); //On libere la mémoire utilisée pour le message

  return nbWritten;
}

message* readMessage(trComm* comm, int* nbRead) {
  message *msg;
  int len;
  *nbRead = commReadFully(comm, &len, sizeof(len));
  if (*nbRead != sizeof(len)) {
      ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__,
        "Error reading\n");
  }
  msg = malloc(len);
  assert(msg != NULL);
  msg->header.len = len;
  *nbRead = commReadFully(comm, &(msg->header.typ),
      msg->header.len - sizeof(len));

  //On ne gère que les cas simples (messages courts et complets)
  if (*nbRead != msg->header.len - sizeof(len)) {
    ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__,
      "Incomplete message\n");
  }
  if (msg->header.len >= 1000) {
    ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__,
      "We don't handle long messages for now\n");
  }

  return msg;
}
