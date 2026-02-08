/*
 * pong.c - UDP ping/pong server code
 *          author: <your name>
 */
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "util.h"

#define PORTNO "1266"

#define MAXBUFLEN 100

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }

  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

int main(int argc, char **argv) {
  int ch;
  int nping = 1;                   // default packet count
  char *pongport = strdup(PORTNO); // default port
  int sockfd;
  struct addrinfo hints, *servinfo, *p;
  int rv;
  int numbytes_from;
  struct sockaddr_storage their_addr;
  char buf_from[MAXBUFLEN];
  int bytes_sent;
  char buf_send[MAXBUFLEN];
  socklen_t addr_len;
  char s[INET_ADDRSTRLEN];

  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    default:
      fprintf(stderr, "usage: pong [-n #pings] [-p port]\n");
    }
  }

  // pong implementation goes here.
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // IPv4
  hints.ai_socktype = SOCK_DGRAM;
  hints.ai_flags = AI_PASSIVE;

  // use getaddrinfo to retrieve info about self
  if ((rv = getaddrinfo(NULL, PORTNO, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results and bind to the first it accepts
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("listener: socket");
      continue;
    }
    // bind to the socket found
    if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(sockfd);
      perror("listener: bind");
      continue;
    }

    break;
  }

  if (p == NULL) {
    fprintf(stderr, "listener: failed to bind socket\n");
    return 2;
  }

  freeaddrinfo(servinfo);

  printf("listener: waiting to recvfrom...\n");

  addr_len = sizeof their_addr;

  int N = strtol(argv[2], NULL, 10);
  for (int i = 0; i <= N; i++) {
    // get the input from the client
    if ((numbytes_from = recvfrom(sockfd, buf_from, MAXBUFLEN - 1, 0,
                                  (struct sockaddr *)&their_addr, &addr_len)) ==
        -1) {
      perror("recvfrom");
      exit(1);
    }

    printf("listener: got packet from %s\n",
           inet_ntop(their_addr.ss_family,
                     get_in_addr((struct sockaddr *)&their_addr), s, sizeof s));
    printf("listener: packet is %d bytes long\n", numbytes_from);
    buf_from[numbytes_from] = '\0';
    printf("listener: packet contains \"%s\"\n", buf_send);

    for (int j = 0; j < sizeof(buf_from); j++) {
      buf_send[j] = buf_from[j] + 1;
    }
    if ((bytes_sent = sendto(sockfd, buf_send, strlen(buf_send), 0, p->ai_addr,
                             p->ai_addrlen)) == -1) {
      perror("sendto");
      exit(1);
    }
  }
  close(sockfd);
  printf("nping: %d pongport: %s\n", nping, pongport);

  return 0;
}
