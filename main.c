
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
#include <winsock2.h>

int socket_init(const char *ip, int port) {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        return -1;
    }

    struct sockaddr_in ServerAddress;
    ServerAddress.sin_family = AF_INET;
    ServerAddress.sin_port = htons(port);
    ServerAddress.sin_addr.s_addr = inet_addr(ip);

    if (connect(sockfd, (struct sockaddr*)&ServerAddress, sizeof(ServerAddress)) < 0) {
        printf("Connection failed: %d\n", WSAGetLastError());
        return -1;
    }

    return sockfd;
}

void socket_send(int sockfd, const char *message) {
    send(sockfd, message, strlen(message), 0);
}

int main() {
    int sockfd = socket_init("127.0.0.1", 12345);
    if (sockfd == -1) {
        printf("Socket connect failed");
        return 1;
    }
    socket_send(sockfd, "Hello, Server!");
    closesocket(sockfd);
    WSACleanup();
    return 0;
}

