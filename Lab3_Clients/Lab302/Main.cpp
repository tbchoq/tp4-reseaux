#undef UNICODE

#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include <iostream>
#include <algorithm>
#include <strstream>
#include <locale>
#include <sstream>


using namespace std;

// Link avec ws2_32.lib
#pragma comment(lib, "ws2_32.lib")

int ValidateInput(char* ip, string portStr) {
	int num;
	int flag = 1;
	int counter = 0;
	bool ipOk = false;
	int port = 0;

	std::istringstream ss(portStr);
	ss >> port;
	if (!ss.fail()) {
		char* p = strtok(ip, ".");

		while (p && flag) {
			num = atoi(p);

			if (num >= 0 && num <= 255 && (counter++ < 4)) {
				flag = 1;
				p = strtok(NULL, ".");

			}
			else {
				flag = 0;
				break;
			}
		}

		ipOk = flag && (counter == 4);
		return ((ipOk && port <= 5050 && port >= 5000) ? port : 0);
	}
	return 0;

}

int __cdecl main(int argc, char **argv)
{
	WSADATA wsaData;
	SOCKET SocketEnvoi;// = INVALID_SOCKET;
	//SOCKET leSocket2;
	struct addrinfo *result = NULL,
		*ptr = NULL,
		hints;
	char motEnvoye[220];
	char motRecu[202];
	string username;
	string password;
	bool stepConnexion = false;
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
	SocketEnvoi = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketEnvoi == INVALID_SOCKET) {
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

	string entreeIP = "";
	string portString = "";
	//bool ipWrong = true;
	int iport = 0;


	while (iport == 0)
	{
		printf("Entrez l'addresse ip du serveur sous format xxx.xxx.xxx.xxx, puis appuyez sur retour de ligne. \t");
		std::cin >> entreeIP;
		printf("Entrez le port sur lequel se connecter entre 5000 5050 puis appuyez sur retour de ligne. \t");
		std::cin >> portString;

		char *ip = new char[entreeIP.length()];
		for (int i = 0; i < entreeIP.length(); i++) {
			ip[i] = entreeIP[i];
		}


		iport = ValidateInput(ip, portString);

		if (iport == 0) {
			printf("Svp, entrez des valeurs valides");
		}

	}

	char *host = &entreeIP[0u];
	char *port = &portString[0u];


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

	string str1 = "";
	string str2 = "";
	iResult = connect(SocketEnvoi, result->ai_addr, (int)(result->ai_addrlen));
	if (iResult == SOCKET_ERROR) {
		printf("Impossible de rejoindre le serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
		freeaddrinfo(result);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}
	else
	{
		printf("\nEntrez votre username. Si vous etes un nouvel utilisateur, entrez votre nouveau username: ");
		cin >> username;
		str1 = username;
		str2 = "10" + str1;
		char* usernameOut = &str2[0u];
		iResult = send(SocketEnvoi, usernameOut, strlen(usernameOut), 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur de send du username: %d\n", WSAGetLastError());
			closesocket(SocketEnvoi);
			WSACleanup();
			printf("Appuyez une touche pour finir\n");
			getchar();

			return 1;
		}

		//printf("Nombre d'octets envoyes : %ld\n", iResult);

		//------------------------------
		// Maintenant, on va recevoir l'information envoyée par le serveur
		iResult = recv(SocketEnvoi, motRecu, strlen(motRecu), 0);
		if (iResult > 0) {
			if (motRecu[0] == '1' && motRecu[1] == '1')
			{
				printf("\nEntrez le mot de passe: ");
				cin >> password;
				str1 = password;
				str2 = "20" + str1;
				char* passOut = &str2[0u];
				iResult = send(SocketEnvoi, passOut, strlen(passOut), 0);
				if (iResult == SOCKET_ERROR) {
					printf("Erreur de send du password: %d\n", WSAGetLastError());
					closesocket(SocketEnvoi);
					WSACleanup();
					printf("Appuyez une touche pour finir\n");
					getchar();

					return 1;
				}
			}
			else
			{
				cout << "Erreur de reception du username." << endl;
				closesocket(SocketEnvoi);
				WSACleanup();
				printf("Appuyez une touche pour finir\n");
				getchar();

				return 1;
			}
		}
		else {
			printf("Erreur de reception : %d\n", WSAGetLastError());
		}
	}

	iResult = recv(SocketEnvoi, motRecu, strlen(motRecu), 0);
	if (iResult > 0) {
		printf("Authentification... %s:%s\n\n", host, port);
		if (motRecu[0] == '2' && motRecu[1] == '1')
		{
			stepConnexion = true;
			printf("Authentifie. Tentative de connection au serveur... %s:%s\n\n", host, port);
		}
		else if(motRecu[0] == '2' && motRecu[1] == '2')
		{
			stepConnexion = false;
			printf("Erreur dans la saisie du mot de passe.\n");

			closesocket(SocketEnvoi);
			WSACleanup();
			printf("Appuyez une touche pour finir\n");
			getchar();

			return 1;

		}
		else
		{
			closesocket(SocketEnvoi);
			WSACleanup();
			printf("Appuyez une touche pour finir\n");
			getchar();

			return 1;
		}

	}
	else {
		printf("Erreur de reception : %d\n", WSAGetLastError());
	}
	if (stepConnexion == false)
	{
		closesocket(SocketEnvoi);
		WSACleanup();

		printf("Appuyez une touche pour finir\n");
		getchar();
		return 0;
	}

	iResult = send(SocketEnvoi, "30", 2, 0);
	if (iResult == SOCKET_ERROR) {
		printf("Erreur de send de la requete d'historique. %d\n", WSAGetLastError());
		closesocket(SocketEnvoi);
		WSACleanup();
		printf("Appuyez une touche pour finir\n");
		getchar();
		return 1;
	}

	bool historiqueRecu = false;
	while (historiqueRecu == false)
	{
		cout << "Reception des derniers messages (Maximum 15):" << endl << endl;
		iResult = recv(SocketEnvoi, motRecu, strlen(motRecu), 0);
		if (iResult > 2) {
			if (motRecu[0] == 3 && motRecu[1] == 0)
			{
				for (int i = 2; i < strlen(motRecu); i++) {
					cout << motRecu[i];
				}
				cout << endl;
			}
			else if (motRecu[0] == 3 && motRecu[1] == 1)
			{
				historiqueRecu = true;
				cout << endl;
			}
			else {
				printf("Erreur de reception : %d\n", WSAGetLastError());
			}
		}
	}	

	freeaddrinfo(result);

	//Partie synchrone -> A modifier
	bool enConnexion = true;
	while (enConnexion == true)
	{
		message = "";
		cin >> message;
		str2 = "31" + message;
		char* messageOut = &str2[0u];
		iResult = send(SocketEnvoi, messageOut, strlen(messageOut), 0);
		if (iResult == SOCKET_ERROR) {
			printf("Erreur de send du message. %d\n", WSAGetLastError());
			closesocket(SocketEnvoi);
			WSACleanup();
			printf("Appuyez une touche pour finir\n");
			getchar();
			return 1;
		}

		iResult = recv(SocketEnvoi, motRecu, strlen(motRecu), 0);
		if (iResult > 2) {
			if (motRecu[0] == 3 && motRecu[1] == 2)
			{
				for (int i = 2; i < strlen(motRecu); i++) {
					cout << motRecu[i];
				}
				cout << endl;
			}
		}
		else if (iResult == 2 && motRecu[0] == 4 && motRecu[1] == 0)
		{
			cout << "Deconnexion du chatroom." << endl << endl;
			enConnexion = false;
		}

	}
	// cleanup
	closesocket(SocketEnvoi);
	WSACleanup();

	printf("Appuyez une touche pour finir\n");
	getchar();
	return 0;
}