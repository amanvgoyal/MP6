/* 
   File: simpleclient.C

   Author: R. Bettati
   Department of Computer Science
   Texas A&M University
   Date  : 2013/01/31

   Simple client main program for MP3 in CSCE 313
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
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <iomanip>
#include <sys/time.h>
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>

#include <vector>
#include <map>

#include "reqchannel.h"
#include "BoundedBuffer.h"
#include "semaphore.h"


using namespace std;

// Globals

// For cmdline ops
int n = 10; // number of data requests per person 
int b = 5; // size of bounded buffer in requests
int w = 100; // number of request channels
int h; // name of server host 
int p; // port number of server host

int c;
timeval begin1, end1;

//map<int, int> joe_hist;
//map<int, int> jane_hist;
//map<int, int> john_hist;

int joe_hist[100];
int jane_hist[100];
int john_hist[100];

int joe_req_ct = 0;
int jane_req_ct = 0;
int john_req_ct = 0;

BoundedBuffer* main_buf;

BoundedBuffer* joe_buf;
BoundedBuffer* jane_buf;
BoundedBuffer* john_buf;
	
//void show_hist (map<int, int> m, string name){
void show_hist(int h[], string name) {
  cout << name << "'s histogram" << endl;
  //for (auto x : m) {
  for (int i = 0,freq=0; i < 10; ++i) {
	  freq=0;
	  for(int j=0; j<10; ++j){
		  freq = freq + h[10*i + j];
	  }
	  //cout << '(' << x.first << ", " << x.second << ')' << endl;
      cout << '(' << 10*i << " to " <<10*i +9<< " , " <<freq<< ')' << endl;
  }
}
	
void* req_thread (void* id){
  int requester = *((int*) id);
  string delivery;

  for(int i = 0; i < n; ++i) {
    switch (requester) {
    case 0:
      delivery = "data joe";
      ++joe_req_ct;
      break;
    case 1:
      delivery = "data jane";
      ++jane_req_ct;
      break;
    case 2:
      delivery = "data john";
      ++john_req_ct;
      break;
    default:
      cerr << "Somehow we got an invalid request id!" << endl;
      break;
    }
    main_buf->add(delivery);
  }
  //cout << "Requester " << requester << " is done." << endl;
}

void* worker_thread(void* req_channel){
  // This is how tanzir said to do it
  RequestChannel* chan = (RequestChannel*) req_channel;
  string removal;
  string to_add;

  while(true){
    removal = main_buf->remove();

    if(removal == "stop") {
      //cout << "WORKER STOPPING" << endl;
      break;
    }		

    //cout << "Sending: " << removal << " to server" << endl;
    to_add = chan->send_request(removal);
    
    if (removal == "data joe") {joe_buf->add(to_add);}
    else if (removal == "data jane") {jane_buf->add(to_add);}
    else if (removal == "data john") {john_buf->add(to_add);}
  }

  chan->send_request("quit");
}
	
void* stat_thread(void* id){
  int requester = *((int*) id);
  string removal;
  for(int i = 0 ; i < n ; ++i){
    switch(requester) {
    case 0:
      removal = joe_buf->remove();
      ++joe_hist[atoi(removal.c_str())];
      break;
    case 1:
      removal = jane_buf->remove();
      ++jane_hist[atoi(removal.c_str())];
      break;
    case 2:
      removal = john_buf->remove();
      ++john_hist[atoi(removal.c_str())];
      break;
    }
  }
  //cout << "Person " << requester << "'s stats thread is done" << endl;
}
	
/*--------------------------------------------------------------------------*/
/* MAIN FUNCTION */
/*--------------------------------------------------------------------------*/

int main(int argc, char * argv[]) {
  opterr = 0;
  
  while ((c = getopt(argc, argv, "n:b:w:h:p")) != -1) {
    switch(c) {
    case 'n':
      n = atoi(optarg);
      break;
    case 'b':
      w = atoi(optarg);
      break;
    case 'w':
      w = atoi(optarg);
      break;
    case 'h':
      h = atoi(optarg);
      break;
    case 'p':
      p = atoi(optarg);
      break;
    default:
      cout << "flag H for help" << endl;
      cout << "flag n for num of data requests per person" << endl;
      cout << "flag b for size of bounded buffer in requests" << endl;
      cout << "flag w for number of request channels" << endl;
      cout << "flag p for the port number of the server host" << endl;
      abort();
    }
  }

  // App code here
  main_buf = new BoundedBuffer(b);

  joe_buf = new BoundedBuffer(b/3); // b/3 noted on piazza
  jane_buf = new BoundedBuffer(b/3);
  john_buf = new BoundedBuffer(b/3); 

  pthread_t joe_req;
  pthread_t jane_req;
  pthread_t john_req;
	
  pthread_t joe_stat;
  pthread_t jane_stat;
  pthread_t john_stat;

  pthread_t workers[w];
  
  int* joe_id = new int(0); 
  int* jane_id = new int(1);
  int* john_id = new  int(2);
  
  const char **farg = new const char*[0]; // nothing in here
  int pid = fork();
  if (pid != 0 && pid != -1) {execv("./dataserver", (char**) farg);}
  
  else {
    cout << "Client started" << endl;
    cout << "Establishing control channel... " << endl; 
    cout << "Going to check if hello succeeds...\n" << flush;

    RequestChannel chan("control", RequestChannel::CLIENT_SIDE);
    //gettimeofday(&start[0],NULL);
	string reply = chan.send_request("hello");
	//gettimeofday(&stop[0],NULL);
    cout << reply << endl;
    cout << "done" << endl << endl;;
	gettimeofday(&begin1,NULL);
    cout << "Beginning creation of joe's threads" << endl;
    pthread_create(&joe_req, NULL, req_thread, (void*) joe_id);
    pthread_create(&joe_stat, NULL, stat_thread, (void*) joe_id);
    cout << "Finished making joe's threads" << endl;

    cout << "Beginning creation of jane's threads" << endl;
    pthread_create(&jane_req, NULL, req_thread, (void*) jane_id);
    pthread_create(&jane_stat, NULL, stat_thread, (void*) jane_id);
    cout << "Finished making jane's threads" << endl;

    cout << "Beginning creation of john's threads" << endl;
    pthread_create(&john_req, NULL, req_thread, (void*) john_id);
    pthread_create(&john_stat, NULL, stat_thread, (void*) john_id);
    cout << "Finished making john's threads" << endl;

    cout << "Beginning to create the " << w << " worker threads." << endl;
    for (int i = 0; i < w; ++i) {
      string req_name = chan.send_request("newthread");
      //RequestChannel* chan2 = new RequestChannel(req_name, RequestChannel::CLIENT_SIDE);
      //pthread_create(&workers[i], NULL, worker_thread, (void*) chan2);
      pthread_create(&workers[i], NULL, worker_thread, (void*) (new RequestChannel(req_name, RequestChannel::CLIENT_SIDE)));
      //usleep(100000);
    }
    cout << "Finished making the worker theads." << endl;


    pthread_join(joe_req, NULL);
    pthread_join(jane_req, NULL);
    pthread_join(john_req, NULL);

    for (int i = 0; i < w; ++i) {
      main_buf->add("stop");
    }

    // Begin joining all threads
    for (int i = 0; i < w; ++i) {
      pthread_join(workers[i], NULL);
    }

    //pthread_join(joe_req, NULL);
    //pthread_join(jane_req, NULL);
    //pthread_join(john_req, NULL);

    pthread_join(joe_stat, NULL);
    pthread_join(jane_stat, NULL);
    pthread_join(john_stat, NULL);

	gettimeofday(&end1,NULL);
    // Finished
    chan.send_request("quit");
    //usleep(100000);
   
    // Show stats
    cout << "showing histograms" << endl;
    show_hist(joe_hist, "Joe");
    show_hist(jane_hist, "Jane");
    show_hist(john_hist, "John");
	
	cout<<"total number of requests: "<<n*3<<endl;
	cout<<"total number of worker threads "<<w<<endl;
	cout<<"Total request time: "<<end1.tv_sec-begin1.tv_sec<<" sec "<<end1.tv_usec-begin1.tv_usec<<" musec"<<endl;

    return 0;
  }
  return -1;
}
