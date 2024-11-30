#include <stdio.h> 
#include <stdlib.h>
#include <Windows.h>

void rollDice() {
	int dice1, dice2;

	srand(time(NULL));
	dice1 = rand() % 6 + 1;
	dice2 = rand() % 6 + 1;
	printf("%d %d",dice1, dice2);
}

struct GoldenKey {
	int id;
	char name[50];
	char desc[100];
};

int main() {
	rollDice();
}