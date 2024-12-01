#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>
#include <time.h>

// --- 기존 코드 ---
void textColor(int colorNum);
void gotoxy(int x, int y);
void drawGameBoard();

void textColor(int colorNum) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), colorNum);
}

void gotoxy(int x, int y) {
    COORD pos = { x * 2, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

struct deed {
    char name[20];
    int areaNum;       // 빨초노검 구역
    int ownerNum;      // 0: 없음
    int buildingLevel; // 건물 레벨
    int price[5];      // 대지, 별장1, 별장2, 빌딩, 호텔 가격
    int toll[5];       // 통행료
};

struct player {
    char name[20];
    int position;
    int money;
};

struct deed Deeds[28];

struct player Players[4];

void init_deed() {
    char names[29][50] = {
        "타이베이", "베이징", "마닐라", "제주도", "싱가포르", "카이로", "이스탄불",
        "아네테", "코펜하겐", "스톡홀름", "콩코드 여객기", "베른", "베를린", "오타와",
        "부에노스 아이레스", "상파울루", "시드니", "부산", "하와이", "리스본", "퀸 엘리자베스 호", "마드리드",
        "도쿄", "컬럼비아호", "파리", "로마", "런던", "뉴욕", "서울"
    };
    int prices[29][5] = {
        {5000, 10000, 15000, 30000, 50000}, {8000, 10000, 15000, 30000, 50000},
        {8000, 10000, 15000, 30000, 50000}, {20000}, {10000, 15000, 30000, 50000},
    };

    for (int i = 0; i < 28; i++) {
        strcpy(Deeds[i].name, names[i]);
        Deeds[i].areaNum = i / 7;
        Deeds[i].ownerNum = -1;
        Deeds[i].buildingLevel = 0;
    }
}

void init_players(int player_cnt) {
    for (int i = 0; i < player_cnt; i++) {
        printf("플레이어 %d 이름 입력: ", i + 1);
        scanf("%s", Players[i].name);
        Players[i].position = 0;
        Players[i].money = 100000;
    }
}

int roll_dice() {
    return (rand() % 6) + 1;
}

void buy_land(struct player* player, struct deed* land) {
    if (land->ownerNum == -1) { // 소유자가 없을 때만 구매 가능
        if (player->money >= land->price[0]) {
            player->money -= land->price[0];
            land->ownerNum = player - player;
            printf("%s가 %s를 구매했습니다!\n", player->name, land->name);
        }
        else {
            printf("자금이 부족합니다!\n");
        }
    }
    else {
        printf("이미 다른 플레이어가 소유하고 있습니다.\n");
    }
}

void pay_toll(struct player* player, struct deed* land) {
    if (land->ownerNum != player - player && land->ownerNum != -1) {
        int toll = land->toll[land->buildingLevel];
        player->money -= toll;
        player[land->ownerNum].money += toll;
        printf("%s가 %s의 통행료 %d원을 지불했습니다!\n", player->name, land->name, toll);
    }
}

void game_start() {
    int turn = 0;

    while (1) {
        struct Player* current = &Players[turn];
        printf("\n%s의 차례입니다.\n", current->name);

        int dice = roll_dice();
        printf("주사위를 굴려 %d가 나왔습니다!\n", dice);

        current->position = (current->position + dice) % 1000;
        struct deed* currentLand = &Deeds[current->position];
        printf("%s가 %s에 도착했습니다.\n", current->name, currentLand->name);

        if (currentLand->ownerNum == -1) {
            printf("%s를 구매하시겠습니까? (1: 예, 0: 아니오): ", currentLand->name);
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
            printf("%s가 파산했습니다! 게임 종료.\n", current->name);
            break;
        }

        turn = (turn + 1) % 4;
    }
}

int main() {
    srand(time(NULL));
    init_deed();
    init_players();
    game_start();
    return 0;
}
