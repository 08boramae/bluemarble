//
// Created by Jaeyoung Jang on 2024-11-11.
//

#include <stdio.h>
#include <Windows.h> // «Ï¥ı∆ƒ¿œ √ﬂ∞°

void gotoxy(int x, int y);
void drawGameBoard();


int main() {
    printf("Hello, World!");
    drawGameBoard();
    return 0;
}

void gotoxy(int x, int y) {
    COORD pos = { x*2, y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void drawGameBoard() {
    for(int i=0; i<10;)
    gotoxy(2, 3);
    printf("°‡");
}

/*


°‡°‡°‡¢√°‡°‡°‡°·°·
°‡Ω∫≈Â  »¶∏ß°‡°·°·
°‡¢ ¢ °‡¢ ¢ °‡°·°·

°‡°‡°‡¢√°‡°‡°·°·
°‡Ω∫≈Â  »¶∏ß°·°·
°‡¢ ¢ °‡¢ ¢ °·°·


°‡°‡°‡
¢   °‡
¢ º≠°‡
°‡  ¢√
¢ øÔ°‡
¢   °‡
°‡°‡°‡
°·°·°·
°·°·°·

                  °‡°‡°‡
                  ¢   °‡
                  ¢ º≠°‡
                  °‡  ¢√
                  ¢ øÔ°‡
                  ¢   °‡
                  °‡°‡°‡
                  °·°·°·
                  °·°·°·           
°‡°‡°‡¢√°‡°‡°‡°·°·
°‡Ω∫≈Â  »¶∏ß°‡°·°·
°‡  17  ∏∏ø¯°‡°·°·
°‡¢ ¢ °‡¢ ¢ °‡°·°·








°·¢ ¢À¢»?¢√¢√¢Ã


*/