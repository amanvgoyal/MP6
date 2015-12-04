  #include "BoundedBuffer.h"
#include "semaphore.h"

#include <iostream>

using namespace std;

BoundedBuffer::BoundedBuffer(int b) {
  empty = new Semaphore(b);
  full = new Semaphore(0);
  mutex = new Semaphore(1);
}

BoundedBuffer::~BoundedBuffer() {
  delete empty;
  delete full;
  delete mutex;
}

void BoundedBuffer::add(string s) {
  empty->P();
  mutex->P();
  data.push_back(s);
  mutex->V();
  full->V();
}

string BoundedBuffer::remove() {
  string s = ""; 
  
  full->P();
  mutex->P();

  s = data[0];
  data.erase(data.begin());
  
  mutex->V();
  empty->V();
  
  return s;
}
