#include "socketUtils.h"


/**CONSTANTS DEFINE**/
#define REDO 1
#define CONTINUE 0


/**SIGNALS DEFINE**/
#define ACCEPT_INTERRUPTED (errno == EINTR || errno == ECHILD)




/**SOCKET CREATION: it returns file descriptor for the socket**/
int Socket(int family,int type,int protocol){
	int tmp;

	if((tmp=socket(family,type,protocol))<0){
		printf("**Error Socket creation: %s.\n", strerror(errno));
	}

return(tmp);
}


/**BIND: it assigns a local address to a socket**/
int Bind(int socket,const SA *address,socklen_t addrlen){
    int tmp;

	if((tmp=bind(socket,address,addrlen))!=0){
		printf("**Error in Bind: %s.\n", strerror(errno));
	}

return(tmp);
}


/**LISTEN: set a socket in passive mode, in order to listen to a connection**/
int Listen(int socket,int backlog){
	char *ptr;
	int tmp;

	if((ptr=getenv("LISTENQ"))!= NULL){
        backlog=atoi(ptr);
	}

	if((tmp=listen(socket,backlog))<0){
        printf("**Error in Listen: %s.\n",strerror(errno));
	}

return(tmp);
}


/**ACCEPT: it manages a new connection after the end of three-way handshake**/
int Accept(int socket,SA *clientAddr,socklen_t *addrlen){
	int connectedSocket;
	int repeat=CONTINUE;

	do{
        connectedSocket=accept(socket,clientAddr,addrlen);
        if(connectedSocket<0){
            //check if there are signals
            if(ACCEPT_INTERRUPTED){
                repeat=REDO;
            }
            else{
                //printf the error condition
                printf("**Error in Accept: %s.\n",strerror(errno));
            }
        }else{
            repeat=CONTINUE;
        }
	}while(repeat==REDO);

return(connectedSocket);
}


/**CONNECT: it establishes a new connection with a server**/
int Connect(int socket,const SA *serverAddress,socklen_t addrlen){
	int tmp;

	if((tmp=connect(socket,serverAddress,addrlen))!=0){
		printf("**Error in Connect: %s.\n",strerror(errno));
	}

return(tmp);
}


/**CLOSE: set a socket as closed*/
int Close(int socket){
	int tmp;

	if ((tmp=close(socket))!= 0)
		printf("**Error in Close: %s.\n",strerror(errno));

return(tmp);
}


/**SELECT**/
int Select(int maxfdp1,fd_set *readset,fd_set *writeset,fd_set *exceptset,struct timeval *timeout){
	int repeat=CONTINUE;
	int tmp;

	do{
	    tmp=select(maxfdp1,readset,writeset,exceptset,timeout);
	    if(tmp<0){
            //check if there are signals
            if(ACCEPT_INTERRUPTED){
                repeat=REDO;
            }
            else{
                //printf the error condition
                printf("**Error in Select: %s.\n",strerror(errno));
            }
        }
        else{
            repeat=CONTINUE;
        }
	}while(repeat==REDO);

return(tmp);
}


/**RECV: it tries to read from the socket**/
int Recv(int socket,void *buffer,int len,int flag){
    int tmp;

    tmp=recv(socket,buffer,len,flag);
    if(tmp<0){
        printf("***Error in Recv: %s.\n",strerror(errno));
    }

return(tmp);
}


/**SEND: it tries to send data to the socket**/
int Send(int socket,void *buffer,int len, int flags, int *epipeError){
    int tmp;

    tmp=send(socket,buffer,len,0);
    *epipeError=0;
    if(tmp<0){
        printf("***Error in Send: %s.\n",strerror(errno));
        if(errno==EPIPE){
            *epipeError=1;
            printf("EPIPE.\n");
        }
    }

return(tmp);
}












/**OTHER UTILS**/

void Inet_pton(int af,const char *strptr,void *addrptr){
	int status;

	status=inet_pton(af, strptr, addrptr);
	if(status<=0){
		printf("Error in inet_pton: %s.\n",strerror(errno));
		exit(FAILURE);
    }
return;
}


void Inet_ntop(int af,const void *addrptr,char *strptr,size_t length){
	if(inet_ntop(af, addrptr, strptr, length)==NULL){
		printf("Error in inet_ntop: %s.\n",strerror(errno));
		exit(FAILURE);
	}
}
