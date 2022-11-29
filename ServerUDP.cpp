#define _WINSOCK_DEPRECATED_NO_WARNINGS

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include "conio.h"

#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Mswsock.lib")
#pragma comment (lib, "AdvApi32.lib")

#define SERVER_PORT1 19000  // Port prve serverske uticnice
#define SERVER_PORT2 19001	// Port druge serverske uticnice

#define BUFFER_SIZE 512 // Veličina buffera

// Structura iz texta, simulira merenje
struct Merenje {
    char nazivGrada[21];
    short indexKvalitetaVazduha;
};

int main()
{
    sockaddr_in serverAddress1; // Struktura za prvog klijenta
    sockaddr_in serverAddress2; // Struktura za drugog klijenta

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup failed with error: %d\n", WSAGetLastError());
        return 1;
    }

    memset((char*)&serverAddress1, 0, sizeof(serverAddress1));
    serverAddress1.sin_family = AF_INET;
    serverAddress1.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress1.sin_port = htons(SERVER_PORT1);

    memset((char*)&serverAddress2, 0, sizeof(serverAddress2));
    serverAddress2.sin_family = AF_INET;
    serverAddress2.sin_addr.s_addr = inet_addr("127.0.0.1");
    serverAddress2.sin_port = htons(SERVER_PORT2);

    SOCKET serverSocket1 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket1 == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    SOCKET serverSocket2 = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (serverSocket2 == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    int iResult = bind(serverSocket1, (SOCKADDR*)&serverAddress1, sizeof(serverAddress1));
    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket1);
        WSACleanup();
        return 1;
    }

    iResult = bind(serverSocket2, (SOCKADDR*)&serverAddress2, sizeof(serverAddress2));
    if (iResult == SOCKET_ERROR)
    {
        printf("Socket bind failed with error: %d\n", WSAGetLastError());
        closesocket(serverSocket2);
        WSACleanup();
        return 1;
    }

    printf("Simple UDP server started and waiting client messages.\n");

    sockaddr_in clientAddress;  // Structura da bi imali informacije o klijentu
    int sockAddrLen = sizeof(clientAddress);

    // Stavljamo utičnice u neblokirajuci rezim
    unsigned long mode = 1;
    if (ioctlsocket(serverSocket1, FIONBIO, &mode) != 0 || ioctlsocket(serverSocket2, FIONBIO, &mode) != 0)
    {
        printf("ioctlsocket failed with error %d\n", WSAGetLastError());
        closesocket(serverSocket1);
        closesocket(serverSocket2);
        WSACleanup();
        return 1;
    }

    Merenje* jedanUzorak1;  // Instanca structure koja nam treba da preuzmemo podatke koje nam je prvi klijent posalo
    Merenje* jedanUzorak2;  // Instanca structure koja nam treba da preuzmemo podatke koje nam je drugi klijent posalo
    int i = 0;	// Brojac za prvog klijenta
    int x = 0;	// Brojac za drugog klijenta

    Merenje prikuljeniPodaci1[5];   // Niz struktura u koje smeštamo primljene strukture, treba nam da bi mogli da čuvamo sve primljene structure od prvog klijenta
    memset(&prikuljeniPodaci1, 0, 5 * sizeof(Merenje)); // Inicijalizujemo memoriju za niz structura sa nulama

    Merenje prikuljeniPodaci2[5];   // Niz struktura u koje smeštamo primljene strukture, treba nam da bi mogli da čuvamo sve primljene structure od prvog klijenta
    memset(&prikuljeniPodaci2, 0, 5 * sizeof(Merenje)); // Inicijalizujemo memoriju za niz structura sa nulama

    Sleep(10000);   // Treba nam da ne bi bacio odma grešku dok čeka da mi unesemo port za prvog klijenta

    iResult = recvfrom(serverSocket1, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddress, &sockAddrLen);
    if (iResult == SOCKET_ERROR)
    {
        printf("Greska prilikom prijema poruke: %d\n", WSAGetLastError());
        closesocket(serverSocket1);
        WSACleanup();
        return 1;
    }

    char poruka[100] = "Uspesna prijava!";  // Poruka koju trebamo poslati klijentu
    iResult = sendto(serverSocket1, poruka, sizeof(poruka), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
        closesocket(serverSocket1);
        WSACleanup();
        return 1;
    }

    Sleep(5000);    // Treba nam da ne bi bacio odma grešku dok čeka da mi unesemo port za drugog klijenta

    iResult = recvfrom(serverSocket2, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddress, &sockAddrLen);
    if (iResult == SOCKET_ERROR)
    {
        printf("Greska prilikom prijema poruke: %d\n", WSAGetLastError());
        closesocket(serverSocket1);
        closesocket(serverSocket2);
        WSACleanup();
        return 1;
    }

    iResult = sendto(serverSocket2, poruka, sizeof(poruka), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
        closesocket(serverSocket1);
        closesocket(serverSocket2);
        WSACleanup();
        return 1;
    }

    while (1)
    {
        memset(&clientAddress, 0, sizeof(clientAddress));

        memset(dataBuffer, 0, BUFFER_SIZE);

        // Prijem poruke sa uticnici br. 1
        iResult = recvfrom(serverSocket1, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddress, &sockAddrLen);

        if (iResult != SOCKET_ERROR)
        {
            if (i < 5) {
                dataBuffer[iResult] = '\0';

                jedanUzorak1 = (Merenje*)dataBuffer;

                if (i == 0)
                {
                    printf("\nNaziv grada: %s  \n", jedanUzorak1->nazivGrada);
                    printf("Index kvaliteta vazduha: %d  \n", jedanUzorak1->indexKvalitetaVazduha);
                    printf("_______________________________  \n");
                    printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

                    strcpy_s(prikuljeniPodaci1[i].nazivGrada, jedanUzorak1->nazivGrada);
                    prikuljeniPodaci1[i].indexKvalitetaVazduha = jedanUzorak1->indexKvalitetaVazduha;
                    i++;
                }
                else
                {
                    printf("\nTrenutno uneti podaci :\n");
                    printf("\nNaziv grada: %s  \n", jedanUzorak1->nazivGrada);
                    printf("Index kvaliteta vazduha: %d  \n", jedanUzorak1->indexKvalitetaVazduha);
                    printf("_______________________________  \n");

                    if (jedanUzorak1->indexKvalitetaVazduha > 0)
                    {
                        strcpy_s(prikuljeniPodaci1[i].nazivGrada, jedanUzorak1->nazivGrada);
                        prikuljeniPodaci1[i].indexKvalitetaVazduha = jedanUzorak1->indexKvalitetaVazduha;
                        i++;

                        int j;
                        printf("\nPodaci koje imamo sacuvano :\n");
                        for (j = 0; j < i; j++) {
                            printf("\nNaziv grada: %s  \n", prikuljeniPodaci1[j].nazivGrada);
                            printf("Index kvaliteta vazduha: %d  \n", prikuljeniPodaci1[j].indexKvalitetaVazduha);
                            printf("_______________________________  \n");
                        }

                        printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                    }
                    else if (jedanUzorak1->indexKvalitetaVazduha < 0) {
                        char s[10] = "ODBACENO!";

                        iResult = sendto(serverSocket1, s, sizeof(s), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
                        if (iResult == SOCKET_ERROR)
                        {
                            printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
                            closesocket(serverSocket1);
                            closesocket(serverSocket2);
                            break;
                        }
                    }
                }

                char s[10] = "UNETO!";

                iResult = sendto(serverSocket1, s, sizeof(s), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
                if (iResult == SOCKET_ERROR)
                {
                    printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
                    closesocket(serverSocket1);
                    closesocket(serverSocket2);
                    break;
                }
            }
            else {
                char s[10] = "ODBACENO!";

                iResult = sendto(serverSocket1, s, sizeof(s), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
                if (iResult == SOCKET_ERROR)
                {
                    printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
                    closesocket(serverSocket1);
                    closesocket(serverSocket2);
                    break;
                }
            }
        }
        else
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                Sleep(3000);
            }
            else {
                printf("Greska prilikom prijema poruke: %d\n", WSAGetLastError());
                closesocket(serverSocket1);
                WSACleanup();
                continue;

            }
        }

        iResult = recvfrom(serverSocket2, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&clientAddress, &sockAddrLen);

        if (iResult != SOCKET_ERROR)
        {
            if (x < 5) {
                dataBuffer[iResult] = '\0';

                jedanUzorak2 = (Merenje*)dataBuffer;

                if (x == 0)
                {
                    printf("\nNaziv grada: %s  \n", jedanUzorak2->nazivGrada);
                    printf("Index kvaliteta vazduha: %d  \n", jedanUzorak2->indexKvalitetaVazduha);
                    printf("_______________________________  \n");
                    printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));

                    strcpy_s(prikuljeniPodaci2[x].nazivGrada, jedanUzorak2->nazivGrada);
                    prikuljeniPodaci2[x].indexKvalitetaVazduha = jedanUzorak2->indexKvalitetaVazduha;
                    x++;
                }
                else
                {
                    printf("\nTrenutno uneti podaci :\n");
                    printf("\nNaziv grada: %s  \n", jedanUzorak2->nazivGrada);
                    printf("Index kvaliteta vazduha: %d  \n", jedanUzorak2->indexKvalitetaVazduha);
                    printf("_______________________________  \n");

                    if (jedanUzorak2->indexKvalitetaVazduha > 0)
                    {
                        strcpy_s(prikuljeniPodaci2[x].nazivGrada, jedanUzorak2->nazivGrada);
                        prikuljeniPodaci2[x].indexKvalitetaVazduha = jedanUzorak2->indexKvalitetaVazduha;
                        x++;

                        int j;
                        printf("\nPodaci koje imamo sacuvano :\n");
                        for (j = 0; j < x; j++) {
                            printf("\nNaziv grada: %s  \n", prikuljeniPodaci2[j].nazivGrada);
                            printf("Index kvaliteta vazduha: %d  \n", prikuljeniPodaci2[j].indexKvalitetaVazduha);
                            printf("_______________________________  \n");
                        }

                        printf("\nPodaci klijenta: %s : %d\n\n", inet_ntoa(clientAddress.sin_addr), ntohs(clientAddress.sin_port));
                    }
                    else if (jedanUzorak2->indexKvalitetaVazduha < 0) {

                        char s[10] = "ODBACENO!";

                        iResult = sendto(serverSocket2, s, sizeof(s), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
                        if (iResult == SOCKET_ERROR)
                        {
                            printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
                            closesocket(serverSocket1);
                            closesocket(serverSocket2);
                            break;
                        }
                    }
                }

                char s[10] = "UNETO!";

                iResult = sendto(serverSocket2, s, sizeof(s), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
                if (iResult == SOCKET_ERROR)
                {
                    printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
                    closesocket(serverSocket1);
                    closesocket(serverSocket2);
                    break;
                }
            }
            else {
                char s[10] = "ODBACENO!";

                iResult = sendto(serverSocket2, s, sizeof(s), 0, (SOCKADDR*)&clientAddress, sizeof(clientAddress));
                if (iResult == SOCKET_ERROR)
                {
                    printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
                    closesocket(serverSocket1);
                    closesocket(serverSocket2);
                    break;
                }
            }
        }
        else
        {
            if (WSAGetLastError() == WSAEWOULDBLOCK) {
                Sleep(3000);
            }
            else {
                printf("Greska prilikom prijema poruke: %d\n", WSAGetLastError());
                closesocket(serverSocket1);
                WSACleanup();
                break;
            }
        }
    }

    iResult = closesocket(serverSocket1);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    iResult = closesocket(serverSocket2);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %ld\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }
    printf("Server successfully shut down.\n");

    WSACleanup();

    return 0;
}