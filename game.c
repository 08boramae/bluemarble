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
    char line[2][10 + 1] = { "" };  // 還夥翎脹 臢蝶お蒂 盪濰й 寡翮
    int len = strlen(text);                        // 臢蝶お 望檜
    int currentLine = 0, currentChar = 0;          // ⑷營 還婁 僥濠 嬪纂

    if (!strcmp(text, "�盛暆乘�")) {
        textColor(6);
    }
    else if (!strcmp(text, "轎嫦      <---")) {
        textColor(RED);
    }
    else if (!strcmp(text, "鼠檣紫")) {
        textColor(GREEN);
    }
    else if (!strcmp(text, "餌�蛹嘗鰡漹� 熱滄籀")) {
        textColor(6);
    }
    else if (!strcmp(text, "辦輿罹ч")) {
        textColor(13);
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
        printf("%s\n", line[i]);
    }
    //if (Players[0].position == idx) {
    //    textColor(RED);
    //    printf("≒");
    //}
    //if (Players[1].position == idx) {
    //    textColor(BLUE);
    //    printf("≒");
    //}
    //textColor(WHITE);

    // 旎擋 轎溘 (蘊曖 葆雖虞 還縑 寡纂)
    if(price) {
        gotoxy(x, y + 3);
        printf("%d錳", price);
    }
    textColor(WHITE);


    // Ы溯檜橫 蜓 轎溘
}

void show_player_status() {
    gotoxy(140, 15);
    textColor(RED);
    printf("Player1 薑爾");
    gotoxy(140, 16);
    printf("嬪纂: %d", Players[0].position);
    gotoxy(140, 17);
    printf("濠旎: %d錳", Players[0].money);

    gotoxy(140, 19);
    textColor(BLUE);
    printf("Player2 薑爾");
    gotoxy(140, 20);
    printf("嬪纂: %d", Players[1].position);
    gotoxy(140, 21);
    printf("濠旎: %d錳", Players[1].money);

    textColor(15);
}

void draw_players() {
    char* player1_marker = "≒";
    char* player2_marker = "≒";
    int p1_x, p1_y, p2_x, p2_y;

    // Player1 嬪纂 啗骯 (說除儀)
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

    // Player2 嬪纂 啗骯 (だ塢儀)
    if (0 <= Players[1].position && Players[1].position <= 9) {
        p2_x = 111 - (Players[1].position * 11) + 2;  // Player1爾棻 擒除 螃艇薹縑 嬪纂
        p2_y = 52;
    }
    else if (10 <= Players[1].position && Players[1].position <= 19) {
        p2_x = 1;
        p2_y = 51 - ((Players[1].position - 10) * 5) + 1;  // Player1爾棻 擒除 嬴楚縑 嬪纂
    }
    else if (20 <= Players[1].position && Players[1].position <= 29) {
        p2_x = 1 + ((Players[1].position - 20) * 11) + 2;
        p2_y = 1;
    }
    else if (30 <= Players[1].position && Players[1].position <= 39) {
        p2_x = 112;
        p2_y = 1 + ((Players[1].position - 30) * 5) + 1;
    }

    // Player1 (說除儀) ル衛
    gotoxy(p1_x, p1_y);
    textColor(RED);
    printf("%s", player1_marker);

    // Player2 (だ塢儀) ル衛
    gotoxy(p2_x, p2_y);
    textColor(BLUE);
    printf("%s", player2_marker);

    // 儀鼻 錳楚渠煎
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

    textColor(YELLOW);
    int key_x=47, key_y=40;
    for (int i = 0; i < 7; i++) {
        gotoxy(key_x, key_y + i);
        puts(key_art[i]);
    }
    textColor(15);


    // (x,y)縑憮 衛濛
    // 螃艇薹戲煎 и蘊 : (x+11, y)
    // 嬴楚薹戲煎 и蘊 : (x, y+5)
    
    char names[40][40] = { "轎嫦      <---", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "鼠檣紫", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹� 熱滄籀", "1", "�盛暆乘�", "1","1","1","1","1","1","1","辦輿罹ч", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹�", "1"};
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


    show_player_status();
    

}


void init_deeds() {
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
    // 輿餌嬪 衛濛 嬪纂 (羅 廓簞 輿餌嬪朝 豭薹, 舒 廓簞朝 螃艇薹)
    int start_x = is_second ? 67 : 47;
    int start_y = 29;

    // 輿餌嬪 纔舒葬諦 頂睡
    char* dice_art[5] = {
        "忙式式式式式式式忖",
        "弛       弛",
        "弛       弛",
        "弛       弛",
        "戌式式式式式式式戎"
    };

    // 輿餌嬪 斜葬晦
    for (int i = 0; i < 5; i++) {
        gotoxy(start_x, start_y + i);
        printf("%s", dice_art[i]);
    }

    // 輿餌嬪 換 斜葬晦
    switch (num) {
    case 1:
        gotoxy(start_x + 4, start_y + 2);
        printf("≒");
        break;
    case 2:
        gotoxy(start_x + 2, start_y + 1);
        printf("≒");
        gotoxy(start_x + 6, start_y + 3);
        printf("≒");
        break;
    case 3:
        gotoxy(start_x + 2, start_y + 1);
        printf("≒");
        gotoxy(start_x + 4, start_y + 2);
        printf("≒");
        gotoxy(start_x + 6, start_y + 3);
        printf("≒");
        break;
    case 4:
        gotoxy(start_x + 2, start_y + 1);
        printf("≒");
        gotoxy(start_x + 6, start_y + 1);
        printf("≒");
        gotoxy(start_x + 2, start_y + 3);
        printf("≒");
        gotoxy(start_x + 6, start_y + 3);
        printf("≒");
        break;
    case 5:
        gotoxy(start_x + 2, start_y + 1);
        printf("≒");
        gotoxy(start_x + 6, start_y + 1);
        printf("≒");
        gotoxy(start_x + 4, start_y + 2);
        printf("≒");
        gotoxy(start_x + 2, start_y + 3);
        printf("≒");
        gotoxy(start_x + 6, start_y + 3);
        printf("≒");
        break;
    case 6:
        gotoxy(start_x + 2, start_y + 1);
        printf("≒");
        gotoxy(start_x + 6, start_y + 1);
        printf("≒");
        gotoxy(start_x + 2, start_y + 2);
        printf("≒");
        gotoxy(start_x + 6, start_y + 2);
        printf("≒");
        gotoxy(start_x + 2, start_y + 3);
        printf("≒");
        gotoxy(start_x + 6, start_y + 3);
        printf("≒");
        break;
    }
}

// 輿餌嬪 掉葬晦 л熱
int roll_dice() {
    srand(time(NULL));

    // 羅 廓簞 輿餌嬪
    int dice1 = rand() % 6 + 1;
    draw_dice(dice1, 0);  // 豭薹縑 斜葬晦

    // 濡衛 渠晦
    Sleep(500);

    // 舒 廓簞 輿餌嬪
    int dice2 = rand() % 6 + 1;
    draw_dice(dice2, 1);  // 螃艇薹縑 斜葬晦

    // 輿餌嬪 м啗 ル衛
    gotoxy(45, 34);
    return dice1 + dice2;
}

void message(int x, int y, char text[60]) {
    gotoxy(x, y);
    puts(text);
}


void land_info(struct player* player) {
    int pos = player->position;
    char names[40][40] = { "轎嫦", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "鼠檣紫", "1", "�盛暆乘�", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹� 熱滄籀", "1", "�盛暆乘�", "1","1","1","1","1","1","1","辦輿罹ч", "1","1","1","1", "�盛暆乘�", "1","1", "餌�蛹嘗鰡漹�", "1" };

    gotoxy(140, 32);
    printf("⑷營 嬪纂: ");

    if (strcmp(names[pos], "1") == 0) {
        // 橾奩 階檣 唳辦
        int deed_index = 0;
        for (int i = 0; i < pos; i++) {
            if (strcmp(names[i], "1") == 0) {
                deed_index++;
            }
        }

        printf("%s", Deeds[deed_index].name);
        gotoxy(140, 33);
        printf("饜雖陛問: %d錳", Deeds[deed_index].price[0]);
        gotoxy(140, 34);
        if (Deeds[deed_index].ownerNum == -1) {
            printf("模嶸濠: 橈擠");
        }
        else {
            printf("模嶸濠: Player%d", Deeds[deed_index].ownerNum + 1);
        }
        gotoxy(140, 35);
        printf("勒僭 溯漣: %d", Deeds[deed_index].buildingLevel);
    }
    else {
        // か熱 蘊檣 唳辦
        printf("%s", names[pos]);
        if (strcmp(names[pos], "�盛暆乘�") == 0) {
            gotoxy(140, 33);
            printf("�盛暆乘頦� 鉻蝗棲棻.");
        }
    }
}

void my_turn(struct player* player) {
    char key;
    int dice_value;

    while (1) {
        message(140, 31, "階 薑爾[Space] | 輿餌嬪 掉葬晦[k]");
        key = _getch();

        if (key == ' ') {  // 蝶む檜蝶夥
            system("cls");
            draw_board();
            land_info(player);
        }
        else if (key == 'k') {  // k酈
            gotoxy(140, 31);
            printf("                                     ");  // 檜瞪 詭衛雖 雖辦晦

            dice_value = roll_dice();
            gotoxy(35, 26);
            printf("輿餌嬪 高: %d", dice_value);

            // 蜓 檜翕
            player->position = (player->position + dice_value) % 40;
            break;
        }
        else {
            message(140, 32, "螢夥腦雖 彊擎 酈殮棲棻!");
            Sleep(1000);
            gotoxy(140, 32);
            printf("                          ");  // 縑楝 詭衛雖 雖辦晦
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
    // Ы溯檜橫 蟾晦��
    init_players(player_cnt);

    // 輿餌嬪 掉溥憮 欐 牖憮 薑ж晦
    int dice_values[4] = { 0 };  // 陝 Ы溯檜橫曖 輿餌嬪 高 盪濰
    int turn_order[4] = { 0, 1, 2, 3 };  // 欐 牖憮 盪濰

    system("cls");
    draw_board();

    // 陝 Ы溯檜橫陛 輿餌嬪 掉葬晦
    for (int i = 0; i < player_cnt; i++) {
        gotoxy(140, 30);
        printf("Player%d 輿餌嬪 掉葬晦 [k]", i + 1);

        while (1) {
            key = _getch();
            if (key == 'k') {
                dice_values[i] = roll_dice();
                gotoxy(35, 26);
                printf("Player%d曖 輿餌嬪: %d", i + 1, dice_values[i]);
                Sleep(1000);  // 濡衛 渠晦
                break;
            }
        }
        system("cls");
        draw_board();
    }

    // 輿餌嬪 高縑 評塭 欐 牖憮 薑溺 (幗綰 薑溺)
    for (int i = 0; i < player_cnt - 1; i++) {
        for (int j = 0; j < player_cnt - 1 - i; j++) {
            if (dice_values[j] < dice_values[j + 1]) {
                // 輿餌嬪 高 掖��
                int temp = dice_values[j];
                dice_values[j] = dice_values[j + 1];
                dice_values[j + 1] = temp;

                // 欐 牖憮 掖��
                temp = turn_order[j];
                turn_order[j] = turn_order[j + 1];
                turn_order[j + 1] = temp;
            }
        }
    }

    // 欐 牖憮 轎溘
    message(140, 30, "欐 牖憮陛 薑п螺蝗棲棻!");
    for (int i = 0; i < player_cnt; i++) {
        gotoxy(140, 32 + i);
        printf("%d廓簞 欐: Player%d (輿餌嬪:%d)", i + 1, turn_order[i] + 1, dice_values[i]);
    }
    Sleep(3000);  // 3蟾 渠晦

    // 啪歜 衛濛
    int current_turn = 0;
    while (1) {
        system("cls");
        draw_board();

        int current_player = turn_order[current_turn];
        gotoxy(140, 30);
        printf("Player%d曖 欐殮棲棻.", current_player + 1);

        my_turn(&Players[current_player]);

        current_turn = (current_turn + 1) % player_cnt;  // 棻擠 欐戲煎
        Sleep(1000);  // 1蟾 渠晦
    }
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
