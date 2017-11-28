//#undef UNICODE
#pragma once
#include "stdafx.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <iostream>
#include <algorithm>
#include <strstream>
#include <locale>
#include <sstream>
#include <string>
#include <stdlib.h>
#include <map>
#include <list>
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
//#include "rapidjson/istreamwrapper.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

using namespace std;
using namespace rapidjson;

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

// External functions
extern DWORD WINAPI EchoHandler(void* sd_);
extern void DoSomething(char *src, char *dest);

// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.




int ValidateInput(char* ip, string portStr) {
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


//// WSAGetLastErrorMessage ////////////////////////////////////////////
// A function similar in spirit to Unix's perror() that tacks a canned 
// interpretation of the value of WSAGetLastError() onto the end of a
// passed string, separated by a ": ".  Generally, you should implement
// smarter error handling than this, but for default cases and simple
// programs, this function is sufficient.
//
// This function returns a pointer to an internal static buffer, so you
// must copy the data from this function before you call it again.  It
// follows that this function is also not thread-safe.
const char* WSAGetLastErrorMessage(const char* pcMessagePrefix, int nErrorID = 0)
{
    // Build basic error string
    static char acErrorBuffer[256];
    ostrstream outs(acErrorBuffer, sizeof(acErrorBuffer));
    outs << pcMessagePrefix << ": ";

    // Tack appropriate canned message onto end of supplied message 
    // prefix. Note that we do a binary search here: gaErrorList must be
    // sorted by the error constant's value.
    ErrorEntry* pEnd = gaErrorList + kNumMessages;
    ErrorEntry Target(nErrorID ? nErrorID : WSAGetLastError());
    ErrorEntry* it = lower_bound(gaErrorList, pEnd, Target);
    if ((it != pEnd) && (it->nID == Target.nID)) {
        outs << it->pcMessage;
    }
    else {
        // Didn't find error in list, so make up a generic one
        outs << "unknown error";
    }
    outs << " (" << Target.nID << ")";

    // Finish error message off and return it.
    outs << ends;
    acErrorBuffer[sizeof(acErrorBuffer) - 1] = '\0';
    return acErrorBuffer;
}

std::map<string, string> users;
std::map<SOCKET, string> usrSockets;
std::list<SOCKET> sockets;
/////////////////////////////////////////////////////////////////////////////////////StartMain
int main(void)
{
    std::map<string, string> users;
    std::map<string, SOCKET> usrSockets;
    list<SOCKET> sockets;

    //----------------------
    // Initialize Winsock.
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != NO_ERROR) {
        cerr << "Error at WSAStartup()\n" << endl;
        return 1;
    }

    //----------------------
    // Create a SOCKET for listening for
    // incoming connection requests.
    SOCKET ServerSocket;
    ServerSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (ServerSocket == INVALID_SOCKET) {
        cerr << WSAGetLastErrorMessage("Error at socket()") << endl;
        WSACleanup();
        return 1;
    }
    char option[] = "1";
    setsockopt(ServerSocket, SOL_SOCKET, SO_REUSEADDR, option, sizeof(option));

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.

    /////////////////////////////imput
    string entreeIP = "";
    string portString = "";
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
    /*if (bind(ServerSocket, (SOCKADDR*)&service, sizeof(service)) == SOCKET_ERROR){
    cerr << WSAGetLastErrorMessage("bind() failed.") << endl;
    closesocket(ServerSocket);
    WSACleanup();
    return 1;
    }*/

    //----------------------
    // Listen for incoming connection requests.
    // on the created socket

    int listenVal = 0;
    do {
        listenVal = listen(ServerSocket, SOMAXCONN);
        if (listenVal == SOCKET_ERROR) {
            cerr << WSAGetLastErrorMessage("Error listening on socket.") << endl;
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
            cout << "Connection acceptee De : " <<
                inet_ntoa(sinRemote.sin_addr) << ":" <<
                ntohs(sinRemote.sin_port) << "." <<
                endl;

            DWORD nThreadID;
            CreateThread(0, 0, EchoHandler, (void*)sd, 0, &nThreadID);
        }
        else {
            cerr << WSAGetLastErrorMessage("Echec d'une connection.") <<
                endl;
            // return 1;
        }
    }
}

DWORD WINAPI EchoHandler(void *socket_) {
    SOCKET sd = (SOCKET)socket_;
//***************************pword check************************************//
    bool loginFailed = false;
    do {
        char toSend[2] = { 1,0 };
        char uName[50];
        char pWord[50];
        recv(sd, uName, 50, 0);
        recv(sd, pWord, 50, 0);

        std::string uNameStr = std::string(uName);
        std::string pWordStr = std::string(pWord);
        if (pWordStr.length() < 1 && uNameStr.length()< 1) {//les champs sont vide, fail
            loginFailed = true;
        }
        else {
            if (users.find(uNameStr) == users.end()) {  ////l'utilisateur n'est pas dans la liste "users"
                users.insert(std::pair<string, string>(uNameStr, pWordStr)); // on l'ajoute ici et dans la bd
            }
            else if (users[uNameStr].compare(pWordStr) == 0) { /// nouvel utilisateur et le mot de passe est valide 
                toSend[1] = '1'; // msg login accepté
				if (usrSockets.find(sd) == usrSockets.end()) {
					usrSockets.insert(std::pair<SOCKET, string>(sd, uNameStr));//on ajoute le socket dans la map de socket/usrname (pour afficher le nom utilisateur plus tard)
				}
				else {
					usrSockets[sd] = uNameStr; // on update la map si l'utilisateur/socket a changé
				}
                loginFailed = true;
            }
        }
        send(sd, toSend, 2, 0);
		///////////ENVOYER LES 15 msgs à sd... on va laisser l'autre envoyer en meme temps, no time to deal
    } while (loginFailed);
	///*************************End Pword*************************************************
	char receive[200];
    while (true) {//on recoit à l'infini
		
        recv(sd, receive, 200, 0);
        std::cout << (usrSockets[sd] + " " + receive + '\n');

        for (auto a : sockets) {//redirection des messages
            send(a, receive, 200, 0);
        }
	

    }	
		return 0;
}
