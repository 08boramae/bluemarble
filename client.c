#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")

#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

typedef struct {
    int money;
    int position;
    int playerID;
    int isMyTurn;
    int inGame;
} PlayerState;

PlayerState playerState;
SOCKET clientSocket;
CRITICAL_SECTION printLock;

// Function declarations
void clearScreen() {
    system("cls");
}

void displayLobbyCommands() {
    printf("\n=== Lobby Commands ===\n");
    printf("CREATE <room_name> - Create a new game room\n");
    printf("JOIN <room_name> - Join an existing game room\n");
    printf("LIST - Show list of available rooms\n");
    printf("QUIT - Exit game\n");
}

void displayGameCommands() {
    printf("\n=== Game Commands ===\n");
    printf("roll - Roll the dice\n");
    printf("buy - Buy current property\n");
    printf("build - Build on owned property\n");
    printf("status - Show current status\n");
    printf("quit - Exit game\n");
}

SOCKET connectToServer() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed\n");
        return INVALID_SOCKET;
    }

    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        printf("Socket creation failed\n");
        WSACleanup();
        return INVALID_SOCKET;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("Connection failed\n");
        closesocket(sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    printf("Connected to server!\n");
    return sock;
}

void handleCommand(const char* command) {
    if (!playerState.inGame) {
        if (strncmp(command, "CREATE ", 7) == 0 ||
            strncmp(command, "JOIN ", 5) == 0 ||
            strcmp(command, "LIST") == 0 ||
            strcmp(command, "QUIT") == 0) {
            send(clientSocket, command, strlen(command), 0);
            return;
        }

        if (strcmp(command, "help") == 0) {
            displayLobbyCommands();
            return;
        }
    } else {
        if (!playerState.isMyTurn &&
            strcmp(command, "status") != 0 &&
            strcmp(command, "quit") != 0) {
            printf("It's not your turn! (Current turn: Player %d)\n",
                   playerState.isMyTurn ? playerState.playerID + 1 : ((playerState.playerID + 1) % 2) + 1);
            return;
        }

        if (strcmp(command, "help") == 0) {
            displayGameCommands();
            return;
        }

        send(clientSocket, command, strlen(command), 0);
    }
}


DWORD WINAPI receiveThread(LPVOID arg) {
    char buffer[BUFFER_SIZE];
    int bytesRead;

    while ((bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesRead] = '\0';

        EnterCriticalSection(&printLock);

        if (strncmp(buffer, "GAME_START:", 11) == 0) {
            sscanf(buffer + 11, "%d", &playerState.playerID);
            playerState.inGame = 1;
            clearScreen();
            printf("Game started! You are Player %d\n", playerState.playerID + 1);
            displayGameCommands();
        }
        else if (strncmp(buffer, "TURN:", 5) == 0) {
            int turn;
            sscanf(buffer + 5, "%d", &turn);
            playerState.isMyTurn = (turn == playerState.playerID);
            if (playerState.isMyTurn) {
                printf("\nIt's your turn!\n");
            }
        }
        else if (strncmp(buffer, "STATE:", 6) == 0) {
            sscanf(buffer + 6, "%d,%d", &playerState.position, &playerState.money);
            printf("\nPosition: %d, Money: $%d\n", playerState.position, playerState.money);
        }
        else {
            printf("%s", buffer);
        }

        LeaveCriticalSection(&printLock);
    }

    if (bytesRead == 0) {
        printf("Server disconnected\n");
    } else {
        printf("Receive error: %d\n", WSAGetLastError());
    }

    closesocket(clientSocket);
    return 0;
}

int main() {
    InitializeCriticalSection(&printLock);

    clientSocket = connectToServer();
    if (clientSocket == INVALID_SOCKET) {
        return 1;
    }

    // Initialize player state
    playerState.money = 3000000;
    playerState.position = 0;
    playerState.playerID = -1;
    playerState.isMyTurn = 0;
    playerState.inGame = 0;

    // Create receive thread
    HANDLE hThread = CreateThread(NULL, 0, receiveThread, NULL, 0, NULL);
    if (hThread == NULL) {
        printf("Failed to create thread\n");
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    printf("Welcome to Blue Marble!\n");
    displayLobbyCommands();

    // Main input loop
    char input[BUFFER_SIZE];
    while (1) {
        printf("Input: ");
        if (fgets(input, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        input[strcspn(input, "\n")] = 0;  // Remove newline

        if (strlen(input) == 0) {
            continue;
        }

        handleCommand(input);
    }

    // Cleanup
    WaitForSingleObject(hThread, INFINITE);
    CloseHandle(hThread);
    DeleteCriticalSection(&printLock);
    closesocket(clientSocket);
    WSACleanup();

    return 0;
}