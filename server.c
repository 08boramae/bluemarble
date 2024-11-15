#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>

#pragma comment(lib, "ws2_32.lib")  // Winsock library

#define PORT 12345
#define BUFFER_SIZE 1024
#define MAX_ROOMS 10
#define MAX_CLIENTS 100

DWORD WINAPI ClientHandler(LPVOID client_sock_ptr);
void broadcast_room_list_to_all();
void broadcast_room_list_to_main_screen();
void broadcast_message_to_room(int room_index, SOCKET sender_sock, const char *message);

// room
typedef struct {
    char room_name[50];
    SOCKET participants[2];
    int participant_count;
} Room;

// Client state structure
typedef struct {
    SOCKET sock;
    int current_room;
    int on_main_screen; // 1 if on main screen, 0 if in a room or other state
} ClientState;

// Global variables for rooms and clients
Room chat_rooms[MAX_ROOMS];
int room_count = 0;
ClientState clients[MAX_CLIENTS];
int client_count = 0;
CRITICAL_SECTION clients_lock; // Critical section for safe client list operations

// init server socket
SOCKET server_socket_init() {
    WSADATA wsa;
    SOCKET server_sock;
    struct sockaddr_in server_addr;

    // init Winsock
    if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0) {
        printf("WSAStartup failed. Error Code : %d\n", WSAGetLastError());
        return INVALID_SOCKET;
    }

    // make socket
    server_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (server_sock == INVALID_SOCKET) {
        printf("Could not create socket : %d\n", WSAGetLastError());
        WSACleanup();
        return INVALID_SOCKET;
    }

    // server address conf
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    // bind socket
    if (bind(server_sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERROR) {
        printf("Bind failed with error code : %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    // Listen
    if (listen(server_sock, 3) == SOCKET_ERROR) {
        printf("Listen failed with error code : %d\n", WSAGetLastError());
        closesocket(server_sock);
        WSACleanup();
        return INVALID_SOCKET;
    }

    printf("Server started on port %d.\n", PORT);
    return server_sock;
}

// accept client connection
SOCKET accept_client(SOCKET server_sock) {
    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    SOCKET client_sock = accept(server_sock, (struct sockaddr *)&client_addr, &client_addr_size);

    if (client_sock == INVALID_SOCKET) {
        printf("Accept failed with error code : %d\n", WSAGetLastError());
    } else {
        printf("Connection accepted from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    }
    return client_sock;
}

// send message to a socket
int send_message(SOCKET sock, const char* message) {
    if (sock == INVALID_SOCKET) {
        return -1;
    }
    int len = strlen(message);
    return send(sock, message, len, 0);
}

// broadcast room when a new room is created
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
            strcat(room_info, "\n");
            send_message(clients[i].sock, room_info);
            send_message(clients[i].sock, "To create a new room, type: CREATE <room_name>\n");
            send_message(clients[i].sock, "To join an existing room, type: JOIN <room_name>\n");
            send_message(clients[i].sock, "Your choice: ");
        }
    }
    LeaveCriticalSection(&clients_lock);
}

// create new room
int create_room(const char *room_name) {
    if (room_count >= MAX_ROOMS) {
        return -1; // no more :(
    }

    strcpy(chat_rooms[room_count].room_name, room_name);
    chat_rooms[room_count].participants[0] = INVALID_SOCKET;
    chat_rooms[room_count].participants[1] = INVALID_SOCKET;
    chat_rooms[room_count].participant_count = 0;
    room_count++;

    return room_count - 1; // index of new room
}

// find a room by name
int find_room_by_name(const char *room_name) {
    for (int i = 0; i < room_count; i++) {
        if (strcmp(chat_rooms[i].room_name, room_name) == 0) {
            return i;
        }
    }
    return -1; // Not found
}

// Join a room
int join_room(int room_index, SOCKET client_sock) {
    if (room_index < 0 || room_index >= room_count) {
        return -1; // Invalid room index
    }

    Room *room = &chat_rooms[room_index];
    if (room->participant_count >= 2) {
        return -2; // full room \
    }

    room->participants[room->participant_count] = client_sock;
    room->participant_count++;
    return 0;
}

// broadcast to room participants
void broadcast_to_room(int room_index, SOCKET sender_sock, const char *message) {
    Room *room = &chat_rooms[room_index];
    for (int i = 0; i < room->participant_count; i++) {
        if (room->participants[i] != sender_sock && room->participants[i] != INVALID_SOCKET) {
            send_message(room->participants[i], message);
        }
    }
}

// remove a client from a room
void leave_room(int room_index, SOCKET client_sock) {
    if (room_index < 0 || room_index >= room_count) {
        return;
    }

    Room *room = &chat_rooms[room_index];
    for (int i = 0; i < room->participant_count; i++) {
        if (room->participants[i] == client_sock) {
            // Shift participants left
            for (int j = i; j < room->participant_count - 1; j++) {
                room->participants[j] = room->participants[j + 1];
            }
            room->participants[room->participant_count - 1] = INVALID_SOCKET;
            room->participant_count--;
            break;
        }
    }
}

// thread function for client
DWORD WINAPI ClientHandler(LPVOID client_sock_ptr) {
    SOCKET client_sock = *(SOCKET *)client_sock_ptr;
    free(client_sock_ptr);

    // Initialize client state
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
        // Server is full
        send_message(client_sock, "Server is full. Please try again later.\n");
        closesocket(client_sock);
        return 1;
    }

    char buffer[BUFFER_SIZE];
    int recv_size;
    int current_room = -1;

    // send room list
    broadcast_room_list_to_main_screen();

    // main loop
    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        recv_size = recv(client_sock, buffer, BUFFER_SIZE, 0);
        if (recv_size <= 0) {
            // Connection closed or error
            printf("Client disconnected.\n");
            break;
        }

        buffer[recv_size] = '\0';
        if (clients[index].on_main_screen) {
            // parsing commands on main screen
            char command[50], room_name[50];
            int parsed_count = sscanf(buffer, "%s %49[^\n]", command, room_name);

            if (parsed_count < 1) {
                send_message(client_sock, "Invalid command. Please try again.\n");
                continue;
            }

            if (strcmp(command, "CREATE") == 0) {
                if (parsed_count < 2) {
                    send_message(client_sock, "Please give a room name. Usage: CREATE <room_name>\n");
                    continue;
                }

                int room_index = find_room_by_name(room_name);
                if (room_index != -1) {
                    send_message(client_sock, "Room already exists.\n");
                    continue;
                }

                room_index = create_room(room_name);
                if (room_index == -1) {
                    send_message(client_sock, "Cannot create more rooms.\n");
                    continue;
                }

                // Join the newly created room
                join_room(room_index, client_sock);
                current_room = room_index;
                clients[index].current_room = room_index;
                clients[index].on_main_screen = 0;
                send_message(client_sock, "Room created and joined successfully.\n");
                send_message(client_sock, "Type 'LEAVE' to exit the room.\n");

                // Update room list for others on the main screen
                broadcast_room_list_to_main_screen();

                // Notify other participants in the room about new user
                broadcast_to_room(room_index, client_sock, "A new user has joined the room.\n");

            } else if (strcmp(command, "JOIN") == 0) {
                if (parsed_count < 2) {
                    send_message(client_sock, "Please give a room name. Usage: JOIN <room_name>\n");
                    continue;
                }

                int room_index = find_room_by_name(room_name);
                if (room_index == -1) {
                    send_message(client_sock, "Room does not exist.\n");
                    continue;
                }

                int join_result = join_room(room_index, client_sock);
                if (join_result == -2) {
                    send_message(client_sock, "Room is full.\n");
                    continue;
                } else if (join_result == -1) {
                    send_message(client_sock, "An error occurred. Please try again.\n");
                    continue;
                }

                current_room = room_index;
                clients[index].current_room = room_index;
                clients[index].on_main_screen = 0;
                send_message(client_sock, "Joined room successfully.\n");
                send_message(client_sock, "Type 'LEAVE' to exit the room.\n");

                // Notify other participants in the room about new user
                broadcast_to_room(room_index, client_sock, "A new user has joined the room.\n");

                // Update room list for others on the main screen
                broadcast_room_list_to_main_screen();

            } else {
                send_message(client_sock, "Invalid command. Please use CREATE or JOIN followed by the room name.\n");
            }
        } else {
            // User is in a room
            if (strcmp(buffer, "LEAVE\n") == 0) {
                // User leaves the room
                leave_room(current_room, client_sock);
                clients[index].current_room = -1;
                clients[index].on_main_screen = 1;

                send_message(client_sock, "You have left the room.\n");
                // Notify remaining participants
                broadcast_to_room(current_room, client_sock, "A user has left the room.\n");
                current_room = -1;

                // Update room list for all clients on main screen
                broadcast_room_list_to_main_screen();

            } else {
                // Broadcast the message to others in the same room
                broadcast_to_room(current_room, client_sock, buffer);
            }
        }
    }

    // Clean up client state
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
    SOCKET server_sock = server_socket_init();
    InitializeCriticalSection(&clients_lock);

    if (server_sock == INVALID_SOCKET) {
        printf("Failed to initialize server.\n");
        DeleteCriticalSection(&clients_lock);
        return 1;
    }

    for (int i = 0; i < MAX_CLIENTS; i++) {
        clients[i].sock = INVALID_SOCKET;
        clients[i].current_room = -1;
        clients[i].on_main_screen = 0;
    }

    while (1) {
        SOCKET client_sock = accept_client(server_sock);

        if (client_sock != INVALID_SOCKET) {
            // Create a thread for each new client
            SOCKET *client_sock_ptr = malloc(sizeof(SOCKET));
            *client_sock_ptr = client_sock;
            CreateThread(NULL, 0, ClientHandler, (LPVOID)client_sock_ptr, 0, NULL);
        }
    }

    closesocket(server_sock); // Close server socket when the server shuts down
    DeleteCriticalSection(&clients_lock);
    WSACleanup();
    return 0;
}
