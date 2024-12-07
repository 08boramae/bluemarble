#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#define _CRT_SECURE_NO_WARNINGS
#pragma warning(disable:4996)

#define RED 12
#define BLUE 9
#define GREEN 10
#define YELLOW 14
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

// 게임 상태 관련 상수
#define STATE_LOBBY 0
#define STATE_IN_GAME 1

// 게임 상수 추가
#define INDIA_POSITION 15  // 인도 위치
#define SALARY_AMOUNT 200000  // 월급

// Define complete Deed structure at the beginning
typedef struct {
    char name[50];
    int price;
    int baseRent;
    int ownerNum;
    int buildingLevel;
    int isCanBuild;
} Deed;

// Forward declare struct player
struct player;

// Define complete player structure
struct player {
    int color;
    int position;
    int money;
    int laps;
    char name[20];
    int isActive;
};

// Define GameRoom structure
typedef struct {
    char name[50];
    struct player players[2];
    int numPlayers;
    int currentTurn;
    int isGameStarted;
    int properties[32];
    int roundCount;
} GameRoom;

// Global variables
Deed Deeds[32];  // 전역 Deeds 배열 추가
struct player Players[2];  // Changed to 2 players only
SOCKET clientSocket;
CRITICAL_SECTION printLock;
int gameState = STATE_LOBBY;
int myPlayerNum = -1;
int playerCount = 0;
int currentTurn = -1;
int roundCount = 1;     // Track current round
int gamePhase = 0;  // 추가: 게임 진행 단계
int waitingForPurchase = 0;
int canBuyPosition = -1;
int lastPurchasePosition = -1;  // Track last purchased position
int currentPosition = -1;  // Track current player's position
int lastDiceRoll = 0;  // Track when dice was last rolled
int diceRollInProgress = 0;
char currentPropertyName[50] = {0};
int currentPlayerPos = -1;  // Track current player's actual position
int turnChangeInProgress = 0;  // Track turn change state

// 함수 선언
void textColor(int colorNum);
void gotoxy(int x, int y);
void draw_board();
void init_deeds();
void init_players(int player_cnt);
int roll_dice();
void handle_network_message(char* message);
void send_network_message(const char* message);
void draw_player_markers();
DWORD WINAPI receive_thread(LPVOID arg);
void printInCell(int x, int y, char* text, int price);
void game_loop();
void handle_toll(GameRoom* room, int position);
void handle_bankruptcy(GameRoom* room, int playerNum, int creditorNum);
void handle_build(GameRoom* room, int playerNum, int position);

// 텍스트 색상 변경 함수
void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

// 커서 위치 이동 함수
void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// deed 초기화 함수
// Deed 초기화 함수
void init_deeds() {
    char names[29][40] = {
        "타이베이", "베이징", "마닐라", "제주도", "싱가포르", "카이로", "이스탄불",
        "아테네", "코펜하겐", "스톡홀름", "콩코드", "베른", "베를린", "오타와",
        "부에노스", "상파울루", "시드니", "부산", "하와이", "리스본", "퀸엘리자베스", "마드리드",
        "도쿄", "컬럼비아", "파리", "로마", "런던", "뉴욕", "서울"
    };

    // 가격 정보 (대지, 별장1, 별장2, 빌딩, 호텔)
    int prices[29][5] = {
        {50000, 50000, 100000, 150000, 250000},
        {80000, 50000, 100000, 150000, 250000},
        {80000, 50000, 100000, 150000, 250000},
        {200000, 0, 0, 0, 0},
        {100000, 50000, 100000, 150000, 250000},
        {100000, 50000, 100000, 150000, 250000},
        {120000, 50000, 100000, 150000, 250000},
        {140000, 100000, 200000, 300000, 500000},
        {160000, 100000, 200000, 300000, 500000},
        {160000, 100000, 200000, 300000, 500000},
        {200000, 0, 0, 0, 0},
        {180000, 100000, 200000, 300000, 500000},
        {180000, 100000, 200000, 300000, 500000},
        {200000, 100000, 200000, 300000, 500000},
        {220000, 150000, 300000, 450000, 750000},
        {240000, 150000, 300000, 450000, 750000},
        {240000, 150000, 300000, 450000, 750000},
        {500000, 0, 0, 0, 0},
        {260000, 150000, 300000, 450000, 750000},
        {260000, 150000, 300000, 450000, 750000},
        {300000, 0, 0, 0, 0},
        {280000, 150000, 300000, 450000, 750000},
        {300000, 200000, 400000, 600000, 1000000},
        {450000, 0, 0, 0, 0},
        {320000, 200000, 400000, 600000, 1000000},
        {320000, 200000, 400000, 600000, 1000000},
        {350000, 200000, 400000, 600000, 1000000},
        {350000, 200000, 400000, 600000, 1000000},
        {1000000, 0, 0, 0, 0}
    };

    // 통행료 정보
    int tolls[29][5] = {
        {2000, 10000, 30000, 90000, 250000},
        {4000, 20000, 60000, 180000, 450000},
        {4000, 20000, 60000, 180000, 450000},
        {300000, 0, 0, 0, 0},
        {6000, 30000, 90000, 270000, 550000},
        {6000, 30000, 90000, 270000, 550000},
        {8000, 40000, 100000, 300000, 600000},
        {10000, 50000, 150000, 450000, 750000},
        {12000, 60000, 180000, 500000, 900000},
        {12000, 60000, 180000, 500000, 900000},
        {300000, 0, 0, 0, 0},
        {14000, 70000, 200000, 550000, 950000},
        {14000, 70000, 200000, 550000, 950000},
        {16000, 80000, 220000, 600000, 1000000},
        {18000, 90000, 250000, 700000, 1050000},
        {20000, 100000, 300000, 750000, 1100000},
        {20000, 100000, 300000, 750000, 1100000},
        {600000, 0, 0, 0, 0},
        {22000, 110000, 330000, 800000, 1150000},
        {22000, 110000, 330000, 800000, 1150000},
        {250000, 0, 0, 0, 0},
        {24000, 120000, 360000, 850000, 1200000},
        {26000, 130000, 390000, 900000, 1270000},
        {300000, 0, 0, 0, 0},
        {28000, 150000, 450000, 1000000, 1400000},
        {28000, 150000, 450000, 1000000, 1400000},
        {35000, 170000, 500000, 1100000, 1500000},
        {35000, 170000, 500000, 1100000, 1500000},
        {2000000, 0, 0, 0, 0}
    };

    for (int i = 0; i < 29; i++) {
        strncpy(Deeds[i].name, names[i], sizeof(Deeds[i].name));
        Deeds[i].ownerNum = -1;
        Deeds[i].buildingLevel = 0;
        Deeds[i].price = prices[i][0];  // Base price
        Deeds[i].baseRent = tolls[i][0]; // Base rent
        Deeds[i].isCanBuild = (prices[i][1] > 0); // 별장1 가격이 0보다 크면 건설 가능
    }
}

// 플레이어 초기화 함수
void init_players(int player_cnt) {
    int colors[4] = {RED, BLUE, GREEN, YELLOW};
    playerCount = player_cnt;
    int initialMoney = (player_cnt <= 2) ? 6000000 : 3000000;

    for (int i = 0; i < player_cnt; i++) {
        Players[i].color = colors[i];
        Players[i].position = 0;
        Players[i].money = initialMoney;
        Players[i].isActive = 1;
        Players[i].laps = 0;
        sprintf(Players[i].name, "Player %d", i + 1);
    }
}

// 주사위 보드 그리기 함수
void draw_dice_board(int dice1, int dice2) {
    int x = 35, y = 26;

    // 주사위 1
    gotoxy(x, y);
    printf("┌??────┐");
    gotoxy(x, y + 1);
    printf("│  %d  │", dice1);
    gotoxy(x, y + 2);
    printf("└─────┘");

    // 주???위 2
    gotoxy(x + 15, y);
    printf("┌─────┐");
    gotoxy(x + 15, y + 1);
    printf("│  %d  │", dice2);
    gotoxy(x + 15, y + 2);
    printf("└─────┘");

    // 합계
    gotoxy(x + 8, y + 4);
    printf("합계: %d", dice1 + dice2);
}

// 게임 ??보 표시 함수 수정
void show_game_info() {
    // Remove duplicate game info display
    gotoxy(140, 3);
    printf("나의 번호: 플레이어 %d", myPlayerNum + 1);

    gotoxy(140, 5);
    printf("┌─ 게임 정보 ─┐");
    gotoxy(140, 6);
    printf("현재 턴: 플레이어 %d", currentTurn + 1);
    gotoxy(140, 7);
    printf("현재 라운드: %d", roundCount);

    for (int i = 0; i < 2; i++) {  // Only show 2 players
        gotoxy(140, 9 + i);
        textColor(Players[i].color);
        printf("플레이어 %d: %d원 (%d바퀴)", i + 1, Players[i].money, Players[i].laps);
        textColor(15);
    }

    // Show controls only once
    gotoxy(140, 14);
    printf("┌─ 조작법 ─┐");
    gotoxy(140, 15);
    printf("SPACE: 주사위 굴리기");
    gotoxy(140, 16);
    printf("Y: 땅 구매");
    gotoxy(140, 17);
    printf("N: 구매 건너뛰기");
}

// 말 그리기 함수 수정
void draw_player_markers() {
    for (int i = 0; i < playerCount; i++) {
        if (Players[i].isActive) {
            int pos = Players[i].position;
            int x, y;

            // 위치에 따른 좌표 계산
            if (pos <= 10) {
                x = 111 - (pos * 11);
                y = 51;
            }
            else if (pos <= 20) {
                x = 1;
                y = 51 - ((pos - 10) * 5);
            }
            else if (pos <= 30) {
                x = 1 + ((pos - 20) * 11);
                y = 1;
            }
            else {
                x = 111;
                y = 1 + ((pos - 30) * 5);
            }

            gotoxy(x + (i * 2), y + 1);
            textColor(Players[i].color);
            printf("●");
            textColor(15);
        }
    }
}

// 주사위 UI 그리기 함수 추가
void draw_dice(int x, int y, int value) {
    char dice[3][6] = {
        "┌─┐",
        "│ │",
        "└─┘"
    };

    for (int i = 0; i < 3; i++) {
        gotoxy(x, y + i);
        printf("%s", dice[i]);
    }

    gotoxy(x + 1, y + 1);
    printf("%d", value);
}

// 셀에 텍스트 출력 함수
void printInCell(int x, int y, char* text, int price) {
    char line[2][10 + 1] = { "" };
    int len = strlen(text);
    int currentLine = 0, currentChar = 0;

    if (!strcmp(text, "황금열쇠")) {
        textColor(6);
    }

    for (int i = 0; i < len; i++) {
        line[currentLine][currentChar++] = text[i];
        if (currentChar == 10 || i == len - 1) {
            line[currentLine][currentChar] = '\0';
            currentLine++;
            currentChar = 0;
            if (currentLine >= 2) break;
        }
    }

    for (int i = 0; i < currentLine; i++) {
        if (strlen(text) <= 4 || !price) {
            gotoxy(x, y + i + 1);
        }
        else {
            gotoxy(x, y + i);
        }
        printf("%s", line[i]);
    }

    if (price) {
        gotoxy(x, y + 3);
        printf("%d원", price);
    }

    // Draw ownership marker if property is owned
    for (int i = 0; i < 32; i++) {
        if (strcmp(Deeds[i].name, text) == 0 && Deeds[i].ownerNum != -1) {
            gotoxy(x + 8, y + 3);  // Position for ownership marker
            textColor(Players[Deeds[i].ownerNum].color);
            printf("■");  // Ownership marker
            textColor(15);
            break;
        }
    }

    textColor(15);
}

// 말 그리기 함수
void draw_player_marker(int playerNum, int x, int y) {
    gotoxy(x, y);
    textColor(Players[playerNum].color);
    printf("●");
    textColor(15);
}

// 네트워크 메시지 처리 함수
void handle_network_message(char* message) {
    char command[32];
    char rest[BUFFER_SIZE];
    char* payload;

    payload = strchr(message, ':');
    if (payload) {
        int cmdLen = payload - message;
        strncpy(command, message, cmdLen);
        command[cmdLen] = '\0';
        payload++; // ':' 다음으로 이동
    } else {
        strcpy(command, message);
        payload = "";
    }

    EnterCriticalSection(&printLock);

    if (strcmp(command, "DICE") == 0) {
        int dice1, dice2;
        if (sscanf(payload, "%d,%d", &dice1, &dice2) == 2) {
            diceRollInProgress = 1;  // Mark dice roll as in progress
            system("cls");  // 화면 초기화
            draw_board();   // 보드 다시 그리기
            draw_dice_board(dice1, dice2);  // 주사위 표시
            show_game_info();  // 정보 업데이트
            gotoxy(0, 57);
            printf("주사위 결과: %d + %d = %d", dice1, dice2, dice1 + dice2);
        }
    }
    else if (strcmp(command, "CAN_BUY") == 0) {
        int position, price;
        char propertyName[50];
        if (sscanf(payload, "%d,%d,%[^\n]", &position, &price, propertyName) == 3) {
            if (currentTurn == myPlayerNum) {
                waitingForPurchase = 1;
                canBuyPosition = position;
                currentPosition = position;  // Update current position

                // Clear previous message and show purchase prompt
                gotoxy(0, 57);
                printf("                                                                              ");
                gotoxy(0, 57);
                printf("%s 땅을 구매하시겠습니까? (Y/N) - 가격: %d원", propertyName, price);
            }
        }
    }
    else if (strcmp(command, "BUILD_SUCCESS") == 0) {
        int position, level, money;
        sscanf(rest, "%d,%d,%d", &position, &level, &money);
        Players[currentTurn].money = money;
        system("cls");
        draw_board();
        show_game_info();
    }
    else if (strcmp(command, "SALARY") == 0) {
        int playerNum, money;
        sscanf(rest, "%d,%d", &playerNum, &money);
        Players[playerNum].money = money;
        gotoxy(0, 57);
        printf("월급 200,000원이 지급되었습니다!");
        Sleep(1000);
    }
    else if (strcmp(command, "MOVE") == 0) {
        int playerNum, newPos;
        if (sscanf(payload, "%d,%d", &playerNum, &newPos) == 2) {
            Players[playerNum].position = newPos;
            if (playerNum == myPlayerNum) {
                currentPlayerPos = newPos;  // Update current player position
            }
            diceRollInProgress = 0;  // Clear dice roll state
            system("cls");
            draw_board();
            draw_player_markers();
            show_game_info();
        }
    }
    else if (strcmp(command, "TURN") == 0) {
        int newTurn, position;
        sscanf(payload, "%d,%d", &newTurn, &position);
        currentTurn = newTurn;
        lastDiceRoll = 0;  // Reset dice roll state for new turn
        waitingForPurchase = 0;  // Reset purchase state
        canBuyPosition = -1;     // Reset buy position
        turnChangeInProgress = 0;

        // Refresh screen
        system("cls");
        draw_board();
        draw_player_markers();
        show_game_info();

        // Show turn message
        gotoxy(0, 56);
        if (currentTurn == myPlayerNum) {
            printf("당신의 차례입니다. 스페이스바를 눌러 주사위를 굴리세요.\n");
        } else {
            printf("플레이어 %d의 차례입니다.\n", currentTurn + 1);
            // Clear any remaining prompts
            gotoxy(0, 57);
            printf("                                                                              ");
        }
    }
    else if (strcmp(command, "PLAYER_NUM") == 0) {
        int playerNum;
        sscanf(payload, "%d", &playerNum);  // Changed from rest to payload
        myPlayerNum = playerNum;
        if (myPlayerNum >= 0 && myPlayerNum < 2) {  // Add validation
            gotoxy(0, 55);
            printf("당신은 플레이어 %d입니다.\n", myPlayerNum + 1);
        } else {
            // Handle invalid player number
            printf("Error: Invalid player number received (%d)\n", myPlayerNum);
            exit(1);
        }
    }
    else if (strcmp(command, "CREATED") == 0) {
        gotoxy(0, 55);
        printf("방이 생성되었습니다. 다른 플레이어를 기다리는 중...\n");
        // 방 생성 성공 시 추가 처리
        gameState = STATE_LOBBY;  // 로비 상태 ???지
    }
    else if (strcmp(command, "JOIN_FAILED") == 0) {
        gotoxy(0, 55);
        printf("방 입장에 실패했습니다. 방 ???름을 확인해주세요.\n");
        Sleep(2000);
        system("cls");
        gameState = STATE_LOBBY;  // 로비 상태로 복귀
    }
    else if (strcmp(command, "GAME_START") == 0) {
        gameState = STATE_IN_GAME;
        system("cls");
        draw_board();
        show_game_info();
    }
    else if (strcmp(command, "PHASE_CHANGE") == 0) {
        gamePhase = atoi(rest);
        gotoxy(0, 57);
        if (gamePhase == 1) {
            printf("후반전이 시작되었습니다! 이제 건물을 지을 수 있습니다.");
        }
        Sleep(2000);
        system("cls");
        draw_board();
        show_game_info();
    }
    else if (strcmp(command, "ROUND") == 0) {
        roundCount = atoi(payload);
        gotoxy(0, 57);
        printf("새로운 라운드가 시작되었습니다! (라운드 %d)", roundCount);
        Sleep(1000);
        system("cls");
        draw_board();
        show_game_info();
    }
    else if (strcmp(command, "FORCE_MOVE") == 0) {
        int playerNum, newPos;
        sscanf(payload, "%d,%d", &playerNum, &newPos);
        Players[playerNum].position = newPos;
        gotoxy(0, 57);
        printf("플레이어 %d가 너무 앞서나가서 인도로 이동되었습니다!", playerNum + 1);
        Sleep(1000);
        system("cls");
        draw_board();
        show_game_info();
    }
    else if (strcmp(command, "PURCHASE_SUCCESS") == 0) {
        int position, playerNum, money;
        sscanf(payload, "%d,%d,%d", &position, &playerNum, &money);
        Players[playerNum].money = money;
        Deeds[position].ownerNum = playerNum;  // Update ownership

        // Clear purchase prompt and update screen
        gotoxy(0, 57);
        printf("                                                                              ");
        system("cls");
        draw_board();
        draw_player_markers();
        show_game_info();

        // Show turn message
        gotoxy(0, 56);
        if (currentTurn == myPlayerNum) {
            printf("당신의 차례입니다. 스페이스바를 눌러 주사위를 굴리세요.\n");
        } else {
            printf("플레이어 %d의 차례입니다.\n", currentTurn + 1);
        }
    }
    else if (strcmp(command, "SKIP_PURCHASE") == 0) {
        // Clear purchase prompt
        gotoxy(0, 57);
        printf("                                                                              ");
        system("cls");
        draw_board();
        draw_player_markers();
        show_game_info();
    }
    LeaveCriticalSection(&printLock);
}

// 수신 스레드 함수
DWORD WINAPI receive_thread(LPVOID arg) {
    char buffer[BUFFER_SIZE];

    while (1) {
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived <= 0) {
            break;
        }
        buffer[bytesReceived] = '\0';
        handle_network_message(buffer);
    }
    return 0;
}

// 게임판 그리기 함수
void draw_board() {
    system("cls");  // 화면 초기화

    // 보드 프레임 그리기
    //?? 윗라인
    gotoxy(0, 0);
    puts("┌──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┐");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("│          ");
        }
        puts("│");
    }
    puts("├──??───────┼──────────┴──────────┴─??────────┴──────────┴──────────┴──────────┴──────────??──────────┴──────────┼──────────┤");

    // 중간 라인
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            puts("│          │                                                                                                  ???          │");
        }
        puts("├──────────┤                                                                                                  ├──────────┤");
    }

    for (int j = 0; j < 4; j++) {
        puts("│          │                                                                                                  │          │");
    }

    // 맨 아랫라인
    puts("├──────────┼──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┬──────────┼──────────┤");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("│          ");
        }
        puts("│");
    }
    puts("└──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┴──────────┘");

    // 블루마블 타이틀 그리기
    char title_art[12][150] = {
        "  ■■     ■■    ■■■■■■■■            ■    ■      ■  ",
        "  ■■     ■■          ■■    ■■■■■■  ■    ■■???■■■■■  ",
        "  ■■■■■■■■■    ■■■■■■■■    ■    ■  ■    ■      ■  ",
        "  ■■     ■■    ■■          ■    ■  ■    ■■■■■■■■  ",
        "  ■■■■■■■■■    ■■          ■    ■  ■■■■            ",
        "               ■■■■■■■■■   ■    ■  ■■■■■■■■■■■■■■■",
        "                           ■    ■  ■       ■■      ",
        "■■■??■■■■■■■???■■■■■■■■■■■■■  ■    ■  ■    ■■■■■■■■  ",
        "      ■■          ■■       ■■■■■■  ■           ■  ",
        "      ■■          ■■               ■    ■■■■■■■■  ",
        "      ■■          ■■               ■    ■          ",
        "      ■■          ■■               ■    ■■■■■???■■■ "
    };

    int title_x = 37, title_y = 10;
    for (int i = 0; i < 12; i++) {
        gotoxy(title_x, title_y + i);
        puts(title_art[i]);
    }

    // 주사위 보드 그리기
    char dice_board_art[13][180] = {
        "┌───────────────────────────────────────────────────────┐",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "│                                                       │",
        "└───────────────────────────────────────────────────────┘",
    };

    int dice_x = 34, dice_y = 25;
    for (int i = 0; i < 13; i++) {
        gotoxy(dice_x, dice_y + i);
        puts(dice_board_art[i]);
    }

    // 황금열쇠 그리기
    char key_art[7][100] = {
        "┌──────────────??─────────────┐",
        "│  .---.                     │",
        "│ /    |\\________________    │",
        "│ | ()  | ________   _   _)  │",
        "│ \\    |/        | | | |     ???",
        "│  `---'         \" - \" |_|   │",
        "└────────────────────────────┘"
    };

    int key_x = 47, key_y = 40;
    for (int i = 0; i < 7; i++) {
        gotoxy(key_x, key_y + i);
        puts(key_art[i]);
    }

    // 땅 정보 표시 (각 칸에 도시 이름??? 가격 출력)
    char names[40][40] = {
        "출발      ", "1", "황금열쇠", "1", "1", "1", "1", "황금??쇠", "1", "1",
        "무인도    ", "1", "황금열쇠", "1", "1", "1", "1", "황금열쇠", "1", "1",
        "사회복지기금", "1", "황금열쇠", "1", "1", "1", "1", "1", "1", "1",
        "우주여행  ", "1", "1", "1", "1", "황금열쇠", "1", "1", "사회복지기금", "1"
    };

    int cnt = 0;
    int curr_x = 111, curr_y = 51;

    // 아래쪽 행 출력 (좌->우)
    for (int i = 0; i < 10; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("■");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_x -= 11;
    }

    // 왼쪽 열 ???력 (아래->위)
    for (int i = 10; i < 20; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("■");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_y -= 5;
    }

    // 위쪽 행 출력 (좌->우)
    for (int i = 20; i < 30; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("■");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_x += 11;
    }

    // 오른쪽 열 출력 (위->아래)
    for (int i = 30; i < 40; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("■");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_y += 5;
    }

    // 게임 정보 출력
    gotoxy(140, 5);
    printf("┌─ 게임 정보 ─┐");
    gotoxy(140, 6);
    printf("현재 턴: 플레이어 %d", currentTurn + 1);

    for (int i = 0; i < playerCount; i++) {
        gotoxy(140, 8 + i);
        textColor(Players[i].color);
        printf("플레이어 %d: %d원", i + 1, Players[i].money);
        textColor(15);
    }

    gotoxy(140, 12);
    printf("┌─ 조작법 ─┐");
    gotoxy(140, 13);
    printf("SPACE: 주사위 굴리기");
    gotoxy(140, 14);
    printf("B: 건물 건설");
    gotoxy(140, 15);
    printf("T: 턴 넘기기");

    // 플레이어 말 그리기
    for (int i = 0; i < playerCount; i++) {
        if (Players[i].isActive) {
            int pos = Players[i].position;
            int x, y;

            // 위치에 따른 좌표 계산
            if (pos <= 10) {
                x = 111 - (pos * 11);
                y = 51;
            }
            else if (pos <= 20) {
                x = 1;
                y = 51 - ((pos - 10) * 5);
            }
            else if (pos <= 30) {
                x = 1 + ((pos - 20) * 11);
                y = 1;
            }
            else {
                x = 111;
                y = 1 + ((pos - 30) * 5);
            }

            gotoxy(x + (i * 2), y + 1);
            textColor(Players[i].color);
            printf("●");
            textColor(15);
        }
    }
}
// 게임 루프 함수 수정
void game_loop() {
    char buffer[BUFFER_SIZE];

    while (1) {
        if (gameState == STATE_IN_GAME && currentTurn == myPlayerNum) {
            if (_kbhit()) {
                char input = _getch();

                if (waitingForPurchase) {
                    switch(input) {
                        case 'y':
                        case 'Y':
                            sprintf(buffer, "BUY:%d", currentPosition);  // Use current position
                            send_network_message(buffer);
                            waitingForPurchase = 0;
                            canBuyPosition = -1;
                            // Clear purchase prompt
                            gotoxy(0, 57);
                            printf("                                                                              ");
                            break;
                        case 'n':
                        case 'N':
                            if (!turnChangeInProgress) {
                                send_network_message("SKIP_PURCHASE");
                                waitingForPurchase = 0;
                                turnChangeInProgress = 1;
                            }
                            break;
                    }
                }
                else if (input == ' ' && !diceRollInProgress && !lastDiceRoll) {
                    diceRollInProgress = 1;
                    lastDiceRoll = 1;
                    gotoxy(0, 57);
                    printf("주사위를 굴립니다...");
                    int dice1 = (rand() % 6) + 1;
                    int dice2 = (rand() % 6) + 1;
                    printf("DEBUG: 주사위 결과 - %d, %d\n", dice1, dice2);
                    sprintf(buffer, "ROLL:%d,%d", dice1, dice2);
                    send_network_message(buffer);
                }
            }
        }

        // Check for network messages
        int bytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, MSG_PEEK);
        if (bytes > 0) {
            bytes = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
            if (bytes > 0) {
                buffer[bytes] = '\0';
                handle_network_message(buffer);
            }
        }

        Sleep(50);  // Add small delay to prevent CPU overuse
    }
}

// game_start 함수에서 초기화 부분 수정
void game_start() {
    srand((unsigned int)time(NULL));

    // Winsock 초기화
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 실패\n");
        return;
    }

    // 소켓 생성
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("소켓 생성 실패\n");
        WSACleanup();
        return;
    }

    // 서버 연결
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("서버 연결 실패\n");
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    printf("서버에 연결되었습니다!\n");

    // 초기화 순서 변경
    init_deeds();
    init_players(2);

    // 버퍼링 모드 변경
    unsigned long mode = 1;
    ioctlsocket(clientSocket, FIONBIO, &mode);

    // 수신 스레드 생성
    HANDLE hThread = CreateThread(NULL, 0, receive_thread, NULL, 0, NULL);
    if (hThread == NULL) {
        printf("수신 스레드 생성 실패\n");
        return;
    }

    // 비동기 모드 해제 (동기 모드로 설정)
    mode = 0;
    ioctlsocket(clientSocket, FIONBIO, &mode);

    // 로비 메뉴
    while (gameState == STATE_LOBBY) {
        printf("\n=== 블루마블 게임 로비 ===\n");
        printf("1. 방 만들기\n");
        printf("2. 방 입장하기\n");
        printf("???택: ");

        char choice = _getch();
        char roomName[50];
        char message[BUFFER_SIZE];
        char response[BUFFER_SIZE];

        system("cls");

        if (choice == '1') {
            printf("\n방 이름을 입력하세??: ");
            scanf("%s", roomName);
            sprintf(message, "CREATE %s", roomName);
            if (send(clientSocket, message, strlen(message), 0) > 0) {
                printf("\n방을 생성하고 다른 플레이어를 기다리는 중...\n");

                // 서버 응답 대기
                int bytes = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
                if (bytes > 0) {
                    response[bytes] = '\0';
                    handle_network_message(response);
                }
            }
        }
        else if (choice == '2') {
            printf("\n방 이름을 입력하세요: ");
            scanf("%s", roomName);
            sprintf(message, "JOIN %s", roomName);
            if (send(clientSocket, message, strlen(message), 0) > 0) {
                printf("\n방 입장을 시도하는 중...\n");

                // 서버 응답 대기
                int bytes = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
                if (bytes > 0) {
                    response[bytes] = '\0';
                    handle_network_message(response);
                }
            }
        }

        Sleep(100);
    }

    // 게임 시작 후 비동기 모드로 전환
    mode = 1;
    ioctlsocket(clientSocket, FIONBIO, &mode);

    // 게임이 시작되면 화면을 지우고 게임 보드 표시
    system("cls");
    draw_board();

    // 게임 루프 실행
    game_loop();
}

// 메인 함수
int main() {
    srand((unsigned)time(NULL));
    InitializeCriticalSection(&printLock);
    game_start();
    DeleteCriticalSection(&printLock);
    return 0;
}

// 통행료 처리 함수 수정
void handle_toll(GameRoom* room, int position) {
    struct player* current = &room->players[room->currentTurn];
    Deed* property = &Deeds[position];  // Changed to use Deed* instead of struct deed*
    int owner = property->ownerNum;

    if (owner != -1 && owner != room->currentTurn) {
        int rent = property->baseRent * room->roundCount;  // Use baseRent instead of toll array
        if (current->money >= rent) {
            current->money -= rent;
            room->players[owner].money += rent;
            char msg[BUFFER_SIZE];
            sprintf(msg, "TOLL:%d,%d,%d", room->currentTurn, owner, rent);
            send(clientSocket, msg, strlen(msg), 0);
        } else {
            handle_bankruptcy(room, room->currentTurn, owner);
        }
    }
}

// 파산 처리 함수 수정
void handle_bankruptcy(GameRoom* room, int playerNum, int creditorNum) {
    struct player* bankrupt = &room->players[playerNum];
    bankrupt->isActive = 0;

    // 모든 소유 재산 이전
    for (int i = 0; i < 29; i++) {
        if (Deeds[i].ownerNum == playerNum) {
            Deeds[i].ownerNum = creditorNum;
            Deeds[i].buildingLevel = 0;  // ??물 초기화
        }
    }

    char msg[BUFFER_SIZE];
    sprintf(msg, "BANKRUPTCY:%d,%d", playerNum, creditorNum);
    send(clientSocket, msg, strlen(msg), 0);
}

// 건물 건설 함수 수정
void handle_build(GameRoom* room, int playerNum, int position) {
    if (gamePhase == 0) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "ERROR:전반전에는 건물을 지을 수 없습니다.");
        send(clientSocket, msg, strlen(msg), 0);
        return;
    }

    struct player* player = &room->players[playerNum];
    Deed* property = &Deeds[position];  // Changed to use Deed* instead of struct deed*

    if (property->ownerNum != playerNum || !property->isCanBuild) {
        return;
    }

    int buildCost = property->price;  // Use base price for simplicity
    if (player->money >= buildCost) {
        player->money -= buildCost;
        property->buildingLevel++;

        char msg[BUFFER_SIZE];
        sprintf(msg, "BUILD_SUCCESS:%d,%d,%d", position, property->buildingLevel, player->money);
        send(clientSocket, msg, strlen(msg), 0);
    }
}

// send_network_message 함수 추가
void send_network_message(const char* message) {
    send(clientSocket, message, strlen(message), 0);
}