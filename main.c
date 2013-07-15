#include <stdio.h>
#include <stdlib.h>



#include "socketUtils.h"
#include "functions.h"


/**CONSTANTS DEFINE**/
#define STRING_LEN 50
#define BACKLOG_QUEUE 1024
#define CONTINUE 0
#define STOP 1
#define BUFFER_LEN 128

#define DEBUG 0

/**PROTOTYPES**/
void sigchld_handler(int s);
int proxyAction(int sockClient);
void sigint_handler(int s);


/**GLOBAL VARIABLES**/
struct sockaddr_in server;
struct sigaction saSignInt,saSignPipe;

int numChildren=0; //number of children (fork)

int main(int argc,char *argv[]){


    /***/
    struct sockaddr_in proxyMaster;
    struct sockaddr_in client;
    socklen_t clientSize; //it is used in Accept
    clientSize=(socklen_t)sizeof(client);
    char *proxyAddress,*serverAddress;


    struct sigaction saDeadChild;


    /**SOCKET**/
    int sockClient,sockClientConnected;



    /**OTHERS**/
    int fail;
    int childpid;
    int reuseC;

    if(argc!=4){
        printf("Wrong parameters: ./socket <listeningPort> <serverAddress> <serverPort>\n");
        return(FAILURE);
    }


    /**CHECK PORT**/
    if(checkPort(argv[1])!=SUCCESS || checkPort(argv[3])!=SUCCESS){
        printf("**Error: wrong port.\n");
        return(FAILURE);
    }


    /**PROXY SETTINGS**/
    memset((void*)&proxyMaster,0,sizeof(proxyMaster)); //clear proxyMaster address

    proxyMaster.sin_family=AF_INET;
    proxyMaster.sin_addr.s_addr=htonl(INADDR_ANY);
    proxyMaster.sin_port=htons(atoi(argv[1]));
    proxyAddress=stringAllocExing(STRING_LEN);
    Inet_ntop(AF_INET,&proxyMaster.sin_addr,proxyAddress,STRING_LEN);


#if DEBUG==0
        //setting print
        printf("-----------------------------\n");
        printf("PROXY parameter:\n");
        printf("\tIP Address: %s\n",proxyAddress);
        printf("\tListening port: %d\n",ntohs(proxyMaster.sin_port));
        printf("-----------------------------\n\n");
#endif

    free(proxyAddress); //after this point it SHOULD BE useless


    /**SERVER SETTINGS**/
    memset((void*)&server,0,sizeof(server)); //clear server address

    server.sin_family=AF_INET;
    Inet_pton(AF_INET,argv[2],&(server.sin_addr));
    server.sin_port=htons(atoi(argv[3]));
    serverAddress=stringAllocExing(STRING_LEN);
    Inet_ntop(AF_INET,&server.sin_addr,serverAddress,STRING_LEN); //just to verify


#if DEBUG==0
        //setting print
        printf("-----------------------------\n");
        printf("SERVER parameter:\n");
        printf("\tIP Address: %s\n",serverAddress);
        printf("\tListening port: %d\n",ntohs(server.sin_port));
        printf("-----------------------------\n\n");
#endif

    free(serverAddress); //after this point it SHOULD BE useless


    /**SOCKET CREATION**/
    sockClient=Socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sockClient<0){
        exit(FAILURE);
    }
#if DEBUG==0
    printf("**SOCKET created.\n");
#endif


    /**SETSOCKOPT SO_REUSEADDR**/
    if(setsockopt(sockClient,SOL_SOCKET,SO_REUSEADDR,&reuseC,sizeof(reuseC))){
        printf("**Error on socket option.\n");
        return(FAILURE);
    }


    /**BINDING**/
    if(Bind(sockClient,(struct sockaddr*)&proxyMaster,sizeof(proxyMaster))!=0){
        exit(FAILURE);
    }
#if DEBUG==0
    printf("**BIND success.\n");
#endif


    /**LISTEN**/
    if(Listen(sockClient,BACKLOG_QUEUE)<0){
        exit(FAILURE);
    }
#if DEBUG==0
    printf("**LISTEN success.\n");
#endif



    /**SETTING HANDLER FOR SIGCHLD**/
    saDeadChild.sa_handler = sigchld_handler; // reap all dead processes
    sigemptyset(&saDeadChild.sa_mask);
    saDeadChild.sa_flags = SA_RESTART;
    if(sigaction(SIGCHLD, &saDeadChild, NULL)==-1){
        printf("**Error while sigaction.\n");
        exit(FAILURE);
    }




    /**SETTING HANDLER FOR SIGINT (ONLY FATHER)**/
    saSignInt.sa_handler=sigint_handler;
    sigemptyset(&saSignInt.sa_mask);
    saSignInt.sa_flags=0;
    if(sigaction(SIGINT, &saSignInt, NULL)==-1){
        printf("**Error while sigaction.\n");
        exit(FAILURE);
    }


    /**SETTING HANDLER FOR SIGPIPE**/
    saSignPipe.sa_handler=SIG_IGN;
    sigemptyset(&saSignPipe.sa_mask);
    saSignPipe.sa_flags = SA_RESTART;
    if (sigaction(SIGPIPE, &saSignPipe, NULL) == -1) {
        printf("Error while sigaction.\n");
        exit(1);
    }




    /**NEVER-ENDED LOOP**/
    while(1){

        fail=SUCCESS;

        memset((void*)&client,0,sizeof(client)); //clear client address
        sockClientConnected=Accept(sockClient,(SA*)&client,&clientSize);
        if(sockClientConnected==-1){
            fail=FAILURE;
        }


        if(fail==SUCCESS){
#if DEBUG==0
    printf("**ACCEPT success.\n");
#endif
            /**FORKING**/
            if((childpid=fork())<0){
                printf("**Error in FORK: %s.\n",strerror(errno));
                //closing sockClientConnected: I do not watch the return value
                Close(sockClientConnected);
            }
            else{
                if(childpid==0){
                    /**CHILD**/
                    if(Close(sockClient)!=0){
                        Close(sockClientConnected);
                        exit(FAILURE);
                    }
                    exit(proxyAction(sockClientConnected));

                }
                else{
                    /**FATHER**/
                    numChildren++;
                    Close(sockClientConnected);

#if DEBUG==0
        printf("***CHILD created.***\n");
        printf("**Child PID: %d\n",childpid);
        char *clientAddress;
        clientAddress=stringAllocExing(STRING_LEN);
        Inet_ntop(AF_INET,&client.sin_addr,clientAddress,STRING_LEN);
        printf("Host connected: %s:%d\n",clientAddress,htons(client.sin_port));
        printf("Children: %d\n",numChildren);
        free(clientAddress);
#endif


                }
            }
        }
    }



return(SUCCESS);
}



/**********SIGNALS*************/

/**SIGCHLD HANDLER**/
void sigchld_handler(int s){
    while(waitpid(-1,NULL,WNOHANG) > 0){
        numChildren--;
    }
}

/**SIGCHLD HANDLER**/
void sigint_handler(int s){
    kill(0,SIGKILL);
    while(waitpid(-1,NULL,WNOHANG) > 0);
}







int proxyAction(int sockClient){

    int sockServer;
    int execution; //just to break while
    int selectRes;

    int byteRead,byteSent;

    void *buffer,*bufferBackup;
    int bufferAllocResult;
    int byteAlreadySent;
    int epipeError;

    fd_set multiplexing;

    int reuseS;


    /**SETTING HANDLER TO NOT IGNORE SIGINT**/
    saSignInt.sa_handler=SIG_DFL;
    sigemptyset(&saSignInt.sa_mask);
    saSignInt.sa_flags=0;
    if(sigaction(SIGINT, &saSignInt, NULL)==-1){
        printf("**Error while sigaction.\n");
        exit(FAILURE);
    }


    /**SOCKET CREATION**/
    sockServer=Socket(PF_INET,SOCK_STREAM,IPPROTO_TCP);
    if(sockClient<0){
        exit(FAILURE);
    }
#if DEBUG==0
    printf("**SOCKET created.\n");
#endif



    /**SETSOCKOPT SO_REUSEADDR**/
    if(setsockopt(sockServer,SOL_SOCKET,SO_REUSEADDR,&reuseS,sizeof(reuseS))){
        printf("**Error on socket option.\n");
        return(FAILURE);
    }




    /**PROXY has to connect to server**/
    if(Connect(sockServer,(SA*)&server,sizeof(server))!=0){
        Close(sockServer);
        exit(FAILURE);
    }
#if DEBUG==0
    printf("**CONNECTED to the server.\n");
#endif


    execution=CONTINUE;
    while(execution==CONTINUE){


            FD_ZERO(&multiplexing);
            FD_SET(sockClient,&multiplexing);
            FD_SET(sockServer,&multiplexing);

            if((selectRes=Select(FD_SETSIZE,&multiplexing,NULL,NULL,NULL))==-1){
                Close(sockClient);
                Close(sockServer);
                exit(FAILURE);
            }

            if(selectRes>0){
                if(FD_ISSET(sockClient,&multiplexing)){
                    /**CLIENT WAKES UP**/

                    buffer=bufferAllocReturning(BUFFER_LEN,&bufferAllocResult);
                    if(bufferAllocResult==1){
                        //program has to be closed: memory error
                        Close(sockClient);
                        Close(sockServer);
                        free(buffer);
                        exit(FAILURE);
                    }

                    byteRead=Recv(sockClient,buffer,BUFFER_LEN,0);

                    //printf("ByteRead :%d.\n",byteRead);
                    //printf("")
                    if(byteRead<0){
                        //program has to be closed: error

                        Close(sockClient);
                        Close(sockServer);
                        free(buffer);
                        exit(FAILURE);
                    }
                    if(byteRead==0){
                        //connection closed by client
#if DEBUG==0
                        printf("**Connection closed by client.\n");
#endif
                        Close(sockClient);
                        Close(sockServer);
                        free(buffer);
                        exit(SUCCESS);
                    }


                    /*PROXY has to send data to server*/

                    byteAlreadySent=0;
                    bufferBackup=buffer;
                    while(byteAlreadySent!=byteRead){
                        byteSent=Send(sockServer,buffer,(byteRead-byteAlreadySent),0,&epipeError);
                        if(byteSent<=0 && epipeError==1){
#if DEBUG==0
                            printf("**Connection lost (server-side).\n");
#endif
                            Close(sockClient);
                            Close(sockServer);
                            free(buffer);
                            exit(SUCCESS);
                        }
                        if(byteSent<0 && epipeError==0){
                            Close(sockClient);
                            Close(sockServer);
                            free(buffer);
                            exit(SUCCESS);
                        }

                        buffer+=byteSent;
                        byteAlreadySent+=byteSent;
                    }
                    free(bufferBackup);

                }
                else{
                    if(FD_ISSET(sockServer,&multiplexing)){
                        /**SERVER WAKES UP**/

                        buffer=bufferAllocReturning(BUFFER_LEN,&bufferAllocResult);
                        if(bufferAllocResult==1){
                            //program has to be closed: memory error
                            Close(sockClient);
                            Close(sockServer);
                            free(buffer);
                            exit(FAILURE);
                        }

                        byteRead=Recv(sockServer,buffer,BUFFER_LEN,0);

                        if(byteRead<0){
                            //program has to be closed: error

                            Close(sockClient);
                            Close(sockServer);
                            free(buffer);
                            exit(FAILURE);
                        }
                        if(byteRead==0){
                            //connection closed by server
    #if DEBUG==0
                            printf("**Connection closed by server.\n");
    #endif
                            Close(sockClient);
                            Close(sockServer);
                            free(buffer);
                            exit(SUCCESS);
                        }


                        /*PROXY has to send data to server*/

                        byteAlreadySent=0;
                        bufferBackup=buffer;
                        while(byteAlreadySent!=byteRead){
                            byteSent=Send(sockClient,buffer,(byteRead-byteAlreadySent),0,&epipeError);
                            if(byteSent<=0 && epipeError==1){
    #if DEBUG==0
                                printf("**Connection lost (client-side).\n");
    #endif
                                Close(sockClient);
                                Close(sockServer);
                                free(buffer);
                                exit(SUCCESS);
                            }

                            if(byteSent<0 && epipeError==0){
                                Close(sockClient);
                                Close(sockServer);
                                free(buffer);
                                exit(SUCCESS);
                            }
                            byteAlreadySent+=byteSent;
                            buffer+=byteSent;
                        }
                        free(bufferBackup);

                        }
                }
            }
    }




return(SUCCESS);
}
