#include "mywebsrv.h"


MyWebSrv::MyWebSrv(int port_number)
{
	MySocket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
/* Init socket structure */
	struct sockaddr_in serv_addr;
	memset((char *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = INADDR_ANY;
	serv_addr.sin_port = htons(port_number);
	
	MyBindSocket(serv_addr);
	MyListenSocket(25);
}

MyWebSrv::~MyWebSrv()
{
}

/* Creates socket */
void MyWebSrv::MySocket(int family, int type, int protocol)
{
	my_listen_socket = socket(family, type, protocol);
	
	if(my_listen_socket < 0)
	{
		//perror("ERROR socket creation");
	}
}

/* Binds socket */
void MyWebSrv::MyBindSocket(struct sockaddr_in &serv_addr)
{
	int val = 1;
	int n;

	n = setsockopt(my_listen_socket, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val));
	if (n < 0)
	{
		//perror("ERROR setsockopt");
	}
	
	n = bind(my_listen_socket, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
	if (n < 0)
	{ 
		//perror("ERROR bind");
	}
}	


/* Listens socket */
void MyWebSrv::MyListenSocket(int size)
{
	listen(my_listen_socket, size);
}

/* Accepts connection */
bool MyWebSrv::MyAcceptSocket()
{
	struct sockaddr_in cli_addr;
	socklen_t clilen;
	clilen = sizeof(cli_addr);
	my_data_socket = accept(my_listen_socket, (struct sockaddr *) &cli_addr, &clilen);
//	my_data_socket = accept(my_listen_socket, NULL, NULL);
	if (my_data_socket < 0)
		return false;
		
	return true;
}

/* Close socket */	
void MyWebSrv::MyCloseSocket()
{
	close(my_data_socket);
}

/* Write to socket */
void MyWebSrv::MyWriteSocket(const char * buff)
{
	int n = write(my_data_socket, buff, strlen(buff));
	//int n = write(4, buff, strlen(buff));
	if (n < 0)
	{
		//perror("ERROR writing to socket");
	}
}

void MyWebSrv::PrintHeader(int httpCode)
{	
	std::ostringstream header;
	std::string errMsg;
	header << "HTTP/1.0";
	
	switch (httpCode)
	{
		case 403:
			header << " 403 Forbidden\r\n";
			errMsg = "Access Denied";
			break;
		case 404: 
			header << " 404 Not Found\r\n"; 
			errMsg = "Not Found\n"; 
			break;
		case 501: 
			header << " 501 Not Implemented\r\n"; 
			errMsg = "Not Implemented\n"; 
			break;		
		default: 
			header << " 400 Bad Request\r\n"; 
			errMsg = "Bad Request\n";
			break;
	}
	
	header << "Connection: close\r\n";	
	
	header 	<< "Content-Type: text/html\r\n" << "Content-Length: " << errMsg.size() << "\r\n" << "\r\n";
	header << "<html><head>\r\n <title>" << errMsg << "</title>\r\n</head>\r\n<body>" << errMsg << "\r\n" << "</body></html>\r\n";

	MyWriteSocket((const char *)header.str().c_str());
}



/* Display directory contetnts */
void MyWebSrv::ListDirectory(std::string& path)
{
	int filecount = -1;
	int i;
	struct dirent** files = {0};
	std::ostringstream dirlink;	
	
	dirlink <<"HTTP/1.0 200 OK\r\nConnection: close\r\nContent-Type: text/html\n\n<html><head>\r\n <title>Index of dir</title>\r\n</head>\r\n<body>\r\n";  	
	
		
/* Add backslash at the end of directory name */
	if (path[path.size()-1] != '/')
		path.append("/");

   
/* Get number of files in a directory */
	filecount = scandir(path.c_str(), &files, 0, alphasort);

/* Loop for every file in the current directory */
	i = 0;
	
	for(i=0; i<filecount; i++)
	{
		if( (strlen(files[i]->d_name) == 1 && files[i]->d_name[0] == '.') || (strlen(files[i]->d_name) == 2 && files[i]->d_name[0] == '.' && files[i]->d_name[1] == '.') )
		{
			dirlink << "<a href=" << files[i]->d_name << "/>" << files[i]->d_name << "/</a><br/>\r\n";
			free(files[i]);
		}

/* Normal file */
		else
		{
			if(files[i]->d_type == DT_DIR)
			{
				dirlink << "<a href=" << path << files[i]->d_name << ">/" << files[i]->d_name << "/</a><br/>\r\n";
				free(files[i]);
			}
			else
			{
				dirlink << "<a href=" << path << files[i]->d_name << ">"<< files[i]->d_name << "</a><br/>\r\n";
				free(files[i]);   
			}
		}
	}
  
/* Closing tags for html code */
	dirlink << "</body></html>\r\n";
  
/* Send result back to browser */
	MyWriteSocket((const char *)dirlink.str().c_str());
}


/* Display file contents */
int MyWebSrv::DisplayFile(std::string& fileName)
{
	int infd;
	size_t n;
	unsigned char buf[4096];
	
	if ( (infd = open(fileName.c_str(), O_RDONLY)) < 0)
	{
		PrintHeader(403);
		//perror("Failed opening file for reading");
		return -1;
	}
	
	while ( (n = read(infd, buf, sizeof(buf))) > 0) 
		MyWriteSocket((const char *)buf);
		
	close(infd);
}

void* MyWebSrv::HandleRequest(void)
{
	char buffer[4096] = {'\0'}; // received string
	char buffer1[4096] = {'\0'}; //output string
	char* queryMethod;
	char* queryPath;
	char* queryProt;
	int numread;  
	int bytes_read;
        MyWebSrv *server = reinterpret_cast<MyWebSrv*>(param);
	
	struct stat statbuf; 
	int filecount = -1; 
	
	while(true) 
	{
		MyAcceptSocket(); 
		numread = recv(my_data_socket, buffer, 4096, 0);
		buffer[numread] = '\0';  
		queryMethod = strtok(buffer, " ");
		queryPath = strtok(NULL, " ");
		queryProt = strtok(NULL, "\r");	   
		
		if(queryMethod == NULL || queryPath == NULL || queryProt == NULL) 
		{
			PrintHeader(400);
		}
		else if (stat(queryPath, &statbuf) < 0) 
		{
			PrintHeader(404);
		}
		
/* Display directory contents */
		else if (S_ISDIR(statbuf.st_mode))
		{
			std::string npath(queryPath); 
			ListDirectory(npath);
		}
		
/* Display file contents */
		else
		{
			std::string npath(queryPath); 
			DisplayFile(npath);
		}
		
		MyCloseSocket();
}
}

/* Run web server */
void MyWebSrv::Start()
{
	int i;
	int result;
	int nthreads = 5;
	int arg;
	//HandleRequest();
	
	pthread_t thread_id[nthreads];
	
	for (int i = 0; i < nthreads; i++) 
	{		
		result = pthread_create(&thread_id[i], NULL, HandleRequest, (void *) arg);
		if (result != 0) 
			perror("ERROR thread create failed");
	}
	
	for (int i = 0; i < nthreads; i++) 
	{
		pthread_join(thread_id[i], NULL);
	}	

/*	
	for (cnt_thread = 0; cnt_thread < 5; cnt_thread++)
	{
		result = pthread_create(&thread_id, NULL, proc_request_in_thread, (void *) my_listen_socket);
		if (result != 0) 
		{
			printf("Could not create thread.\n");
			//return 0;
		}
		
		sched_yield();
	}
	
	pthread_join (thread_id, NULL);   */
}
