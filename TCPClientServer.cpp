#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

int client (char * server_name, char* port)
{
  struct addrinfo hints, *res;
  int sockfd;

  // first, load up address structs with getaddrinfo():

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  int status;
  //getaddrinfo("www.example.com", "3490", &hints, &res);
  if ((status = getaddrinfo(server_name, port, &hints, &res)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
    return -1;
  }

  // make a socket:
  sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if (sockfd < 0)
    {
      perror ("Error creating socket\n");
      return -1;
    }

  // connect!
  if (connect(sockfd, res->ai_addr, res->ai_addrlen)<0)
    {
      perror ("connect error\n");
      return -1;
    }
  printf ("Successfully connected to the server %s\n", server_name);
  //printf ("Now Attempting to send a message to the server\n", server_name);
  return 0;
}

int server ()
{
  int sockfd, new_fd;  // listen on sock_fd, new connection on new_fd
  struct addrinfo hints, *servinfo, *p;
  struct sockaddr_storage their_addr; // connector's address information
  socklen_t sin_size;
  struct sigaction sa;
  int yes=1;
  char s[INET6_ADDRSTRLEN];
  int rv;

  memset(&hints, 0, sizeof hints);
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE; // use my IP

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    return -1;
  }

  if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
    perror("server: socket");
    continue;
  }

  if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
    close(sockfd);
    perror("server: bind");
    continue;
  }
  freeaddrinfo(servinfo); // all done with this structure

  if (listen(sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }
  
  printf("server: waiting for connections...\n");
  while(1) 
    {  // main accept() loop
      sin_size = sizeof their_addr;
      new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
      if (new_fd == -1) {
	perror("accept");
	continue;
      }
      //inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
      printf("server: got connection\n");

      // send 
      if (send(new_fd, "Hello, world!", 13, 0) == -1)
	{
	  perror("send");
        }
      close(new_fd);
    }

  return 0;
}



int main ()
{
  if (client ("google.com", "80")<0)
    {
      printf ("Problem running the client");
      return -1;
    }
  return 0;
}
