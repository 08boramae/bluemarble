#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <openssl/sha.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_ROOMS 10
#define MAX_CLIENTS 100

DWORD WINAPI ClientHandler(LPVOID client_sock_ptr);
void broadcast_room_list_to_main_screen();
void broadcast_to_room(int room_index, SOCKET sender_sock, const char *message);

typedef struct {
    char room_name[50];
    SOCKET participants[2];
    int participant_count;
} Room;

typedef struct {
    SOCKET sock;
    int current_room;
    int on_main_screen;
} ClientState;

Room chat_rooms[MAX_ROOMS];
int room_count = 0;
ClientState clients[MAX_CLIENTS];
int client_count = 0;
CRITICAL_SECTION clients_lock;

const char *websocket_guid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

void base64_encode(const unsigned char* input, size_t input_len, char* output, size_t output_size) {
    static const char* encoding_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    size_t output_len = 4 * ((input_len + 2) / 3);
    for (size_t i = 0, j = 0; i < input_len;) {
        uint32_t octet_a = i < input_len ? (unsigned char)input[i++] : 0;
        uint32_t octet_b = i < input_len ? (unsigned char)input[i++] : 0;
        uint32_t octet_c = i < input_len ? (unsigned char)input[i++] : 0;
        uint32_t triple = (octet_a << 16) | (octet_b << 8) | octet_c;
        output[j++] = encoding_table[(triple >> 18) & 0x3F];
        output[j++] = encoding_table[(triple >> 12) & 0x3F];
        output[j++] = encoding_table[(triple >> 6) & 0x3F];
        output[j++] = encoding_table[triple & 0x3F];
    }
    for (int i = 0; i < (int)(output_len % 4); i++) {
        output[output_len - 1 - i] = '=';
    }
    output[output_len] = '\0';
}

int send_websocket_frame(SOCKET sock, const char* message) {
    if (sock == INVALID_SOCKET) {
        return -1;
    }
    size_t message_length = strlen(message);
    int frame_size = 2;

    if (message_length <= 125) {
        frame_size += message_length;
    } else if (message_length <= 65535) {
        frame_size += 2 + message_length;
    } else {
        frame_size += 8 + message_length;
    }

    unsigned char *frame = malloc(frame_size);
    frame[0] = 0x81;  // FIN bit set and text frame
    if (message_length <= 125) {
        frame[1] = (unsigned char)message_length;
        memcpy(&frame[2], message, message_length);
    } else if (message_length <= 65535) {
        frame[1] = 126;
        frame[2] = (message_length >> 8) & 0xFF;
        frame[3] = message_length & 0xFF;
        memcpy(&frame[4], message, message_length);
    } else {
        frame[1] = 127;
        for (int i = 0; i < 8; i++) {
            frame[2 + i] = (unsigned char)((message_length >> ((7 - i) * 8)) & 0xFF);
        }
        memcpy(&frame[10], message, message_length);
    }

    int sent = send(sock, (char *)frame, frame_size, 0);
    free(frame);
    return sent;
}

int recv_websocket_frame(SOCKET sock, char* buffer, int buffer_size) {
    char temp[BUFFER_SIZE];
    int ret = recv(sock, temp, BUFFER_SIZE, 0);
    if (ret <= 0) {
        return ret;
    }

    unsigned char opcode = temp[0] & 0x0F;
    if (opcode == 0x8) {
        // Close frame
        printf("Close frame received. Closing connection.\n");
        return 0;
    }

    int masked = (temp[1] & 0x80) != 0;
    size_t payloadLength = temp[1] & 0x7F;
    size_t payloadDataIndex = 2;

    if (payloadLength == 126) {
        payloadLength = (temp[2] << 8) | temp[3];
        payloadDataIndex = 4;
    } else if (payloadLength == 127) {
        payloadLength = 0;
        for (int i = 0; i < 8; i++) {
            payloadLength = (payloadLength << 8) | (unsigned char)temp[2 + i];
        }
        payloadDataIndex = 10;
    }

    unsigned char *maskingKey = NULL;
    if (masked) {
        maskingKey = (unsigned char*)&temp[payloadDataIndex];
        payloadDataIndex += 4;
    }

    if (payloadLength > (size_t)(buffer_size - 1)) {
        printf("Payload is too large to fit into the buffer.\n");
        return -1;
    }

    for (size_t i = 0; i < payloadLength; i++) {
        unsigned char byteData = temp[payloadDataIndex + i];
        if (masked) {
            byteData ^= maskingKey[i % 4];
        }
        buffer[i] = byteData;
    }

    buffer[payloadLength] = '\0';
    return (int)payloadLength;
}

SOCKET server_socket_init() {
    WSADATA wsa;
    SOCKET server_sock;
    struct sockaddr_in server_addr;

    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed: %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Could not create socket: %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    if (listen(server_sock, 3) == SOCKET_ERROR) {
        printf("Listen failed: %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    printf("Server started on port %d.\n", PORT);
    return server_sock;
}

SOCKET accept_client(SOCKET server_sock) {
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    SOCKET client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);

    if (client_sock == INVALID_SOCKET) {
        printf("Accept failed: %d\n", WSAGetLastError());
    } else {
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    return client_sock;
}

int websocket_handshake(SOCKET client_sock) {
    char buffer[BUFFER_SIZE];
    int received = recv(client_sock, buffer, BUFFER_SIZE, 0);
    if (received <= 0) {
        return 0;
    }

    buffer[received] = '\0';

    if (strstr(buffer, "Upgrade: websocket") == NULL) {
        printf("No WebSocket upgrade requested.\n");
        return 0;
    }

    char *keyHeader = strstr(buffer, "Sec-WebSocket-Key: ");
    if (!keyHeader) {
        printf("Sec-WebSocket-Key not found.\n");
        return 0;
    }

    keyHeader += 19;
    char secWebSocketKey[60] = {0};
    int i = 0;
    while (keyHeader[i] != '\r' && keyHeader[i] != '\n' && i < 60) {
        secWebSocketKey[i] = keyHeader[i];
        i++;
    }

    char concatenated[128];
    sprintf(concatenated, "%s%s", secWebSocketKey, websocket_guid);

    unsigned char shaHash[SHA_DIGEST_LENGTH];
    SHA1((unsigned char*)concatenated, strlen(concatenated), shaHash);

    char secWebSocketAccept[64] = {0};
    base64_encode(shaHash, SHA_DIGEST_LENGTH, secWebSocketAccept, sizeof(secWebSocketAccept));

    char response[BUFFER_SIZE];
    sprintf(response,
        "HTTP/1.1 101 Switching Protocols\r\n"
        "Upgrade: websocket\r\n"
        "Connection: Upgrade\r\n"
        "Sec-WebSocket-Accept: %s\r\n\r\n",
        secWebSocketAccept
    );

    send(client_sock, response, strlen(response), 0);
    return 1;
}

void broadcast_room_list_to_main_screen() {
    EnterCriticalSection(&clients_lock);
    for (int i = 0; i < client_count; i++) {
        if (clients[i].on_main_screen && clients[i].sock != INVALID_SOCKET) {
            char room_info[BUFFER_SIZE] = {0};
            strcat(room_info, "Available rooms:\n");
            for (int j = 0; j < room_count; j++) {
                char room_line[100];
                sprintf(room_line, "%s (Participants: %d/2)\n", chat_rooms[j].room_name, chat_rooms[j].participant_count);
                strcat(room_info, room_line);
            }
            if (room_count == 0) {
                strcat(room_info, "No rooms available.\n");
            }
            strcat(room_info, "\nTo create a new room, type: CREATE <room_name>\n");
            strcat(room_info, "To join an existing room, type: JOIN <room_name>\n");
            strcat(room_info, "Your choice: ");
            send_websocket_frame(clients[i].sock, room_info);
        }
    }
    LeaveCriticalSection(&clients_lock);
}

int create_room(const char *room_name) {
    if (room_count >= MAX_ROOMS) {
        return -1;
    }

    strcpy(chat_rooms[room_count].room_name, room_name);
    chat_rooms[room_count].participants[0] = INVALID_SOCKET;
    chat_rooms[room_count].participants[1] = INVALID_SOCKET;
    chat_rooms[room_count].participant_count = 0;
    room_count++;

    return room_count - 1;
}

int find_room_by_name(const char *room_name) {
    for (int i = 0; i < room_count; i++) {
        if (strcmp(chat_rooms[i].room_name, room_name) == 0) {
            return i;
        }
    }
    return -1;
}

int join_room(int room_index, SOCKET client_sock) {
    if (room_index < 0 || room_index >= room_count) {
        return -1;
    }

    Room *room = &chat_rooms[room_index];
    if (room->participant_count >= 2) {
        return -2;
    }

    room->participants[room->participant_count] = client_sock;
    room->participant_count++;
    return 0;
}

void broadcast_to_room(int room_index, SOCKET sender_sock, const char *message) {
    Room *room = &chat_rooms[room_index];
    for (int i = 0; i < room->participant_count; i++) {
        if (room->participants[i] != sender_sock && room->participants[i] != INVALID_SOCKET) {
            send_websocket_frame(room->participants[i], message);
        }
    }
}

void leave_room(int room_index, SOCKET client_sock) {
    if (room_index < 0 || room_index >= room_count) {
        return;
    }

    Room *room = &chat_rooms[room_index];
    for (int i = 0; i < room->participant_count; i++) {
        if (room->participants[i] == client_sock) {
            for (int j = i; j < room->participant_count - 1; j++) {
                room->participants[j] = room->participants[j + 1];
            }
            room->participants[room->participant_count - 1] = INVALID_SOCKET;
            room->participant_count--;
            break;
        }
    }
}

DWORD WINAPI ClientHandler(LPVOID client_sock_ptr) {
    SOCKET client_sock = *(SOCKET *)client_sock_ptr;
    free(client_sock_ptr);

    if (!websocket_handshake(client_sock)) {
        closesocket(client_sock);
        return 1;
    }

    int index = -1;
    EnterCriticalSection(&clients_lock);
    for (int i = 0; i < MAX_CLIENTS; i++) {
        if (clients[i].sock == INVALID_SOCKET) {
            clients[i].sock = client_sock;
            clients[i].current_room = -1;
            clients[i].on_main_screen = 1;
            index = i;
            if (i >= client_count) {
                client_count = i + 1;
            }
            break;
        }
    }
    LeaveCriticalSection(&clients_lock);

    if (index == -1) {
        send_websocket_frame(client_sock, "Server is full. Please try again later.\n");
        closesocket(client_sock);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int current_room = -1;

    broadcast_room_list_to_main_screen();

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int payload_length = recv_websocket_frame(client_sock, buffer, BUFFER_SIZE);
        if (payload_length <= 0) {
            printf("Client disconnected or closed WebSocket.\n");
            break;
        }

        if (clients[index].on_main_screen) {
            char command[50], room_name[50];
            int parsed_count = sscanf(buffer, "%s %49[^\n]", command, room_name);

            if (parsed_count < 1) {
                send_websocket_frame(client_sock, "Invalid command. Please try again.\n");
                continue;
            }

            if (strcmp(command, "CREATE") == 0) {
                if (parsed_count < 2) {
                    send_websocket_frame(client_sock, "Please give a room name. Usage: CREATE <room_name>\n");
                    continue;
                }

                int room_index = find_room_by_name(room_name);
                if (room_index != -1) {
                    send_websocket_frame(client_sock, "Room already exists.\n");
                    continue;
                }

                room_index = create_room(room_name);
                if (room_index == -1) {
                    send_websocket_frame(client_sock, "Cannot create more rooms.\n");
                    continue;
                }

                join_room(room_index, client_sock);
                current_room = room_index;
                clients[index].current_room = room_index;
                clients[index].on_main_screen = 0;
                send_websocket_frame(client_sock, "Room created and joined successfully.\nType 'LEAVE' to exit the room.\n");

                broadcast_room_list_to_main_screen();

                broadcast_to_room(room_index, client_sock, "A new user has joined the room.\n");

            } else if (strcmp(command, "JOIN") == 0) {
                if (parsed_count < 2) {
                    send_websocket_frame(client_sock, "Please give a room name. Usage: JOIN <room_name>\n");
                    continue;
                }

                int room_index = find_room_by_name(room_name);
                if (room_index == -1) {
                    send_websocket_frame(client_sock, "Room does not exist.\n");
                    continue;
                }

                int join_result = join_room(room_index, client_sock);
                if (join_result == -2) {
                    send_websocket_frame(client_sock, "Room is full.\n");
                    continue;
                } else if (join_result == -1) {
                    send_websocket_frame(client_sock, "An error occurred. Please try again.\n");
                    continue;
                }

                current_room = room_index;
                clients[index].current_room = room_index;
                clients[index].on_main_screen = 0;
                send_websocket_frame(client_sock, "Joined room successfully.\nType 'LEAVE' to exit the room.\n");

                broadcast_to_room(room_index, client_sock, "A new user has joined the room.\n");

                broadcast_room_list_to_main_screen();

            } else {
                send_websocket_frame(client_sock, "Invalid command. Please use CREATE or JOIN followed by the room name.\n");
            }
        } else {
            if (strcmp(buffer, "LEAVE") == 0) {
                leave_room(current_room, client_sock);
                clients[index].current_room = -1;
                clients[index].on_main_screen = 1;
                send_websocket_frame(client_sock, "You have left the room.\n");
                broadcast_to_room(current_room, client_sock, "A user has left the room.\n");
                current_room = -1;

                broadcast_room_list_to_main_screen();
            } else {
                broadcast_to_room(current_room, client_sock, buffer);
            }
        }
    }

    if (current_room != -1) {
        leave_room(current_room, client_sock);
        broadcast_to_room(current_room, client_sock, "A user has left the room.\n");
    }

    closesocket(client_sock);

    EnterCriticalSection(&clients_lock);
    clients[index].sock = INVALID_SOCKET;
    clients[index].current_room = -1;
    clients[index].on_main_screen = 0;
    LeaveCriticalSection(&clients_lock);

    printf("Client handler thread closing.\n");
    return 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed. Error: %d\n", WSAGetLastError());
        return 1;
    }

    SOCKET server_sock = server_socket_init();
    InitializeCriticalSection(&clients_lock);

    if (server_sock == INVALID_SOCKET) {
        printf("Failed to initialize server.\n");
        DeleteCriticalSection(&clients_lock);
        WSACleanup();
        return 1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sock = INVALID_SOCKET;
        clients[i].current_room = -1;
        clients[i].on_main_screen = 0;
    }

    printf("Waiting for new WebSocket clients...\n");

    while (1) {
        SOCKET client_sock = accept_client(server_sock);

        if (client_sock != INVALID_SOCKET) {
            SOCKET *client_sock_ptr = malloc(sizeof(SOCKET));
            *client_sock_ptr = client_sock;
            CreateThread(NULL, 0, ClientHandler, (LPVOID)client_sock_ptr, 0, NULL);
        }
    }

    closesocket(server_sock);
    DeleteCriticalSection(&clients_lock);
    WSACleanup();
    return 0;
}
