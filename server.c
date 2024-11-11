#include <stdio.h>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")  // Winsock 라이브러리 링크

#define PORT 12345
#define BUFFER_SIZE 1024))

// 서버 소켓 초기화 및 클라이언트 연결 대기
int server_socket_init() {
    WSADATA wsa;
    SOCKET server_sock, client_sock;
    struct sockaddr_in server_addr, client_addr;
    int client_addr_size = sizeof(client_addr);

    // Winsock 초기화
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code : %d\n", WSAGetLastError());
        return -1;
    }

    // 소켓 생성
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Could not create socket : %d\n", WSAGetLastError());
        WSACleanup();
        return -1;
    }

    // 서버 주소 설정
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // 소켓 바인딩
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return -1;
    }

    // 연결 대기
    if (listen(server_sock, 3) == SOCKET_ERROR) {
        printf("Listen failed with error code : %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return -1;
    }

    printf("Waiting for incoming connections on port %d...\n", PORT);

    // 클라이언트 연결 허용
    client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);
    if (client_sock == INVALID_SOCKET) {
        printf("Accept failed with error code : %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return -1;
    }

    printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    return client_sock;
}

// 문자열 수신 함수
void socket_receive_string(SOCKET client_sock) {
    char buffer[BUFFER_SIZE];
    int recv_size;

    recv_size = recv(client_sock, buffer, BUFFER_SIZE, 0);
    if (recv_size == SOCKET_ERROR) {
        printf("Receive failed : %d\n", WSAGetLastError());
    } else {
        buffer[recv_size] = '\0';  // 널 종료
        printf("Received string: %s\n", buffer);
    }
}

// 소켓 종료 함수
void socket_close(SOCKET sock) {
    closesocket(sock);
    WSACleanup();
    printf("Connection closed\n");
}

int main() {
    SOCKET client_sock = server_socket_init();

    if (client_sock == -1) {
        printf("Failed to initialize server.\n");
        return 1;
    }

    // 문자열 수신
    socket_receive_string(client_sock);

    // 소켓 닫기
    socket_close(client_sock);

    return 0;
}
