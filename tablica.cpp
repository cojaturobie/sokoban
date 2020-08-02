#include<math.h>
#include<stdio.h>
#include<string.h>

//rozmiar tablicy pole
#define POLEX 14
#define POLEY 10

enum element {
	GRACZ,
	KORYTARZ,
	SKRZYNIA,
	SCIANA,
};

void nowa(int t[POLEY][POLEX], bool c[POLEY][POLEX]) {
	for (int i = 0; i < POLEY; i++) {
		for (int j = 0; j < POLEX; j++) {
			if (i == 0 || i == POLEY - 1)t[i][j] = SCIANA;
			else if (j == 0 || j == POLEX - 1)t[i][j] = SCIANA;
			else if (i > 0 && i < POLEY - 1 && j>0 && j < POLEX - 1)t[i][j] = KORYTARZ;
			c[i][j] = false;
		}
	}
	t[1][1] = GRACZ;
	t[1][3] = SKRZYNIA;
	t[2][3] = SKRZYNIA;
	t[4][4] = SKRZYNIA;
	t[5][4] = SCIANA;
	t[5][5] = SCIANA;
	t[6][4] = SCIANA;
	t[6][5] = SCIANA;
	c[1][9] = true;
	c[4][5] = true;
	c[3][6] = true;
}


int wygrana(int t[POLEY][POLEX], bool c[POLEY][POLEX]) {
	for (int i = 0; i < POLEY; i++) {
		for (int j = 0; j < POLEX; j++) {
			if ((t[i][j] == KORYTARZ || t[i][j] == GRACZ) && c[i][j] == true)return 1;
		}
	}
	return 0;
}

void wczytaj(int t[POLEY][POLEX], bool c[POLEY][POLEX], FILE *plik) {
	for (int i = 0; i < POLEY; i++) {
		for (int j = 0; j < POLEX; j++) {
			fscanf(plik, "%d ", &t[i][j]);
		}
		fscanf(plik, "\n", NULL);
	}
	fclose(plik);
	for (int i = 0; i < POLEY; i++) {
		for (int j = 0; j < POLEX; j++) {
			if (t[i][j] == 0)t[i][j] = SCIANA;
			else if (t[i][j] == 1) {
				t[i][j] = KORYTARZ;
				c[i][j] = false;
			}
			else if (t[i][j] == 2)t[i][j] = SKRZYNIA;
			else if (t[i][j] == 3) {
				t[i][j] = KORYTARZ;
				c[i][j] = true;
			}
			else if (t[i][j] == 4) {
				t[i][j] = GRACZ;
			}
		}
	}
}