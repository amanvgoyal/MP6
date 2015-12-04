#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string>
#include <iostream>

#include "NetworkRequestChannel.h"

using namespace std;

/*
Creates a CLIENT-SIDE local copy of the channel. The channel is connected to teh given port number at the given server host. THIS CONSTRUCTOR IS CALLED BY THE CLIENT
*/
// Beej way + linux.die.net/man way
NetworkRequestChannel::NetworkRequestChannel(const string server_host_name,
		      const unsigned short port_no){
  struct addrinfo hints, *res, *rp;
  int sockfd, s;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC; // IPv4 or IPv6
  hints.ai_socktype = SOCK_STREAM; // TCP
  hints.ai_flags = AI_PASSIVE; 

  // Get info from server, check for errors
  s = getaddrinfo(NULL, server_host_name.c_str(), &hints, &res);
  if (s != 0) {
    cout <<"getaddrinfo: " <<  gai_strerror(s) << endl;;
    exit(EXIT_FAILURE);
  }

  // Traverse ll to get a good IP addr
  for (rp = res; rp != NULL; rp = rp->ai_next) {
    sockfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
    if (sockfd == -1) {continue;}
    // Success
    //if (bind(sockfd, rp->ai_addr, rp->ai_addrlen) == 0) {break;}
    //close(sockfd);
  }

  //if (rp == NULL) {
  //  fprintf(stderr, "Unable to bind\n");
  //  exit(EXIT_FAILURE);
  //}
  
  int con = connect(sockfd, res->ai_addr, res->ai_addrlen);
  if (con == -1) {
    cerr << "not connected!" << endl;
    exit(EXIT_FAILURE);
  }

  listenerfd = sockfd;
}


/*
Creates a SERVER-SIDE local copy of the channel that is accepting connections 
at the given port number. NOTE that the mult. clients can be connected to teh same server side end of the req. chan. Whenever a new connection comes in, it is accepted by the server, and the given connection handler is invoked. The parameter to the connection handler is the FD of the slave socket returned by teh accept call. NOTE that the connection handler does not want to deal with closing the socket. You will have to close the socket once the connection handler is done.
*/
/*
socket
bind
listen
while(1){hs=accept(); pthreadcreate(connectionhandler, hs)}
*/
NetworkRequestChannel::NetworkRequestChannel(const unsigned short port_no, 
		      void* (*connection_handler) (int*)) {
  int sockfd, new_fd, rv;
  struct addrinfo hints, *serv; // listen on sock_Fd, new conn on new_fd;
  struct sockaddr_storage their_addr; // connector's addr
  char s[INET6_ADDRSTRLEN];
  char buf[1024];

  socklen_t sin_size;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, (char*) port_no, &hints, &serv)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(EXIT_FAILURE);
  }
  
  if ((sockfd = socket(serv->ai_family, serv->ai_socktype, serv->ai_protocol)) == -1) {
    perror("server: socket");
    exit(EXIT_FAILURE);
  }

  if (bind(sockfd, serv->ai_addr, serv->ai_addrlen) == -1) {
    close(sockfd);
    perror("server: bind");
    exit(EXIT_FAILURE);
  }

  freeaddrinfo(serv); // done with this structure

  // 20 to avoid connection refusals
  if (listen(sockfd, 20) == -1) {
    perror("listen");
    exit(1);
  }

  cout << "server: waiting for connections...\n" << endl;

  //pthread_t thread;
  while(true) {
    pthread_t thread;
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr*) &their_addr, &sin_size);
    
    if (new_fd == -1) {
      perror("accept");
      continue;
    }
    
    pthread_create(&thread, (pthread_attr_t*) connection_handler, (void*) new_fd);
  }
}

NetworkRequestChannel::~NetworkRequestChannel() {
  close(listenerfd);
}

/*
Send a str over teh chan and wait for a reply
 */
string NetworkRequestChannel::send_request(string request) {

}

/* 
Blocking read of data from teh channel. Returns a string of characters read from the channel. Returns NULL if read failed.
*/
// just like reqchannel.cpp
string NetworkRequestChannel::cread() {
  string str_buf;
  char buf[256];
  size_t r_len = read(listenerfd, buf, 256);
  if (r_len == -1) {
    cerr << "Error occured during read!" << endl;
    //exit(EXIT_FAILURE);
  }
  
  str_buf = buf;
  return str_buf;
}

/*
Write teh data to the chan. The funciton returns the number of characters written to the channel.
*/
int NetworkRequestChannel::cwrite(string msg) {
  if (msg.length() >= 256) {
    cerr << "Message too long for chan!" << endl;
    return -1;
  }

  const char* s = msg.c_str();
  if (write(listenerfd, s, strlen(s) + 1) < 0) {
    cerr << "Error occured during write!" << endl;
  }
}



