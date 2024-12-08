#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <windows.h>
#include <time.h>

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#pragma comment(lib, "ws2_32.lib")

#define MAX_PLAYERS 2
#define MAX_ROOMS 10
#define BUFFER_SIZE 1024
#define INITIAL_MONEY 3000000
#define INDIA_POSITION 15
#define MAX_PROPERTIES 40  // Match the Deeds array size

typedef struct {
    char name[50];
    int price;
    int baseRent;
} Deed;

// Replace the Deeds array definition with corrected property sequence
const Deed Deeds[40] = {
    {"출발", 0, 0},
    {"타이베이", 50000, 5000},
    {"", 0, 0},  // 빈 칸
    {"베이징", 80000, 8000},
    {"마닐라", 80000, 8000},
    {"제주도", 200000, 20000},
    {"싱가포르", 100000, 10000},
    {"", 0, 0},  // 빈 칸
    {"카이로", 100000, 10000},
    {"이스탄불", 120000, 12000},
    {"인도", 0, 0},  // 특수 칸
    {"아테네", 140000, 14000},
    {"", 0, 0},  // 빈 칸
    {"코펜하겐", 160000, 16000},
    {"스톡홀름", 160000, 16000},
    {"베른", 180000, 18000},
    {"", 0, 0},  // 빈 칸
    {"베를린", 180000, 18000},
    {"오타와", 200000, 20000},
    {"", 0, 0},  // 빈 칸
    {"부에노스", 220000, 22000},
    {"", 0, 0},  // 빈 칸
    {"상파울루", 240000, 24000},
    {"시드니", 240000, 24000},
    {"부산", 500000, 50000},
    {"하와이", 260000, 26000},
    {"리스본", 260000, 26000},
    {"퀸엘리자베스", 300000, 30000},
    {"도쿄", 350000, 35000},
    {"마드리드", 350000, 35000},
    {"", 0, 0},   // 빈 칸
    {"도쿄", 300000, 30000},
    {"컬럼비아", 450000, 45000},
    {"파리", 320000, 32000},
    {"로마", 320000, 32000},
    {"", 0, 0},   // 빈 칸
    {"런던", 350000, 35000},
    {"뉴욕", 350000, 35000},
    {"", 0, 0},   // 빈 칸
    {"서울", 1000000, 100000},
};

typedef struct {
    int position;
    int money;
    int laps;
    SOCKET socket;
    int isActive;
    int playerNum;
} Player;

typedef struct {
    char name[50];
    Player players[2];
    int numPlayers;
    int currentTurn;
    int isGameStarted;
    int properties[32];  // Stores owner of each property (-1 for unowned)
    int roundCount;      // Add round counter
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
void handle_property_purchase(GameRoom* room, int playerNum, int position);
void give_salary(GameRoom* room, int playerNum);
void handle_turn(GameRoom* room);
void handle_bankruptcy(GameRoom* room, int playerNum, int creditorNum);
void handle_toll(GameRoom* room, int position);
void init_room(GameRoom* room);

// 방 초기화 함수 구현 추가 (함수 선언 아래에 위치)
void init_room(GameRoom* room) {
    memset(room, 0, sizeof(GameRoom));  // 전체 구조체를 0으로 초기화

    // 모든 땅의 소유자를 -1로 초기화
    for (int i = 0; i < 32; i++) {
        room->properties[i] = -1;
    }

    // 플레이어 초기화
    for (int i = 0; i < 2; i++) {
        Player* player = &room->players[i];
        player->isActive = 0;
        player->money = INITIAL_MONEY;
        player->position = 0;
        player->playerNum = i;
        player->laps = 0;
    }

    room->numPlayers = 0;
    room->currentTurn = 0;
    room->isGameStarted = 0;
    room->roundCount = 1;
}

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
// Add new function to check and collect tolls for passed properties
void check_passed_properties(GameRoom* room, int playerIndex, int oldPos, int newPos) {
    Player* player = &room->players[playerIndex];
    int start = oldPos;
    int end = newPos;

    // If passed through starting point, handle wrap-around
    if (newPos < oldPos) {
        // Check properties from oldPos to 39
        for (int i = start + 1; i < 40; i++) {
            if (room->properties[i] != -1 && room->properties[i] != playerIndex) {
                handle_toll(room, i);
            }
        }
        // Then check from 0 to newPos
        start = 0;
        end = newPos;
    }

    // Check all properties between start and end
    for (int i = start + 1; i < end; i++) {
        if (room->properties[i] != -1 && room->properties[i] != playerIndex) {
            handle_toll(room, i);
        }
    }
}

// Modify handle_dice_roll to include toll check for passed properties
void handle_dice_roll(GameRoom* room, int playerIndex, int dice1, int dice2) {
    printf("DEBUG: handle_dice_roll called for player %d\n", playerIndex);

    if (room->currentTurn != playerIndex) {
        printf("DEBUG: Not current player's turn (%d tried to roll but it's %d's turn)\n",
            playerIndex, room->currentTurn);
        return;
    }

    Player* player = &room->players[playerIndex];
    Player* otherPlayer = &room->players[1 - playerIndex];
    char msg[BUFFER_SIZE];

    // Broadcast dice roll
    sprintf(msg, "DICE:%d,%d", dice1, dice2);
    printf("DEBUG: Broadcasting dice roll: %s\n", msg);
    broadcastToRoom(room, msg, INVALID_SOCKET);
    Sleep(500);

    // Move player
    int oldPos = player->position;
    player->position = (oldPos + dice1 + dice2) % 40;  // Changed to 40 to match board size
    printf("DEBUG: Player %d moved to position %d\n", playerIndex, player->position);

    // Check tolls for passed properties
    check_passed_properties(room, playerIndex, oldPos, player->position);

    // Force broadcast and processing of movement
    sprintf(msg, "DICE:%d,%d", dice1, dice2);
    broadcastToRoom(room, msg, INVALID_SOCKET);
    Sleep(500);  // Give clients time to process dice roll

    sprintf(msg, "MOVE:%d,%d", playerIndex, player->position);
    broadcastToRoom(room, msg, INVALID_SOCKET);
    Sleep(500);  // Give clients time to process movement

    // Broadcast movement
    sprintf(msg, "MOVE:%d,%d", playerIndex, player->position);
    printf("DEBUG: Broadcasting movement: %s\n", msg);
    broadcastToRoom(room, msg, INVALID_SOCKET);
    Sleep(500);

    // Check for lap completion
    if (player->position < oldPos) {
        player->laps++;
        player->money += 200000;
        sprintf(msg, "SALARY:%d,%d", playerIndex, player->money);
        broadcastToRoom(room, msg, INVALID_SOCKET);

        // Check for new round
        if (player->laps == otherPlayer->laps) {
            room->roundCount++;
            sprintf(msg, "ROUND:%d", room->roundCount);
            broadcastToRoom(room, msg, INVALID_SOCKET);
        }
    }

    // Check for forced movement to India
    if (player->laps > otherPlayer->laps + 1) {
        player->position = INDIA_POSITION;
        player->laps = otherPlayer->laps + 1;
        sprintf(msg, "FORCE_MOVE:%d,%d", playerIndex, INDIA_POSITION);
        broadcastToRoom(room, msg, INVALID_SOCKET);
    }

    // If landed on special square or empty square, just pass turn
    if (Deeds[player->position].price == 0) {
        printf("DEBUG: Not a purchasable property, changing turn\n");
        handle_turn(room);
        return;
    }

    // After movement, check if landed on another player's property
    int position = player->position;
    int propertyOwner = room->properties[position];

    if (propertyOwner != -1 && propertyOwner != playerIndex) {
        // Calculate and collect toll
        int rent = Deeds[position].baseRent * room->roundCount;
        handle_toll(room, position);
    }

    // Handle property actions after movement
    if (room->properties[player->position] == -1 &&
        player->position > 0 &&
        player->position < 32 &&
        Deeds[player->position].price > 0) {

        // Include property name and position in CAN_BUY message
        printf("DEBUG: Sending CAN_BUY message\n");
        char msg[BUFFER_SIZE];
        sprintf(msg, "CAN_BUY:%d,%d,%s",
            player->position,
            Deeds[player->position].price,
            Deeds[player->position].name);
        send(player->socket, msg, strlen(msg), 0);
    } else if (room->properties[player->position] != playerIndex) {
        printf("DEBUG: Property owned, handling toll and changing turn\n");
        handle_toll(room, player->position);
        handle_turn(room);
    } else {
        handle_turn(room);
    }
}

// 땅 구매 처리 함수
void handle_property_purchase(GameRoom* room, int playerNum, int position) {
    printf("DEBUG: Processing purchase for player %d at position %d\n", playerNum, position);

    if (position < 0 || position >= MAX_PROPERTIES) {
        printf("DEBUG: Invalid position %d\n", position);
        return;
    }

    Player* player = &room->players[playerNum];
    const Deed* property = &Deeds[position];
    char msg[BUFFER_SIZE];

    // Verify player position matches purchase position
    if (player->position != position) {
        printf("DEBUG: Position mismatch - player at %d, trying to buy %d\n", player->position, position);
        sprintf(msg, "ERROR:잘못된 위치입니다");
        send(player->socket, msg, strlen(msg), 0);
        return;
    }

    if (room->properties[position] != -1) {
        printf("DEBUG: Property already owned\n");
        return;
    }

    int cost = property->price;
    if (player->money >= cost) {
        player->money -= cost;
        room->properties[position] = playerNum;

        // Send detailed purchase success message
        sprintf(msg, "PURCHASE_SUCCESS:%d,%d,%d",
            position, playerNum, player->money);
        printf("DEBUG: Purchase success - broadcasting message: %s\n", msg);
        broadcastToRoom(room, msg, INVALID_SOCKET);

        Sleep(500);  // Give time for UI update
    } else {
        sprintf(msg, "ERROR:돈이 부족합니다");
        send(player->socket, msg, strlen(msg), 0);
    }

    // Always change turn after purchase attempt
    printf("DEBUG: Changing turn after purchase\n");
    Sleep(500);  // Give time for purchase message processing
    handle_turn(room);
}

// 월급 지급 함수 추가
void give_salary(GameRoom* room, int playerNum) {
    Player* player = &room->players[playerNum];
    player->money += 200000; // 20만원 월급

    char msg[BUFFER_SIZE];
    sprintf(msg, "SALARY:%d,%d", playerNum, player->money);
    broadcastToRoom(room, msg, INVALID_SOCKET);
}

// 턴 처리 함수 수정
void handle_turn(GameRoom* room) {
    printf("DEBUG: handle_turn called - current turn is %d\n", room->currentTurn);

    Player* current = &room->players[room->currentTurn];
    Player* other = &room->players[1 - room->currentTurn];

    // Check if round is complete
    if (current->laps == other->laps && current->position < other->position) {
        room->roundCount++;
        char msg[BUFFER_SIZE];
        sprintf(msg, "ROUND:%d", room->roundCount);
        broadcastToRoom(room, msg, INVALID_SOCKET);
    }

    // Switch turn
    room->currentTurn = 1 - room->currentTurn;
    printf("DEBUG: Turn changed to player %d\n", room->currentTurn);

    Sleep(500);

    // Send turn message with current player position
    char msg[BUFFER_SIZE];
    sprintf(msg, "TURN:%d,%d", room->currentTurn,
        room->players[room->currentTurn].position);
    printf("DEBUG: Broadcasting turn message: %s\n", msg);
    broadcastToRoom(room, msg, INVALID_SOCKET);
}

// handle_toll 함수 구현 추가 (파산 처리 함수 앞에 위치)
// Ensure handle_toll sends proper messages
void handle_toll(GameRoom* room, int position) {
    Player* current = &room->players[room->currentTurn];
    int owner = room->properties[position];

    if (owner != -1 && owner != room->currentTurn) {
        const Deed* property = &Deeds[position];
        int rent = property->baseRent * room->roundCount;

        printf("DEBUG: Handling toll payment - Player %d to Player %d, Amount: %d\n",
            room->currentTurn, owner, rent);

        if (current->money >= rent) {
            current->money -= rent;
            room->players[owner].money += rent;

            char msg[BUFFER_SIZE];
            sprintf(msg, "TOLL:%d,%d,%d", room->currentTurn, owner, rent);
            broadcastToRoom(room, msg, INVALID_SOCKET);
            Sleep(1000);  // Give time for toll message display
        } else {
            handle_bankruptcy(room, room->currentTurn, owner);
        }
    }
}

// handle_bankruptcy 함수 구현 추가
void handle_bankruptcy(GameRoom* room, int playerNum, int creditorNum) {
    Player* bankrupt = &room->players[playerNum];

    // 플레이어를 파산 상태로 변경
    bankrupt->isActive = 0;
    bankrupt->money = 0;

    // 모든 소유 부동산을 처리
    for (int i = 0; i < 32; i++) {
        if (room->properties[i] == playerNum) {
            if (creditorNum == -1) {
                // 은행에 의한 파산: 부동산은 다시 구매 가능한 상태로
                room->properties[i] = -1;
            } else {
                // 다른 플레이어에 의한 파산: 부동산은 채권자에게 이전
                room->properties[i] = creditorNum;
            }
        }
    }

    // 파산 알림 전송
    char msg[BUFFER_SIZE];
    sprintf(msg, "BANKRUPTCY:%d,%d", playerNum, creditorNum);
    broadcastToRoom(room, msg, INVALID_SOCKET);

    // 게임 종료 조건 체크 (활성 플레이어가 1명 이하면 게임 종료)
    int activePlayers = 0;
    int winner = -1;
    for (int i = 0; i < room->numPlayers; i++) {
        if (room->players[i].isActive) {
            activePlayers++;
            winner = i;
        }
    }

    if (activePlayers <= 1 && winner != -1) {
        sprintf(msg, "GAME_OVER:%d", winner);
        broadcastToRoom(room, msg, INVALID_SOCKET);
    }
}

// 클라이언트 처리 스레드
// Add roomIndex tracking in JOIN handler and fix client_handler function
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

        // Add debug message for received commands
        printf("Received command from client: %s\n", buffer);

        EnterCriticalSection(&roomLock);

        // 방 생성 처리
        if (strncmp(buffer, "CREATE ", 7) == 0) {
            if (numRooms < MAX_ROOMS) {
                char* roomName = buffer + 7;
                if (find_room(roomName) == NULL) {
                    init_room(&rooms[numRooms]);  // 방 초기화
                    strncpy(rooms[numRooms].name, roomName, sizeof(rooms[numRooms].name) - 1);
                    rooms[numRooms].name[sizeof(rooms[numRooms].name) - 1] = '\0';

                    // 방장 초기화
                    Player* player = &rooms[numRooms].players[0];
                    memset(player, 0, sizeof(Player));  // 플레이어 구조체 초기화
                    player->socket = sock;
                    player->isActive = 1;
                    player->money = INITIAL_MONEY;
                    player->position = 0;
                    player->playerNum = 0;

                    roomIndex = numRooms;
                    playerIndex = 0;
                    rooms[numRooms].numPlayers = 1;
                    numRooms++;

                    char msg[BUFFER_SIZE];
                    sprintf(msg, "PLAYER_NUM:%d", 0);
                    send(sock, msg, strlen(msg), 0);
                    send(sock, "CREATED:방이 생성되었습니다.\n", strlen("CREATED:방이 생성되었습니다.\n"), 0);
                }
            }
        }
        // 방 참가 처리
        else if (strncmp(buffer, "JOIN ", 5) == 0) {
            char* roomName = buffer + 5;
            GameRoom* room = find_room(roomName);
            if (room && !room->isGameStarted && room->numPlayers < MAX_PLAYERS) {
                roomIndex = room - rooms;  // Calculate room index for joining player
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
            printf("DEBUG: Processing command in room %d for player %d (current turn: %d)\n",
                roomIndex, playerIndex, room->currentTurn);

            if (strncmp(buffer, "ROLL:", 5) == 0) {
                // Add debug message for dice roll
                printf("Processing dice roll command for player %d (current turn: %d)\n",
                    playerIndex, room->currentTurn);

                if (room->currentTurn == playerIndex) {
                    int dice1, dice2;
                    if (sscanf(buffer + 5, "%d,%d", &dice1, &dice2) == 2) {
                        printf("Dice values: %d, %d - processing roll\n", dice1, dice2);
                        handle_dice_roll(room, playerIndex, dice1, dice2);
                    }
                } else {
                    printf("DEBUG: Ignoring dice roll - not player's turn (player: %d, current turn: %d)\n",
                        playerIndex, room->currentTurn);
                }
            }
            else if (strncmp(buffer, "BUY:", 4) == 0) {
                int position;
                sscanf(buffer + 4, "%d", &position);
                handle_property_purchase(room, playerIndex, position);
            }
            else if (strncmp(buffer, "SKIP_PURCHASE", 12) == 0) {
                handle_turn(room);  // 구매를 건너뛰고 턴 진행
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
        printf("소켓 생성 실패\n");
        return 1;
    }

    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(12345);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR) {
        printf("바인딩 실패\n");
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