#ifndef BOUNDED_BUF_H
#define BOUNDED_BUF_H

#include "semaphore.h"
#include <string>
#include <vector>

class BoundedBuffer {
 public:
  BoundedBuffer(int);
  ~BoundedBuffer();
  void add(std::string);
  std::string remove();

 private:
  Semaphore* empty;
  Semaphore* full;
  Semaphore* mutex;
  
  std::vector<std::string> data;
};

#endif
