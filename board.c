#include <stdio.h>
#include <Windows.h>

void textColor(int colorNum);
void gotoxy(int x, int y);
void drawGameBoard();

void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}


void gotoxy(int x, int y) {
    COORD pos = { x*2, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

struct area {
    int colorNum;
    int constructPrices[3];
};

struct deed {
    char name[20];
    int areaNum;
    int buildingLevel;
    int toll;

};

struct area areaInfo[4];

void init_area(int constructPrice[]) {
    areaInfo[0].colorNum = 1;
    for (int i = 0; i < 3; i++) {
        areaInfo[0].constructPrices[i] = constructPrice[i];
    }
}

//char cityList[23][20] = {
//    "타이베이", "베이징", "마닐라", "싱가포르", "카이로", "이스탄불", 
//    "아테네", "코펜하겐", "스톡홀름", "베른", "베를린", "오타와",
//    "부에노스 아이레스", "상파울루", "시드니", "하와이", "리스본", "마드리드",
//    "도쿄", "파리", "로마", "런던", "뉴욕"
//};

void drawGameBoard() {
    textColor(1);
}

int main() {
    int tmpArray
    initPrice(0, );

    printf("%d", strlen("부에노스 아이레스"));
    //drawGameBoard();
}

/*



                  □□□▮□□□
                  ▨  □▮▨  □
                  ▨서□▮▨서□
                  □  ▣▮□  ▣
                  ▨울□▮▨울□
                  ▨  □▮▨  □
                  □□□▮□□□
                  ■■■▮■■■
                  ■■■▮■■■

□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨▨▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬      ▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬▬
□□□▣□□□■■      ■■□□□▣□□□
□  서  울  □■■      ■■□  서  울  □
□▨▨□▨▨□■■      ■■□▨▨□▨▨□
━━━━━━━━┓■■■
                ┃■■■
                ┃□□□
                ┃▨  □
                ┃▨서□
                ┃□  ▣
                ┃▨울□
                ┃▨  □
                ┃□□□
    
□□□□□■■
□▨▨▨□■■
□□□□□■■




*/