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

void textColor(int colorNum);
void gotoxy(int x, int y);
void printInCell(int x, int y, char* text, int price);
void draw_board();
struct deed* init_deeds();
struct player* init_players(int player_cnt);
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
struct player Players[4];

void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

void gotoxy(int x, int y) {
    COORD pos = { x, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void printInCell(int x, int y, char* text, int price) {
    char line[2][10 + 1] = { "" };  // �ٹٲ޵� �ؽ�Ʈ�� ������ �迭
    int len = strlen(text);                        // �ؽ�Ʈ ����
    int currentLine = 0, currentChar = 0;          // ���� �ٰ� ���� ��ġ

    if (!strcmp(text, "Ȳ�ݿ���")) {
        textColor(6);
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
        printf("%s", line[i]);
    }

    // �ݾ� ��� (ĭ�� ������ �ٿ� ��ġ)
    if(price) {
        gotoxy(x, y + 3);
        printf("%d��", price);
    }
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

    for (int i = 0; i < 7; i++) {

    }


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

    int key_x=47, key_y=40;
    for (int i = 0; i < 7; i++) {
        gotoxy(key_x, key_y + i);
        puts(key_art[i]);
    }


    // (x,y)���� ����
    // ���������� ��ĭ : (x+11, y)
    // �Ʒ������� ��ĭ : (x, y+5)
    
    char names[40][40] = { "���      <---", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "���ε�", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ������� ����ó", "1", "Ȳ�ݿ���", "1","1","1","1","1","1","1","���ֿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ�������", "1"};
    int cnt = 0;
    int curr_x = 111, curr_y = 51;
    for (int i = 0; i < 10; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0);
            cnt++;
            curr_x -= 11;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0);
            curr_x -= 11;
        }
    }

    for (int i = 10; i < 20; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0);
            cnt++;
            curr_y -= 5;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0);
            curr_y -= 5;
        }
    }

    for (int i = 20; i < 30; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0);
            cnt++;
            curr_x += 11;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0);
            curr_x += 11;
        }
    }

    for (int i = 30; i < 40; i++) {
        if (!strcmp(names[i], "1")) {
            printInCell(curr_x, curr_y, Deeds[cnt].name, 0);
            cnt++;
            curr_y += 5;
        }
        else {
            printInCell(curr_x, curr_y, names[i], 0);
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

    

}


struct deed* init_deeds() {
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

    return Deeds;
}

struct player* init_players(int player_cnt) {
    for (int i = 0; i < player_cnt; i++) {
        Players[i].position = 0;
        Players[i].money = 2430000;
    }
    return Players;
}

int pick_golden_key(struct player* player) {
    printf("1");
}

int roll_dice() {
    int dice = (rand() % 6) + 1;

    return dice;
}
void message(int x, int y, char text[60]) {
    gotoxy(x, y);
    puts(text);
}

void game_start() {
    srand(time(NULL));
    system("cls");
    int player_cnt;
    int curr_turn = 0;
    char key;
    int curr_arrow_pos = 0;

    Sleep(10);
    init_deeds();
    char names[40][40] = { "���      <---", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "���ε�", "1", "Ȳ�ݿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ������� ����ó", "1", "Ȳ�ݿ���", "1","1","1","1","1","1","1","���ֿ���", "1","1","1","1", "Ȳ�ݿ���", "1","1", "��ȸ�������", "1" };

    draw_board();

    ////////////player_cnt��  �� �� �����ߴ��� �������� �ޱ�
    message(140, 30, "����̼� �Ծ�?");
    message(140, 31, "[2] [3] [4]");
    while (1) {
        key = _getch();
        player_cnt = key - 48;
        if (!(player_cnt >= 2 && player_cnt <= 4)) {
            message(140, 32, "�ùٸ��� ����!");
        }
        else {
            printf("%d", player_cnt);
            break;
        }
    }
    /////////////////////////////

    struct player * Players = init_players(player_cnt);

    system("cls");
    draw_board();

    int my_color = 0; // 0 : �� | 1 : �� | 2: �� | 3 : ��
    int dice1, dice2;

    while (1) {
        if (curr_turn == my_color) {
            message(140, 30, "�ֻ��� ������ [k]");
            key = _getch();
            if (key == 'k') {
                dice1 = roll_dice(); // �ֻ��� ���� �� ���� ū ���ڰ� ù��° ��
                                    // ���� ������ ��
                dice2 = roll_dice();
                gotoxy(35, 26);
                printf("%d %d", dice1, dice2);

                break;
            }
            else {
                message(140, 32, "�ùٸ��� ���� Ű");
            }
        }
        else {
            message(140, 30, "�ڽ��� ���� �� �� ���� ��ٷ��ּ���");
        }
    }
    

    /*
    *
    * ���� �� ���� �Ǹ� �� �Ͽ� ���� �ֻ��� ������
    * 
    * �������� ���� �ٲ�� �����ָ�, �ڱ� ��ȣ�� üũ�ؼ� ��ġ�ϸ� �ֻ��� ������ �ϰ�, �ƴϸ� ��ٸ����ϱ�
    */

    //system("cls");
    //draw_board();

    curr_turn = 0;

    while (1) {
        
        gotoxy(140, 30);
        printf("���� Player%d���� �����Դϴ�.", curr_turn);
        if (GetAsyncKeyState(VK_LEFT)) {
            if (0 <= curr_arrow_pos && curr_arrow_pos <= 9) {
                curr_arrow_pos += 1;
            }
            draw_board();
        }








        if (curr_turn == my_color) {
            message(140, 31, "�ֻ��� ������ [k]");
            key = _getch();
            if (key == 'k') {
                dice1 = roll_dice(); // �ֻ��� ���� �� ���� ū ���ڰ� ù��° ��
                dice2 = roll_dice();
                // ���� ������ ��
                gotoxy(35, 26);
                printf("%d %d", dice1, dice2);
                
                // ���� �Ŀ��� �� ����
                curr_turn += 1; // �ӽ�
                if (curr_turn == 4) curr_turn = 0;
                // 
                break;
            }
            else {
                message(140, 32, "�ùٸ��� ���� Ű");
            }
        }
        else {
            message(140, 30, "�ڽ��� ���� �� �� ���� ��ٷ��ּ���");
        }

        
    }

    
    
    
    /*
    while (1) {
        struct Player* current = &Players[turn];
        printf("\n%s�� �����Դϴ�.\n", current->name);

        int dice = roll_dice();
        printf("�ֻ����� ���� %d�� ���Խ��ϴ�!\n", dice);

        current->position = (current->position + dice) % 1000;
        struct deed* currentLand = &Deeds[current->position];
        printf("%s�� %s�� �����߽��ϴ�.\n", current->name, currentLand->name);

        if (currentLand->ownerNum == -1) {
            printf("%s�� �����Ͻðڽ��ϱ�? (1: ��, 0: �ƴϿ�): ", currentLand->name);
            int choice;
            scanf("%d", &choice);
            if (choice == 1) {
                buy_land(current, currentLand);
            }
        }
        else {
            pay_toll(current, currentLand);
        }

        if (current->money <= 0) {
            printf("%s�� �Ļ��߽��ϴ�! ���� ����.\n", current->name);
            break;
        }

        turn = (turn + 1) % 4;
    }*/

    gotoxy(60, 60);
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
