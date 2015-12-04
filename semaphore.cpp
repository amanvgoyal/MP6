// https://en.wikipedia.org/wiki/Semaphore_(programming)
// http://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread.h.html
#include "semaphore.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <iomanip>
#include <sys/time.h>

#include <errno.h>
#include <unistd.h>
#include <pthread.h>

using namespace std;

//http://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread_cond_init.html
// http://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread_mutex_init.html
Semaphore::Semaphore(int _val) {
  value = _val;
  c = PTHREAD_COND_INITIALIZER; 
  m = PTHREAD_MUTEX_INITIALIZER;
}

// pthread_exit does not release resources
Semaphore::~Semaphore() {
  int ret = 0;
  
  
  ret = pthread_mutex_destroy(&m);
  if (ret != 0) {cerr << "error with pthread_mutex_destroy" << endl;}

  ret = pthread_cond_destroy(&c);
  if (ret != 0) {cerr << "error with pthread_cond_destroy" << endl;}
   
  //ret = pthread_mutex_destroy(&m);
  //if (ret != 0) {cerr << "error with pthread_mutex_destroy" << endl;}
}

// Wait
// http://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread_cond_wait.html
int Semaphore::P() {
  int ret = 0;
  
  ret = pthread_mutex_lock(&m);
  if (ret != 0) {return ret;}

  while (value <= 0) {
    ret = pthread_cond_wait(&c, &m);
    if (ret != 0) {return ret;}
  }
  value--;
  /*  if (ret != 0) {return ret;}

  if (value >= 0) {--value;}
  
  // block process until val is >= 1
  while (value < 0) {
    ret = pthread_cond_wait(&c, &m);
    if (ret != 0) {return ret;}
  }
  //  --value;
  */
  ret = pthread_mutex_unlock(&m);
  
  return ret;
}

// Signal
//http://pubs.opengroup.org/onlinepubs/007908799/xsh/pthread_cond_signal.html
int Semaphore::V() {
  int ret = 0;

  ret = pthread_mutex_lock(&m);
  if (ret != 0) {return ret;}

  ++value;
  // transfer a blocked process from waiting queue to ready
  //if ((value - 1) < 0) {
    ret = pthread_cond_signal(&c); 
    if (ret != 0) {return ret;}
    //}

  ret = pthread_mutex_unlock(&m);
  
  return ret;
}

