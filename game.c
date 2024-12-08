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

// Constants related to game state
#define STATE_LOBBY 0
#define STATE_IN_GAME 1

// Add game constants
#define INDIA_POSITION 15  // india location
#define SALARY_AMOUNT 200000  // salary

// Define complete Deed structure at the beginning
typedef struct {
    char name[50];
    int price;
    int baseRent;
    int ownerNum;
    int buildingLevel;
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
Deed Deeds[40] = {
    {"轎嫦", 0, 0, -1, 0},
    {"顫檜漆檜", 50000, 5000, -1, 0},
    {"", 0, 0, -1, 0},  // 綴 蘊
    {"漆檜癒", 80000, 8000, -1, 0},
    {"葆棍塭", 80000, 8000, -1, 0},
    {"薯輿紫", 200000, 20000, -1, 0},
    {"諒陛ん腦", 100000, 10000, -1, 0},
    {"", 0, 0, -1, 0},  // 綴 蘊
    {"蘋檜煎", 100000, 10000, -1, 0},
    {"檜蝶驕碳", 120000, 12000, -1, 0},
    {"檣紫", 0, 0, -1, 0},  // か熱 蘊
    {"嬴纔啻", 140000, 14000, -1, 0},
    {"", 0, 0, -1, 0},  // 綴 蘊
    {"囀もж啦", 160000, 16000, -1, 0},
    {"蝶驚�次�", 160000, 16000, -1, 0},
    {"漆艇", 180000, 18000, -1, 0},
    {"", 0, 0, -1, 0},  // 綴 蘊
    {"漆蒂萼", 180000, 18000, -1, 0},
    {"螃顫諦", 200000, 20000, -1, 0},
    {"", 0, 0, -1, 0},  // 綴 蘊
    {"睡縑喻蝶", 220000, 22000, -1, 0},
    {"", 0, 0, -1, 0},  // 綴 蘊
    {"鼻だ選瑞", 240000, 24000, -1, 0},
    {"衛萄棲", 240000, 24000, -1, 0},
    {"睡骯", 500000, 50000, -1, 0},
    {"ж諦檜", 260000, 26000, -1, 0},
    {"葬蝶獄", 260000, 26000, -1, 0},
    {"覽縣葬濠漆蝶", 300000, 30000, -1, 0},
    {"渠掘", 350000, 35000, -1, 0},
    {"葆萄葬萄", 350000, 35000, -1, 0},
    {"", 0, 0, -1, 0},   // 綴 蘊
    {"紫瓖", 300000, 30000, -1, 0},
    {"鏽歲綠嬴", 450000, 45000, -1, 0},
    {"だ葬", 320000, 32000, -1, 0},
    {"煎葆", 320000, 32000, -1, 0},
    {"", 0, 0, -1, 0},   // 綴 蘊
    {"楛湍", 350000, 35000, -1, 0},
    {"景踹", 350000, 35000, -1, 0},
    {"", 0, 0, -1, 0},   // 綴 蘊
    {"憮選", 1000000, 100000, -1, 0}
};
struct player Players[2];  // Changed to 2 players only
SOCKET clientSocket;
CRITICAL_SECTION printLock;
int gameState = STATE_LOBBY;
int myPlayerNum = -1;
int playerCount = 0;
int currentTurn = -1;
int roundCount = 1;     // Track current round
int gamePhase = 0;  // Game Progression Steps
int waitingForPurchase = 0;
int canBuyPosition = -1;
int lastPurchasePosition = -1;  // Track last purchased position
int currentPosition = -1;  // Track current player's position
int lastDiceRoll = 0;  // Track when dice was last rolled
int diceRollInProgress = 0;
char currentPropertyName[50] = {0};
int currentPlayerPos = -1;  // Track current player's actual position
int turnChangeInProgress = 0;  // Track turn change state

// Add city names array for local display
const char* CityNames[40] = {
    "轎嫦", "顫檜漆檜", "", "漆檜癒", "葆棍塭", "薯輿紫", "諒陛ん腦", "", "蘋檜煎", "檜蝶驕碳",
    "檣紫", "嬴纔啻", "", "囀もж啦", "蝶驚�次�", "漆艇", "", "漆蒂萼", "螃顫諦", "", "睡縑喻蝶",
    "", "鼻だ選瑞", "衛萄棲", "睡骯", "ж諦檜", "葬蝶獄", "覽縣葬濠漆蝶", "渠掘", "葆萄葬萄", "",
    "紫瓖", "鏽歲綠嬴", "だ葬", "煎葆", "", "楛湍", "景踹", "", "憮選"
};

// л熱 摹樹
void textColor(int colorNum);
void gotoxy(int x, int y);
void draw_board();
void init_players(int player_cnt);
int roll_dice();
void handle_network_message(char* message);
void send_network_message(const char* message);
void draw_player_markers();
DWORD WINAPI receive_thread(LPVOID arg);
void printInCell(int x, int y, const Deed* deed);
void game_loop();
void handle_toll(GameRoom* room, int position);
void handle_bankruptcy(GameRoom* room, int playerNum, int creditorNum);
void handle_build(GameRoom* room, int playerNum, int position);

// Text color change function
void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

// Cursor position movement function
void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// player initialization function
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

// Dice board drawing function
void draw_dice_board(int dice1, int dice2) {
    int x = 35, y = 26;

    // dice 1
    gotoxy(x, y);
    printf("忙式式式式式忖");
    gotoxy(x, y + 1);
    printf("弛  %d  弛", dice1);
    gotoxy(x, y + 2);
    printf("戌式式式式式戎");

    // dice 2
    gotoxy(x + 15, y);
    printf("忙式式式式式忖");
    gotoxy(x + 15, y + 1);
    printf("弛  %d  弛", dice2);
    gotoxy(x + 15, y + 2);
    printf("戌式式式式式戎");

    // sum
    gotoxy(x + 8, y + 4);
    printf("м啗: %d", dice1 + dice2);
}

// Game information display function
void show_game_info() {
    gotoxy(140, 3);
    printf("釭曖 廓��: Ы溯檜橫 %d", myPlayerNum + 1);

    gotoxy(140, 5);
    printf("忙式 啪歜 薑爾 式忖");
    gotoxy(140, 6);
    printf("⑷營 欐: Ы溯檜橫 %d", currentTurn + 1);
    gotoxy(140, 7);
    printf("⑷營 塭遴萄: %d", roundCount);

    for (int i = 0; i < 2; i++) {  // Only show 2 players
        gotoxy(140, 9 + i);
        textColor(Players[i].color);
        printf("Ы溯檜橫 %d: %d錳", i + 1, Players[i].money);
        textColor(15);
    }

    // Show controls only once
    gotoxy(140, 14);
    printf("忙式 褻濛徹 式忖");
    gotoxy(140, 15);
    printf("SPACE: 輿餌嬪 掉葬晦");
    gotoxy(140, 16);
    printf("Y: 階 掘衙");
    gotoxy(140, 17);
    printf("N: 掘衙 勒傘嗨晦");
}

// horse drawing function
void draw_player_markers() {
    for (int i = 0; i < playerCount; i++) {
        if (Players[i].isActive) {
            int pos = Players[i].position;
            int x, y;

            // 嬪纂縑 評艇 謝ル 啗骯
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
            printf("≒");
            textColor(15);
        }
    }
}

// Dice UI drawing function
void draw_dice(int x, int y, int value) {
    char dice[3][6] = {
        "忙式忖",
        "弛 弛",
        "戌式戎"
    };

    for (int i = 0; i < 3; i++) {
        gotoxy(x, y + i);
        printf("%s", dice[i]);
    }

    gotoxy(x + 1, y + 1);
    printf("%d", value);
}

// Function to print text in a cell
void printInCell(int x, int y, const Deed* deed) {
    if (strlen(deed->name) == 0) {
        gotoxy(x, y + 2);
        printf("�盛暆乘�");
        return;
    }

    // Print property name
    gotoxy(x, y);
    printf("%s", deed->name);

    if (deed->price > 0) {
        gotoxy(x, y + 3);

        if (deed->ownerNum >= 0) {
            // Show rent with owner's color
            textColor(Players[deed->ownerNum].color);
            printf("%d", deed->baseRent * roundCount);
        } else {
            // Show property price in white
            textColor(15);  // White color
            printf("%d", deed->price);
        }
        textColor(15);  // Reset color
    }

    // Show ownership marker
    if (deed->ownerNum >= 0) {
        gotoxy(x + 8, y + 3);
        textColor(Players[deed->ownerNum].color);
        printf("﹥");
        textColor(15);
    }
}

// horse drawing function
void draw_player_marker(int playerNum, int x, int y) {
    gotoxy(x, y);
    textColor(Players[playerNum].color);
    printf("≒");
    textColor(15);
}

// Network message processing function
void handle_network_message(char* message) {
    char command[32];
    char rest[BUFFER_SIZE];
    char* payload;

    payload = strchr(message, ':');
    if (payload) {
        int cmdLen = payload - message;
        strncpy(command, message, cmdLen);
        command[cmdLen] = '\0';
        payload++; // ':' 棻擠戲煎 檜翕
    } else {
        strcpy(command, message);
        payload = "";
    }

    EnterCriticalSection(&printLock);

    if (strcmp(command, "DICE") == 0) {
        int dice1, dice2;
        if (sscanf(payload, "%d,%d", &dice1, &dice2) == 2) {
            diceRollInProgress = 1;  // Mark dice roll as in progress
            system("cls");
            draw_board();
            draw_dice_board(dice1, dice2);
            show_game_info();
            gotoxy(0, 57);
            printf("輿餌嬪 唸婁: %d + %d = %d", dice1, dice2, dice1 + dice2);
        }
    }
    else if (strcmp(command, "CAN_BUY") == 0) {
        int position, price, cityIndex;
        if (sscanf(payload, "%d,%d,%d", &position, &price, &cityIndex) == 3) {
            if (currentTurn == myPlayerNum) {
                waitingForPurchase = 1;
                canBuyPosition = position;
                currentPosition = position;  // Update current position

                // Clear previous message and show purchase prompt
                gotoxy(0, 57);
                printf("                                                                              ");
                gotoxy(0, 57);
                printf("%s 階擊 掘衙ж衛啊蝗棲梱? (Y/N) - 陛問: %d錳",
                    CityNames[cityIndex], price);  // Use local city name array
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
        sscanf(payload, "%d,%d", &playerNum, &money);
        Players[playerNum].money = money;

        // Display salary message and update game state
        gotoxy(0, 57);
        printf("Ы溯檜橫 %d 陛 錯晝 200,000 錳擊 嫡懊蝗棲棻.", playerNum + 1);
        Sleep(1000);

        // Clear message and continue game
        gotoxy(0, 57);
        printf("                                                           ");

        // Only update turn message if it's the current player's turn
        if (currentTurn == myPlayerNum) {
            gotoxy(0, 57);
            printf("Your turn. Press SPACE to roll the dice.");
        } else {
            gotoxy(0, 57);
            printf("Player %d's turn.", currentTurn + 1);
        }

        // Reset dice roll state to allow next turn
        lastDiceRoll = 0;
        diceRollInProgress = 0;
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
            printf("渡褐曖 離滔殮棲棻. 蝶む檜蝶夥蒂 揚楝 輿餌嬪蒂 掉葬撮蹂.\n");
        } else {
            printf("Ы溯檜橫 %d曖 離滔殮棲棻.\n", currentTurn + 1);
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
            printf("渡褐擎 Ы溯檜橫 %d殮棲棻.\n", myPlayerNum + 1);
        } else {
            // Handle invalid player number
            printf("Error: Invalid player number received (%d)\n", myPlayerNum);
            exit(1);
        }
    }
    else if (strcmp(command, "CREATED") == 0) {
        gotoxy(0, 55);
        printf("寞檜 儅撩腎歷蝗棲棻. 棻艇 Ы溯檜橫蒂 晦棻葬朝 醞...\n");
    }
    else if (strcmp(command, "JOIN_FAILED") == 0) {
        gotoxy(0, 55);
        printf("寞 殮濰縑 褒ぬц蝗棲棻. 寞 檜葷擊 �挫恉媮祤撚�.\n");
        Sleep(2000);
        system("cls");
        gameState = STATE_LOBBY;  // return to lobby
    }
    else if (strcmp(command, "GAME_START") == 0) {
        gameState = STATE_IN_GAME;
        system("cls");
        draw_board();
        show_game_info();
    }
    // not used
    else if (strcmp(command, "PHASE_CHANGE") == 0) {
        gamePhase = atoi(rest);
        gotoxy(0, 57);
        if (gamePhase == 1) {
            printf("�贏暌�檜 衛濛腎歷蝗棲棻! 檜薯 勒僭擊 雖擊 熱 氈蝗棲棻.");
        }
        Sleep(2000);
        system("cls");
        draw_board();
        show_game_info();
    }

    else if (strcmp(command, "ROUND") == 0) {
        roundCount = atoi(payload);
        gotoxy(0, 57);
        printf("億煎遴 塭遴萄陛 衛濛腎歷蝗棲棻! (塭遴萄 %d)", roundCount);
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
        printf("Ы溯檜橫 %d陛 傘鼠 擅憮釭陛憮 檣紫煎 檜翕腎歷蝗棲棻!", playerNum + 1);
        Sleep(1000);
        system("cls");
        draw_board();
        show_game_info();
    }
    else if (strcmp(command, "PURCHASE_SUCCESS") == 0) {
        int position, playerNum, money;
        sscanf(payload, "%d,%d,%d", &position, &playerNum, &money);
        Players[playerNum].money = money;
        Deeds[position].ownerNum = playerNum;  // Now this should work

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
            printf("渡褐曖 離滔殮棲棻. 蝶む檜蝶夥蒂 揚楝 輿餌嬪蒂 掉葬撮蹂.\n");
        } else {
            printf("Ы溯檜橫 %d曖 離滔殮棲棻.\n", currentTurn + 1);
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
    else if (strcmp(command, "TOLL") == 0) {
        int fromPlayer, toPlayer, amount;
        if (sscanf(payload, "%d,%d,%d", &fromPlayer, &toPlayer, &amount) == 3) {
            Players[fromPlayer].money -= amount;
            Players[toPlayer].money += amount;

            gotoxy(0, 57);
            printf("                                                                              ");
            gotoxy(0, 57);
            if (fromPlayer == myPlayerNum) {
                printf("Ы溯檜橫 %d縑啪 鱔ч猿 %d錳擊 雖碳ц蝗棲棻.", toPlayer + 1, amount);
            } else if (toPlayer == myPlayerNum) {
                printf("Ы溯檜橫 %d縑啪憮 鱔ч猿 %d錳擊 嫡懊蝗棲棻.", fromPlayer + 1, amount);
            }
            Sleep(1500);

            system("cls");
            draw_board();
            draw_player_markers();
            show_game_info();
        }
    }
    LeaveCriticalSection(&printLock);
}

// receiving thread function
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

// Game board drawing function
void draw_board() {
    system("cls");  // init screen

    // board frame
    // upper line
    gotoxy(0, 0);
    puts("忙式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式忖");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("弛          ");
        }
        puts("弛");
    }
    puts("戍式式式式式式式式式式托式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式托式式式式式式式式式式扣");

    // stay in the middle
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            puts("弛          弛                                                                                                  弛          弛");
        }
        puts("戍式式式式式式式式式式扣                                                                                                  戍式式式式式式式式式式扣");
    }

    for (int j = 0; j < 4; j++) {
        puts("弛          弛                                                                                                  弛          弛");
    }

    // below line
    puts("戍式式式式式式式式式式托式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式托式式式式式式式式式式扣");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("弛          ");
        }
        puts("弛");
    }
    puts("戌式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式戎");

    // title
    char title_art[12][150] = {
        "  ﹥﹥     ﹥﹥    ﹥﹥﹥﹥﹥﹥﹥﹥            ﹥    ﹥      ﹥  ",
        "  ﹥﹥     ﹥﹥          ﹥﹥    ﹥﹥﹥﹥﹥﹥  ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "  ﹥﹥﹥﹥﹥﹥﹥﹥﹥    ﹥﹥﹥﹥﹥﹥﹥﹥    ﹥    ﹥  ﹥    ﹥      ﹥  ",
        "  ﹥﹥     ﹥﹥    ﹥﹥          ﹥    ﹥  ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "  ﹥﹥﹥﹥﹥﹥﹥﹥﹥    ﹥﹥          ﹥    ﹥  ﹥               ",
        "               ﹥﹥﹥﹥﹥﹥﹥﹥﹥   ﹥    ﹥  ﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥",
        "                           ﹥    ﹥  ﹥       ﹥﹥      ",
        "﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥ ﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥ ﹥﹥﹥﹥﹥﹥ ﹥  ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "      ﹥﹥          ﹥﹥               ﹥           ﹥  ",
        "      ﹥﹥          ﹥﹥               ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "      ﹥﹥          ﹥﹥               ﹥    ﹥          ",
        "      ﹥﹥          ﹥﹥               ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥﹥ "
    };

    int title_x = 37, title_y = 10;
    for (int i = 0; i < 12; i++) {
        gotoxy(title_x, title_y + i);
        puts(title_art[i]);
    }

    // dice board but not used
    char dice_board_art[13][180] = {
        "忙式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式忖",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "弛                                                       弛",
        "戌式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式式戎",
    };

    int dice_x = 34, dice_y = 25;
    for (int i = 0; i < 13; i++) {
        gotoxy(dice_x, dice_y + i);
        puts(dice_board_art[i]);
    }

    // golden key but NOT USED
    char key_art[7][100] = {
        "忙式式式式式式式式式式式式式式式式式式式式式式式式式式式式忖",
        "弛  .---.                     弛",
        "弛 /    |\\________________    弛",
        "弛 | ()  | ________   _   _)  弛",
        "弛 \\    |/        | | | |     弛",
        "弛  `---'         \" - \" |_|   弛",
        "戌式式式式式式式式式式式式式式式式式式式式式式式式式式式式戎"
    };

    int key_x = 47, key_y = 40;
    for (int i = 0; i < 7; i++) {
        gotoxy(key_x, key_y + i);
        puts(key_art[i]);
    }

    // Display land information (print city name and price in each cell)
    int curr_x = 111, curr_y = 51;
    int deedIndex = 0;

    // 嬴楚薹 ч 轎溘 (謝->辦)
    for (int i = 0; i < 10; i++) {
        printInCell(curr_x, curr_y, &Deeds[deedIndex++]);
        curr_x -= 11;
    }

    // 豭薹 翮 轎溘 (嬴楚->嬪)
    for (int i = 0; i < 10; i++) {
        printInCell(curr_x, curr_y, &Deeds[deedIndex++]);
        curr_y -= 5;
    }

    // 嬪薹 ч 轎溘 (謝->辦)
    for (int i = 0; i < 10; i++) {
        printInCell(curr_x, curr_y, &Deeds[deedIndex++]);
        curr_x += 11;
    }

    // 螃艇薹 翮 轎溘 (嬪->嬴楚)
    for (int i = 0; i < 10; i++) {
        printInCell(curr_x, curr_y, &Deeds[deedIndex++]);
        curr_y += 5;
    }

    // Game information output
    gotoxy(140, 5);
    printf("忙式 啪歜 薑爾 式忖");
    gotoxy(140, 6);
    printf("⑷營 欐: Ы溯檜橫 %d", currentTurn + 1);

    for (int i = 0; i < playerCount; i++) {
        gotoxy(140, 8 + i);
        textColor(Players[i].color);
        printf("Ы溯檜橫 %d: %d錳", i + 1, Players[i].money);
        textColor(15);
    }

    gotoxy(140, 12);
    printf("忙式 褻濛徹 式忖");
    gotoxy(140, 13);
    printf("SPACE: 輿餌嬪 掉葬晦");
    gotoxy(140, 14);
    printf("B: 勒僭 勒撲");
    gotoxy(140, 15);
    printf("T: 欐 剩晦晦");

    // players char? horse? draw on board
    for (int i = 0; i < playerCount; i++) {
        if (Players[i].isActive) {
            int pos = Players[i].position;
            int x, y;

            // Calculate coordinates based on location
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
            printf("≒");
            textColor(15);
        }
    }
}

// game loop
void game_loop() {
    char buffer[BUFFER_SIZE];

    while (1) {
        if (gameState == STATE_IN_GAME && currentTurn == myPlayerNum) {
            if (_kbhit()) {
                char input = _getch();

                if (input == ' ' && !diceRollInProgress && !lastDiceRoll) {
                    diceRollInProgress = 1;
                    lastDiceRoll = 1;
                    gotoxy(0, 57);
                    printf("輿餌嬪蒂 掉董棲棻...");
                    int dice1 = (rand() % 6) + 1;
                    int dice2 = (rand() % 6) + 1;
                    sprintf(buffer, "ROLL:%d,%d", dice1, dice2);
                    send_network_message(buffer);
                }

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

// game_start function
void game_start() {
    srand((unsigned int)time(NULL));

    // Winsock init
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), & wsaData) != 0) {
        printf("WSAStartup 褒ぬ\n");
        return;
    }

    // create socket
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("模鰍 儅撩 褒ぬ\n");
        WSACleanup();
        return;
    }

    // server connect
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr * ) & serverAddr, sizeof(serverAddr)) < 0) {
        printf("憮幗 翱唸 褒ぬ\n");
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    printf("憮幗縑 翱唸腎歷蝗棲棻!\n");

    // Change initialization order
    init_players(2);

    // Change buffering mode
    unsigned long mode = 1;
    ioctlsocket(clientSocket, FIONBIO, & mode);

    // Create a receiving thread
    HANDLE hThread = CreateThread(NULL, 0, receive_thread, NULL, 0, NULL);
    if (hThread == NULL) {
        printf("熱褐 蝶溯萄 儅撩 褒ぬ\n");
        return;
    }

    // Turn off asynchronous mode (set to synchronous mode)
    mode = 0;
    ioctlsocket(clientSocket, FIONBIO, & mode);

    // lobby menu
    while (gameState == STATE_LOBBY) {
        printf("\n=== 綰瑞葆綰 啪歜 煎綠 ===\n");
        printf("1. 寞 虜菟晦\n");
        printf("2. 寞 殮濰ж晦\n");
        printf("摹鷗: ");

        char choice = _getch();
        char roomName[50];
        char message[BUFFER_SIZE];
        char response[BUFFER_SIZE];

        system("cls");

        if (choice == '1') {
            printf("\n寞 檜葷擊 殮溘ж撮蹂: ");
            scanf("%s", roomName);
            sprintf(message, "CREATE %s", roomName);
            if (send(clientSocket, message, strlen(message), 0) > 0) {
                printf("\n寞擊 儅撩ж堅 棻艇 Ы溯檜橫蒂 晦棻葬朝 醞...\n");

                // wait for server response
                int bytes = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
                if (bytes > 0) {
                    response[bytes] = '\0';
                    handle_network_message(response);
                }
            }
        } else if (choice == '2') {
            printf("\n寞 檜葷擊 殮溘ж撮蹂: ");
            scanf("%s", roomName);
            sprintf(message, "JOIN %s", roomName);
            if (send(clientSocket, message, strlen(message), 0) > 0) {
                printf("\n寞 殮濰擊 衛紫ж朝 醞...\n");

                // wait for server response
                int bytes = recv(clientSocket, response, BUFFER_SIZE - 1, 0);
                if (bytes > 0) {
                    response[bytes] = '\0';
                    handle_network_message(response);
                }
            }
        }

        Sleep(100);
    }

    // Switch to asynchronous mode after starting the game
    mode = 1;
    ioctlsocket(clientSocket, FIONBIO, & mode);

    // When the game starts, clear the screen and show the game board
    system("cls");
    draw_board();

    // game loop launch lets goooooo
    game_loop();
}

// main
int main() {
    srand((unsigned) time(NULL));
    InitializeCriticalSection( & printLock);
    game_start();
    DeleteCriticalSection( & printLock);
    return 0;
}

// Toll processing function
void handle_toll(GameRoom * room, int position) {
    struct player * current = & room -> players[room -> currentTurn];
    Deed * property = & Deeds[position];
    int owner = property -> ownerNum;

    if (owner != -1 && owner != room -> currentTurn) {
        int rent = property -> baseRent * room -> roundCount;
        if (current -> money >= rent) {
            current -> money -= rent;
            room -> players[owner].money += rent;
            char msg[BUFFER_SIZE];
            sprintf(msg, "TOLL:%d,%d,%d", room -> currentTurn, owner, rent);
            send(clientSocket, msg, strlen(msg), 0);
        } else {
            handle_bankruptcy(room, room -> currentTurn, owner);
        }
    }
}

// bankruptcy processing function
void handle_bankruptcy(GameRoom * room, int playerNum, int creditorNum) {
    struct player * bankrupt = & room -> players[playerNum];
    bankrupt -> isActive = 0;

    // transfer all owned property
    for (int i = 0; i < 40; i++) {
        if (Deeds[i].ownerNum == playerNum) {
            Deeds[i].ownerNum = creditorNum;
            Deeds[i].buildingLevel = 0;
        }
    }

    char msg[BUFFER_SIZE];
    sprintf(msg, "BANKRUPTCY:%d,%d", playerNum, creditorNum);
    send(clientSocket, msg, strlen(msg), 0);
}

// building construction function but **NOT USED** because simplified bluemarble sorry
void handle_build(GameRoom * room, int playerNum, int position) {
    if (gamePhase == 0) {
        char msg[BUFFER_SIZE];
        sprintf(msg, "ERROR:瞪奩瞪縑朝 勒僭擊 雖擊 熱 橈蝗棲棻.");
        send(clientSocket, msg, strlen(msg), 0);
        return;
    }

    struct player * player = & room -> players[playerNum];
    Deed * property = & Deeds[position];

    // Check if the property is buildable
    if (property -> ownerNum != playerNum || property -> price <= 0) {
        return;
    }

    int buildCost = property -> price;
    if (player -> money >= buildCost) {
        player -> money -= buildCost;
        property -> buildingLevel++;

        char msg[BUFFER_SIZE];
        sprintf(msg, "BUILD_SUCCESS:%d,%d,%d", position, property -> buildingLevel, player -> money);
        send(clientSocket, msg, strlen(msg), 0);
    }
}

// send_network_message function
void send_network_message(const char * message) {
    send(clientSocket, message, strlen(message), 0);
}
