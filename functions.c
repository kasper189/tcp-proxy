
#include "functions.h"


char *stringAllocExing(int len){
    char *tmp;

    tmp=(char*)malloc(len*sizeof(char));
    if(tmp==NULL){
        printf("**Error while allocating string.\n");
        exit(FAILURE);
    }

return(tmp);
}


char *stringAllocReturning(int len,int *result){
    char *tmp;

    tmp=(char*)malloc(len*sizeof(char));
    if(tmp==NULL){
        printf("**Error while allocating string.\n");
        *result=1;
    }else{
        *result=0;
    }

return(tmp);
}


void *bufferAllocReturning(int len,int *result){
    void *tmp;

    tmp=(void*)malloc(len*sizeof(void));
    if(tmp==NULL){
        printf("**Error while allocating buffer.\n");
        *result=1;
    }else{
        *result=0;
    }

return(tmp);
}

int checkPort(char *port){
    int i,len,intPort;

    len=strlen(port);
    for(i=0;i<len;i++){
        if(isdigit(port[i])==0){
            return(FAILURE);
        }
    }

    intPort=atoi(port);

    if(intPort<0 || intPort>USHRT_MAX){
        return(FAILURE);
    }


return(SUCCESS);
}
