/*H********************************************************************************
* Ime datoteke: serverLinux.cpp
*
* Opis:
*		Enostaven stre�nik, ki zmore sprejeti le enega klienta naenkrat.
*		Stre�nik sprejme klientove podatke in jih v nespremenjeni obliki po�lje
*		nazaj klientu - odmev.
*
* Testiranje strežnika:
*		nc -v localhost 1024
*
*H*/

//Vklju�imo ustrezna zaglavja
#include<stdio.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<pthread.h>
/*
Definiramo vrata (port) na katerem bo stre�nik poslu�al
in velikost medponilnika za sprejemanje in po�iljanje podatkov
*/
#define PORT 1024
#define BUFFER_SIZE 256
#define CLIENT_NUMBER 2

struct clientData {
	int free;
	int clientSock;
};

void *serveClientThread (void* arg) {

		struct clientData *data;

		data = (struct clientData *) arg;

		int clientSock = data->clientSock;
		int iResult;

		char buff[BUFFER_SIZE];

		//Postrezi povezanemu klientu
		do{
			//Sprejmi podatke
			iResult = recv(clientSock, buff, BUFFER_SIZE, 0);
			if (iResult > 0) {
				printf("Bytes received: %d\n", iResult);

				//Vrni prejete podatke po�iljatelju
				iResult = send(clientSock, buff, iResult, 0 );
				if (iResult == -1) {
					printf("send failed!\n");
					close(clientSock);
					break;
				}
				printf("Bytes sent: %d\n", iResult);
			}
			else if (iResult == 0)
				printf("Connection closing...\n");
			else{
				printf("%d recv failed! %d \n", iResult, clientSock);
				close(clientSock);
				break;
			}

		} while (iResult > 0);

		close(clientSock);
		data->free = 1;
}

int main(int argc, char **argv){

	//Spremenjlivka za preverjane izhodnega statusa funkcij
	int iResult;

	/*
	Ustvarimo nov vti�, ki bo poslu�al
	in sprejemal nove kliente preko TCP/IP protokola
	*/
	int listener=socket(AF_INET, SOCK_STREAM, 0);
	if (listener == -1) {
		printf("Error creating socket\n");
		return 1;
	}

	//Nastavimo vrata in mre�ni naslov vti�a
	sockaddr_in  listenerConf;
	listenerConf.sin_port=htons(PORT);
	listenerConf.sin_family=AF_INET;
	listenerConf.sin_addr.s_addr=INADDR_ANY;

	//Vti� pove�emo z ustreznimi vrati
	iResult = bind( listener, (sockaddr *)&listenerConf, sizeof(listenerConf));
	if (iResult == -1) {
		printf("Bind failed\n");
		close(listener);
		return 1;
    }

	//Za�nemo poslu�ati
	if ( listen( listener, 5 ) == -1 ) {
		printf( "Listen failed\n");
		close(listener);
		return 1;
	}

	//Definiramo nov vti� in medpomnilik
	int clientSock;
	char buff[BUFFER_SIZE];

	struct clientData data[CLIENT_NUMBER];
	pthread_t thread[CLIENT_NUMBER];
	for(int i=0; i<CLIENT_NUMBER; i++) {
		data[i].free = 1;
	}

	int threadFound = 0;

	/*
	V zanki sprejemamo nove povezave
	in jih stre�emo (najve� eno naenkrat)
	*/

	while (1)
	{
		threadFound = 0;
		clientSock = accept(listener,NULL,NULL);

		for(int i=0; i<CLIENT_NUMBER; i++) {
			if(data[i].free == 1) {
				threadFound = 1;
				//Sprejmi povezavo in ustvari nov vti�
				printf("connection accepted, %d \n", i);
				if (clientSock == -1) {
					printf("Accept failed\n");
					break;
					close(clientSock);
					return 1;
				}
				data[i].clientSock = clientSock;
				data[i].free = 0;

				pthread_create(&thread[i], NULL, serveClientThread, (void *) &data[i]);
				pthread_detach(thread[i]);

				break;
			}
		}


		if(threadFound != 1 && clientSock != -1) {
			printf("server full\n");
			close(clientSock);
		}

		clientSock = -1;
	}

	pthread_exit(NULL);

	//Po�istimo vse vti�e
	close(listener);

	return 0;
}
