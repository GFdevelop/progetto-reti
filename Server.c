#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
//#include <sys/types.h>
//#include <netinet/in.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
//#include <netdb.h>
#include <arpa/inet.h>

#define MAXLINE	512
/*
int checkToken(char d[],int i){
  if (strcmp(d,"\0")==0) return -1;
  printf("1\n");
  if((d[i]!='S')&(d[0]!='s')&(d[0]!='D')&(d[0]!='d')&(d[0]!='A')&(d[0]!='a')&(d[0]!='B')&(d[0]!='b')) return -1;
  if (d[i+1]!='(') return -1;
  if (d[i+2]=='-') i=i+3;
  else i=i+2;
  printf("2\n");
  int Is=i;
  while ((d[i]=='1')|(d[i]=='2')|(d[i]=='3')|(d[i]=='4')|(d[i]=='5')|(d[i]=='6')|(d[i]=='7')|(d[i]=='8')|(d[i]=='9')|(d[i]=='0'))
    i=i+1;
  printf("3\n");
  if (i==Is) return -1;
  printf("33\n");
  if (d[i]!=')') return -1;
  printf("34\n");
  if ((d[i+1]!=' ')&(d[i+1]!='\0')&(d[i+1]!='\n')&(d[i+1]!='\r')) return -1;
  printf("4\n");
  return i+1;
}

int checkString(char d[]){
    int i = -1;
	do{
		i=checkToken(d,i+1);
	}
    while ((i!=-1)&(d[i]!='\0')&(d[i]!='\n')&(d[i]!='\r'));
	if (i==-1) return -1;
    return 0;
}*/

int check(char d[]){
	int i = 2;
	if (strcmp(d,"\0")==0) return -1;
    if((d[0]!='S')&(d[0]!='s')&(d[0]!='D')&(d[0]!='d')&(d[0]!='A')&(d[0]!='a')&(d[0]!='B')&(d[0]!='b')) return -1;
	if (d[1]!='(') return -1;
	if (d[2]=='-') i=3;
	while ((d[i]=='1')|(d[i]=='2')|(d[i]=='3')|(d[i]=='4')|(d[i]=='5')|(d[i]=='6')|(d[i]=='7')|(d[i]=='8')|(d[i]=='9')|(d[i]=='0'))
		i=i+1;
	if ((i==2)|((i==3) & ((i-1)=='-'))) return -1;
	if (d[i]!=')') return -1;
	//if ((d[i+1]!='\0')&(d[i+1]!='\n')&(d[i+1]!='\r')) return -1;
	return 0;
}

void tokenize(char buf[]){
	char * punt = &buf[0];
	char * token;
	int boolean = 0;
	int n;
	while ((punt[0] != '\0')&(boolean != -1)){
		sscanf(punt,"%ms%n",&token,&n);
		punt = &punt[n+1];
		//punt=strchr(punt+1,' ');
		boolean = check(token);
	}
	if (boolean == -1){
		sprintf(buf,"Errore stringa: %s\0",token);
	}
	else {
		strcpy(buf,"ok");
	}
	free(token);
}

int main(int argc, char* argv[]){
	struct sockaddr_in servaddr, client_addr;
	int listenfd, connfd, errore, lung, pid, boolean = 0;
	char buf[MAXLINE];
	int len=sizeof(client_addr);
	char * str;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
		printf("socket%d\n",listenfd);

	memset(&servaddr,0 , sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));
	printf("local address: IP %s port %d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port) );

	errore = bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));
		printf("bind%d\n",errore);
	
	errore = listen(listenfd, 10);
		printf("listen%d\n",errore);

	for( ; ; ){
		strcpy(buf,"\0");
		connfd = accept(listenfd, (struct sockaddr*) &client_addr, &len);
		printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
		pid = fork();
		if ( pid !=0 ){
			close ( connfd );
		}
		else {
			close(listenfd);
			while ((lung = recv(connfd, buf, MAXLINE, 0))>0){
				buf[lung] = '\0';
				printf("%s\n",buf);
			//	send(connfd, buf, strlen(buf)+1, 0);
				/*for (str = strtok(buf," "); (str != NULL)&(boolean != -1); str = strtok(NULL, " ")){
					//puts(str);
					boolean = check(str);
					printf("%s\n",str);
				}*/
				tokenize(buf);
				printf("%s\n",buf);
				/*
				boolean = checkString(buf);
				printf("%d\n",boolean);
				sprintf(buf,"%d\0",boolean);*/
				send(connfd, buf, strlen(buf)+1, 0);
				if (boolean == -1){
					send(connfd, str, strlen(str)+1, 0);
				}
				//send(connfd, buf, strlen(buf)+1, 0);
				strcpy(buf,"\0");
			}
			printf("%s:%d connection closed\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			close(connfd);
			exit(0);
		}
	}
    return 0;
}