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

#define SERVER_IP_ADDRESS "127.0.0.1"   // Adresa servera
#define BUFFER_SIZE 512 // Veličina buffera

// Structura koja se traži u zadatku, simulira merenj
struct Merenje {
    char nazivGrada[21];
    short indexKvalitetaVazduha;
};

int main()
{
    sockaddr_in serverAddress;

    int sockAddrLen = sizeof(serverAddress);

    char dataBuffer[BUFFER_SIZE];

    WSADATA wsaData;

    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed with error: %d\n", iResult);
        return 1;
    }

    memset((char*)&serverAddress, 0, sizeof(serverAddress));

    // Unošenje porta
    printf("Unesite port servera (19000 or 19001):\n");
    gets_s(dataBuffer, BUFFER_SIZE);
    unsigned short serverPort = atoi(dataBuffer);

    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = inet_addr(SERVER_IP_ADDRESS);
    serverAddress.sin_port = htons(serverPort);

    SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket == INVALID_SOCKET)
    {
        printf("Creating socket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    char poruka[100] = "Prijava!";  // Poruka koja treba da se pošalje serveru
    iResult = sendto(clientSocket, poruka, sizeof(poruka), 0, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
    if (iResult == SOCKET_ERROR)
    {
        printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    iResult = recvfrom(clientSocket, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&serverAddress, &sockAddrLen);
    if (iResult == SOCKET_ERROR)
    {
        printf("Greska prilikom primanja poruke: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Primljena poruka od servera je: %s\n", dataBuffer);

    Merenje jedanUzorak;    // Instanca structure služi da bi mogli da popunimo podatke i da ih pošaljemo
    short index; // Pomoćna promenljiva za dodelu vrednosti polju strukture "indexKvalitetaVazduha"

    while (true)
    {
        // Unos potrebnih podataka koji ce se poslati serveru
        printf("Unesite naziv grada: ");
        gets_s(jedanUzorak.nazivGrada, 21);

        printf("Unesite index kvaliteta vazduha: ");
        scanf_s("%d", &index);
        getchar();    //pokupiti enter karakter iz bafera tastature
        jedanUzorak.indexKvalitetaVazduha = index;

        iResult = sendto(clientSocket, (char*)&jedanUzorak, (int)sizeof(jedanUzorak), 0, (SOCKADDR*)&serverAddress, sizeof(serverAddress));
        if (iResult == SOCKET_ERROR)
        {
            printf("Greska prilikom slanja poruke: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }
        printf("_______________________________  \n");
        printf("Poruka je uspesno poslata. Velicina u bajtima je: %ld\n\n", iResult);

        iResult = recvfrom(clientSocket, dataBuffer, BUFFER_SIZE, 0, (SOCKADDR*)&serverAddress, &sockAddrLen);
        if (iResult == SOCKET_ERROR)
        {
            printf("Greska prilikom primanja poruke: %d\n", WSAGetLastError());
            closesocket(clientSocket);
            WSACleanup();
            return 1;
        }

        printf("Primljena poruka od servera je: %s\n", dataBuffer);
        printf("______________________________________________________________ \n\n");

        printf("\nAko zelite da zavrsite unesite 'Kraj': \n");
        gets_s(dataBuffer, BUFFER_SIZE);
        if (strcmp(dataBuffer, "Kraj") == 0) {
            break;
        }
    }

    iResult = closesocket(clientSocket);
    if (iResult == SOCKET_ERROR)
    {
        printf("closesocket failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    WSACleanup();

    return 0;
}