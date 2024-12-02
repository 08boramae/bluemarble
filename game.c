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
    char name[40];      // 饜雖 檜葷
    int areaNum;        // 說蟾喻匐 掘羲
    int ownerNum;       // 0: 橈擠
    int buildingLevel;  // 勒僭 溯漣
    int price[5];       // 渠雖, 滌濰1, 滌濰2, 網註, ��蘿 陛問
    int toll[5];        // 鱔ч猿
    int isCanBuild;     // 勒撲 陛棟 罹睡
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
    char line[2][10 + 1] = { "" };  // 還夥翎脹 臢蝶お蒂 盪濰й 寡翮
    int len = strlen(text);                        // 臢蝶お 望檜
    int currentLine = 0, currentChar = 0;          // ⑷營 還婁 僥濠 嬪纂

    if (!strcmp(text, "�盛暆乘�")) {
        textColor(6);
    }

    // 僥濠翮擊 蘊 傘綠縑 蜃啪 釭援晦
    for (int i = 0; i < len; i++) {
        line[currentLine][currentChar++] = text[i];
        if (currentChar == 10 || i == len - 1) {
            line[currentLine][currentChar] = '\0'; // 還 部縑 NULL 蹺陛
            currentLine++;
            currentChar = 0;
            if (currentLine >= 2) break;   // 譆渠 還 熱 蟾婁 衛 醞欽
        }
    }

    // 釭換 臢蝶お 轎溘 (蘊 頂睡縑 蜃醮 轎溘)
    for (int i = 0; i < currentLine; i++) {
        if (strlen(text) <= 4 || !price) {
            gotoxy(x, y + i+ 1);
        }
        else {
            gotoxy(x, y + i);                        // 陝 還曖 嬪纂 撲薑
        }
        printf("%s", line[i]);
    }

    // 旎擋 轎溘 (蘊曖 葆雖虞 還縑 寡纂)
    if(price) {
        gotoxy(x, y + 3);
        printf("%d錳", price);
    }
    textColor(15);
}


void draw_board() {
    //裔 嶺塭檣
    gotoxy(0, 0);
    puts("忙式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式成式式式式式式式式式式忖");
    for (int i = 0; i < 4; i++) {
        for (int i = 0; i < 11; i++) {
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
        for (int i = 0; i < 11; i++) {
            printf("弛          ");
        }
        puts("弛");
    }
    puts("戌式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式扛式式式式式式式式式式戎");

    for (int i = 0; i < 7; i++) {

    }


    char title_art[12][150] = {
        //"    ﹥﹥          ﹥﹥      ﹥﹥﹥﹥﹥﹥﹥﹥                      ﹥        ﹥            ﹥    ",
        //"    ﹥﹥          ﹥﹥                  ﹥﹥      ﹥﹥﹥﹥﹥﹥    ﹥        ﹥﹥﹥﹥﹥﹥﹥﹥    ",
        //"    ﹥﹥﹥﹥﹥﹥﹥﹥﹥      ﹥﹥﹥﹥﹥﹥﹥﹥      ﹥        ﹥    ﹥        ﹥            ﹥    ",
        //"    ﹥﹥          ﹥﹥      ﹥﹥                  ﹥        ﹥    ﹥        ﹥﹥﹥﹥﹥﹥﹥﹥    ",
        //"    ﹥﹥﹥﹥﹥﹥﹥﹥﹥      ﹥﹥                  ﹥        ﹥    ﹥﹥﹥﹥                      ",
        //"                            ﹥﹥﹥﹥﹥﹥﹥﹥﹥    ﹥        ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥",
        //"                                                  ﹥        ﹥    ﹥              ﹥﹥          ",
        //"﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥﹥  ﹥        ﹥    ﹥        ﹥﹥﹥﹥﹥﹥﹥﹥    ",
        //"          ﹥﹥                    ﹥﹥            ﹥﹥﹥﹥﹥﹥    ﹥                      ﹥    ",
        //"          ﹥﹥                    ﹥﹥                            ﹥        ﹥﹥﹥﹥﹥﹥﹥﹥    ",
        //"          ﹥﹥                    ﹥﹥                            ﹥        ﹥                  ",
        //"          ﹥﹥                    ﹥﹥                            ﹥        ﹥﹥﹥﹥﹥﹥﹥﹥﹥  "
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
        "      ﹥﹥          ﹥﹥               ﹥    ﹥﹥﹥﹥﹥﹥﹥﹥﹥ ",

    }; 

    int title_x=37, title_y=10;
    for (int i = 0; i < 12; i++) {
        gotoxy(title_x, title_y+i);
        puts(title_art[i]);
    }

    char key_art[7][100] = {
        "忙式式式式式式式式式式式式式式式式式式式式式式式式式式式式忖",
        "弛  .---.                     弛",
        "弛 /    |\\________________    弛",
        "弛 | ()  | ________   _   _)  弛",
        "弛 \\    |/        | | | |     弛",
        "弛  `---'         \" - \" |_|   弛",
        "戌式式式式式式式式式式式式式式式式式式式式式式式式式式式式戎"
    };

    int key_x=47, key_y=40;
    for (int i = 0; i < 7; i++) {
        gotoxy(key_x, key_y + i);
        puts(key_art[i]);
    }


    // (x,y)縑憮 衛濛
    // 螃艇薹戲煎 и蘊 : (x+11, y)
    // 嬴楚薹戲煎 и蘊 : (x, y+5)
    
    char names[40][40] = { "轎嫦      <---", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "鼠檣紫", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹� 熱滄籀", "1", "�盛暆乘�", "1","1","1","1","1","1","1","辦輿罹ч", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹�", "1"};
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
    curr_x = 34, curr_y = 25;
    for (int i = 0; i < 13; i++) {
        gotoxy(curr_x, curr_y + i);
        puts(dice_board_art[i]);
    }

    

}


struct deed* init_deeds() {
    char names[29][40] = {
    "顫檜漆檜", "漆檜癒", "葆棍塭", "薯輿紫", "諒陛ん腦", "蘋檜煎", "檜蝶驕碳",
    "嬴啻纔", "囀もж啦", "蝶驚�次�", "攝囀萄 罹偌晦", "漆艇", "漆蒂萼", "螃顫諦",
    "睡縑喻蝶 嬴檜溯蝶", "鼻だ選瑞", "衛萄棲", "睡骯", "ж諦檜", "葬蝶獄", "覽 縣葬濠漆蝶 ��", "葆萄葬萄",
    "紫瓖", "鏽歲綠嬴��", "だ葬", "煎葆", "楛湍", "景踹", "憮選"
    };
    int each_area_num[4] = { 7, 7, 8, 7 };
    int ownerNum = 0;
    int buildingLevel = 0;
    int prices[29][5] = { // 掘衙陛 : 饜雖, 滌濰, 滌濰2, 網註, ��蘿
        {50000, 50000, 100000, 150000, 250000}, {80000, 50000, 100000, 150000, 250000}, {80000, 50000, 100000, 150000, 250000}, {200000}, {100000, 50000, 100000, 150000, 250000}, {100000, 50000, 100000, 150000, 250000}, {120000, 50000, 100000, 150000, 250000},
        {140000, 100000, 200000, 300000, 500000}, {160000, 100000, 200000, 300000, 500000}, {160000, 100000, 200000, 300000, 500000}, {200000}, {180000, 100000, 200000, 300000, 500000}, {180000, 100000, 200000, 300000, 500000}, {200000, 100000, 200000, 300000, 500000},
        {220000, 150000, 300000, 400000, 750000}, {240000, 150000, 300000, 450000, 750000}, {240000, 150000, 300000, 450000, 750000}, {500000}, {260000, 150000, 300000, 450000, 750000}, {260000, 150000, 300000, 450000, 750000}, {300000}, {280000, 150000, 300000, 450000, 750000},
        {300000, 200000, 400000, 600000, 1000000}, {450000}, {320000, 200000, 400000, 600000, 1000000}, {320000, 200000, 400000, 600000, 1000000}, {350000, 200000, 400000, 600000, 1000000}, {350000, 200000, 400000, 600000, 1000000}, {1000000}
    };
    int tolls[29][5] = { // 鱔ч猿: 饜雖, 滌濰, 滌濰2, 網註, ��蘿
        {2000, 10000, 30000, 90000, 250000}, {4000, 20000, 60000, 180000, 450000}, {4000, 20000, 60000, 180000, 450000}, {300000}, {6000, 30000, 90000, 270000, 550000}, {6000, 30000, 90000, 270000, 550000}, {8000, 40000, 100000, 300000, 600000},
        {10000, 50000, 150000, 450000, 750000}, {12000, 60000, 180000, 500000, 900000}, {12000, 60000, 180000, 500000, 900000}, {300000}, {14000, 70000, 200000, 550000, 950000}, {14000, 70000, 200000, 550000, 950000}, {16000, 80000, 220000, 600000, 1000000},
        {180000, 90000, 250000, 700000, 1050000}, {20000, 100000, 300000, 750000, 1100000}, {20000, 100000, 300000, 750000, 1100000}, {600000}, {22000, 110000, 330000, 800000, 1150000}, {22000, 110000, 330000, 800000, 1150000}, {250000}, {24000, 120000, 360000, 850000, 1200000},
        {26000, 130000, 390000, 900000, 1270000}, {300000}, {28000, 150000, 450000, 1000000, 1400000}, {28000, 150000, 450000, 1000000, 1400000}, {35000, 170000, 500000, 1100000, 1500000}, {35000, 170000, 500000, 1100000, 1500000}, {2000000}
    };

    for (int i = 0; i < 29; i++) { // Deeds init
        strncpy(Deeds[i].name, names[i], sizeof(Deeds[i].name));
        Deeds[i].ownerNum = -1; // -1 : 模嶸濠X
        Deeds[i].buildingLevel = 0;
        for (int j = 0; j < 5; j++) {
            Deeds[i].price[j] = prices[i][j];
            Deeds[i].toll[j] = tolls[i][j];
        }
    }

    for (int i = 0; i < 4; i++) { // areaNum 蟾晦��, 0123: 說蟾喻匐
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
    char names[40][40] = { "轎嫦      <---", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "鼠檣紫", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹� 熱滄籀", "1", "�盛暆乘�", "1","1","1","1","1","1","1","辦輿罹ч", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹�", "1" };

    draw_board();

    ////////////player_cnt縑  賃 貲 霤陛ц朝雖 憮幗縑憮 嫡晦
    message(140, 30, "賃貲檜憮 諮橫?");
    message(140, 31, "[2] [3] [4]");
    while (1) {
        key = _getch();
        player_cnt = key - 48;
        if (!(player_cnt >= 2 && player_cnt <= 4)) {
            message(140, 32, "螢夥腦雖 彊擠!");
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

    int my_color = 0; // 0 : 說 | 1 : だ | 2: 蟾 | 3 : 喻
    int dice1, dice2;

    while (1) {
        if (curr_turn == my_color) {
            message(140, 30, "輿餌嬪 掉葬晦 [k]");
            key = _getch();
            if (key == 'k') {
                dice1 = roll_dice(); // 輿餌嬪 掉萼 �� 薯橾 贖 璋濠陛 羅廓簞 欐
                                    // 薑爾 爾頂賊 脾
                dice2 = roll_dice();
                gotoxy(35, 26);
                printf("%d %d", dice1, dice2);

                break;
            }
            else {
                message(140, 32, "螢夥腦雖 彊擎 酈");
            }
        }
        else {
            message(140, 30, "濠褐曖 欐檜 螢 陽 梱雖 晦棻溥輿撮蹂");
        }
    }
    

    /*
    *
    * 欐檜 棻 唸薑 腎賊 陝 欐縑 評塭憮 輿餌嬪 掉葬晦
    * 
    * 憮幗縑憮 欐檜 夥莎勘 爾頂輿賊, 濠晦 廓�ㄥ� 羹觼п憮 橾纂ж賊 輿餌嬪 掉葬啪 ж堅, 嬴棲賊 晦棻葬啪ж晦
    */

    //system("cls");
    //draw_board();

    curr_turn = 0;

    while (1) {
        
        gotoxy(140, 30);
        printf("⑷營 Player%d椒曖 離滔殮棲棻.", curr_turn);
        if (GetAsyncKeyState(VK_LEFT)) {
            if (0 <= curr_arrow_pos && curr_arrow_pos <= 9) {
                curr_arrow_pos += 1;
            }
            draw_board();
        }








        if (curr_turn == my_color) {
            message(140, 31, "輿餌嬪 掉葬晦 [k]");
            key = _getch();
            if (key == 'k') {
                dice1 = roll_dice(); // 輿餌嬪 掉萼 �� 薯橾 贖 璋濠陛 羅廓簞 欐
                dice2 = roll_dice();
                // 薑爾 爾頂賊 脾
                gotoxy(35, 26);
                printf("%d %d", dice1, dice2);
                
                // 掉萼 �醴●� 欐 滲唳
                curr_turn += 1; // 歜衛
                if (curr_turn == 4) curr_turn = 0;
                // 
                break;
            }
            else {
                message(140, 32, "螢夥腦雖 彊擎 酈");
            }
        }
        else {
            message(140, 30, "濠褐曖 欐檜 螢 陽 梱雖 晦棻溥輿撮蹂");
        }

        
    }

    
    
    
    /*
    while (1) {
        struct Player* current = &Players[turn];
        printf("\n%s曖 離滔殮棲棻.\n", current->name);

        int dice = roll_dice();
        printf("輿餌嬪蒂 掉溥 %d陛 釭諮蝗棲棻!\n", dice);

        current->position = (current->position + dice) % 1000;
        struct deed* currentLand = &Deeds[current->position];
        printf("%s陛 %s縑 紫雜ц蝗棲棻.\n", current->name, currentLand->name);

        if (currentLand->ownerNum == -1) {
            printf("%s蒂 掘衙ж衛啊蝗棲梱? (1: 蕨, 0: 嬴棲螃): ", currentLand->name);
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
            printf("%s陛 だ骯ц蝗棲棻! 啪歜 謙猿.\n", current->name);
            break;
        }

        turn = (turn + 1) % 4;
    }*/

    gotoxy(60, 60);
}
void set_fullscreen() {
    system("mode con: cols=1000 lines=80");  // 孺紫辦 cmd 璽 觼晦蒂 撲薑 (瞪羹�飛橨雪� 爾檜啪)
}

int main() {
    set_fullscreen();
    game_start();
    system("pause");
    gotoxy(60, 60);
}


/*
void buy_land(struct player* player, struct deed* land) {
    if (land->ownerNum == -1) { // 模嶸濠陛 橈擊 陽虜 掘衙 陛棟
        if (player->money >= land->price[0]) {
            player->money -= land->price[0];
            land->ownerNum = player - player;
            printf("%s陛 %s蒂 掘衙ц蝗棲棻!\n", player->name, land->name);
        }
        else {
            printf("濠旎檜 睡褶м棲棻!\n");
        }
    }
    else {
        printf("檜嘐 棻艇 Ы溯檜橫陛 模嶸ж堅 氈蝗棲棻.\n");
    }
}

void pay_toll(struct player* player, struct deed* land) {
    if (land->ownerNum != player - player && land->ownerNum != -1) {
        int toll = land->toll[land->buildingLevel];
        player->money -= toll;
        player[land->ownerNum].money += toll;
        printf("%s陛 %s曖 鱔ч猿 %d錳擊 雖碳ц蝗棲棻!\n", player->name, land->name, toll);
    }
}*/
