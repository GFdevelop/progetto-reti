#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define MAXLINE	512

int check(char d[], int *a, int *b){
	if (strcmp(d,"\0")==0) return -1;
	if (d[1]!='(') return -1;
	char* p = &d[2];
	int num = (int)strtoul(p, &p, 10);
	
	if (((d[2])=='-')&&(num > 0)) return -1;
	else if (((d[2])=='+')&&(num < 0)) return -1;
	else if ((((int)d[2] >= 0)&&((int)d[2] <= 9))&&(num < 0)) return -1;
	
	if ((*p!=')')||(d[2]==')')) return -1;
	if (p[1]!='\0') return -1;
	if ((d[0]=='S')||(d[0]=='s'))*a = -(-(*a)+num);
	else if ((d[0]=='D')||(d[0]=='d'))*a=*a+num;
	else if ((d[0]=='A')||(d[0]=='a'))*b=*b+num;
	else if ((d[0]=='B')||(d[0]=='b'))*b = -(-(*b)+num);
	else return -1;
	return 0;
}

void tokenize(char buf[], int *x, int *y){
	char * punt = &buf[0];
	char * token;
	int n, boolean = 0;
	int a = *x;
	int b = *y;
	while ((punt[0] != '\0')&(boolean != -1)){
		sscanf(punt,"%ms%n",&token,&n);
		punt = &punt[n+1];
		boolean = check(token, &a, &b);
	}
	if (boolean == -1){
		sprintf(buf,"Errore stringa: %s",token);
	}
	else {
		*x=a;
		*y=b;
		sprintf(buf,"[%d,%d]",*x,*y);
	}
	free(token);
}

int main(int argc, char* argv[]){
    if (argc!=2){
        printf("Hai dato un numero sbagliato di argomenti!!\n");
        return 0;
    }
	struct sockaddr_in servaddr, client_addr;
	int listenfd, connfd, pid = 1, x = 0, y = 0;
	char buf[MAXLINE];
	socklen_t len=sizeof(client_addr);
	ssize_t lung;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	if (listenfd < 0){
		printf("Errore socket\n");
		exit(1);
	}

	memset(&servaddr,0 , sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port        = htons(atoi(argv[1]));
	printf("local address: IP %s port %d\n", inet_ntoa(servaddr.sin_addr), ntohs(servaddr.sin_port) );

	if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr))<0){
		printf("Errore bind\n");
		exit(2);
	}
	if (listen(listenfd, 10)<0){
		printf("Errore listen\n");
		pid = 0;
	}

	while(pid > 0){
		memset ( &client_addr, 0, sizeof(client_addr) );
		len=sizeof(client_addr);
		connfd = accept(listenfd, (struct sockaddr*) &client_addr, &len);
		if (connfd < 0){
			printf("Errore accept\n");
			pid = 0;
		}
		else{
			printf("%s:%d connected\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
			pid = fork();
			if ( pid > 0 ){
				close(connfd);
			}
			else if (pid == 0){
				close(listenfd);
				strcpy(buf,"\0");
				while ((lung = recv(connfd, buf, MAXLINE, 0)) > 0){
					buf[lung] = '\0';
					tokenize(buf, &x, &y);
					if (send(connfd, buf, strlen(buf)+1, 0) < 0){
						printf("Errore send\n");
						exit (3);
					}
					strcpy(buf,"\0");
				}
				if (lung == -1){
					printf("Errore recv\n");
					exit (4);
				}
				printf("%s:%d connection closed\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
				close(connfd);
			}
			else {
				printf("Errore fork\n");
				close(connfd);
				close(listenfd);
				exit(4);
			}
		}
	}
    return 0;
}