
//
// Created by Jaeyoung Jang on 2024-11-11.
//

#include <stdio.h>
#include <Windows.h> // ������� �߰�

void gotoxy(int x, int y);
void drawGameBoard();

void gotoxy(int x, int y) {
    COORD pos = { x*2, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void drawGameBoard() {
    for(int i=0; i<10;)
    gotoxy(2, 3);
    printf("��");
}

/*


����á�����
�ེ��  Ȧ������
��ʢʡ�ʢʡ���

����á����
�ེ��  Ȧ�����
��ʢʡ�ʢʡ��


����
��  ��
�ʼ���
��  ��
�ʿ��
��  ��
����
����
����

                  ����
                  ��  ��
                  �ʼ���
                  ��  ��
                  �ʿ��
                  ��  ��
                  ����
                  ����
                  ����
����á�����
�ེ��  Ȧ������
��  17  ��������
��ʢʡ�ʢʡ���








��ʢˢ�?�ââ�


*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")  // Winsock library

#define BUFFER_SIZE 1024

// Thread function for receiving messages from the server
DWORD WINAPI receive_thread(LPVOID socket_desc);

// Initialize socket and connect to server
SOCKET socket_init(const char *ip, int port) {
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        printf("WSAStartup failed with error: %d\n", result);
        return INVALID_SOCKET;
    }

    SOCKET sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    struct sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    serverAddress.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
        printf("Connection failed: %d\n", WSAGetLastError());
        closesocket(sockfd);
        WSACleanup();
        return INVALID_SOCKET;
    }

    printf("Connected to server at %s:%d\n", ip, port);
    return sockfd;
}

// send a message to server
void socket_send(SOCKET sockfd, const char *message) {
    send(sockfd, message, strlen(message), 0);
}

int main() {
    // connect server
    SOCKET sockfd = socket_init("127.0.0.1", 12345);
    if (sockfd == INVALID_SOCKET) {
        return 1;
    }

    // make thread
    HANDLE recv_thread = CreateThread(NULL, 0, receive_thread, (LPVOID)sockfd, 0, NULL);
    if (recv_thread == NULL) {
        printf("Could not create receive thread.\n");
        closesocket(sockfd);
        WSACleanup();
        return 1;
    }

    // continue to read and send
    char user_input[BUFFER_SIZE];
    while (1) {
        if (fgets(user_input, BUFFER_SIZE, stdin) == NULL) {
            break;
        }

        // 만약 사용자 입력이 개행 문자만 있는 경우 무시
        if (strcmp(user_input, "\n") == 0) {
            continue;
        }

        // send
        socket_send(sockfd, user_input);
    }

    // wait to recieve
    WaitForSingleObject(recv_thread, INFINITE);

    // goodbye
    CloseHandle(recv_thread);
    closesocket(sockfd);
    WSACleanup();

    printf("Connection closed.\n");
    return 0;
}

// thread fuction for recieve message
DWORD WINAPI receive_thread(LPVOID socket_desc) {
    SOCKET sockfd = (SOCKET)socket_desc;
    char buffer[BUFFER_SIZE];
    int recv_size;

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);

        recv_size = recv(sockfd, buffer, BUFFER_SIZE, 0);
        if (recv_size == SOCKET_ERROR || recv_size == 0) {
            // Connection closed or error
            break;
        }

        buffer[recv_size] = '\0';  // Null terminate the received string
        printf("%s", buffer);
    }

    return 0;
}
