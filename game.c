#define _CRT_SECURE_NO_WARNINGS
#pragma comment(lib, "ws2_32")
#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>

#define RED 12
#define BLUE 9
#define GREEN 10
#define YELLOW 14
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"
#define SERVER_PORT 12345

// 啪歜 鼻鷓 婦溼 鼻熱
#define STATE_LOBBY 0
#define STATE_IN_GAME 1

// 掘褻羹 薑曖
struct deed {
    char name[40];      // 饜雖 檜葷
    int areaNum;        // 說蟾喻匐 掘羲
    int ownerNum;       // 0: 橈擠
    int buildingLevel;  // 勒僭 溯漣
    int price[5];       // 渠雖, 滌濰1, 滌濰2, 網註, ��蘿 陛問
    int toll[5];        // 鱔ч猿
    int isCanBuild;     // 勒撲 陛棟 罹睡
};

struct player {
    int color;          // Ы溯檜橫 儀鼻
    int position;       // ⑷營 嬪纂
    int money;          // ⑷營 爾嶸 旎擋
    char name[20];      // Ы溯檜橫 檜葷
    int isActive;       // �側瘓� 鼻鷓
};

// 瞪羲 滲熱
struct deed Deeds[29];
struct player Players[4];
SOCKET clientSocket;
CRITICAL_SECTION printLock;
int gameState = STATE_LOBBY;
int myPlayerNum = -1;
int playerCount = 0;
int currentTurn = -1;

// л熱 摹樹
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

// 臢蝶お 儀鼻 滲唳 л熱
void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

// 醴憮 嬪纂 檜翕 л熱
void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// deed 蟾晦�� л熱
// Deed 蟾晦�� л熱
void init_deeds() {
    char names[29][40] = {
        "顫檜漆檜", "漆檜癒", "葆棍塭", "薯輿紫", "諒陛ん腦", "蘋檜煎", "檜蝶驕碳",
        "嬴纔啻", "囀もж啦", "蝶驚�次�", "攝囀萄", "漆艇", "漆蒂萼", "螃顫諦",
        "睡縑喻蝶", "鼻だ選瑞", "衛萄棲", "睡骯", "ж諦檜", "葬蝶獄", "覽縣葬濠漆蝶", "葆萄葬萄",
        "紫瓖", "鏽歲綠嬴", "だ葬", "煎葆", "楛湍", "景踹", "憮選"
    };

    // 陛問 薑爾 (渠雖, 滌濰1, 滌濰2, 網註, ��蘿)
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

    // 鱔ч猿 薑爾
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
        for (int j = 0; j < 5; j++) {
            Deeds[i].price[j] = prices[i][j];
            Deeds[i].toll[j] = tolls[i][j];
        }
        Deeds[i].isCanBuild = (prices[i][1] > 0); // 滌濰1 陛問檜 0爾棻 觼賊 勒撲 陛棟
    }
}

// Ы溯檜橫 蟾晦�� л熱
void init_players(int player_cnt) {
    int colors[4] = {RED, BLUE, GREEN, YELLOW};
    playerCount = player_cnt;

    for (int i = 0; i < player_cnt; i++) {
        Players[i].color = colors[i];
        Players[i].position = 0;
        Players[i].money = 3000000;
        Players[i].isActive = 1;
        sprintf(Players[i].name, "Player %d", i + 1);
    }
}

// 輿餌嬪 爾萄 斜葬晦 л熱
void draw_dice_board(int dice1, int dice2) {
    int x = 35, y = 26;

    // 輿餌嬪 1
    gotoxy(x, y);
    printf("忙式式式式式忖");
    gotoxy(x, y + 1);
    printf("弛  %d  弛", dice1);
    gotoxy(x, y + 2);
    printf("戌式式式式式戎");

    // 輿餌嬪 2
    gotoxy(x + 15, y);
    printf("忙式式式式式忖");
    gotoxy(x + 15, y + 1);
    printf("弛  %d  弛", dice2);
    gotoxy(x + 15, y + 2);
    printf("戌式式式式式戎");

    // м啗
    gotoxy(x + 8, y + 4);
    printf("м啗: %d", dice1 + dice2);
}

void show_game_info() {
    gotoxy(140, 3);
    printf("釭曖 廓��: Ы溯檜橫 %d", myPlayerNum + 1);

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
}

// 蜓 斜葬晦 л熱 熱薑
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

// 輿餌嬪 UI 斜葬晦 л熱 蹺陛
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

// 撚縑 臢蝶お 轎溘 л熱
void printInCell(int x, int y, char* text, int price) {
    char line[2][10 + 1] = { "" };
    int len = strlen(text);
    int currentLine = 0, currentChar = 0;

    if (!strcmp(text, "�盛暆乘�")) {
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
        printf("%d錳", price);
    }
    textColor(15);
}

// 蜓 斜葬晦 л熱
void draw_player_marker(int playerNum, int x, int y) {
    gotoxy(x, y);
    textColor(Players[playerNum].color);
    printf("≒");
    textColor(15);
}

// 啻お錶觼 詭衛雖 籀葬 л熱
void handle_network_message(char* message) {
    char command[32];
    char rest[BUFFER_SIZE];
    sscanf(message, "%[^:]:%s", command, rest);

    EnterCriticalSection(&printLock);

    if (strcmp(command, "DICE") == 0) {
        int dice1, dice2;
        sscanf(rest, "%d,%d", &dice1, &dice2);
        draw_dice_board(dice1, dice2);
        // 1蟾 渠晦
        Sleep(1000);
    }
    else if (strcmp(command, "CAN_BUY") == 0) {
        int position;
        sscanf(rest, "%d", &position);
        gotoxy(0, 57);
        printf("檜 階擊 掘衙ж衛啊蝗棲梱? (Y/N) - 陛問: %d錳", Deeds[position].price[0]);
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
        printf("錯晝 200,000錳檜 雖晝腎歷蝗棲棻!");
        Sleep(1000);
    }
    else if (strcmp(command, "MOVE") == 0) {
        int playerNum, newPos;
        sscanf(rest, "%d,%d", &playerNum, &newPos);
        Players[playerNum].position = newPos;
        system("cls");
        draw_board();
        show_game_info();
    }
    else if (strcmp(command, "TURN") == 0) {
        sscanf(rest, "%d", &currentTurn);
        gotoxy(0, 56);
        if (currentTurn == myPlayerNum) {
            printf("渡褐曖 離滔殮棲棻. 蝶む檜蝶夥蒂 揚楝 輿餌嬪蒂 掉葬撮蹂.\n");
        } else {
            printf("Ы溯檜橫 %d曖 離滔殮棲棻.\n", currentTurn + 1);
        }
        show_game_info();
    }
    else if (strcmp(command, "PLAYER_NUM") == 0) {
        int playerNum;
        sscanf(rest, "%d", &playerNum);
        myPlayerNum = playerNum;
        gotoxy(0, 55);
        printf("渡褐擎 Ы溯檜橫 %d殮棲棻.\n", myPlayerNum + 1);
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
    }
    else if (strcmp(command, "GAME_START") == 0) {
        gameState = STATE_IN_GAME;
        system("cls");
        draw_board();
        show_game_info();
    }
    LeaveCriticalSection(&printLock);
}

// 熱褐 蝶溯萄 л熱
DWORD WINAPI receive_thread(LPVOID arg) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        handle_network_message(buffer);
    }

    return 0;
}

// 啪歜っ 斜葬晦 л熱
void draw_board() {
    //裔 嶺塭檣
    gotoxy(0, 0);
    puts("忙式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式忖");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("弛          ");
        }
        puts("弛");
    }
    puts("戍式式式式式式式式式式托式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式托式式式式式式式式式式扣");

    // 醞除 塭檣
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            puts("弛          弛                                                                                                  弛          弛");
        }
        puts("戍式式式式式式式式式式扣                                                                                                  戍式式式式式式式式式式扣");
    }

    for (int j = 0; j < 4; j++) {
        puts("弛          弛                                                                                                  弛          弛");
    }

    // 裔 嬴概塭檣
    puts("戍式式式式式式式式式式托式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式托式式式式式式式式式式扣");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("弛          ");
        }
        puts("弛");
    }
    puts("戌式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式??式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式戎");

    // 綰瑞葆綰 顫檜ぎ 斜葬晦
    char title_art[12][150] = {
        "  ﹥﹥     ﹥﹥    ﹥﹥﹥﹥﹥﹥﹥﹥            ﹥    ﹥      ﹥  ",
        "  ﹥﹥     ﹥﹥          ﹥﹥    ﹥﹥﹥﹥﹥﹥  ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "  ﹥﹥﹥﹥﹥﹥﹥﹥﹥    ﹥﹥﹥﹥﹥﹥﹥﹥    ﹥    ﹥  ﹥    ﹥      ﹥  ",
        "  ﹥﹥     ﹥﹥    ﹥﹥          ﹥    ﹥  ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "  ﹥﹥﹥﹥﹥﹥﹥﹥﹥    ﹥﹥          ﹥    ﹥  ﹥﹥﹥﹥            ",
        "               ﹥﹥﹥﹥﹥﹥﹥﹥﹥   ﹥    ﹥  ﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥",
        "                           ﹥    ﹥  ﹥       ﹥﹥      ",
        "﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥  ﹥    ﹥  ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "      ﹥﹥          ﹥﹥       ﹥﹥﹥﹥﹥﹥  ﹥           ﹥  ",
        "      ﹥﹥          ﹥﹥               ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥  ",
        "      ﹥﹥          ﹥﹥               ﹥    ﹥          ",
        "      ﹥﹥          ﹥﹥               ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥﹥ "
    };

    int title_x = 37, title_y = 10;
    for (int i = 0; i < 12; i++) {
        gotoxy(title_x, title_y + i);
        puts(title_art[i]);
    }

    // 輿餌嬪 爾萄 斜葬晦
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

    // �盛暆乘� 斜葬晦
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

    // 階 薑爾 ル衛 (陝 蘊縑 紫衛 檜葷婁 陛問 轎溘)
    char names[40][40] = {
        "轎嫦      ", "1", "�盛暆乘�", "1", "1", "1", "1", "�盛暆乘�", "1", "1",
        "鼠檣紫    ", "1", "�盛暆乘�", "1", "1", "1", "1", "�盛暆乘�", "1", "1",
        "餌�蛹嘗鰡漹�", "1", "�盛暆乘�", "1", "1", "1", "1", "1", "1", "1",
        "辦輿罹ч  ", "1", "1", "1", "1", "�盛暆乘�", "1", "1", "餌�蛹嘗鰡漹�", "1"
    };

    int cnt = 0;
    int curr_x = 111, curr_y = 51;

    // 嬴楚薹 ч 轎溘 (謝->辦)
    for (int i = 0; i < 10; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("﹥");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_x -= 11;
    }

    // 豭薹 翮 轎溘 (嬴楚->嬪)
    for (int i = 10; i < 20; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("﹥");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_y -= 5;
    }

    // 嬪薹 ч 轎溘 (謝->辦)
    for (int i = 20; i < 30; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("﹥");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_x += 11;
    }

    // 螃艇薹 翮 轎溘 (嬪->嬴楚)
    for (int i = 30; i < 40; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("﹥");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_y += 5;
    }

    // 啪歜 薑爾 轎溘
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

    // Ы溯檜橫 蜓 斜葬晦
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
// 啪歜 瑞Щ л熱
void game_loop() {
    char input;

    while (1) {
        if (gameState == STATE_IN_GAME && currentTurn == myPlayerNum) {
            if (_kbhit()) {
                input = _getch();
                if (input == ' ') {
                    int dice1 = (rand() % 6) + 1;
                    int dice2 = (rand() % 6) + 1;

                    char message[BUFFER_SIZE];
                    sprintf(message, "ROLL:%d,%d", dice1, dice2);
                    send(clientSocket, message, strlen(message), 0);
                    Sleep(500);
                }
                else if (input == 'b' || input == 'B') {
                    gotoxy(0, 57);
                    printf("勒僭擊 勒撲й 嬪纂蒂 殮溘ж撮蹂 (0-31): ");
                    int position;
                    scanf("%d", &position);

                    char message[BUFFER_SIZE];
                    sprintf(message, "BUILD:%d", position);
                    send(clientSocket, message, strlen(message), 0);
                }
                else if (input == 'y' || input == 'Y') {
                    char message[BUFFER_SIZE];
                    sprintf(message, "BUY:%d", Players[myPlayerNum].position);
                    send(clientSocket, message, strlen(message), 0);
                }
            }
        }
        Sleep(50);  // CPU 餌辨睦 馬模
    }
}

void game_start() {
    srand((unsigned int)time(NULL));

    // Winsock 蟾晦��
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup 褒ぬ\n");
        return;
    }

    // 模鰍 儅撩
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("模鰍 儅撩 褒ぬ\n");
        WSACleanup();
        return;
    }

    // 憮幗 翱唸
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("憮幗 翱唸 褒ぬ\n");
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    printf("憮幗縑 翱唸腎歷蝗棲棻!\n");

    // в蹂и 蟾晦�� 熱ч
    init_deeds();
    init_players(2);

    // 熱褐 蝶溯萄 儅撩
    CreateThread(NULL, 0, receive_thread, NULL, 0, NULL);

    // 煎綠 詭景
    while (gameState == STATE_LOBBY) {  // 啪歜 衛濛 瞪梱雖 奩d犒
        printf("\n1. 寞 虜菟晦\n");
        printf("2. 寞 殮濰ж晦\n");
        printf("摹鷗: ");

        char choice = _getch();
        char roomName[50];
        char message[BUFFER_SIZE];

        system("cls");  // �飛� 雖辦晦

        if (choice == '1') {
            printf("\n寞 檜??擊 殮溘ж撮蹂: ");
            scanf("%s", roomName);
            sprintf(message, "CREATE %s", roomName);
            send(clientSocket, message, strlen(message), 0);
            printf("\n寞擊 儅撩ж堅 棻艇 Ы溯檜橫蒂 晦棻葬朝 醞...\n");
        }
        else if (choice == '2') {
            printf("\n寞 檜葷擊 殮溘ж撮蹂: ");
            scanf("%s", roomName);
            sprintf(message, "JOIN %s", roomName);
            send(clientSocket, message, strlen(message), 0);
            printf("\n寞 殮濰擊 衛紫ж朝 醞...\n");
        }

        Sleep(100);  // 濡衛 渠晦
    }

    // 啪歜檜 衛濛腎賊 �飛橉� 雖辦堅 啪歜 爾萄 ル衛
    system("cls");
    draw_board();

    // 啪歜 瑞Щ 褒ч
    game_loop();
}

// 詭檣 л熱
int main() {
    srand((unsigned)time(NULL));
    InitializeCriticalSection(&printLock);
    game_start();
    DeleteCriticalSection(&printLock);
    return 0;
}