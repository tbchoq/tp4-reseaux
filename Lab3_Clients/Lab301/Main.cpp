#undef UNICODE 
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <sstream>

// link with Ws2_32.lib
#pragma comment( lib, "ws2_32.lib" )

// Fonctions auxiliares
extern void wsStartup( void );

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

int __cdecl main( void )
{

    //-----------------------------------------
    // Declare and initialize variables
    DWORD dwRetval;

    int i = 1;
    
    struct addrinfo *result = NULL;
    struct addrinfo *ptr = NULL;
    struct sockaddr_in  *sockaddr_ipv4;
    struct sockaddr_in6 *sockaddr_ipv6;

    char ipstringbuffer[46];
    DWORD ipbufferlength = 46;

	//--------------------------------
	// Initialiser la biblioteque Winsock
	wsStartup();

	//--------------------------------
	// Le nom du host dont on veut savoir l'adresse IP
	char *host = "L4708-03";
    
	//--------------------------------
	// Appeller getaddrinfo(). Si l'appel reussi,
	// la variable result contiendra la liste des
	// structures addrinfo avec toutes les adresses du host
    dwRetval = getaddrinfo(host, NULL, NULL, &result);
    if ( dwRetval != 0 ) {
        printf("Erreur %d avec getaddrinfo.\n", dwRetval);
        WSACleanup();
        return 1;
    }

    printf("getaddrinfo a reussi!\n");
    
    // Extraire chaque address et afficher son détail
    for(ptr = result; ptr != NULL; ptr = ptr->ai_next) {
		printf("Adresse # %d\n", i++);
        printf("\tFlags: 0x%x\n", ptr->ai_flags);
        printf("\tFamille: ");
        switch (ptr->ai_family) {
            case AF_UNSPEC:
                printf("Non spécifiée\n");
                break;
            case AF_INET:
                printf("AF_INET (IPv4)\n");
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
                printf("\tAdresse IPv4: %s\n",
                    inet_ntoa(sockaddr_ipv4->sin_addr) );
                break;
            case AF_INET6:
                printf("AF_INET6 (IPv6)\n");
                // the InetNtop function is available on Windows Vista and later
                sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
                printf("\tAdresse IPv6: %s\n",
                    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );                
                break;
            case AF_NETBIOS:
                printf("AF_NETBIOS (NetBIOS)\n");
                break;
            default:
                printf("Autre type %ld\n", ptr->ai_family);
                break;
        }
        printf("\tTaille de cette adresse: %d octets\n", ptr->ai_addrlen);
        printf("\tNom canonique: %s\n", ptr->ai_canonname);
    }

    freeaddrinfo(result);
    WSACleanup();

	printf("Appuyer une touche pour finir...\n");
	getchar();
    return 0;
}

void wsStartup( void )
{
    WSADATA wsaData;
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0) {
        printf("WSAStartup failed: %d\n", iResult);
        exit(1);
    }
}