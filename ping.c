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
  int ch, error_mark;
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
  double round_time;
  double avg_time;
  while ((ch = getopt(argc, argv, "h:n:p:s:")) != -1) {
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

  // error if no socket is found to create a connection with
  if (p == NULL) {
    fprintf(stderr, "talker: failed to create socket\n");
    return 2;
  }

  // initialize total_time to 0 to add on to as go along
  total_time = 0;
  // loop over the num of ping packets you want to send (N-times)
  for (int i = 0; i < nping; i++) {
    // start timer
    start_time = get_wctime();

    // send array elements in single packet
    if ((bytes_sent = sendto(sockfd, buf_send, arraysize, 0, p->ai_addr,
                             p->ai_addrlen)) == -1) {
      perror("sendto");
      exit(1);
    }

    // use the server's info for length of address
    addr_len = sizeof(their_addr);
    // wait to receive the reply from pong
    if ((numbytes = recvfrom(sockfd, buf_rec, arraysize, 0,
                             (struct sockaddr *)&their_addr, &addr_len)) ==
        -1) {
      perror("recv");
      exit(1);
    }

    // stop timer
    stop_time = get_wctime();

    // validate the result
    // each value + 1
    for (int j = 0; j < numbytes; j++) {
      if ((unsigned char)buf_rec[j] != (unsigned char)buf_send[j] + 1) {
        perror("Invalid return");
        error_mark = 1;
        break;
      }
    }

    // print out time
    round_time = (stop_time - start_time) * 1000;
    total_time += round_time;
    // printf("Total time: %.3f ms\n", total_time);
    printf("ping[%d] : round-trip time: %f ms\n", i, total_time);
  }
  if (error_mark != 1) {
    printf("no errors detected\n");
  }
  avg_time = total_time / nping;
  printf("time to send %d packets of %d bytes %.3f (%.3f avg per packet)\n",
         nping, numbytes, total_time, avg_time);
  // printf("nping: %d arraysize: %d errors: %d ponghost: %s pongport: %s\n",
  //        nping, arraysize, errors, ponghost, pongport);

  return 0;
}
