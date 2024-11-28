#include <stdio.h>
#include <string.h>
#include <Windows.h>

#define DARKGREEN 2    // 도시들
#define CYAN 3         // 우주정거장, 무인도 등
#define GREEN 10       // 스위스, 그리스 등
#define YELLOW 6       // 황금열쇠

void setTextColor(int color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void drawBoard() {
    SetConsoleOutputCP(65001);  // UTF-8 인코딩 설정

    // 상단 테두리
    setTextColor(GREEN);
    printf("┌────────┬────────┬────────┬────────┬────────┬────────┬────────┐\n");

    // 첫 번째 줄 (출발~무인도)
    setTextColor(CYAN);    // 청록색
    printf("│ 출발   ");
    setTextColor(DARKGREEN);   // 진한 초록색
    printf("│ 태국   ");
    setTextColor(YELLOW);   // 노란색
    printf("│황금열쇠");
    setTextColor(CYAN);    // 청록색
    printf("│우주정거");
    setTextColor(YELLOW);   // 노란색
    printf("│황금열쇠");
    setTextColor(GREEN);    // 초록색
    printf("│프랑스  ");
    setTextColor(CYAN);    // 청록색
    printf("│무인도  │\n");

    // 독일, 스위스 줄 (2줄)
    for(int i = 0; i < 2; i++) {
        setTextColor(GREEN);
        printf("├────────┼────────┼────────┼────────┼────────┼────────┼────────┤\n");
        setTextColor(DARKGREEN);
        printf("│ 독일   │        │        │        │        │        ");
        setTextColor(GREEN);
        printf("│스위스  │\n");
    }

    // 이탈리아, 그리스 줄 (3줄)
    for(int i = 0; i < 3; i++) {
        setTextColor(GREEN);
        printf("├────────┼────────┼────────┼────────┼────────┼────────┼────────┤\n");
        setTextColor(DARKGREEN);
        printf("│이탈리아│        │        │        │        │        ");
        setTextColor(GREEN);
        printf("│그리스  │\n");
    }

    // 마지막 줄 테두리
    setTextColor(GREEN);
    printf("├────────┼────────┼────────┼────────┼────────┼────────┼────────┤\n");

    // 마지막 줄 (사회복지~우주여행)
    setTextColor(CYAN);    // 청록색
    printf("│사회복지");
    setTextColor(DARKGREEN);   // 진한 초록색
    printf("│ 영국   ");
    setTextColor(YELLOW);   // 노란색
    printf("│황금열쇠");
    setTextColor(DARKGREEN);   // 진한 초록색
    printf("│ 미국   ");
    setTextColor(YELLOW);   // 노란색
    printf("│황금열쇠");
    setTextColor(DARKGREEN);   // 진한 초록색
    printf("│캐나다  ");
    setTextColor(CYAN);    // 청록색
    printf("│우주여행│\n");

    // 하단 테두리
    setTextColor(GREEN);
    printf("└────────┴────────┴────────┴────────┴────────┴────────┴────────┘\n");

    setTextColor(7);  // 기본 색상으로 복귀
    printf("\nPress any key to continue . . .");
}

int main() {
    // 콘솔 창 크기 설정
    HWND console = GetConsoleWindow();
    RECT r;
    GetWindowRect(console, &r);
    MoveWindow(console, r.left, r.top, 800, 600, TRUE);

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