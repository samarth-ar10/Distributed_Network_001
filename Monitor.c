#include "header.h"
#define TRUE 1
#define FALSE 0

struct nodule{
	char *name;
	int socketAddress;
	pid_t pid;
	pid_t ppid;
	struct nodule *next;
	struct nodule *prev;
};

struct pending{
	struct nodule *node;
	char *data;
	int size;
	struct pending *prev;
	struct pending *next;
};

void nodeInsert(struct nodule *node, char *recv_buffer, int n){
	struct nodule *temp;
	temp = (struct nodule *)malloc(sizeof(struct nodule));
	char *str,*c;
	c = (char *)malloc(sizeof(char)*100);
	str =  (char *)malloc(sizeof(char)*100);

	int p=0;
	while(recv_buffer[n++]!='@')
		c[p++]=	recv_buffer[n-1];
	c[p]='\0';
	temp->socketAddress=atoi(c);
	printf("socketAddress -> %d \n", temp->socketAddress);

	p=0;
	while(recv_buffer[n++]!='@')
		c[p++]=	recv_buffer[n-1];
	c[p]='\0';
	temp->pid=atoi(c);
	printf("pid -> %d \n", temp->pid );

	p=0;
	while(recv_buffer[n++]!='@')
		c[p++]=	recv_buffer[n-1];
	c[p]='\0';
	temp->ppid=atoi(c);
	printf("ppid -> %d\n", temp->ppid );

	p=0;
	while(recv_buffer[n++]!='@')
		str[p++] = recv_buffer[n-1];
	str[p]='\0';
	temp->name = str;
	printf("name -> %s\n", temp->name );

	while (node->next != NULL)
		node = node->next;

	node->next = temp;
	temp->next = NULL;
	temp->prev = node;

	return;
}

void nodeDelete(struct nodule *node, char *recv_buffer, int n){
	
	struct nodule *temp;
	temp = (struct nodule *)malloc(sizeof(struct nodule));
	char str[100],*c;
	int p=0;
	c = (char *)malloc(sizeof(char)*100);

	//----------------------temporarily checking only the name
	while(recv_buffer[n++]!='@')
		c[p++]=	recv_buffer[n-1];
	c[p]='\0';

	p=0;
	while(recv_buffer[n++]!='@')
		c[p++]=	recv_buffer[n-1];
	c[p]='\0';
	
	p=0;
	while(recv_buffer[n++]!='@')
		c[p++]=	recv_buffer[n-1];
	c[p]='\0';
	
	p=0;
	while(recv_buffer[n++]!='@')
		str[p++] = recv_buffer[n-1];
	str[p]='\0';
	
	printf("name -> %s\n", str );

	while (node != NULL && strcmp(node->name, str) )
		node = node->next;
	
	if (node != NULL && !strcmp(node->name, str)){
		temp=node->prev;
		temp->next=node->next;
	}
	/*
	if(node->next != NULL){
		temp=node->next;
		temp->prev=node->prev;
	}
	free (node);
	*/
	return;
}

void pendingInsert(struct pending *pend, struct nodule *node, char *recv_buffer , int n, int Ssockfd){
	char str[100];
	int p=0;
	//----------------------temporarily checking only the name

	
}

void main(int argc, char *argv[]){
	
	//-------------------------------------------------------------initialization process 
	int Ssockfd,Csocketfd;
	int Slen,Clen;
	struct sockaddr_in Saddr,Caddr;
	int err;
	char *recv_buffer,*send_buffer,str[100];
	recv_buffer = (char *)malloc(sizeof(char)*256);
	send_buffer = (char *)malloc(sizeof(char)*256);


	//------------------creating the log file
	FILE *fp = fopen("log_file.txt","w");
	fprintf(fp,"\t\t\t\t\t\t\t\t\tStarting log\n");
	printf("printed");
	//------------------current node defination
	
	struct nodule *node;
	node = (struct nodule *)malloc(sizeof (struct nodule)*100);
	node->name="main";
	node->pid=getpid();
	node->prev=node->next=NULL;
	node->ppid=getppid();

	struct pending *pend = NULL;

	//-------------------------------------------------------------socket Defining 
	Ssockfd = socket(AF_INET,SOCK_STREAM,0);
	if (Ssockfd < 0){
		perror("\n!!!Error: socket setup failed!!!\n");
		getchar();
		exit(1);
	}
	//------------------setting up the server address
	
	Saddr.sin_family = AF_INET;
	Saddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	Saddr.sin_port = 0;
	Slen = sizeof(Saddr);

	//------------------setting up Caddr to register main
	//Caddr.sin_family = 	AF_INET;
	//Caddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	//Caddr.sin_port = atoi(argv[1]);
	//------------------binding to the socket
	
	err=bind(Ssockfd, (struct sockaddr *)&Saddr, Slen);
	if (err < 0){
		perror("\n!!!Error: bind failed!!!\n");
		getchar();
		exit(1);
	}
	//------------------listening to the socket

	err=listen(Ssockfd,100);
	
	
	if (err < 0){
		perror("\n!!!Error: listen failed!!!\n");
		getchar();
		exit(1);
	}
	//------------------setting up the current port address
	
	getsockname(Ssockfd, (struct sockaddr *)&Saddr, &Slen);
	node->socketAddress = Saddr.sin_port;
	printf("\n%d\n",node->socketAddress);
	//------------------attaching the main module using the process id passed in argv[1]

	//------------------woring on the sockets(accepting, reading, wrting, etc.)

	while (TRUE){
		Csocketfd=accept(Ssockfd,(struct sockaddr *)&Caddr,&Clen);
		
		if (Csocketfd < 0){
			perror("\n!!!Error: accept failed!!!\n");
			getchar();
			continue;
		}

		//Clen=sizeof(Caddr);
		recv(Csocketfd , recv_buffer, 256*sizeof(char), MSG_WAITALL);
		printf("%s\n", recv_buffer);
		int n=0; 
			while(recv_buffer[n++]!='@')
				str[n-1] = recv_buffer[n-1];
		str[n-1]='\0';
		//------------------attaching the connection to the node
		if(!strcmp(str,"attach")){
			printf("attached\n");
			nodeInsert(node, recv_buffer,n);
		}
			
		//------------------detaching the connection to the node
		if (!strcmp(str,"detach"))
			nodeDelete(node, recv_buffer, n);
	/*

		//------------------processing request to forward the data
		if(!strcmp(str,"push"))
			pendingInsert(pend, node, recv_buffer , n, Ssockfd);

		//------------------processing request to pull the data
		if (!strcmp(str,"pull"))
		{
			
		}
	*/
	}
	
	
	return;	
}