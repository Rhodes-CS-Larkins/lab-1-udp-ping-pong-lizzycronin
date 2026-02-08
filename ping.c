/*
 * ping.c - UDP ping/pong client code
 *          author: <your name>
 */
#include <arpa/inet.h>
#include <errno.h>
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
#define MAXDATASIZE 100

int main(int argc, char **argv) {
  int ch, errors = 0;
  int nping = 1;                        // default packet count
  char *ponghost = strdup("localhost"); // default host
  char *pongport = strdup(PORTNO);      // default port
  int arraysize = 100;                  // default packet size

  int rv, sockfd, numbytes, bytes_sent;
  double start_time, stop_time, total_time;
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr;
  socklen_t addr_len;
  char *buf_rec;
  while ((ch = getopt(argc, argv, "h:n:p:")) != -1) {
    switch (ch) {
    case 'h':
      ponghost = strdup(optarg);
      break;
    case 'n':
      nping = atoi(optarg);
      break;
    case 'p':
      pongport = strdup(optarg);
      break;
    case 's':
      arraysize = atoi(optarg);
      break;
    default:
      fprintf(stderr,
              "usage: ping [-h host] [-n #pings] [-p port] [-s size]\n");
    }
  }

  // UDP ping implemenation goes here

  // create an array of N elements set to 200
  arraysize = nping;
  char *buf_send = malloc(arraysize);
  memset(buf_send, 200, arraysize);

  buf_rec = malloc(MAXDATASIZE);
  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_INET; // set to AF_INET to use IPv4
  hints.ai_socktype = SOCK_DGRAM;

  // get socket information and store in servinfo (IP and port info)
  rv = getaddrinfo(ponghost, pongport, &hints, &servinfo);
  if (rv != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return 1;
  }

  // loop through all the results from getaddrinfo and make a socket
  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
      perror("talker: socket");
      continue;
    }
    // break once the socket is found
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "talker: failed to create socket\n");
    return 2;
  }

  // loop over the num of ping packets you want to send (N-times)
  for (int i = 0; i <= nping; i++) {
    // start timer
    start_time = get_wctime();

    // send array elements in single packet
    if ((bytes_sent = sendto(sockfd, buf_send, arraysize, 0, p->ai_addr,
                             p->ai_addrlen)) == -1) {
      perror("sendto");
      exit(1);
    }

    addr_len = sizeof(their_addr);
    // wait to receive the reply from pong
    if ((numbytes = recvfrom(sockfd, buf_rec, MAXDATASIZE - 1, 0,
                             (struct sockaddr *)&their_addr, &addr_len)) ==
        -1) {
      perror("recv");
      exit(1);
    }

    // stop timer
    stop_time = get_wctime();

    // validate the result
    for (int j = 0; j < sizeof(buf_rec); j++) {
      if ((unsigned char)buf_rec[j] != (unsigned char)buf_send[i] + 1) {
        perror("Invalid return");
        exit(1);
      }
    }

    // print out time
    total_time = (stop_time - start_time) * 1000;
    printf("Total time: %.3f ms\n", total_time);
  }

  printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n",
         nping, arraysize, errors, ponghost, pongport);

  return 0;
}
