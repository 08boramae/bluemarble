#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define BOARD_SIZE 11
#define DARKGREEN 2    // 도시들
#define CYAN 3         // 우주정거장, 무인도 등
#define GREEN 10       // 스위스, 그리스 등
#define YELLOW 6       // 황금열쇠

void setTextColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void drawHorizontalLine() {
    setTextColor(GREEN);
    printf("├────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┤\n");
}

struct deed {
    char name[20];
    int areaNum; // 빨초노검 구역
    int isBlank;
    int buildingLevel;
    int price[4]; // 대지, 별장1, 별장2, 빌딩, 호텔
    int toll[4]; // 대지

};

void init_deed(int constructPrice[]) {
    
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

  void drawEmptyRow() {
    setTextColor(GREEN);
    printf("│        │        │        │        │        │        │        │        │        │        │        │\n");
}

void drawBoard() {
    SetConsoleOutputCP(65001);

    printf("Hello\n");

    // 상단 테두리
    setTextColor(GREEN);
    printf("┌────────┬────────┬────────┬────────┬────────┬────────┬────────┬────────┬────────┬────────┬────────┐\n");

    // 첫 번째 줄 (상단)
    drawEmptyRow();
    setTextColor(CYAN);
    printf("│ 출발   │");
    setTextColor(DARKGREEN);
    printf(" 타이완 │");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(DARKGREEN);
    printf(" 싱가폴 │");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(CYAN);
    printf("우주정거│");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(DARKGREEN);
    printf(" 태국   │");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(DARKGREEN);
    printf(" 홍콩   │");
    setTextColor(CYAN);
    printf(" 무인도 │\n");
    drawEmptyRow();

    // 중간 부분
    for(int i = 0; i < 9; i++) {
        drawHorizontalLine();
        drawEmptyRow();
        if(i == 0) {
            setTextColor(DARKGREEN);
            printf("│ 마카오 │        │        │        │        │        │        │        │        │        │ 중국   │\n");
        } else if(i == 1) {
            setTextColor(DARKGREEN);
            printf("│ 베트남 │        │        │        │        │        │        │        │        │        │ 일본   │\n");
        } else if(i == 2) {
            setTextColor(DARKGREEN);
            printf("│ 필리핀 │        │        │        │        │        │        │        │        │        │ 러시아 │\n");
        } else if(i == 3) {
            setTextColor(DARKGREEN);
            printf("│대한민국│        │        │        │        │        │        │        │        │        │ 프랑스 │\n");
        } else if(i == 4) {
            setTextColor(DARKGREEN);
            printf("│ 인도   │        │        │        │        │        │        │        │        │        │ 이태리 │\n");
        } else if(i == 5) {
            setTextColor(DARKGREEN);
            printf("│ 독일   │        │        │        │        │        │        │        │        │        │ 영국   │\n");
        } else if(i == 6) {
            setTextColor(DARKGREEN);
            printf("│ 캐나다 │        │        │        │        │        │        │        │        │        │ 스페인 │\n");
        } else if(i == 7) {
            setTextColor(DARKGREEN);
            printf("│브라질  │        │        │        │        │        │        │        │        │        │ 호주   │\n");
        } else if(i == 8) {
            setTextColor(DARKGREEN);
            printf("│멕시코  │        │        │        │        │        │        │        │        │        │스위스  │\n");
        }
        drawEmptyRow();
    }

    // 마지막 줄 테두리
    setTextColor(GREEN);
    printf("├────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┼────────┤\n");

    // 마지막 줄 (하단)
    drawEmptyRow();
    setTextColor(CYAN);
    printf("│사회복지│");
    setTextColor(DARKGREEN);
    printf(" 미국   │");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(DARKGREEN);
    printf("그리스  │");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(CYAN);
    printf("세계여행│");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(DARKGREEN);
    printf("스웨덴  │");
    setTextColor(YELLOW);
    printf("황금열쇠│");
    setTextColor(DARKGREEN);
    printf("덴마크  │");
    setTextColor(CYAN);
    printf("우주여행│\n");
    drawEmptyRow();

    // 하단 테두리
    setTextColor(GREEN);
    printf("└────────┴────────┴────────┴────────┴────────┴────────┴────────┴────────┴────────┴────────┴────────┘\n");

    setTextColor(7);
    printf("\nPress any key to continue . . .");
}

int main() {
    // 콘솔 창 크기 설정
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);
    MoveWindow(console, r.left, r.top, 1200, 800, TRUE);

    // 콘솔 폰트 설정
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_FONT_INFOEX cfi;
    cfi.cbSize = sizeof(cfi);
    cfi.nFont = 0;
    cfi.dwFontSize.X = 0;
    cfi.dwFontSize.Y = 16;
    cfi.FontFamily = FF_DONTCARE;
    cfi.FontWeight = FW_NORMAL;
    wcscpy(cfi.FaceName, L"D2Coding");
    SetCurrentConsoleFontEx(hConsole, FALSE, &cfi);

    system("cls");
    drawBoard();
    system("pause");

    return 0;
}