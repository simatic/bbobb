/* Client of a client-server application designed to check
 that Comm module works correctly between different machines

 Syntax = server port
 */
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "errorTrains.h"
#include "comm.h"
#include "trains.h" // To have message typedef
#define AVERAGE_SIZE 32 //This default value is estimated by considering the average size of received message


#define ID_REQUEST_MESSAGE "IDREQUEST"
#define ID_REPLY_MESSAGE "IDREPLY:%d"
#define BROADCAST_MESSAGE "WHAT_IS_THE_ANSWER?"

pthread_mutex_t client_id_lock;
int client_id_counter = 0;


int sendMessage(trComm* comm, char* msg_payload);
message* readMessage(trComm* comm, int* nbRead);

void *clientHandler(void *arg){
  trComm *aComm = (trComm*) arg;
  message *msg;
  int nbRead;
  int clientId = -1;

  printf("\tNew connection\n");
  do {

    msg = readMessage(aComm, &nbRead);
    if (nbRead == 0) {
      //Le message vide indique que le client ferme la connection
      break;
    }

    //On réagit en fonction du message reçu
    //Si c'est un ID Request
    if (strcmp(msg->payload, ID_REQUEST_MESSAGE) == 0) {
      printf("[NEW CLIENT] ID Request\n");
      //On lock le code suivant pour pas que 2 threads lisent/ecrivent
      //client_id_counter au meme moment
      pthread_mutex_lock(&client_id_lock);
      char* reply_message = malloc(sizeof (strlen(ID_REPLY_MESSAGE)) + 20);
      //On insere le clientId dans le Reply message
      sprintf(reply_message, ID_REPLY_MESSAGE, client_id_counter);
      clientId = client_id_counter;
      client_id_counter++;
      pthread_mutex_unlock(&client_id_lock);
      //On a fini, on peut unlock

      printf("\t[CLIENT %d] Sending ID Reply \n", clientId);
      sendMessage(aComm, reply_message);
    }
    //Si c'est un broadcast message
    else if (strcmp(msg->payload, BROADCAST_MESSAGE) == 0) {
      printf("[CLIENT %d] Recieved broadcast message\n", clientId);
      sendMessage(aComm, "42");
      printf("[CLIENT %d] Question answered\n", clientId);
    }
    //Sinon
    else {
      ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__,
        "Unknown message:\n\t %s", msg->payload);
    }
    free(msg);

  } while (1);

  freeComm(aComm);

  if (nbRead == 0) {
    printf("\t...Connection has been closed\n");
  } else if (errno == EINTR) {
    printf("\t...comm_readFully was aborted\n");
  } else {
    ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__, "comm_readFully");
  }

  return NULL ;
}

int main(int argc, char *argv[]){
  trComm *commForAccept;
  trComm *aComm;

  //On verifie les arguments
  if (argc != 2) {
    fprintf(stderr, "USAGE = %s port\n", argv[0]);
    exit(EXIT_FAILURE);
  }
  //On ecoute sur le port passé en parametre
  printf("Accepting connections on port %s...\n", argv[1]);
  commForAccept = commNewForAccept(argv[1]);
  if (commForAccept == NULL )
    ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__, "comm_newForAccept");

  do {
    //On recoit une nouvelle connection
    aComm = commAccept(commForAccept);
    if (aComm != NULL ) {
      // On demare un thread pour gerer le nouveau client
      pthread_t thread;
      int rc = pthread_create(&thread, NULL, &clientHandler, (void *) aComm);
      if (rc < 0)
        ERROR_AT_LINE(EXIT_FAILURE, rc, __FILE__, __LINE__, "pthread_create");
      rc = pthread_detach(thread);
      if (rc < 0)
        ERROR_AT_LINE(EXIT_FAILURE, rc, __FILE__, __LINE__, "pthread_detach");
    }
  } while (aComm != NULL );

  if (errno == EINTR) {
    printf("\t...comm_accept was aborted\n");
    freeComm(commForAccept);
  } else
    ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__, "comm_accept");

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
  if (*nbRead == 0) {
    return NULL;
  }
  if (*nbRead != sizeof(len)) {
      ERROR_AT_LINE(EXIT_FAILURE, errno, __FILE__, __LINE__,
        "Error reading : %d\n", *nbRead);
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
