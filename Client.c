#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <unistd.h>

struct sockaddr_in address; 
int sock = 0, valread; 
struct sockaddr_in serv_addr; 
char buffer[5000]={0};
int conn=0;

char s[5000],tokens[5][5000],*p;
int nos,i,c,a1,a2,turn;
    
    
void connect1(char *ip,char *port)
{
    if(conn==1)
    {
        printf("Error:You are already connected to a server...\nYou need to disconnect before connecting to another server\n");
        return;
    }
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return; 
    } 
    memset(&serv_addr, '0', sizeof(serv_addr)); 
    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(atoi(port)); 
       
    if(inet_pton(AF_INET,ip, &serv_addr.sin_addr)<=0)  
    { 
        printf("\nInvalid address/ Address not supported \n");
        return; 
    }
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return;
    }
    conn=1;
    printf("Connected");
}

void disconnect()
{
    if(conn==0)
    {
        printf("Error:You are not connected to server...\nYou need to connect before disconnecting\n");
        return;
    }
    char *s="disconnect";
    send(sock,s,strlen(s),0);
    shutdown(sock,2);
    conn=0;
    printf("Disconnected\n");
}

int common(char *s)
{
    int i;
    i=0;
    p=s;
    nos=c=0;
    //printf("%s\n\n\n\n",s);
    for(i=0;i<strlen(s) && s[i]!='\n';i++)
    {
        if(s[i]==' ')
        {    
        	if(nos==0)
            	tokens[nos][c]='\0';
            nos++;
            if(nos==4)
            {
                nos-=1;
                for(;i<strlen(buffer);i++)
                    tokens[nos][c++]=buffer[i];
                break;
            }
            c=0;
        }
        else
            tokens[nos][c++]=s[i];
    }
    tokens[nos][c]='\0';
    if(!strcmp(tokens[0],"connect") && nos==2)
    {
        printf("Connecting");
        connect1(tokens[1],tokens[2]);
    }
    else if(!strcmp(tokens[0],"disconnect") && nos==0)
    {
        printf("Disconnecting...\n");
        disconnect();
    }
    else
    {
        if(conn==0)
            printf("Error:Connect to server...\n");   
        else if(conn==1 && !strcmp(tokens[0],"create") && nos>=3)
        {
            send(sock,p,strlen(p),0);
            valread = read( sock , buffer, 5000); 
            printf("%s\n",buffer );
        }
        else if(conn==1 && !strcmp(tokens[0],"read") && nos==1)
        {
            send(sock,p,strlen(p),0);
            valread = read( sock , buffer, 5000); 
            printf("%s\n",buffer );   
        }
        else if(conn==1 && !strcmp(tokens[0],"update") && nos>=3)
        {
            send(sock,p,strlen(p),0);
            valread = read( sock , buffer, 5000); 
            printf("%s\n",buffer );   
        }
        else if(conn==1 && !strcmp(tokens[0],"delete") && nos==1)
        {
            send(sock,p,strlen(p),0);
            valread = read( sock , buffer, 5000); 
            printf("%s\n",buffer );
        }
        else if(conn==1 && !strcmp(tokens[0],"display") && nos==0)
        {
            send(sock,p,strlen(p),0);
        }
        else
            printf("Incorrect arguments entered...\n");
        for(i=0;i<5000;i++)
            buffer[i]='\0';
    }
    for(i=0;i<5000;i++)
        s[i]=' ';
    return 1;
}

void interactive()
{
    int x;
    while(1)
    {   
        fflush(stdin);
        printf("Enter:");
        fgets(s,4096,stdin);
        x=common(s);
    } 
    
}
   
void batch(const char *abc)
{
    FILE *fptr;
    char c;
    int i=0,x=-1;
    long int y=0;
    fptr=fopen(abc,"r");
    c=fgetc(fptr);
    while(c!=EOF)
    {
        if(c!='\n')
            s[i++]=c;
        else
        {
            s[i]='\0';
            //for(y=0;y<strlen(s);y++)
            //    printf("%c",s[y]);
            i=0;
            //printf("\n%ld",y);
            sleep(2);
            x=common(s);
            while(x!=1);
            x=0;
        }
        c=fgetc(fptr);
    }
    s[i]='\0';
    common(s);
    fclose(fptr);
}

int main(int argv, char const *argc[]) 
{ 
    if(argv==2 && !strcmp(argc[1],"interactive"))
    {
        printf("You are operating in interactive mode...\n");
        interactive();
    }
    else if(argv==3 && !strcmp(argc[1],"batch"))
    {
        //printf("%s\n",argc[2]);
        printf("You are operating in batch mode...\n");
        if( access(argc[2], F_OK ) == -1)
            printf("Error: File not found...\nExiting...\n");
        else
            batch(argc[2]);
    }
    else
        printf("Incorrect arguments passed\nExiting...\n");
    return 0; 
 
} 