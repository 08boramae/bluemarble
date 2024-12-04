#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <time.h>
#include <conio.h>

#define RED 12
#define BLUE 9
#define GREEN 10
#define YELLOW 14
#define WHITE 15

void textColor(int colorNum);
void gotoxy(int x, int y);
void printInCell(int x, int y, char* text, int price, int idx);
void draw_board();
void init_deeds();
void init_players(int player_cnt);
int roll_dice();
void buy_land(struct player* player, struct deed* land);
void pay_toll(struct player* player, struct deed* land);
void game_start();

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
    int color;
    int position;
    int money;
};

struct goldenKey {
    int id;
    char name[50];
    char desc[100];
};

struct deed Deeds[29];
struct player Players[2];

int curr_arrow_pos;

void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void printInCell(int x, int y, char* text, int price, int idx) {
    char line[2][10 + 1] = { "" };  // �ٹٲ޵� �ؽ�Ʈ�� ������ �迭
    int len = strlen(text);                        // �ؽ�Ʈ ����
    int currentLine = 0, currentChar = 0;          // ���� �ٰ� ���� ��ġ

    if (!strcmp(text, "Ȳ�ݿ���")) {
        textColor(6);
    }
    else if (!strcmp(text, "���      <---")) {
        textColor(RED);
    }
    else if (!strcmp(text, "���ε�")) {
        textColor(GREEN);
    }
    else if (!strcmp(text, "��ȸ������� ����ó")) {
        textColor(6);
    }
    else if (!strcmp(text, "���ֿ���")) {
        textColor(13);
    }

    // ���ڿ��� ĭ �ʺ� �°� ������
    for (int i = 0; i < len; i++) {
        line[currentLine][currentChar++] = text[i];
        if (currentChar == 10 || i == len - 1) {
            line[currentLine][currentChar] = '\0'; // �� ���� NULL �߰�
            currentLine++;
            currentChar = 0;
            if (currentLine >= 2) break;   // �ִ� �� �� �ʰ� �� �ߴ�
        }
    }

    // ���� �ؽ�Ʈ ��� (ĭ ���ο� ���� ���)
    for (int i = 0; i < currentLine; i++) {
        if (strlen(text) <= 4 || !price) {
            gotoxy(x, y + i+ 1);
        }
        else {
            gotoxy(x, y + i);                        // �� ���� ��ġ ����
        }
        printf("%s\n", line[i]);
    }
    //if (Players[0].position == idx) {
    //    textColor(RED);
    //    printf("��");
    //}
    //if (Players[1].position == idx) {
    //    textColor(BLUE);
    //    printf("��");
    //}
    //textColor(WHITE);

    // �ݾ� ��� (ĭ�� ������ �ٿ� ��ġ)
    if(price) {
        gotoxy(x, y + 3);
        printf("%d��", price);
    }
    textColor(WHITE);


    // �÷��̾� �� ���
}

void show_player_status() {
    gotoxy(140, 15);
    textColor(RED);
    printf("Player1 ����");
    gotoxy(140, 16);
    printf("��ġ: %d", Players[0].position);
    gotoxy(140, 17);
    printf("�ڱ�: %d��", Players[0].money);

    gotoxy(140, 19);
    textColor(BLUE);
    printf("Player2 ����");
    gotoxy(140, 20);
    printf("��ġ: %d", Players[1].position);
    gotoxy(140, 21);
    printf("�ڱ�: %d��", Players[1].money);

    textColor(15);
}

void draw_players() {
    char* player1_marker = "��";
    char* player2_marker = "��";
    int p1_x, p1_y, p2_x, p2_y;

    // Player1 ��ġ ��� (������)
    if (0 <= Players[0].position && Players[0].position <= 9) {
        p1_x = 111 - (Players[0].position * 11);
        p1_y = 52;
    }
    else if (10 <= Players[0].position && Players[0].position <= 19) {
        p1_x = 1;
        p1_y = 51 - ((Players[0].position - 10) * 5);
    }
    else if (20 <= Players[0].position && Players[0].position <= 29) {
        p1_x = 1 + ((Players[0].position - 20) * 11);
        p1_y = 1;
    }
    else if (30 <= Players[0].position && Players[0].position <= 39) {
        p1_x = 112;
        p1_y = 1 + ((Players[0].position - 30) * 5);
    }

    // Player2 ��ġ ��� (�Ķ���)
    if (0 <= Players[1].position && Players[1].position <= 9) {
        p2_x = 111 - (Players[1].position * 11) + 2;  // Player1���� �ణ �����ʿ� ��ġ
        p2_y = 52;
    }
    else if (10 <= Players[1].position && Players[1].position <= 19) {
        p2_x = 1;
        p2_y = 51 - ((Players[1].position - 10) * 5) + 1;  // Player1���� �ణ �Ʒ��� ��ġ
    }
    else if (20 <= Players[1].position && Players[1].position <= 29) {
        p2_x = 1 + ((Players[1].position - 20) * 11) + 2;
        p2_y = 1;
    }
    else if (30 <= Players[1].position && Players[1].position <= 39) {
        p2_x = 112;
        p2_y = 1 + ((Players[1].position - 30) * 5) + 1;
    }

    // Player1 (������) ǥ��
    gotoxy(p1_x, p1_y);
    textColor(RED);
    printf("%s", player1_marker);

    // Player2 (�Ķ���) ǥ��
    gotoxy(p2_x, p2_y);
    textColor(BLUE);
    printf("%s", player2_marker);

    // ���� �������
    textColor(15);
}

void draw_board() {
    //�� ������
    gotoxy(0, 0);
    puts("����������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������");
    for (int i = 0; i < 4; i++) {
        for (int i = 0; i < 11; i++) {
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
        for (int i = 0; i < 11; i++) {
            printf("��          ");
        }
        puts("��");
    }
    puts("����������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������");



    char title_art[12][150] = {
        //"    ���          ���      ���������                      ��        ��            ��    ",
        //"    ���          ���                  ���      �������    ��        ���������    ",
        //"    ����������      ���������      ��        ��    ��        ��            ��    ",
        //"    ���          ���      ���                  ��        ��    ��        ���������    ",
        //"    ����������      ���                  ��        ��    �����                      ",
        //"                            ����������    ��        ��    ����������������",
        //"                                                  ��        ��    ��              ���          ",
        //"�������������������������  ��        ��    ��        ���������    ",
        //"          ���                    ���            �������    ��                      ��    ",
        //"          ���                    ���                            ��        ���������    ",
        //"          ���                    ���                            ��        ��                  ",
        //"          ���                    ���                            ��        ����������  "
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
        "      ���          ���               ��    ���������� ",

    }; 

    int title_x=37, title_y=10;
    for (int i = 0; i < 12; i++) {
        gotoxy(title_x, title_y+i);
        puts(title_art[i]);
    }

    char key_art[7][100] = {
        "������������������������������������������������������������",
        "��  .---.                     ��",
        "�� /    |\\________________    ��",
        "�� | ()  | ________   _   _)  ��",
        "�� \\    |/        | | | |     ��",
        "��  `---'         \" - \" |_|   ��",
        "������������������������������������������������������������"
    };

    textColor(YELLOW);
    int key_x=47, key_y=40;
    for (int i = 0; i < 7; i++) {
        gotoxy(key_x, key_y + i);
        puts(key_art[i]);
    }
    textColor(15);


    // (x,y)���� ����
    // ���������� ��ĭ : (x+11, y)
    // �Ʒ������� ��ĭ : (x, y+5)
    
    char names[40][40] = { "���      <---", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "���ε�", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ������� ����ó", "1", "Ȳ�ݿ���", "1","1","1","1","1","1","1","���ֿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ�������", "1"};
    int cnt = 0;
    int curr_x = 111, curr_y = 51;
    for (int i = 0; i < 10; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0, i);
            cnt++;
            curr_x -= 11;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0, i);
            curr_x -= 11;
        }
    }

    for (int i = 10; i < 20; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0, i);
            cnt++;
            curr_y -= 5;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0, i);
            curr_y -= 5;
        }
    }

    for (int i = 20; i < 30; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0, i);
            cnt++;
            curr_x += 11;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0, i);
            curr_x += 11;
        }
    }

    for (int i = 30; i < 40; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0, i);
            cnt++;
            curr_y += 5;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0, i);
            curr_y += 5;
        }
    }

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
    curr_x = 34, curr_y = 25;
    for (int i = 0; i < 13; i++) {
        gotoxy(curr_x, curr_y + i);
        puts(dice_board_art[i]);
    }


    show_player_status();
    

}


void init_deeds() {
    char names[29][40] = {
    "Ÿ�̺���", "����¡", "���Ҷ�", "���ֵ�", "�̰�����", "ī�̷�", "�̽�ź��", 
    "�Ƴ���", "�����ϰ�", "����Ȧ��", "���ڵ� ������", "����", "������", "��Ÿ��",
    "�ο��뽺 ���̷���", "���Ŀ��", "�õ��", "�λ�", "�Ͽ���", "������", "�� �����ں��� ȣ", "���帮��",
    "����", "�÷����ȣ", "�ĸ�", "�θ�", "����", "����", "����"
    };
    int each_area_num[4] = { 7, 7, 8, 7 };
    int ownerNum = 0;
    int buildingLevel = 0;
    int prices[29][5] = { // ���Ű� : ����, ����, ����2, ����, ȣ��
        {50000, 50000, 100000, 150000, 250000}, {80000, 50000, 100000, 150000, 250000}, {80000, 50000, 100000, 150000, 250000}, {200000}, {100000, 50000, 100000, 150000, 250000}, {100000, 50000, 100000, 150000, 250000}, {120000, 50000, 100000, 150000, 250000},
        {140000, 100000, 200000, 300000, 500000}, {160000, 100000, 200000, 300000, 500000}, {160000, 100000, 200000, 300000, 500000}, {200000}, {180000, 100000, 200000, 300000, 500000}, {180000, 100000, 200000, 300000, 500000}, {200000, 100000, 200000, 300000, 500000},
        {220000, 150000, 300000, 400000, 750000}, {240000, 150000, 300000, 450000, 750000}, {240000, 150000, 300000, 450000, 750000}, {500000}, {260000, 150000, 300000, 450000, 750000}, {260000, 150000, 300000, 450000, 750000}, {300000}, {280000, 150000, 300000, 450000, 750000},
        {300000, 200000, 400000, 600000, 1000000}, {450000}, {320000, 200000, 400000, 600000, 1000000}, {320000, 200000, 400000, 600000, 1000000}, {350000, 200000, 400000, 600000, 1000000}, {350000, 200000, 400000, 600000, 1000000}, {1000000}
    };
    int tolls[29][5] = { // �����: ����, ����, ����2, ����, ȣ��
        {2000, 10000, 30000, 90000, 250000}, {4000, 20000, 60000, 180000, 450000}, {4000, 20000, 60000, 180000, 450000}, {300000}, {6000, 30000, 90000, 270000, 550000}, {6000, 30000, 90000, 270000, 550000}, {8000, 40000, 100000, 300000, 600000},
        {10000, 50000, 150000, 450000, 750000}, {12000, 60000, 180000, 500000, 900000}, {12000, 60000, 180000, 500000, 900000}, {300000}, {14000, 70000, 200000, 550000, 950000}, {14000, 70000, 200000, 550000, 950000}, {16000, 80000, 220000, 600000, 1000000},
        {180000, 90000, 250000, 700000, 1050000}, {20000, 100000, 300000, 750000, 1100000}, {20000, 100000, 300000, 750000, 1100000}, {600000}, {22000, 110000, 330000, 800000, 1150000}, {22000, 110000, 330000, 800000, 1150000}, {250000}, {24000, 120000, 360000, 850000, 1200000},
        {26000, 130000, 390000, 900000, 1270000}, {300000}, {28000, 150000, 450000, 1000000, 1400000}, {28000, 150000, 450000, 1000000, 1400000}, {35000, 170000, 500000, 1100000, 1500000}, {35000, 170000, 500000, 1100000, 1500000}, {2000000}
    };

    for (int i = 0; i < 29; i++) { // Deeds init
        strncpy(Deeds[i].name, names[i], sizeof(Deeds[i].name));
        Deeds[i].ownerNum = -1; // -1 : ������X
        Deeds[i].buildingLevel = 0;
        for (int j = 0; j < 5; j++) {
            Deeds[i].price[j] = prices[i][j];
            Deeds[i].toll[j] = tolls[i][j];
        }
    }

    for (int i = 0; i < 4; i++) { // areaNum �ʱ�ȭ, 0123: ���ʳ��
        for (int j = 0; j < each_area_num[i]; j++) {
            Deeds[j].areaNum = i;
        }
    }
}

void init_players(int player_cnt) {
    for (int i = 0; i < player_cnt; i++) {
        Players[i].position = 0;
        Players[i].money = 2430000;
    }
}

int pick_golden_key(struct player* player) {
    printf("1");
}

void draw_dice(int num, int is_second) {
    // �ֻ��� ���� ��ġ (ù ��° �ֻ����� ����, �� ��°�� ������)
    int start_x = is_second ? 67 : 47;
    int start_y = 29;

    // �ֻ��� �׵θ��� ����
    char* dice_art[5] = {
        "������������������",
        "��       ��",
        "��       ��",
        "��       ��",
        "������������������"
    };

    // �ֻ��� �׸���
    for (int i = 0; i < 5; i++) {
        gotoxy(start_x, start_y + i);
        printf("%s", dice_art[i]);
    }

    // �ֻ��� �� �׸���
    switch (num) {
    case 1:
        gotoxy(start_x + 4, start_y + 2);
        printf("��");
        break;
    case 2:
        gotoxy(start_x + 2, start_y + 1);
        printf("��");
        gotoxy(start_x + 6, start_y + 3);
        printf("��");
        break;
    case 3:
        gotoxy(start_x + 2, start_y + 1);
        printf("��");
        gotoxy(start_x + 4, start_y + 2);
        printf("��");
        gotoxy(start_x + 6, start_y + 3);
        printf("��");
        break;
    case 4:
        gotoxy(start_x + 2, start_y + 1);
        printf("��");
        gotoxy(start_x + 6, start_y + 1);
        printf("��");
        gotoxy(start_x + 2, start_y + 3);
        printf("��");
        gotoxy(start_x + 6, start_y + 3);
        printf("��");
        break;
    case 5:
        gotoxy(start_x + 2, start_y + 1);
        printf("��");
        gotoxy(start_x + 6, start_y + 1);
        printf("��");
        gotoxy(start_x + 4, start_y + 2);
        printf("��");
        gotoxy(start_x + 2, start_y + 3);
        printf("��");
        gotoxy(start_x + 6, start_y + 3);
        printf("��");
        break;
    case 6:
        gotoxy(start_x + 2, start_y + 1);
        printf("��");
        gotoxy(start_x + 6, start_y + 1);
        printf("��");
        gotoxy(start_x + 2, start_y + 2);
        printf("��");
        gotoxy(start_x + 6, start_y + 2);
        printf("��");
        gotoxy(start_x + 2, start_y + 3);
        printf("��");
        gotoxy(start_x + 6, start_y + 3);
        printf("��");
        break;
    }
}

// �ֻ��� ������ �Լ�
int roll_dice() {
    srand(time(NULL));

    // ù ��° �ֻ���
    int dice1 = rand() % 6 + 1;
    draw_dice(dice1, 0);  // ���ʿ� �׸���

    // ��� ���
    Sleep(500);

    // �� ��° �ֻ���
    int dice2 = rand() % 6 + 1;
    draw_dice(dice2, 1);  // �����ʿ� �׸���

    // �ֻ��� �հ� ǥ��
    gotoxy(45, 34);
    return dice1 + dice2;
}

void message(int x, int y, char text[60]) {
    gotoxy(x, y);
    puts(text);
}


void land_info(struct player* player) {
    int pos = player->position;
    char names[40][40] = { "���", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "���ε�", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ������� ����ó", "1", "Ȳ�ݿ���", "1","1","1","1","1","1","1","���ֿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ�������", "1" };

    gotoxy(140, 32);
    printf("���� ��ġ: ");

    if (strcmp(names[pos], "1") == 0) {
        // �Ϲ� ���� ���
        int deed_index = 0;
        for (int i = 0; i < pos; i++) {
            if (strcmp(names[i], "1") == 0) {
                deed_index++;
            }
        }

        printf("%s", Deeds[deed_index].name);
        gotoxy(140, 33);
        printf("��������: %d��", Deeds[deed_index].price[0]);
        gotoxy(140, 34);
        if (Deeds[deed_index].ownerNum == -1) {
            printf("������: ����");
        }
        else {
            printf("������: Player%d", Deeds[deed_index].ownerNum + 1);
        }
        gotoxy(140, 35);
        printf("�ǹ� ����: %d", Deeds[deed_index].buildingLevel);
    }
    else {
        // Ư�� ĭ�� ���
        printf("%s", names[pos]);
        if (strcmp(names[pos], "Ȳ�ݿ���") == 0) {
            gotoxy(140, 33);
            printf("Ȳ�ݿ��踦 �̽��ϴ�.");
        }
    }
}

void my_turn(struct player* player) {
    char key;
    int dice_value;

    while (1) {
        message(140, 31, "�� ����[Space] | �ֻ��� ������[k]");
        key = _getch();

        if (key == ' ') {  // �����̽���
            system("cls");
            draw_board();
            land_info(player);
        }
        else if (key == 'k') {  // kŰ
            gotoxy(140, 31);
            printf("                                     ");  // ���� �޽��� �����

            dice_value = roll_dice();
            gotoxy(35, 26);
            printf("�ֻ��� ��: %d", dice_value);

            // �� �̵�
            player->position = (player->position + dice_value) % 40;
            break;
        }
        else {
            message(140, 32, "�ùٸ��� ���� Ű�Դϴ�!");
            Sleep(1000);
            gotoxy(140, 32);
            printf("                          ");  // ���� �޽��� �����
        }
    }
}

void game_start() {
    srand(time(NULL));
    system("cls");
    int player_cnt;
    char key;

    init_deeds();
    draw_board();


    player_cnt = 2;
    // �÷��̾� �ʱ�ȭ
    init_players(player_cnt);

    // �ֻ��� ������ �� ���� ���ϱ�
    int dice_values[4] = { 0 };  // �� �÷��̾��� �ֻ��� �� ����
    int turn_order[4] = { 0, 1, 2, 3 };  // �� ���� ����

    system("cls");
    draw_board();

    // �� �÷��̾ �ֻ��� ������
    for (int i = 0; i < player_cnt; i++) {
        gotoxy(140, 30);
        printf("Player%d �ֻ��� ������ [k]", i + 1);

        while (1) {
            key = _getch();
            if (key == 'k') {
                dice_values[i] = roll_dice();
                gotoxy(35, 26);
                printf("Player%d�� �ֻ���: %d", i + 1, dice_values[i]);
                Sleep(1000);  // ��� ���
                break;
            }
        }
        system("cls");
        draw_board();
    }

    // �ֻ��� ���� ���� �� ���� ���� (���� ����)
    for (int i = 0; i < player_cnt - 1; i++) {
        for (int j = 0; j < player_cnt - 1 - i; j++) {
            if (dice_values[j] < dice_values[j + 1]) {
                // �ֻ��� �� ��ȯ
                int temp = dice_values[j];
                dice_values[j] = dice_values[j + 1];
                dice_values[j + 1] = temp;

                // �� ���� ��ȯ
                temp = turn_order[j];
                turn_order[j] = turn_order[j + 1];
                turn_order[j + 1] = temp;
            }
        }
    }

    // �� ���� ���
    message(140, 30, "�� ������ ���������ϴ�!");
    for (int i = 0; i < player_cnt; i++) {
        gotoxy(140, 32 + i);
        printf("%d��° ��: Player%d (�ֻ���:%d)", i + 1, turn_order[i] + 1, dice_values[i]);
    }
    Sleep(3000);  // 3�� ���

    // ���� ����
    int current_turn = 0;
    while (1) {
        system("cls");
        draw_board();

        int current_player = turn_order[current_turn];
        gotoxy(140, 30);
        printf("Player%d�� ���Դϴ�.", current_player + 1);

        my_turn(&Players[current_player]);

        current_turn = (current_turn + 1) % player_cnt;  // ���� ������
        Sleep(1000);  // 1�� ���
    }
}

void set_fullscreen() {
    system("mode con: cols=1000 lines=80");  // ������ cmd â ũ�⸦ ���� (��üȭ��ó�� ���̰�)
}

int main() {
    set_fullscreen();
    game_start();
    system("pause");
    gotoxy(60, 60);
}


/*
void buy_land(struct player* player, struct deed* land) {
    if (land->ownerNum == -1) { // �����ڰ� ���� ���� ���� ����
        if (player->money >= land->price[0]) {
            player->money -= land->price[0];
            land->ownerNum = player - player;
            printf("%s�� %s�� �����߽��ϴ�!\n", player->name, land->name);
        }
        else {
            printf("�ڱ��� �����մϴ�!\n");
        }
    }
    else {
        printf("�̹� �ٸ� �÷��̾ �����ϰ� �ֽ��ϴ�.\n");
    }
}

void pay_toll(struct player* player, struct deed* land) {
    if (land->ownerNum != player - player && land->ownerNum != -1) {
        int toll = land->toll[land->buildingLevel];
        player->money -= toll;
        player[land->ownerNum].money += toll;
        printf("%s�� %s�� ����� %d���� �����߽��ϴ�!\n", player->name, land->name, toll);
    }
}*/
