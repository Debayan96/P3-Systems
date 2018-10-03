#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<pthread.h>
//THREAD VALUES
int v1 = 1;
int v2 = 2;
int v3 = 3;
int v4 = 4;
//SERVER DETAILS
int server_fd, new_socket, valread; 
struct sockaddr_in address; 
int opt = 1; 
int addrlen = sizeof(address);
char buffer[1024];
//THE THREADS
pthread_t t1,t2,t3,t4;
//THE MUTEXS
pthread_mutex_t mutex_table = PTHREAD_MUTEX_INITIALIZER;
//THE CONDITIONAL VARIABLES
pthread_cond_t busy= PTHREAD_COND_INITIALIZER;
//HASH TABLE
typedef struct hashtable
{
	int key;
	char *value;
	struct hashtable* next;
}ht; 
//HASH TABLE CREATED
ht *table=NULL;

ht* create(int key,int size,char *value)//CHECKED: create a new node
{
	ht *h=(ht*)malloc(sizeof(ht));
	h->key=key;
	h->value=(char*)malloc(sizeof(char)*size);
	strcpy((h->value),value);
	*(h->value+size)='\0';
	h->next=NULL;
	return h;
}

int htsearch(int key)//CHECKED: checks whether key is present or not
{
	if(table==NULL)
		return -1;
	ht *p=table;
	while(p!=NULL && p->key!=key)
		p=p->next;
	if(p!=NULL)
		return 1;
	else 
		return -1;
}

char* hvalue(int key)//CHECKED: returns value corresponding to the key
{
	ht *p=table;
	while(p->key!=key)
		p=p->next;
	return p->value;
}

void htappend(int key,int size,char* value)//CHECKED: Appends to the table
{
	ht *p=table;
	if(table==NULL)
	{
		table=create(key,size,value);
		return;
	}
	while(p->next!=NULL)
		p=p->next;
	p->next=create(key,size,value);
}

void delete(int key)//CHECKED: Deletes key value pair from table
{
	if(table->key==key)
	{
		ht *x=table;
		table=table->next;
		free(x);
		return;
	}
	ht *p=table;
	while(p->next->key!=key)
		p=p->next;
	ht *x=p->next;
	p->next=p->next->next;
	free(x);
}

void update(int key,int size,char* value)//CHECKED: Updates the key value pair
{
	ht *p=table;
	while(p->key!=key)
		p=p->next;
	free(p->value);
	p->value=(char*)malloc(size*sizeof(char));
	strcpy(p->value,value);
	*(p->value+size)='\0';
}

void display()//CHECKED: Displays the contents
{
	ht *p=table;// p points to table
	while(p!=NULL)
	{
		printf("key=%d value=%s\n",p->key,p->value);
		p=p->next;
	}
}

void *serve(void *data)//NOT CHECKED: Handle the requests 
{
	char tokens[4][1000],*msg;
	char *msg1="OK",*msg2="Error:Key already exists",*msg3="Error:Key does not exist";
	int stillconnect=0,c,nos,i;
	while(1)
	{
		int new_socket;
		new_socket = accept(server_fd, (struct sockaddr *)&address,(socklen_t*)&addrlen);
		stillconnect=1;
		while(stillconnect)
		{
			free(msg);
			msg=(char*)malloc(1024*sizeof(char));
			for(i=0;i<1024;i++)
				buffer[i]='\0';
			read(new_socket , buffer, 1024);
			if(!strcmp(buffer,""))
			{
				stillconnect=0;
				break;
			}
			c=nos=0;
			printf("recvd:%s\n",buffer);
			for(i=0;i<strlen(buffer) && buffer[i]!='\n';i++)
	        {
	            if(buffer[i]==' ')
	            {    
	                tokens[nos][c]='\0';
	                nos++;
	                c=0;
	            }
	            else
	                tokens[nos][c++]=buffer[i];
	        }
	        if(!strcmp(buffer,"disconnect"))
			{	
				stillconnect=0;
				break;
			}
			else if(!strcmp(tokens[0],"create"))
			{	
				pthread_mutex_lock(&mutex_table);
				if(htsearch(atoi(tokens[1]))==1)
				{
					send(new_socket , msg2 , strlen(msg2) , 0 );
				}
				else
				{
					htappend(atoi(tokens[1]),atoi(tokens[2]),tokens[3]);
					send(new_socket , msg1 , strlen(msg1) , 0 );	
				}
				pthread_mutex_unlock(&mutex_table);
			}
			else if(!strcmp(tokens[0],"read"))
			{	
				pthread_mutex_lock(&mutex_table);
				if(htsearch(atoi(tokens[1]))==-1)
				{
					send(new_socket , msg3 , strlen(msg3) , 0 );
				}
				else
				{
					char *msg5=hvalue(atoi(tokens[1]));
					send(new_socket , msg5 , strlen(msg5) , 0 );	
				}
				pthread_mutex_unlock(&mutex_table);
			}
			else if(!strcmp(tokens[0],"update"))
			{
				pthread_mutex_lock(&mutex_table);	
				if(htsearch(atoi(tokens[1]))==-1)
				{
					send(new_socket , msg3 , strlen(msg3) , 0 );
				}
				else
				{
					update(atoi(tokens[1]),atoi(tokens[2]),tokens[3]);
					send(new_socket , msg1 , strlen(msg1) , 0 );	
				}
				pthread_mutex_unlock(&mutex_table);
			}
			else if(!strcmp(tokens[0],"delete"))
			{
				pthread_mutex_lock(&mutex_table);	
				if(htsearch(atoi(tokens[1]))==-1)
				{
					send(new_socket , msg3 , strlen(msg3) , 0 );
				}
				else
				{
					//printf("%s",tokens[1]);
					delete(atoi(tokens[1]));
					send(new_socket , msg1 , strlen(msg1) , 0 );	
				}
				pthread_mutex_unlock(&mutex_table);
			}
			else if(!strcmp(tokens[0],"display"))
			{
				pthread_mutex_lock(&mutex_table);	
				display();
				pthread_mutex_unlock(&mutex_table);
			}
		}
	}
}

void sockcreate(char *ip,char *port)//CHECKED: Creates the socket
{
	if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        printf("Error: Socket not created"); 
        exit(1); 
    }    
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT,&opt, sizeof(opt))) 
    { 
       	printf("Error:setsockopt setup failure"); 
        exit(1); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons(atoi(port)); 
    if (bind(server_fd, (struct sockaddr *)&address,sizeof(address))<0) 
    { 
       	printf("Error:Binding failed"); 
        exit(1); 
    } 
    if (listen(server_fd, 4) < 0) 
    { 
       	printf("Error:Failed to listen"); 
        exit(1); 
    }
}

int main(int argv,char* argc[])
{
	if(argv!=3)
	{
		printf("Error: Parameters missing\n");
		return 0;
	}
     
	sockcreate(argc[1],argc[2]);
     
    pthread_create(&t1, NULL,serve, (void *)&v1);
    pthread_create(&t2, NULL,serve, (void *)&v2);
    pthread_create(&t3, NULL,serve, (void *)&v3);
    pthread_create(&t4, NULL,serve, (void *)&v4);
    
    pthread_join(t1,NULL);
    pthread_join(t2,NULL);
    pthread_join(t3,NULL);
    pthread_join(t4,NULL);
    
    return 0;
}