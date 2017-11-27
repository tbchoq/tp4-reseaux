#undef UNICODE

#include <winsock2.h>
#include "rapidjson/filereadstream.h"
#include "rapidjson/filewritestream.h"
#include "rapidjson/istreamwrapper.h"
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



using namespace std;
using namespace rapidjson;

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

// External functions
extern DWORD WINAPI EchoHandler(void* sd_) ;
extern void DoSomething( char *src, char *dest );
extern bool checkUsername(char *src);
extern bool checkPassword(char *pass, char *user);
extern bool createNewUser(char *pass, char *user);
extern string obtenirMessage(int i);
extern string ecrireMessage(char* user, char* ip, u_short port, char *text);
extern const char *GetJsonText(Document doc);


// List of Winsock error constants mapped to an interpretation string.
// Note that this list must remain sorted by the error constants'
// values, because we do a binary search on the list when looking up
// items.
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

int main(void) 
{

	//string idString;
	//string message0;
	//for (int i = 1; i < 16; i++)
	//{
	//	idString = to_string(i);
	//	message0 = "message" + idString;
	//	Value key(idString.c_str(), docMessages.GetAllocator());
	//	Value val(message0.c_str(), docMessages.GetAllocator());
	//	docMessages.AddMember(key, val, docMessages.GetAllocator());
	//}

	//FILE* fpWrite = fopen("Messages.json", "wb"); // non-Windows use "w"
	//char writeBuf[65536];
	//FileWriteStream os(fpWrite, writeBuf, sizeof(writeBuf));
	//Writer<FileWriteStream> writer(os);
	//docMessages.Accept(writer);

	//fclose(fpWrite);


	

	//----------------------
	// Initialize Winsock.

	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
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

	string entreeIP = "";
	while (entreeIP != "132.207.156.238")
	{
		printf("\nVeuillez entrer l'adresse IP du poste sur lequel le serveur sera execute (Poste present: 132.207.156.238) : ");
		cin >> entreeIP;
		
		if (entreeIP != "132.207.156.238")
		{
			printf("\nL'adresse IP est incorrecte. ");
			if (entreeIP.size() != 16)
				printf("L'adresse entree n'est pas sur 4 octets. ");
			bool charIncorrect = false;
			for (int i = 0; i < entreeIP.size(); i++)
			{
				if (entreeIP[i] == '1' || entreeIP[i] == '2' || entreeIP[i] == '3' || entreeIP[i] == '4' || entreeIP[i] == '5' || entreeIP[i] == '6' 
					|| entreeIP[i] == '7' || entreeIP[i] == '8' || entreeIP[i] == '9' || entreeIP[i] == '0' || entreeIP[i] == '.') {}
				else
					charIncorrect = true;
					
			}
			if (charIncorrect == true)
				printf("L'adresse contient des characteres incorrects. ");
					
		}
			
	}

	int port = 0;
	while (port < 5000 || port > 5050)
	{
		printf("\nVeuillez entrer un port d'ecoute entre 5000 et 5050 : ");
		cin >> port;

		if (port < 5000 || port > 5050)
			printf("\nPort incorrect.");
	}

    //----------------------
    // The sockaddr_in structure specifies the address family,
    // IP address, and port for the socket that is being bound.
	//int port=5030;

	const char* charIP = entreeIP.c_str();
    
	//Recuperation de l'adresse locale
	hostent *thisHost;

	//TODO Modifier l'adresse IP ci-dessous pour celle de votre poste.
	thisHost=gethostbyname(charIP);
	char* ip;
	ip=inet_ntoa(*(struct in_addr*) *thisHost->h_addr_list);
	printf("Adresse locale trouvee %s : \n\n",ip);
	sockaddr_in service;
    service.sin_family = AF_INET;
    //service.sin_addr.s_addr = inet_addr("127.0.0.1");
	//	service.sin_addr.s_addr = INADDR_ANY;
	service.sin_addr.s_addr = inet_addr(ip);
    service.sin_port = htons(port);

    if (bind(ServerSocket, (SOCKADDR*) &service, sizeof(service)) == SOCKET_ERROR) {
		cerr << WSAGetLastErrorMessage("bind() failed.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}
	
	//----------------------
	// Listen for incoming connection requests.
	// on the created socket
	if (listen(ServerSocket, 30) == SOCKET_ERROR) {
		cerr << WSAGetLastErrorMessage("Error listening on socket.") << endl;
		closesocket(ServerSocket);
		WSACleanup();
		return 1;
	}


	printf("En attente des connections des clients sur le port %d...\n\n",ntohs(service.sin_port));

    while (true) {	

		sockaddr_in sinRemote;
		 int nAddrSize = sizeof(sinRemote);
		// Create a SOCKET for accepting incoming requests.
		// Accept the connection.
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


//// EchoHandler ///////////////////////////////////////////////////////
// Handles the incoming data by reflecting it back to the sender.

DWORD WINAPI EchoHandler(void* sd_) 
{
	SOCKET sd = (SOCKET)sd_;
	sockaddr* adresse;
	int* adrsize = new int(16);

	char* CurrentUsername;
	bool newUser = false;

	char readBuffer[250];
	char outBuffer[250];
	// Read Data from client
	//char *outBuffer = "";
	int readBytes;

	string str1 = "";
	string str2 = "";
	readBytes = recvfrom(sd, readBuffer, strlen(readBuffer),0,adresse,adrsize);
	struct sockaddr_in *sin = (struct sockaddr_in *) adresse;
	char * ip = inet_ntoa(sin->sin_addr);
	u_short port = ntohs(sin->sin_port);
	if (readBytes > 0) {
		cout << "Received " << readBytes << " bytes from client." << endl;
		cout << "Received " << readBuffer << " from client." << endl;

		//Username sent
		if (readBuffer[0] == '1' && readBuffer[1] == '0')
		{
			if (checkUsername(readBuffer) == true)
			{
				CurrentUsername = readBuffer;
				newUser = false;
				str2 = "11StepPassword";
			}
			else
			{
				CurrentUsername = readBuffer;
				newUser = true;
				//autre shits / same shit cote client
				str2 = "11StepPassword";
			}
		}
		//Password received
		else if (readBuffer[0] == '2' && readBuffer[1] == '0')
		{
			if (newUser == false)
			{
				//Username: OK
				if (checkPassword(readBuffer, CurrentUsername) == true)
				{
					//Password: Ok -> 15 Last messages -> Unlock
					Document docMessages;

					FILE* fpRead2 = fopen("Messages.json", "rb");
					char readBuf2[65536];
					FileReadStream is2(fpRead2, readBuf2, sizeof(readBuf2));

					if (docMessages.ParseStream(is2).HasParseError())
						return 1;

					fclose(fpRead2);

					if (docMessages.MemberCount() < 15)
					{
						for (int i = 1; i < docMessages.MemberCount(); i++)
						{
							str1 = obtenirMessage(i);
							str2 = "31" + str1;
							char* outBuffer = &str2[0u];
							send(sd, outBuffer, strlen(outBuffer), 0);
						}
						str2 = "32";
						char* outBuffer = &str2[0u];
						send(sd, outBuffer, strlen(outBuffer), 0);
					}
					else
					{
						for (int i = docMessages.MemberCount() - 15; i < docMessages.MemberCount(); i++)
						{
							str1 = obtenirMessage(i);
							str2 = "31" + str1;
							str2 = "31" + str1;
							char* outBuffer = &str2[0u];
							send(sd, outBuffer, strlen(outBuffer), 0);
						}
						str2 = "32";
						char* outBuffer = &str2[0u];
						send(sd, outBuffer, strlen(outBuffer), 0);
					}
				}
				else
				{
					str2 = "20StepPasswordWrong";
				}
			}
			else
			{
				if (createNewUser(readBuffer, CurrentUsername) == false)
					return 1;
			}

		}
		//Nouveau Message
		else if (readBuffer[0] == '3' && readBuffer[0] == '1')
		{
			Document docMessages;

			FILE* fpRead2 = fopen("Messages.json", "rb");
			char readBuf2[65536];
			FileReadStream is2(fpRead2, readBuf2, sizeof(readBuf2));

			if (docMessages.ParseStream(is2).HasParseError())
				return 1;

			fclose(fpRead2);

			str1 = ecrireMessage(CurrentUsername, ip, port, readBuffer);
			str2 = "31" + str1;
			char* outBuffer = &str2[0u];

			int newID = docMessages.MemberCount() + 1;
			Value key("0" + newID, docMessages.GetAllocator());
			Value val(outBuffer, docMessages.GetAllocator());
			docMessages.AddMember(key, val, docMessages.GetAllocator());

			FILE* fpWrite = fopen("Data.json", "wb");
			char writeBuf[65536];
			FileWriteStream os(fpWrite, writeBuf, sizeof(writeBuf));
			Writer<FileWriteStream> writer(os);
			docMessages.Accept(writer);

			fclose(fpWrite);

			char* outBuffer = &str2[0u];
			send(sd, outBuffer, strlen(outBuffer), 0);
			
		}
		//DoSomething(readBuffer, outBuffer);

		//A mettre dans les cases
		send(sd, outBuffer, strlen(outBuffer), 0);
	}
	else if (readBytes == SOCKET_ERROR) {
		cout << WSAGetLastErrorMessage("Echec de la reception !") << endl;
	}
	closesocket(sd);

	return 0;
}


// Do Something with the information
void DoSomething( char *src, char *dest )
{
	for (auto i = 0; i < strlen(src); i++)
	{
		dest[i] = toupper(src[i]);
	}
}

bool checkUsername(char *src)
{
	FILE* fpRead1 = fopen("Data.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docUsers;

	if (docUsers.ParseStream(is1).HasParseError())
		return false;

	fclose(fpRead1);

	if (docUsers.HasMember(src) == true)
		return true;
	else
		return false;
}

bool checkPassword(char *pass, char *user)
{
	FILE* fpRead1 = fopen("Data.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docUsers;

	if (docUsers.ParseStream(is1).HasParseError())
		return false;

	Value key(user, docUsers.GetAllocator());

	if (docUsers[key] == pass)
		return true;
	else
		return false;
}

bool createNewUser(char *pass, char* user)
{
	FILE* fpRead1 = fopen("Data.json", "rb");
	char readBuf1[65536];
	FileReadStream is1(fpRead1, readBuf1, sizeof(readBuf1));

	Document docUsers;

	if (docUsers.ParseStream(is1).HasParseError())
		return false;

	fclose(fpRead1);

	Value key(user, docUsers.GetAllocator());
	Value val(pass, docUsers.GetAllocator());
	docUsers.AddMember(key, val, docUsers.GetAllocator());

	FILE* fpWrite = fopen("Data.json", "wb");
	char writeBuf[65536];
	FileWriteStream os(fpWrite, writeBuf, sizeof(writeBuf));
	Writer<FileWriteStream> writer(os);
	docUsers.Accept(writer);

	fclose(fpWrite);

	return true;
}

string obtenirMessage(int i)
{
	Document docMessages;

	FILE* fpRead2 = fopen("Messages.json", "rb");
	char readBuf2[65536];
	FileReadStream is2(fpRead2, readBuf2, sizeof(readBuf2));

	if (docMessages.ParseStream(is2).HasParseError())
		return "Erreur de Parse.";

	fclose(fpRead2);

	string index = to_string(i);
	Value key(index.c_str(), docMessages.GetAllocator());

	if (docMessages.HasMember(key) == false)
		return "Message inexistant.";

	return docMessages[key].GetString();
}

string ecrireMessage(char* user, char* ip, u_short port, char *text)
{
	char buff1[20];
	char buff2[20];
	time_t now = time(NULL);
	strftime(buff1, 20, "%Y-%m-%d", localtime(&now));
	strftime(buff2, 20, "%H:%M:%S", localtime(&now));

	string message;
	string temp1(user);
	string temp2(ip);
	string temp3(text);
	message = "[" + temp1 + " " + temp2 + ":" + to_string(port) + " - " + buff1 + "@" + buff2 + "]:" + temp3;

	return message;
}

Document ouvrirDocument(char* c)
{
	FILE* fp = fopen(c, "rb");
	char buffer[65536];
	FileReadStream is(fp, buffer, sizeof(buffer));
	Document doc;
	doc.ParseStream(is);
	fclose(fp);
	return doc;
}

const char *GetJsonText(Document doc)
{
	rapidjson::StringBuffer buffer;

	buffer.Clear();

	rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
	doc.Accept(writer);

	return strdup(buffer.GetString());
}