
#ifndef NETWORK_REQ_CHAN_H                   
#define NETWORK_REQ_CHAN_H

#include <string>
#include <pthread.h>

class NetworkRequestChannel {
private:
  int listenerfd;
  

public:
  NetworkRequestChannel(const std::string server_host_name,
			const unsigned short port_no);

  NetworkRequestChannel(const unsigned short port_no, 
			void* (*connection_handler) (void*));
  //                                          was    int*

  ~NetworkRequestChannel();
std::string send_request(std::string request);
std::string cread();

int cwrite(std::string msg);
};


#endif


