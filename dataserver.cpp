/* 
    File: dataserver.C

    Author: R. Bettati
            Department of Computer Science
            Texas A&M University
    Date  : 2012/07/16

    Dataserver main program for MP3 in CSCE 313
*/

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include <cassert>
#include <cstring>
#include <sstream>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>

#include <pthread.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>

#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <string.h>

#include "reqchannel.h"
#include "NetworkRequestChannel.h"

using namespace std;

/*--------------------------------------------------------------------------*/
/* DATA STRUCTURES */ 
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* CONSTANTS */
/*--------------------------------------------------------------------------*/

/* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* VARIABLES */
/*--------------------------------------------------------------------------*/

static int nthreads = 0;

int p = 13370; // port number for data server
int b = 20; // backlog of the server socket

/*--------------------------------------------------------------------------*/
/* FORWARDS */
/*--------------------------------------------------------------------------*/

void handle_process_loop(RequestChannel & _channel);

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- SUPPORT FUNCTIONS */
/*--------------------------------------------------------------------------*/

string int2string(int number) {
  stringstream ss;//create a stringstream
  ss << number;//add number to the stream
  return ss.str();//return a string with the contents of the stream
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THREAD FUNCTIONS */
/*--------------------------------------------------------------------------*/

void * handle_data_requests(void * args) {

  RequestChannel * data_channel =  (RequestChannel*)args;

  // -- Handle client requests on this channel. 
  
  handle_process_loop(*data_channel);

  // -- Client has quit. We remove channel.
 
  delete data_channel;
}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- INDIVIDUAL REQUESTS */
/*--------------------------------------------------------------------------*/

void process_hello(RequestChannel & _channel, const string & _request) {
  _channel.cwrite("hello to you too");
}

void process_data(RequestChannel & _channel, const string &  _request) {
  usleep(1000 + (rand() % 5000));
  //_channel.cwrite("here comes data about " + _request.substr(4) + ": " + int2string(random() % 100));
  _channel.cwrite(int2string(rand() % 100));
}

void process_newthread(RequestChannel & _channel, const string & _request) {
  int error;
  nthreads ++;

  // -- Name new data channel

  string new_channel_name = "data" + int2string(nthreads) + "_";
  //  cout << "new channel name = " << new_channel_name << endl;

  // -- Pass new channel name back to client

  _channel.cwrite(new_channel_name);

  // -- Construct new data channel (pointer to be passed to thread function)
  
  RequestChannel * data_channel = new RequestChannel(new_channel_name, RequestChannel::SERVER_SIDE);

  // -- Create new thread to handle request channel

  pthread_t thread_id;
  //  cout << "starting new thread " << nthreads << endl;
  if (error = pthread_create(& thread_id, NULL, handle_data_requests, data_channel)) {
    fprintf(stderr, "p_create failed: %s\n", strerror(error));
  }  

}

/*--------------------------------------------------------------------------*/
/* LOCAL FUNCTIONS -- THE PROCESS REQUEST LOOP */
/*--------------------------------------------------------------------------*/

void process_request(RequestChannel & _channel, const string & _request) {

  if (_request.compare(0, 5, "hello") == 0) {
    process_hello(_channel, _request);
  }
  else if (_request.compare(0, 4, "data") == 0) {
    process_data(_channel, _request);
  }
  else if (_request.compare(0, 9, "newthread") == 0) {
    process_newthread(_channel, _request);
  }
  else {
    _channel.cwrite("unknown request");
  }

}

void handle_process_loop(RequestChannel & _channel) {

  for(;;) {

    cout << "Reading next request from channel (" << _channel.name() << ") ..." << flush;
    string request = _channel.cread();
    cout << " done (" << _channel.name() << ")." << endl;
    cout << "New request is " << request << endl;

    if (request.compare("quit") == 0) {
      _channel.cwrite("bye");
      usleep(10000);          // give the other end a bit of time.
      break;                  // break out of the loop;
    }

    process_request(_channel, request);
  }
  
}

void* connection_handler(void* farg) {
  int sock = *(int*) farg;
  char buf[1024];
  string strbuf;
  
  string msg;
  while (true) {
    recv(sock, buf, sizeof(buf), 0);
    printf("server: recieved msg: %s\n", buf);

    strbuf = buf;
    if (strbuf == "quit") {break;}
    else if (strbuf == "hello" || strbuf == "Hello") {
      msg = "hello to you too";
      if (send(sock, msg.c_str(), msg.size() + 1, 0) == -1) {perror("send");}
    }
    else if (strbuf.substr(0, 4) == "data") {
      usleep(1000 + (rand() % 5000));
      //_channel.cwrite(int2string(rand() % 100));
      msg = int2string(rand() % 100);
      if (send(sock, msg.c_str(), strlen(msg.c_str()) + 1, 0) == -1) {perror("send");}
    }
    else {
      msg = "unknown string was: " + strbuf;
      if (send(sock, msg.c_str(), strlen(msg.c_str()) + 1, 0) == -1) {perror("send");}
    }
  }
  close(sock);
}

/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {

  //  cout << "Establishing control channel... " << flush;
  //  RequestChannel control_channel("control", RequestChannel::SERVER_SIDE);
  //  cout << "done.\n" << flush;

  //handle_process_loop(control_channel);

  char c;
  opterr = 0;
  
  while ((c = getopt(argc, argv, "p:b:")) != -1) {
    switch(c) {
    case 'p':
      p = atoi(optarg);
      break;
    case 'b':
      b = atoi(optarg);
      break;
    default:
      cout << "flag H for help" << endl;
      cout << "flag p for port no for data server" << endl;
      cout << "flag b for backlog of server socket" << endl;
      abort();
    }
  }

  // App code here
  cout << "Starting server using port " << p << endl;
  NetworkRequestChannel chan(p, connection_handler);
  
}
