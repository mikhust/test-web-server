#ifndef MYWERSRV_H_
#define MYWERSRV_H_

#include <iostream>
#include <string>
#include <sstream>
#include <cerrno>

#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <signal.h>
#include <dirent.h>
#include <malloc.h>
#include <syslog.h>

#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>


class MyWebSrv
{
	public:
		MyWebSrv(int port_number);
		~MyWebSrv();
		void Start();

	private:
/* Wrappers for socket functions */
		bool MyAcceptSocket();
		void MyBindSocket(struct sockaddr_in &serv_addr);
		void MyListenSocket(int size);
		void MySocket(int family, int type, int protocol);
		void MyCloseSocket();		
		void MyWriteSocket(const char* buff);

		static void* HandleRequest(void* param);
		void PrintHeader(int httpCode);
		void ListDirectory(std::string& path);
		int DisplayFile(std::string& fileName);	

		
	private:
		int my_listen_socket;
		int my_data_socket;
};

#endif //MYWERSRV_H_
