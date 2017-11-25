#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <algorithm>
#include <strstream>
#include <locale>


using namespace std;

// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET leSocket1;// = INVALID_SOCKET;
	//SOCKET leSocket2;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char motEnvoye[250];
	char motRecu[250];
	char username[20];
	char password[20];
	string message;
	int iResult;

	//--------------------------------------------
	// InitialisATION de Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("Erreur de WSAStartup: %d\n", iResult);
		return 1;
	}
	// On va creer le socket pour communiquer avec le serveur
	leSocket1 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (leSocket1 == INVALID_SOCKET) {
		printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	//// On va creer le socket pour communiquer avec le serveur
	//leSocket2 = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	//if (leSocket2 == INVALID_SOCKET) {
	//	printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
	//	freeaddrinfo(result);
	//	WSACleanup();
	//	printf("Appuyez une touche pour finir\n");
	//	getchar();
	//	return 1;
	//}
	//--------------------------------------------
	// On va chercher l'adresse du serveur en utilisant la fonction getaddrinfo.
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;        // Famille d'adresses
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;  // Protocole utilisé par le serveur

	// On indique le nom et le port du serveur auquel on veut se connecter
	//char *host = "L4708-XX";
	//char *host = "L4708-XX.lerb.polymtl.ca";
	//char *host = "add_IP locale";
	//char *host = "132.207.29.XXX";
	printf("Entrez une addresse ip entre 132.207.29.101 et 132.207.29.127.");
	char host[16];
	char *port = "5030";
	gets_s(host);

	// getaddrinfo obtient l'adresse IP du host donné
	iResult = getaddrinfo(host, port, &hints, &result);
	if (iResult != 0) {
		printf("Erreur de getaddrinfo: %d\n", iResult);
		WSACleanup();
		return 1;
	}
	//---------------------------------------------------------------------		
	//On parcours les adresses retournees jusqu'a trouver la premiere adresse IPV4
	while ((result != NULL) && (result->ai_family != AF_INET))
		result = result->ai_next;

	//	if ((result != NULL) &&(result->ai_family==AF_INET)) result = result->ai_next;  

		//-----------------------------------------
	if (((result == NULL) || (result->ai_family != AF_INET))) {
		freeaddrinfo(result);
		printf("Impossible de recuperer la bonne adresse\n\n");
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	sockaddr_in *adresse;
	adresse = (struct sockaddr_in *) result->ai_addr;
	//----------------------------------------------------
	//printf("Adresse trouvee pour le serveur %s : %s\n\n", host,inet_ntoa(adresse->sin_addr));
	//printf("Tentative de connexion au serveur %s avec le port %s\n\n", inet_ntoa(adresse->sin_addr),port);

	// On va se connecter au serveur en utilisant l'adresse qui se trouve dans
	// la variable result.
	
	printf("\nEntrez le le username: ");
	cin >> username;
	printf("\nEntrez le mot de passe: ");
	cin >> password;

	iResult = connect(leSocket1, result->ai_addr, (int)(result->ai_addrlen));
	if (iResult == SOCKET_ERROR) {
		printf("Impossible de se connecter au serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}
	else
	{
		iResult = send(leSocket1, username, strlen(username), 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur du send: %d\n", WSAGetLastError());
			closesocket(leSocket1);
			WSACleanup();
			printf("Appuyez une touche pour finir\n");
			getchar();

			return 1;
		}

		printf("Nombre d'octets envoyes : %ld\n", iResult);

		//------------------------------
		// Maintenant, on va recevoir l' information envoyée par le serveur
		iResult = recv(leSocket1, motRecu, strlen(motRecu), 0);
		if (iResult > 0) {
			printf("Nombre d'octets recus: %d\n", iResult);
			motRecu[iResult] = '\0';
			printf("Le mot recu est %*s\n", iResult, motRecu);
		}
		else {
			printf("Erreur de reception : %d\n", WSAGetLastError());
		}
	}

	printf("Connecte au serveur %s:%s\n\n", host, port);

	//iResult = connect(leSocket2, result->ai_addr, (int)(result->ai_addrlen));
	//if (iResult == SOCKET_ERROR) {
	//	printf("Impossible de se connecter au serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
	//	freeaddrinfo(result);
	//	WSACleanup();
	//	printf("Appuyez une touche pour finir\n");
	//	getchar();
	//	return 1;
	//}
	printf("Connecte au serveur %s:%s\n\n", host, port);

	freeaddrinfo(result);

	bool enConnexion = true;
	while (enConnexion == true)
	{
		printf("\nEnvoyez un message (0Message: Authentification, 1Message: Message): ");
		cin >> message;

		if (message[0] == '0')
		{
		}

		else if (message[0] == '1')
		{
			//----------------------------
			// Demander à l'usager un mot a envoyer au serveur
			printf("Saisir un mot de 7 lettres pour envoyer au serveur: ");
			cin >> motEnvoye;

			//-----------------------------
			// Envoyer le mot au serveur
			iResult = send(leSocket1, motEnvoye, 10, 0);
			if (iResult == SOCKET_ERROR) {
				printf("Erreur du send: %d\n", WSAGetLastError());
				closesocket(leSocket1);
				WSACleanup();
				printf("Appuyez une touche pour finir\n");
				getchar();

				return 1;
			}

			printf("Nombre d'octets envoyes : %ld\n", iResult);

			//------------------------------
			// Maintenant, on va recevoir l' information envoyée par le serveur
			iResult = recv(leSocket1, motRecu, 10, 0);
			if (iResult > 0) {
				printf("Nombre d'octets recus: %d\n", iResult);
				motRecu[iResult] = '\0';
				printf("Le mot recu est %*s\n", iResult, motRecu);
			}
			else {
				printf("Erreur de reception : %d\n", WSAGetLastError());
			}
		}

		else if (message[0] == '3')
		{
			enConnexion = false;
		}
	}
	// cleanup
	closesocket(leSocket1);
	//closesocket(leSocket2);
	WSACleanup();

	printf("Appuyez une touche pour finir\n");
	getchar();
	return 0;
}