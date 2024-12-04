#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#pragma comment(lib, "ws2_32.lib")

#define MAX_PLAYERS 4
#define MAX_ROOMS 10
#define BUFFER_SIZE 1024
#define INITIAL_MONEY 3000000

// 땅 정보 구조체 수정
typedef struct {
    char name[50];
    int price[5];    // 땅값, 집1, 집2, 호텔, 빌딩 순
    int toll[5];     // 통행료 정보 추가
} Deed;

const Deed Deeds[32] = {
    {"출발", {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
    {"타이베이", {50000, 50000, 100000, 150000, 250000}, {2000, 10000, 30000, 90000, 250000}},
    {"베이징", {80000, 50000, 100000, 150000, 250000}, {4000, 20000, 60000, 180000, 450000}},
    {"마닐라", {80000, 50000, 100000, 150000, 250000}, {4000, 20000, 60000, 180000, 450000}},
    {"제주도", {200000, 0, 0, 0, 0}, {300000, 0, 0, 0, 0}},
    {"싱가포르", {100000, 50000, 100000, 150000, 250000}, {6000, 30000, 90000, 270000, 550000}},
    {"카이로", {100000, 50000, 100000, 150000, 250000}, {6000, 30000, 90000, 270000, 550000}},
    {"이스탄불", {120000, 50000, 100000, 150000, 250000}, {8000, 40000, 100000, 300000, 600000}},
    {"아테네", {140000, 100000, 200000, 300000, 500000}, {10000, 50000, 150000, 450000, 750000}},
    {"코펜하겐", {160000, 100000, 200000, 300000, 500000}, {12000, 60000, 180000, 500000, 900000}},
    {"스톡홀름", {160000, 100000, 200000, 300000, 500000}, {12000, 60000, 180000, 500000, 900000}},
    {"콩코드", {200000, 0, 0, 0, 0}, {300000, 0, 0, 0, 0}},
    {"베른", {180000, 100000, 200000, 300000, 500000}, {14000, 70000, 200000, 550000, 950000}},
    {"베를린", {180000, 100000, 200000, 300000, 500000}, {14000, 70000, 200000, 550000, 950000}},
    {"오타와", {200000, 100000, 200000, 300000, 500000}, {16000, 80000, 220000, 600000, 1000000}},
    {"부에노스", {220000, 150000, 300000, 450000, 750000}, {18000, 90000, 250000, 700000, 1050000}},
    {"상파울루", {240000, 150000, 300000, 450000, 750000}, {20000, 100000, 300000, 750000, 1100000}},
    {"시드니", {240000, 150000, 300000, 450000, 750000}, {20000, 100000, 300000, 750000, 1100000}},
    {"부산", {500000, 0, 0, 0, 0}, {600000, 0, 0, 0, 0}},
    {"하와이", {260000, 150000, 300000, 450000, 750000}, {22000, 110000, 330000, 800000, 1150000}},
    {"리스본", {260000, 150000, 300000, 450000, 750000}, {22000, 110000, 330000, 800000, 1150000}},
    {"퀸엘리자베스", {300000, 0, 0, 0, 0}, {250000, 0, 0, 0, 0}},
    {"마드리드", {280000, 150000, 300000, 450000, 750000}, {24000, 120000, 360000, 850000, 1200000}},
    {"도쿄", {300000, 200000, 400000, 600000, 1000000}, {26000, 130000, 390000, 900000, 1270000}},
    {"컬럼비아", {450000, 0, 0, 0, 0}, {300000, 0, 0, 0, 0}},
    {"파리", {320000, 200000, 400000, 600000, 1000000}, {28000, 150000, 450000, 1000000, 1400000}},
    {"로마", {320000, 200000, 400000, 600000, 1000000}, {28000, 150000, 450000, 1000000, 1400000}},
    {"런던", {350000, 200000, 400000, 600000, 1000000}, {35000, 170000, 500000, 1100000, 1500000}},
    {"뉴욕", {350000, 200000, 400000, 600000, 1000000}, {35000, 170000, 500000, 1100000, 1500000}},
    {"서울", {1000000, 0, 0, 0, 0}, {2000000, 0, 0, 0, 0}},
    {"사회복지기금", {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}},
    {"기금납부", {0, 0, 0, 0, 0}, {0, 0, 0, 0, 0}}
};

// 게임룸 구조체
typedef struct {
    int position;
    int money;
    int properties[32];  // 소유한 땅 정보
    int buildingLevel[32];  // 각 땅의 건물 레벨
    SOCKET socket;
    int isActive;
    int playerNum;     // 플레이어 번호 (0-3)
} Player;

typedef struct {
    char name[50];
    Player players[MAX_PLAYERS];
    int numPlayers;
    int currentTurn;
    int isGameStarted;
    int properties[32];  // 각 땅의 소유자 정보 (-1: 없음, 0~3: 플레이어 번호)
} GameRoom;

// 전역 변수
GameRoom rooms[MAX_ROOMS];
int numRooms = 0;
CRITICAL_SECTION roomLock;

// 함수 선언
void broadcastToRoom(GameRoom* room, const char* message, SOCKET excludeSocket);
GameRoom* find_room(const char* roomName);
void start_game(GameRoom* room);
void handle_dice_roll(GameRoom* room, int playerIndex, int dice1, int dice2);
void handle_property_purchase(GameRoom* room, int playerIndex);
void handle_build(GameRoom* room, int playerIndex, int position);

// 방 찾기 함수
GameRoom* find_room(const char* roomName) {
    for (int i = 0; i < numRooms; i++) {
        if (strcmp(rooms[i].name, roomName) == 0) {
            return &rooms[i];
        }
    }
    return NULL;
}

// 게임 시작 함수
void start_game(GameRoom* room) {
    room->isGameStarted = 1;
    room->currentTurn = 0;  // 첫 번째 플레이어(0)부터 시작

    char buffer[BUFFER_SIZE];
    // 게임 시작 알림
    for (int i = 0; i < room->numPlayers; i++) {
        sprintf(buffer, "GAME_START:0");  // 모든 플레이어에게 동일한 시작 메시지
        send(room->players[i].socket, buffer, strlen(buffer), 0);
    }

    Sleep(500);  // 잠시 대기

    // 첫 턴 알림
    sprintf(buffer, "TURN:0");  // 첫 턴은 항상 0번 플레이어
    broadcastToRoom(room, buffer, INVALID_SOCKET);
}

// 브로드캐스트 함수
void broadcastToRoom(GameRoom* room, const char* message, SOCKET excludeSocket) {
    for (int i = 0; i < room->numPlayers; i++) {
        if (room->players[i].isActive && room->players[i].socket != excludeSocket) {
            send(room->players[i].socket, message, strlen(message), 0);
        }
    }
}

// 주사위 굴리기 처리
void handle_dice_roll(GameRoom* room, int playerIndex, int dice1, int dice2) {
    if (room->currentTurn != playerIndex) {
        return;  // 자신의 턴이 아니면 무시
    }

    Player* player = &room->players[playerIndex];
    char msg[BUFFER_SIZE];

    // 주사위 결과 브로드캐스트
    sprintf(msg, "DICE:%d,%d", dice1, dice2);
    broadcastToRoom(room, msg, INVALID_SOCKET);

    Sleep(1000);  // 주사위 결과를 보여줄 시간

    // 말 이동
    int oldPos = player->position;
    player->position = (oldPos + dice1 + dice2) % 40;

    // 이동 결과 브로드캐스트
    sprintf(msg, "MOVE:%d,%d", playerIndex, player->position);
    broadcastToRoom(room, msg, INVALID_SOCKET);

    Sleep(1000);  // 이동을 보여줄 시간

    // 출발지 통과 시 월급 지급
    if (oldPos > player->position) {
        player->money += 200000;
        sprintf(msg, "SALARY:%d,%d", playerIndex, player->money);
        broadcastToRoom(room, msg, INVALID_SOCKET);
    }

    // 땅 구매 가능 여부 확인 및 알림
    if (player->position < 32 && room->properties[player->position] == 0) {
        sprintf(msg, "CAN_BUY:%d", player->position);
        send(player->socket, msg, strlen(msg), 0);
    }

    // 더블이 아닐 경우에만 턴 변경
    if (dice1 != dice2) {
        room->currentTurn = (room->currentTurn + 1) % room->numPlayers;
        sprintf(msg, "TURN:%d", room->currentTurn);
        broadcastToRoom(room, msg, INVALID_SOCKET);
    }
}

// 건물 건설 처리 함수 추가
void handle_build(GameRoom* room, int playerIndex, int position) {
    Player* player = &room->players[playerIndex];
    char msg[BUFFER_SIZE];
    int buildCost;

    // 건물 건설 가능 여부 확인
    if (position < 0 || position >= 32 ||
        player->properties[position] != 1 ||
        player->money < Deeds[position].price[player->buildingLevel[position] + 1]) {
        sprintf(msg, "BUILD_FAILED:%d", position);
        send(player->socket, msg, strlen(msg), 0);
        return;
    }

    buildCost = Deeds[position].price[player->buildingLevel[position] + 1];
    player->money -= buildCost;
    player->buildingLevel[position]++;

    sprintf(msg, "BUILD_SUCCESS:%d,%d,%d", position, player->buildingLevel[position], player->money);
    broadcastToRoom(room, msg, INVALID_SOCKET);
}

// 클라이언트 처리 스레드
DWORD WINAPI client_handler(LPVOID clientSocket) {
    SOCKET sock = *(SOCKET*)clientSocket;
    free(clientSocket);
    char buffer[BUFFER_SIZE];
    int roomIndex = -1;
    int playerIndex = -1;

    while (1) {
        int bytesReceived = recv(sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            break;
        }
        buffer[bytesReceived] = '\0';

        EnterCriticalSection(&roomLock);

        // 방 생성 처리
        if (strncmp(buffer, "CREATE ", 7) == 0) {
            if (numRooms < MAX_ROOMS) {
                char* roomName = buffer + 7;
                if (find_room(roomName) == NULL) {
                    strcpy(rooms[numRooms].name, roomName);
                    rooms[numRooms].numPlayers = 1;
                    rooms[numRooms].isGameStarted = 0;
                    rooms[numRooms].currentTurn = 0;

                    // 방장은 항상 플레이어 0
                    Player* player = &rooms[numRooms].players[0];
                    player->socket = sock;
                    player->isActive = 1;
                    player->money = INITIAL_MONEY;
                    player->position = 0;
                    player->playerNum = 0;

                    roomIndex = numRooms;
                    playerIndex = 0;
                    numRooms++;

                    // 방장에게 플레이어 번호 전송
                    char msg[BUFFER_SIZE];
                    sprintf(msg, "PLAYER_NUM:%d", 0);
                    send(sock, msg, strlen(msg), 0);

                    send(sock, "CREATED:방이 생성되었습니다.\n", 100, 0);
                }
            }
        }
        // 방 참가 처리
        else if (strncmp(buffer, "JOIN ", 5) == 0) {
            char* roomName = buffer + 5;
            GameRoom* room = find_room(roomName);
            if (room && !room->isGameStarted && room->numPlayers < MAX_PLAYERS) {
                playerIndex = room->numPlayers;
                Player* player = &room->players[playerIndex];
                player->socket = sock;
                player->isActive = 1;
                player->money = INITIAL_MONEY;
                player->position = 0;
                player->playerNum = playerIndex;
                room->numPlayers++;

                // 참가자에게 플레이어 번호 전송
                char msg[BUFFER_SIZE];
                sprintf(msg, "PLAYER_NUM:%d", playerIndex);
                send(sock, msg, strlen(msg), 0);

                // 모든 플레이어에게 참가 알림
                sprintf(msg, "JOIN:플레이어 %d가 참가했습니다. (%d/%d)\n",
                        playerIndex + 1, room->numPlayers, MAX_PLAYERS);
                broadcastToRoom(room, msg, INVALID_SOCKET);

                if (room->numPlayers >= 2) {
                    Sleep(1000);  // 잠시 대기
                    start_game(room);
                }
            } else {
                // 방 입장 실패 메시지 전송
                send(sock, "JOIN_FAILED:방을 찾을 수 없거나 입장할 수 없습니다.\n", 100, 0);
            }
        }
        // 게임 진행 중 명령 처리
        else if (roomIndex != -1 && rooms[roomIndex].isGameStarted) {
            GameRoom* room = &rooms[roomIndex];

            if (strncmp(buffer, "ROLL:", 5) == 0) {
                if (room->currentTurn == playerIndex) {
                    int dice1, dice2;
                    sscanf(buffer + 5, "%d,%d", &dice1, &dice2);

                    // 주사위 결과 브로드캐스트
                    char msg[BUFFER_SIZE];
                    sprintf(msg, "DICE:%d,%d", dice1, dice2);
                    broadcastToRoom(room, msg, INVALID_SOCKET);

                    // 말 이동
                    int newPos = (room->players[playerIndex].position + dice1 + dice2) % 40;
                    room->players[playerIndex].position = newPos;
                    sprintf(msg, "MOVE:%d,%d", playerIndex, newPos);
                    broadcastToRoom(room, msg, INVALID_SOCKET);

                    // 더블이 아니면 턴 변경
                    if (dice1 != dice2) {
                        room->currentTurn = (room->currentTurn + 1) % room->numPlayers;
                        sprintf(msg, "TURN:%d", room->currentTurn);
                        broadcastToRoom(room, msg, INVALID_SOCKET);
                    }
                }
            }
            else if (strncmp(buffer, "BUILD:", 6) == 0) {
                int position;
                sscanf(buffer + 6, "%d", &position);
                handle_build(room, playerIndex, position);
            }
        }

        LeaveCriticalSection(&roomLock);
    }

    // 연결 종료 처리
    if (roomIndex != -1 && playerIndex != -1) {
        EnterCriticalSection(&roomLock);
        GameRoom* room = &rooms[roomIndex];
        room->players[playerIndex].isActive = 0;
        char msg[BUFFER_SIZE];
        sprintf(msg, "QUIT:플레이어 %d가 나갔습니다.\n", playerIndex + 1);
        broadcastToRoom(room, msg, sock);
        LeaveCriticalSection(&roomLock);
    }

    closesocket(sock);
    return 0;
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 실패\n");
        return 1;
    }

    InitializeCriticalSection(&roomLock);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
        printf("소켓 ���성 실패\n");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("바인드 실패\n");
        closesocket(serverSocket);
        return 1;
    }

    if (listen(serverSocket, 5) == SOCKET_ERROR) {
        printf("리슨 실패\n");
        closesocket(serverSocket);
        return 1;
    }

    printf("서버가 시작되었습니다. (포트: 12345)\n");

    while (1) {
        SOCKET* clientSocket = malloc(sizeof(SOCKET));
        *clientSocket = accept(serverSocket, NULL, NULL);

        if (*clientSocket != INVALID_SOCKET) {
            CreateThread(NULL, 0, client_handler, clientSocket, 0, NULL);
        }
    }

    DeleteCriticalSection(&roomLock);
    closesocket(serverSocket);
    WSACleanup();
    return 0;
}