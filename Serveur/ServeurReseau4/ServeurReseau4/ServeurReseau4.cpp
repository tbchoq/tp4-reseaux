//#undef UNICODE
#pragma once
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include <cstdio>
#include <ctime>
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <strstream>
#include <locale>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <map>
#include <list>

using namespace rapidjson;

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

// External functions
extern DWORD WINAPI EchoHandler(void* sd_);
extern void DoSomething(char *src, char *dest);
extern bool checkUsername(std::string user);
extern bool checkPassword(std::string user, std::string pass);
extern bool createNewUser(std::string user, std::string pass);
extern std::string obtenirMessage(int i);
extern std::string ecrireMessage(std::string user, char* ip, u_short port, std::string text);
extern std::map<std::string, std::string> updateUsermap();
extern std::list<std::string> updateMessageList();
extern bool addMessageBD(std::string message);

// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.

std::string buildString(std::string msg) {
	int size = msg.length();
	std::string sizeStr = "";
	if (size < 100)
		sizeStr = "0";
	if (size < 10)
		sizeStr.append("0");
	sizeStr.append(std::to_string(size));
	sizeStr.append(msg);
	return sizeStr;
}


int ValidateInput(char* ip, std::string portStr) {
    int num;
    int flag = 1;
    int counter = 0;
    bool ipOk = false;
    int port = 0;

    std::istringstream ss(portStr);
    ss >> port;
    if (!ss.fail()) {
        char* p = strtok(ip,  ".");

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
        return ( (ipOk && port <= 5050 && port >= 5000) ? port : 0);
    }
    return 0;

}

static struct ErrorEntry {
    int nID;
    const char* pcMessage;

    ErrorEntry(int id, const char* pc = 0) :
        nID(id),
        pcMessage(pc)
    {
    }

    bool operator<(const ErrorEntry& rhs) const
    {
        return nID < rhs.nID;
    }
} gaErrorList[] = {
    ErrorEntry(0,                  "No error"),
    ErrorEntry(WSAEINTR,           "Interrupted system call"),
    ErrorEntry(WSAEBADF,           "Bad file number"),
    ErrorEntry(WSAEACCES,          "Permission denied"),
    ErrorEntry(WSAEFAULT,          "Bad address"),
    ErrorEntry(WSAEINVAL,          "Invalid argument"),
    ErrorEntry(WSAEMFILE,          "Too many open sockets"),
    ErrorEntry(WSAEWOULDBLOCK,     "Operation would block"),
    ErrorEntry(WSAEINPROGRESS,     "Operation now in progress"),
    ErrorEntry(WSAEALREADY,        "Operation already in progress"),
    ErrorEntry(WSAENOTSOCK,        "Socket operation on non-socket"),
    ErrorEntry(WSAEDESTADDRREQ,    "Destination address required"),
    ErrorEntry(WSAEMSGSIZE,        "Message too long"),
    ErrorEntry(WSAEPROTOTYPE,      "Protocol wrong type for socket"),
    ErrorEntry(WSAENOPROTOOPT,     "Bad protocol option"),
    ErrorEntry(WSAEPROTONOSUPPORT, "Protocol not supported"),
    ErrorEntry(WSAESOCKTNOSUPPORT, "Socket type not supported"),
    ErrorEntry(WSAEOPNOTSUPP,      "Operation not supported on socket"),
    ErrorEntry(WSAEPFNOSUPPORT,    "Protocol family not supported"),
    ErrorEntry(WSAEAFNOSUPPORT,    "Address family not supported"),
    ErrorEntry(WSAEADDRINUSE,      "Address already in use"),
    ErrorEntry(WSAEADDRNOTAVAIL,   "Can't assign requested address"),
    ErrorEntry(WSAENETDOWN,        "Network is down"),
    ErrorEntry(WSAENETUNREACH,     "Network is unreachable"),
    ErrorEntry(WSAENETRESET,       "Net connection reset"),
    ErrorEntry(WSAECONNABORTED,    "Software caused connection abort"),
    ErrorEntry(WSAECONNRESET,      "Connection reset by peer"),
    ErrorEntry(WSAENOBUFS,         "No buffer space available"),
    ErrorEntry(WSAEISCONN,         "Socket is already connected"),
    ErrorEntry(WSAENOTCONN,        "Socket is not connected"),
    ErrorEntry(WSAESHUTDOWN,       "Can't send after socket shutdown"),
    ErrorEntry(WSAETOOMANYREFS,    "Too many references, can't splice"),
    ErrorEntry(WSAETIMEDOUT,       "Connection timed out"),
    ErrorEntry(WSAECONNREFUSED,    "Connection refused"),
    ErrorEntry(WSAELOOP,           "Too many levels of symbolic links"),
    ErrorEntry(WSAENAMETOOLONG,    "File name too long"),
    ErrorEntry(WSAEHOSTDOWN,       "Host is down"),
    ErrorEntry(WSAEHOSTUNREACH,    "No route to host"),
    ErrorEntry(WSAENOTEMPTY,       "Directory not empty"),
    ErrorEntry(WSAEPROCLIM,        "Too many processes"),
    ErrorEntry(WSAEUSERS,          "Too many users"),
    ErrorEntry(WSAEDQUOT,          "Disc quota exceeded"),
    ErrorEntry(WSAESTALE,          "Stale NFS file handle"),
    ErrorEntry(WSAEREMOTE,         "Too many levels of remote in path"),
    ErrorEntry(WSASYSNOTREADY,     "Network system is unavailable"),
    ErrorEntry(WSAVERNOTSUPPORTED, "Winsock version out of range"),
    ErrorEntry(WSANOTINITIALISED,  "WSAStartup not yet called"),
    ErrorEntry(WSAEDISCON,         "Graceful shutdown in progress"),
    ErrorEntry(WSAHOST_NOT_FOUND,  "Host not found"),
    ErrorEntry(WSANO_DATA,         "No host data of that type was found")
};
const int kNumMessages = sizeof(gaErrorList) / sizeof(ErrorEntry);



const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, int nErrorID = 0)
{
    // Build basic error string
    static char acErrorBuffer[256];
    std::ostrstream outs(acErrorBuffer, sizeof(acErrorBuffer));
    outs << pcMessagePrefix << ": ";


    ErrorEntry* pEnd = gaErrorList + kNumMessages;
    ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
    ErrorEntry* it = std::lower_bound(gaErrorList, pEnd, Target);
    if ((it != pEnd) && (it->nID == Target.nID)) {
        outs << it->pcMessage;
    }
    else {
        // Didn't find error in list, so make up a generic one
        outs << "unknown error";
    }
    outs << " (" << Target.nID << ")";

    // Finish error message off and return it.
    outs << std::ends;
    acErrorBuffer[sizeof(acErrorBuffer) - 1] = '\0';
    return acErrorBuffer;
}


std::map<std::string, std::string> users;
std::map<SOCKET, std::string> usrSockets;
std::list<SOCKET> sockets;
std::list<std::string> messages;
int main(void)
{

//TODO : AJOUTE LES INFO DE LA BS DANS USERS//////////////////////////////////////////////////////////////////////////////////// 
	users = updateUsermap();
	messages = updateMessageList();
    //----------------------
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        std::cerr << "Error at WSAStartup()\n" << std::endl;
        return 1;
    }

    //----------------------
    // Create a SOCKET for listening for
    // incoming connection requests.
    SOCKET ServerSocket;
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSocket == INVALID_SOCKET) {
        std::cerr << WSAGetLastErrorMessage("Error at socket()") << std::endl;
        WSACleanup();
        return 1;
    }
    char option[] = "1";
    setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, option, sizeof(option));

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.

    /////////////////////////////imput
    std::string entreeIP = "";
    std::string portString = "";
    //bool ipWrong = true;
    int port = 0;


    while (port == 0)
    {
        printf("Entrez l'addresse ip du serveur sous format xxx.xxx.xxx.xxx, puis appuyez sur retour de ligne. \t");
        std::cin >> entreeIP;
        printf("Entrez le port sur lequel se connecter entre 5000 5050 puis appuyez sur retour de ligne. \t");
        std::cin >> portString;

        char *ip = new char[entreeIP.length()];
        for (unsigned int i = 0; i < entreeIP.length(); i++) {
            ip[i] = entreeIP[i];
        }


        port = ValidateInput(ip, portString);

        if (port == 0) {
            printf("Svp, entrez des valeurs valides.\n");
        }
        

    }


    //Recuperation de l'adresse locale
    hostent *thisHost;


    thisHost = gethostbyname(entreeIP.c_str());
    char* ip;
    ip = inet_ntoa(*(struct in_addr*) *thisHost->h_addr_list);
    printf("Adresse locale trouvee %s : \n\n", ip);
    sockaddr_in service;
    service.sin_family = AF_INET;
    //service.sin_addr.s_addr = inet_addr("127.0.0.1");
    //	service.sin_addr.s_addr = INADDR_ANY;
    service.sin_addr.s_addr = inet_addr(ip);
    service.sin_port = htons(port);
    //int retourbind = (int)
    bind(ServerSocket, (SOCKADDR*)&service, sizeof(service));

    int listenVal = 0;
    do {
        listenVal = listen(ServerSocket, SOMAXCONN);
        if (listenVal == SOCKET_ERROR) {
            std::cerr << WSAGetLastErrorMessage("Error listening on socket.") << std::endl;
            closesocket(ServerSocket);
            WSACleanup();
            return 1;
        }
        sockaddr_in sinRemote;
        int nAddrSize = sizeof(sinRemote);
        SOCKET ClientSocket = accept(ServerSocket, (sockaddr*)&sinRemote, &nAddrSize);
        DWORD nThreadID;
        CreateThread(0, 0, EchoHandler, (void*)ClientSocket, 0, &nThreadID);
        //_beginthread(ClientConnect, 0, (void *)ClientSocket);
        sockets.push_back(ClientSocket);

    } while (1);

    printf("En attente des connections des clients sur le port %d...\n\n", ntohs(service.sin_port));

    while (true) {

        sockaddr_in sinRemote;
        int nAddrSize = sizeof(sinRemote);


        SOCKET sd = accept(ServerSocket, (sockaddr*)&sinRemote, &nAddrSize);
        if (sd != INVALID_SOCKET) {
            std::cout << "Connection acceptee De : " <<
                inet_ntoa(sinRemote.sin_addr) << ":" <<
                ntohs(sinRemote.sin_port) << "." <<
                std::endl;

            DWORD nThreadID;
            CreateThread(0, 0, EchoHandler, (void*)sd, 0, &nThreadID);
        }
        else {
            std::cerr << WSAGetLastErrorMessage("Echec d'une connection.") <<
                std::endl;
            // return 1;
        }
    }
}

DWORD WINAPI EchoHandler(void *socket_) {
    SOCKET sd = (SOCKET)socket_;
//***************************pword check************************************//
	users = updateUsermap();
	messages = updateMessageList();

    bool loginFailed = false;
    std::string uNameStr = "";
    int sendResult = 0; 
    int receiveResult = 0;
    do {
        char toSend[2] = { '1', '0' };
        char uName[50];
        char pWord[50];
        receiveResult = recv(sd, uName, 50, 0);
        if (receiveResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(sd);
            WSACleanup();
            return 1;
        }
        receiveResult =  recv(sd, pWord, 50, 0);
        if (receiveResult == SOCKET_ERROR) {
            printf("shutdown failed with error: %d\n", WSAGetLastError());
            closesocket(sd);
            WSACleanup();
            return 1;
        }

		char nomlengthChar[3] = {uName[0], uName[1], uName[2] };
		int nomLength = atoi(nomlengthChar);
		char wordlengthChar[3] = { pWord[0], pWord[1], pWord[2] };
		int wordLength = atoi(wordlengthChar);

        uNameStr = std::string(uName).substr(3, nomLength);
        std::string pWordStr = std::string(pWord).substr(3, wordLength);

        if (pWordStr.length() < 1 && uNameStr.length()< 1) {//les champs sont vide, fail
            loginFailed = true;
        }
        else {
            if (users.find(uNameStr) == users.end()) {  ////l'utilisateur n'est pas dans la liste "users"
                users.insert(std::pair<std::string, std::string>(uNameStr, pWordStr)); // on l'ajoute ici et dans la bd
				createNewUser(uNameStr, pWordStr);
				usrSockets.insert(std::pair<SOCKET, std::string>(sd, uNameStr));
				loginFailed = false;
				toSend[1] = '1';
				
            }
			//// utilisateur trouvé
			else { 
				if (users[uNameStr] == pWordStr)
				{
					toSend[1] = '1'; // msg login accepté
					if (usrSockets.find(sd) == usrSockets.end()) {
						usrSockets.insert(std::pair<SOCKET, std::string>(sd, uNameStr));//on ajoute le socket dans la map de socket/usrname (pour afficher le nom utilisateur plus tard)
					}
					loginFailed = false;
				}
				else {
					//usrSockets[sd] = uNameStr; // on update la map si l'utilisateur/socket a changé
					loginFailed = true;
				}
            }
        }
        sendResult = send(sd, toSend, 2, 0);
        if (sendResult == SOCKET_ERROR) {
            printf("send failed with error: %d\n", WSAGetLastError());
            closesocket(sd);
            WSACleanup();
            return 1;
        }
    } while (loginFailed);

	///////////ENVOYER LES 15 msgs à sd
	//Si moins ou egal a 15 messages dans la BD
	char send15[200];
	if (messages.size() <15)
	{
		for (auto itr = messages.begin(); itr != messages.end(); itr++)
		{
			std::string temp = buildString(*itr);
			strcpy(send15, temp.c_str());

			sendResult = send(sd, send15, 200, 0);
			if (sendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(sd);
				WSACleanup();
				return 1;
			}
		}
	}
	//Si plus de 15 messages dans la BD
	else
	{
		auto itr = messages.end();
		for (int i = 0; i <= 15; i++)
			itr--;
		for (itr; itr != messages.end(); itr++)
		{
			std::string temp = buildString(*itr);
			strcpy(send15, temp.c_str());

			sendResult = send(sd, send15, 200, 0);
			if (sendResult == SOCKET_ERROR) {
				printf("send failed with error: %d\n", WSAGetLastError());
				closesocket(sd);
				WSACleanup();
				return 1;
			}
		}
	}
	//Message de confirmation que les 15 messages sont envoyés
	sendResult = send(sd, "999", 200, 0);
	if (sendResult == SOCKET_ERROR) {
		printf("send failed with error: %d\n", WSAGetLastError());
		closesocket(sd);
		WSACleanup();
		return 1;
	}
	///*************************End Pword*************************************************
	char receive[200];
    while (true) {//on recoit à l'infini
		users = updateUsermap();
		messages = updateMessageList();

		sockaddr_in sinAddr;
		int nAddrSize = sizeof(sinAddr);

        recvfrom(sd, receive, 200, 0, (sockaddr*)&sinAddr, &nAddrSize);

		char * ip = inet_ntoa(sinAddr.sin_addr);
		u_short port = ntohs(sinAddr.sin_port);

		char lengthChar[3] = { receive[0], receive[1], receive[2] };
		int wordLength = atoi(lengthChar);
		std::string strReceive = std::string(receive).substr(1, wordLength +2);

		std::string messageCompose = ecrireMessage(usrSockets[sd], ip, port, strReceive);

		//Ajouter le nouveau message a la BD
		if (addMessageBD(messageCompose) == false)
		{
			printf("Erreur d'ecriture du message dans la BD: %d\n", WSAGetLastError());
			closesocket(sd);
			WSACleanup();
			return 1;
		}


        std::cout << messageCompose << '\n';
		////////////AJOUTER LE MESSAGE AUX 15 MESAGES//////////////////////////////////////////////
        for (auto a : usrSockets) {//redirection des messages
            //if (a.second.compare( uNameStr) ==0) {} // dont self to self
            //else {
                std::string temp = buildString(messageCompose);
				char *cstr = new char[messageCompose.length() + 1];
                strcpy(cstr, temp.c_str());

                sendResult = send(a.first, cstr, 200, 0);
                if (sendResult == SOCKET_ERROR) {
                    printf("send failed with error: %d\n", WSAGetLastError());
                    closesocket(a.first);
                    WSACleanup();
                    return 1;
                }
           // }
        }
	

    }	
		return 0;
}

bool checkUsername(std::string username)
{
	FILE* fpRead1 = fopen("Data.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docUsers;

	if (docUsers.ParseStream(is1).HasParseError())
		return false;

	fclose(fpRead1);

	if (docUsers.HasMember(username.c_str()) == true)
		return true;
	else
		return false;
}

bool checkPassword(std::string user, std::string pass)
{
	FILE* fpRead1 = fopen("Data.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docUsers;

	if (docUsers.ParseStream(is1).HasParseError())
		return false;

	Value key(user.c_str(), docUsers.GetAllocator());

	if (docUsers[key] == pass.c_str())
		return true;
	else
		return false;
}

bool createNewUser(std::string user, std::string pass)
{
	FILE* fpRead1 = fopen("Data.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docUsers;

	if (docUsers.ParseStream(is1).HasParseError())
		return false;

	fclose(fpRead1);

	Value key(user.c_str(), docUsers.GetAllocator());
	Value val(pass.c_str(), docUsers.GetAllocator());
	docUsers.AddMember(key, val, docUsers.GetAllocator());

	FILE* fpWrite = fopen("Data.json", "wb");
	char writeBuf[65536];
	FileWriteStream os(fpWrite, writeBuf, sizeof(writeBuf));
	Writer<FileWriteStream> writer(os);
	docUsers.Accept(writer);

	fclose(fpWrite);

	return true;
}

std::string ecrireMessage(std::string user, char* ip, u_short port, std::string text)
{
	char buff1[20];
	char buff2[20];
	time_t now = time(NULL);
	strftime(buff1, 20, "%Y-%m-%d", localtime(&now));
	strftime(buff2, 20, "%H:%M:%S", localtime(&now));

	std::string message;
	std::string temp1(ip);
	std::string temp2 = text;
	temp2.erase(0, 2);
	message = "[" + user + " " + temp1 + ":" + std::to_string(port) + " - " + buff1 + "@" + buff2 + "]:" + temp2;

	return message;
}

std::map<std::string, std::string> updateUsermap()
{
	FILE* fpRead1 = fopen("Data.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docUsers;

	docUsers.ParseStream(is1);

	fclose(fpRead1);

	std::map<std::string, std::string> newUsers;


	for (Value::ConstMemberIterator itr = docUsers.MemberBegin(); itr != docUsers.MemberEnd(); itr++)
	{
		std::string n = itr->name.GetString();
		std::string v = itr->value.GetString();

		newUsers.insert(std::pair<std::string,std::string>(n,v));
	}

	return newUsers;
}

std::list<std::string> updateMessageList()
{
	FILE* fpRead1 = fopen("Messages.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docMessages;

	docMessages.ParseStream(is1);

	fclose(fpRead1);

	std::list<std::string> newMessages;

	for (Value::MemberIterator itr = docMessages.MemberBegin(); itr != docMessages.MemberEnd(); itr++)
	{
		std::string v = itr->value.GetString();

		newMessages.push_back(v);
	}

	return newMessages;
}

bool addMessageBD(std::string message)
{
	FILE* fpRead1 = fopen("Messages.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docMessages;

	if (docMessages.ParseStream(is1).HasParseError())
		return false;

	fclose(fpRead1);

	int index = docMessages.MemberCount() + 1;

	Value key(std::to_string(index).c_str(), docMessages.GetAllocator());
	Value val(message.c_str(), docMessages.GetAllocator());
	docMessages.AddMember(key, val, docMessages.GetAllocator());

	FILE* fpWrite = fopen("Messages.json", "wb");
	char writeBuf[65536];
	FileWriteStream os(fpWrite, writeBuf, sizeof(writeBuf));
	Writer<FileWriteStream> writer(os);
	docMessages.Accept(writer);

	fclose(fpWrite);

	return true;
}
