// client4.cpp : Defines the entry point for the console application.
//


#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <sstream>
#include <iostream>
#include <process.h>

#pragma comment(lib, "Ws2_32.lib")


int ValidateInput(char* ip, std::string portStr) {
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
//appends msg length to begin of msg
std::string buildString(std::string msg) {
	int size = msg.length;
	std::string sizeStr = "";
	if (size < 100) 
		sizeStr = "0";
	 if (size < 10) 
		sizeStr.append("0");
	 sizeStr.append(std::to_string(size));
	 return sizeStr;

	
}
void Quitter(SOCKET leSocket) {
    shutdown(leSocket, SD_SEND);
    WSACleanup();
    printf("Appuyez une touche pour finir\n");
    getchar();

}

bool SocketFailed(SOCKET leSocket, int iResult) {
    if (iResult == SOCKET_ERROR) {
        printf("Erreur du send: %d\n", WSAGetLastError());
        Quitter(leSocket);
        return true;
    }
    return false;
}

void Receive(void *leSocket) {
    int iResult;
    char* motRecu = new char[250];
    while (true) {
        iResult = recv((SOCKET)leSocket, motRecu, 250, 0);
    }
    printf(motRecu + '\n');
}

int main()
{

    WSADATA wsaData;
    SOCKET leSocket = INVALID_SOCKET;
    struct addrinfo *result = NULL,
        *ptr = NULL,
        hints;

    int iResult;

    // InitialisATION de Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("Erreur de WSAStartup: %d\n", iResult);
        return 1;
    }
    // On va creer le socket pour communiquer avec le serveur
    leSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (leSocket == INVALID_SOCKET) {
        printf("Erreur de socket(): %ld\n\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        printf("Appuyez une touche pour finir\n");
        getchar();
        return 1;
    }
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
    std::string entreeIP = "";
    std::string portString = "";
    //bool ipWrong = true;
    int iport = 0;


    while (iport == 0)
    {
        printf("Entrez l'addresse ip du serveur sous format xxx.xxx.xxx.xxx, puis appuyez sur retour de ligne. \t");
        std::cin >> entreeIP;
        printf("Entrez le port sur lequel se connecter entre 5000 5050 puis appuyez sur retour de ligne. \t");
        std::cin >> portString;

        char *ip = new char[entreeIP.length()];
        for (unsigned int i = 0; i < entreeIP.length(); i++) {
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
    printf("Adresse trouvee pour le serveur %s : %s\n\n", host, inet_ntoa(adresse->sin_addr));
    printf("Tentative de connexion au serveur %s avec le port %s\n\n", inet_ntoa(adresse->sin_addr), port);


    // On va se connecter au serveur en utilisant l'adresse qui se trouve dans
    // la variable result.
    iResult = connect(leSocket, result->ai_addr, (int)(result->ai_addrlen));
    if (iResult == SOCKET_ERROR) {
        printf("Impossible de se connecter au serveur %s sur le port %s\n\n", inet_ntoa(adresse->sin_addr), port);
        freeaddrinfo(result);
        WSACleanup();
        printf("Appuyez une touche pour finir\n");
        getchar();
        return 1;
    }

    std::string username = "";
    std::string pword = "";
    printf("Connecte au serveur %s:%s\n\n. Pour quiter entrez \"quiter\" seul.\n", host, port);

    /////////////////////////////////////////// login
    char *reponseServeur = new char[2];
    do {
        do {
            printf("Entrez le nom d'utlilisateur\n");
            std::cin >> username;
            if (username.compare("quiter") == 0) {
                Quitter(leSocket);
                return 0;
            }
            printf("Entrez le mot de passe\n");
            std::cin >> pword;
            if (pword.compare("quiter") == 0) {
                Quitter(leSocket);
                return 0;
            }
        } while (username.length() < 1 && pword.length() < 1);

        char *toSendChar = new char[username.length() + 3];
        strcpy(toSendChar, username.c_str());
        //send username
        iResult = send(leSocket, toSendChar, username.length(), 0);
        delete[] toSendChar;
        if (SocketFailed(leSocket, iResult)) {
            return 0;
        }

        char *toSendCharpword = new char[pword.length() + 3];
        strcpy(toSendCharpword, pword.c_str());
        //send pwrd
        iResult = send(leSocket, toSendCharpword, pword.length(), 0);
        delete[] toSendCharpword;
        if (SocketFailed(leSocket, iResult)) {
            return 0;
        }
        recv((SOCKET)leSocket, reponseServeur, 2, 0);
        if ((reponseServeur[0] == '1' && reponseServeur[1] == '0')) {
            printf("Le nom d'utilisateur est utilisé ou le mot de passe est invalide.");

        }

    } while (reponseServeur[0] == '1' && reponseServeur[1] == '0'); //while fail
	printf("Vous etes cnnectés.\n");
    //_beginthreadex();                                                          //const UINT envoieLength =  + 200;
   // _beginthread(Receive, 0, (void *)leSocket);

    std::string envoieString = "";


    //////////////////////////Connected
    while (true) {
        envoieString = "";
        std::getline(std::cin, envoieString);
        if (envoieString.compare("quiter") == 0) {
            Quitter(leSocket);
            return 0;
        }
        if (envoieString.length() <= 200 && envoieString.length() > 0) {
            char *toSendChar = new char[envoieString.length() + 3];

            strcpy(toSendChar, envoieString.c_str());
            iResult = send(leSocket, toSendChar, envoieString.length(), 0);
            if (SocketFailed(leSocket, iResult)) {
                return 0;
            }
            delete[] toSendChar;
        }

    }

    ///quitter

    return 0;
}

