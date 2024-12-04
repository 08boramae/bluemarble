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

// ���� ���� ���� ���
#define STATE_LOBBY 0
#define STATE_IN_GAME 1

// ����ü ����
struct deed {
    char name[40];      // ���� �̸�
    int areaNum;        // ���ʳ�� ����
    int ownerNum;       // 0: ����
    int buildingLevel;  // �ǹ� ����
    int price[5];       // ����, ����1, ����2, ����, ȣ�� ����
    int toll[5];        // �����
    int isCanBuild;     // �Ǽ� ���� ����
};

struct player {
    int color;          // �÷��̾� ����
    int position;       // ���� ��ġ
    int money;          // ���� ���� �ݾ�
    char name[20];      // �÷��̾� �̸�
    int isActive;       // Ȱ��ȭ ����
};

// ���� ����
struct deed Deeds[29];
struct player Players[4];
SOCKET clientSocket;
CRITICAL_SECTION printLock;
int gameState = STATE_LOBBY;
int myPlayerNum = -1;
int playerCount = 0;
int currentTurn = -1;

// �Լ� ����
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

// �ؽ�Ʈ ���� ���� �Լ�
void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

// Ŀ�� ��ġ �̵� �Լ�
void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

// deed �ʱ�ȭ �Լ�
// Deed �ʱ�ȭ �Լ�
void init_deeds() {
    char names[29][40] = {
        "Ÿ�̺���", "����¡", "���Ҷ�", "���ֵ�", "�̰�����", "ī�̷�", "�̽�ź��",
        "���׳�", "�����ϰ�", "����Ȧ��", "���ڵ�", "����", "������", "��Ÿ��",
        "�ο��뽺", "���Ŀ��", "�õ��", "�λ�", "�Ͽ���", "������", "�������ں���", "���帮��",
        "����", "�÷����", "�ĸ�", "�θ�", "����", "����", "����"
    };

    // ���� ���� (����, ����1, ����2, ����, ȣ��)
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

    // ����� ����
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
        Deeds[i].isCanBuild = (prices[i][1] > 0); // ����1 ������ 0���� ũ�� �Ǽ� ����
    }
}

// �÷��̾� �ʱ�ȭ �Լ�
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

// �ֻ��� ���� �׸��� �Լ�
void draw_dice_board(int dice1, int dice2) {
    int x = 35, y = 26;

    // �ֻ��� 1
    gotoxy(x, y);
    printf("��������������");
    gotoxy(x, y + 1);
    printf("��  %d  ��", dice1);
    gotoxy(x, y + 2);
    printf("��������������");

    // �ֻ��� 2
    gotoxy(x + 15, y);
    printf("��������������");
    gotoxy(x + 15, y + 1);
    printf("��  %d  ��", dice2);
    gotoxy(x + 15, y + 2);
    printf("��������������");

    // �հ�
    gotoxy(x + 8, y + 4);
    printf("�հ�: %d", dice1 + dice2);
}

void show_game_info() {
    gotoxy(140, 3);
    printf("���� ��ȣ: �÷��̾� %d", myPlayerNum + 1);

    gotoxy(140, 5);
    printf("���� ���� ���� ����");
    gotoxy(140, 6);
    printf("���� ��: �÷��̾� %d", currentTurn + 1);

    for (int i = 0; i < playerCount; i++) {
        gotoxy(140, 8 + i);
        textColor(Players[i].color);
        printf("�÷��̾� %d: %d��", i + 1, Players[i].money);
        textColor(15);
    }

    gotoxy(140, 12);
    printf("���� ���۹� ����");
    gotoxy(140, 13);
    printf("SPACE: �ֻ��� ������");
    gotoxy(140, 14);
    printf("B: �ǹ� �Ǽ�");
    gotoxy(140, 15);
    printf("T: �� �ѱ��");
}

// �� �׸��� �Լ� ����
void draw_player_markers() {
    for (int i = 0; i < playerCount; i++) {
        if (Players[i].isActive) {
            int pos = Players[i].position;
            int x, y;

            // ��ġ�� ���� ��ǥ ���
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
            printf("��");
            textColor(15);
        }
    }
}

// �ֻ��� UI �׸��� �Լ� �߰�
void draw_dice(int x, int y, int value) {
    char dice[3][6] = {
        "������",
        "�� ��",
        "������"
    };

    for (int i = 0; i < 3; i++) {
        gotoxy(x, y + i);
        printf("%s", dice[i]);
    }

    gotoxy(x + 1, y + 1);
    printf("%d", value);
}

// ���� �ؽ�Ʈ ��� �Լ�
void printInCell(int x, int y, char* text, int price) {
    char line[2][10 + 1] = { "" };
    int len = strlen(text);
    int currentLine = 0, currentChar = 0;

    if (!strcmp(text, "Ȳ�ݿ���")) {
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
        printf("%d��", price);
    }
    textColor(15);
}

// �� �׸��� �Լ�
void draw_player_marker(int playerNum, int x, int y) {
    gotoxy(x, y);
    textColor(Players[playerNum].color);
    printf("��");
    textColor(15);
}

// ��Ʈ��ũ �޽��� ó�� �Լ�
void handle_network_message(char* message) {
    char command[32];
    char rest[BUFFER_SIZE];
    sscanf(message, "%[^:]:%s", command, rest);

    EnterCriticalSection(&printLock);

    if (strcmp(command, "DICE") == 0) {
        int dice1, dice2;
        sscanf(rest, "%d,%d", &dice1, &dice2);
        draw_dice_board(dice1, dice2);
        // 1�� ���
        Sleep(1000);
    }
    else if (strcmp(command, "CAN_BUY") == 0) {
        int position;
        sscanf(rest, "%d", &position);
        gotoxy(0, 57);
        printf("�� ���� �����Ͻðڽ��ϱ�? (Y/N) - ����: %d��", Deeds[position].price[0]);
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
        printf("���� 200,000���� ���޵Ǿ����ϴ�!");
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
            printf("����� �����Դϴ�. �����̽��ٸ� ���� �ֻ����� ��������.\n");
        } else {
            printf("�÷��̾� %d�� �����Դϴ�.\n", currentTurn + 1);
        }
        show_game_info();
    }
    else if (strcmp(command, "PLAYER_NUM") == 0) {
        int playerNum;
        sscanf(rest, "%d", &playerNum);
        myPlayerNum = playerNum;
        gotoxy(0, 55);
        printf("����� �÷��̾� %d�Դϴ�.\n", myPlayerNum + 1);
    }
    else if (strcmp(command, "CREATED") == 0) {
        gotoxy(0, 55);
        printf("���� �����Ǿ����ϴ�. �ٸ� �÷��̾ ��ٸ��� ��...\n");
    }
    else if (strcmp(command, "JOIN_FAILED") == 0) {
        gotoxy(0, 55);
        printf("�� ���忡 �����߽��ϴ�. �� �̸��� Ȯ�����ּ���.\n");
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

// ���� ������ �Լ�
DWORD WINAPI receive_thread(LPVOID arg) {
    char buffer[BUFFER_SIZE];
    int bytesReceived;

    while ((bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0)) > 0) {
        buffer[bytesReceived] = '\0';
        handle_network_message(buffer);
    }

    return 0;
}

// ������ �׸��� �Լ�
void draw_board() {
    //�� ������
    gotoxy(0, 0);
    puts("����������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("��          ");
        }
        puts("��");
    }
    puts("����������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������");

    // �߰� ����
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 4; j++) {
            puts("��          ��                                                                                                  ��          ��");
        }
        puts("������������������������                                                                                                  ������������������������");
    }

    for (int j = 0; j < 4; j++) {
        puts("��          ��                                                                                                  ��          ��");
    }

    // �� �Ʒ�����
    puts("����������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������");
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 11; j++) {
            printf("��          ");
        }
        puts("��");
    }
    puts("��������������������������������������������������������������������������������������������������������������������������������������������??������������������������������������������������������������������������������������������������������");

    // ��縶�� Ÿ��Ʋ �׸���
    char title_art[12][150] = {
        "  ���     ���    ���������            ��    ��      ��  ",
        "  ���     ���          ���    �������  ��    ���������  ",
        "  ����������    ���������    ��    ��  ��    ��      ��  ",
        "  ���     ���    ���          ��    ��  ��    ���������  ",
        "  ����������    ���          ��    ��  �����            ",
        "               ����������   ��    ��  ����������������",
        "                           ��    ��  ��       ���      ",
        "��������������������������  ��    ��  ��    ���������  ",
        "      ���          ���       �������  ��           ��  ",
        "      ���          ���               ��    ���������  ",
        "      ���          ���               ��    ��          ",
        "      ���          ���               ��    ���������� "
    };

    int title_x = 37, title_y = 10;
    for (int i = 0; i < 12; i++) {
        gotoxy(title_x, title_y + i);
        puts(title_art[i]);
    }

    // �ֻ��� ���� �׸���
    char dice_board_art[13][180] = {
        "������������������������������������������������������������������������������������������������������������������",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "��                                                       ��",
        "������������������������������������������������������������������������������������������������������������������",
    };

    int dice_x = 34, dice_y = 25;
    for (int i = 0; i < 13; i++) {
        gotoxy(dice_x, dice_y + i);
        puts(dice_board_art[i]);
    }

    // Ȳ�ݿ��� �׸���
    char key_art[7][100] = {
        "������������������������������������������������������������",
        "��  .---.                     ��",
        "�� /    |\\________________    ��",
        "�� | ()  | ________   _   _)  ��",
        "�� \\    |/        | | | |     ��",
        "��  `---'         \" - \" |_|   ��",
        "������������������������������������������������������������"
    };

    int key_x = 47, key_y = 40;
    for (int i = 0; i < 7; i++) {
        gotoxy(key_x, key_y + i);
        puts(key_art[i]);
    }

    // �� ���� ǥ�� (�� ĭ�� ���� �̸��� ���� ���)
    char names[40][40] = {
        "���      ", "1", "Ȳ�ݿ���", "1", "1", "1", "1", "Ȳ�ݿ���", "1", "1",
        "���ε�    ", "1", "Ȳ�ݿ���", "1", "1", "1", "1", "Ȳ�ݿ���", "1", "1",
        "��ȸ�������", "1", "Ȳ�ݿ���", "1", "1", "1", "1", "1", "1", "1",
        "���ֿ���  ", "1", "1", "1", "1", "Ȳ�ݿ���", "1", "1", "��ȸ�������", "1"
    };

    int cnt = 0;
    int curr_x = 111, curr_y = 51;

    // �Ʒ��� �� ��� (��->��)
    for (int i = 0; i < 10; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("��");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_x -= 11;
    }

    // ���� �� ��� (�Ʒ�->��)
    for (int i = 10; i < 20; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("��");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_y -= 5;
    }

    // ���� �� ��� (��->��)
    for (int i = 20; i < 30; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("��");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_x += 11;
    }

    // ������ �� ��� (��->�Ʒ�)
    for (int i = 30; i < 40; i++) {
        if (strcmp(names[i], "1") == 0) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, Deeds[cnt].price[0]);
            if (Deeds[cnt].ownerNum >= 0) {
                gotoxy(curr_x + 8, curr_y + 3);
                textColor(Players[Deeds[cnt].ownerNum].color);
                printf("��");
                textColor(15);
            }
            cnt++;
        } else {
            printInCell(curr_x, curr_y, names[i], 0);
        }
        curr_y += 5;
    }

    // ���� ���� ���
    gotoxy(140, 5);
    printf("���� ���� ���� ����");
    gotoxy(140, 6);
    printf("���� ��: �÷��̾� %d", currentTurn + 1);

    for (int i = 0; i < playerCount; i++) {
        gotoxy(140, 8 + i);
        textColor(Players[i].color);
        printf("�÷��̾� %d: %d��", i + 1, Players[i].money);
        textColor(15);
    }

    gotoxy(140, 12);
    printf("���� ���۹� ����");
    gotoxy(140, 13);
    printf("SPACE: �ֻ��� ������");
    gotoxy(140, 14);
    printf("B: �ǹ� �Ǽ�");
    gotoxy(140, 15);
    printf("T: �� �ѱ��");

    // �÷��̾� �� �׸���
    for (int i = 0; i < playerCount; i++) {
        if (Players[i].isActive) {
            int pos = Players[i].position;
            int x, y;

            // ��ġ�� ���� ��ǥ ���
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
            printf("��");
            textColor(15);
        }
    }
}
// ���� ���� �Լ�
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
                    printf("�ǹ��� �Ǽ��� ��ġ�� �Է��ϼ��� (0-31): ");
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
        Sleep(50);  // CPU ���� ����
    }
}

void game_start() {
    srand((unsigned int)time(NULL));

    // Winsock �ʱ�ȭ
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup ����\n");
        return;
    }

    // ���� ����
    clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (clientSocket == INVALID_SOCKET) {
        printf("���� ���� ����\n");
        WSACleanup();
        return;
    }

    // ���� ����
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SERVER_IP);

    if (connect(clientSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        printf("���� ���� ����\n");
        closesocket(clientSocket);
        WSACleanup();
        return;
    }

    printf("������ ����Ǿ����ϴ�!\n");

    // �ʿ��� �ʱ�ȭ ����
    init_deeds();
    init_players(2);

    // ���� ������ ����
    CreateThread(NULL, 0, receive_thread, NULL, 0, NULL);

    // �κ� �޴�
    while (gameState == STATE_LOBBY) {  // ���� ���� ������ ��d��
        printf("\n1. �� �����\n");
        printf("2. �� �����ϱ�\n");
        printf("����: ");

        char choice = _getch();
        char roomName[50];
        char message[BUFFER_SIZE];

        system("cls");  // ȭ�� �����

        if (choice == '1') {
            printf("\n�� ��??�� �Է��ϼ���: ");
            scanf("%s", roomName);
            sprintf(message, "CREATE %s", roomName);
            send(clientSocket, message, strlen(message), 0);
            printf("\n���� �����ϰ� �ٸ� �÷��̾ ��ٸ��� ��...\n");
        }
        else if (choice == '2') {
            printf("\n�� �̸��� �Է��ϼ���: ");
            scanf("%s", roomName);
            sprintf(message, "JOIN %s", roomName);
            send(clientSocket, message, strlen(message), 0);
            printf("\n�� ������ �õ��ϴ� ��...\n");
        }

        Sleep(100);  // ��� ���
    }

    // ������ ���۵Ǹ� ȭ���� ����� ���� ���� ǥ��
    system("cls");
    draw_board();

    // ���� ���� ����
    game_loop();
}

// ���� �Լ�
int main() {
    srand((unsigned)time(NULL));
    InitializeCriticalSection(&printLock);
    game_start();
    DeleteCriticalSection(&printLock);
    return 0;
}