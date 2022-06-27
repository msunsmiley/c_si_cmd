#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include <conio.h>
#include <math.h>
#include <windows.h>

// macros
#define ARR_LEN(array) sizeof(array)/sizeof(array[0]) //size of array
#define SET_SEED(seed) srand((unsigned)seed) //set seed for randomness
#define ENEMY_MOVE frame % (enemyMoveFrameNum - spd) == 0 // enemy moves at a particular pace

#define SHOT 0b1 // some flags <vv
#define OVER 0b10
#define THIRD 0b100
#define FOURTH 0b1000

//enums 
enum {
    KEY_SPACE = 32,

    KEY_ZERO = 48,
    KEY_ONE = 49,
    KEY_TWO = 50,
    KEY_THREE = 51,
    KEY_FOUR = 52,
    KEY_FIVE = 53,
    KEY_SIX = 54,
    KEY_SEVEN = 55,
    KEY_EIGHT = 56,
    KEY_NINE = 57,

    KEY_A = 65,
    KEY_B = 66,
    KEY_C = 67,
    KEY_D = 68,
    KEY_E = 69,
    KEY_F = 70,
    KEY_G = 71,
    KEY_H = 72,
    KEY_I = 73,
    KEY_J = 74,
    KEY_K = 75,
    KEY_L = 76,
    KEY_M = 77,
    KEY_N = 78,
    KEY_O = 79,
    KEY_P = 80,
    KEY_Q = 81,
    KEY_R = 82,
    KEY_S = 83,
    KEY_T = 84,
    KEY_U = 85,
    KEY_V = 86,
    KEY_W = 87,
    KEY_X = 88,
    KEY_Y = 89,
    KEY_Z = 90,
	KEY_ESC = 27
}; //ascii keys to decimal value

typedef enum Scenes {
    Intro,
    Game,
    Pause,
    Over
} Scenes;
Scenes Scene = Intro;

//variables
int init;
char ch;

FILE* f_hs;

const int spacing = 8;
const int baroffx = 4;
const int baroffy = 5;
const int barStartH = 6;

const char grid[18][36];
//
//
unsigned int flags = 0;
int i, j, k, lives, score, frame, move, hs=0;
int dead, x, y, bullx, bully;
int ebullL=6, ebullx[6], ebully[6], ebulls=0;
int spd;

char icon;

const char go[] = "G A M E   O V E R";
int gotimer;
int lowest_row;
const int enemyMoveFrameNum = 13;

//Structs
typedef struct Bar {
    //no arrays?
    int Lx, Rx, TLx, TRx, Tx;
    int Ly, Ry, TLy, TRy, Ty;
    int Lh, Rh, TLh, TRh, Th;
} Bar;
Bar bar[4];

typedef struct Enemy {
    int x;
    int y;
    int h;
    int val;
    char icon[2];
} Enemy;
Enemy enemy[55];
//0-10 10pts +
//11-31 20pts ~
//32-54 40pts Z

void scene_(int a) {
    Scene = a;
    init = 0;
}

//functions
void setEntities() {
	//bars
	for (i = 0; i < ARR_LEN(bar); i++) {
		bar[i].Lx = baroffx + i * spacing; bar[i].Ly = ARR_LEN(grid) - 5;
		bar[i].Rx = baroffx + 2 + i * spacing; bar[i].Ry = ARR_LEN(grid) - 5;
		bar[i].TLx = baroffx + i * spacing; bar[i].TLy = ARR_LEN(grid) - baroffy - 1;
		bar[i].TRx = baroffx + 2 + i * spacing; bar[i].TRy = ARR_LEN(grid) - baroffy - 1;
		bar[i].Tx = baroffx + 1 + i * spacing; bar[i].Ty = ARR_LEN(grid) - baroffy - 1;
		bar[i].Lh = barStartH, bar[i].Rh = barStartH, bar[i].TLh = barStartH, bar[i].TRh = barStartH, bar[i].Th = barStartH;
	}
	//enemies
	for (i = 0; i < ARR_LEN(enemy); i++)
	{
		if (i <= 10) { enemy[i].val = 40; enemy[i].icon[0] = '&'; enemy[i].icon[1] = '8'; }
		else if (i <= 32) { enemy[i].val = 20; enemy[i].icon[0] = '~'; enemy[i].icon[1] = '-'; }
		else if (i <= 54) { enemy[i].val = 10; enemy[i].icon[0] = 'S'; enemy[i].icon[1] = '5'; }

		enemy[i].x = (7 + i * 2) - (22 * (i / 11)); // <--yay
		enemy[i].y = 3 + i / 11;
		enemy[i].h = 1;
	}
	//"disabling" all ebulls
	for (i = 0; i < 6; i++) { ebully[i] = 0; }
}
void reset() {
	ebulls = 0;
	gotimer = 0;
	flags = 0;
	bully = 0;
	dead = 0;
	lives = 3;
	score = 0;
	y = ARR_LEN(grid) - 3;
	x = 5;
	setEntities();
	spd = 1;
	icon = '=';
}
void drawGame() {
	if (flags & OVER) {
		if (gotimer < ARR_LEN(go)-1) gotimer++;
		if (gotimer == 1) {
			if (score > hs) {
				f_hs = fopen("si.komic", "w");
				fprintf(f_hs, "%d", score);
				fclose(f_hs);
				hs = score;
			}
		}
	}
	printf("\nSpace Invaders by alexa in C :--)\n\n");
	printf("SCORE %d   HI SCORE %d   LIVES  ", score, hs);
	for (i = 0; i < lives; i++) printf("= ");
	if(!(flags&OVER))printf("     Quit: ESC    Restart: R     Pause: P");

	printf("\n");
	for (i = 0; i < ARR_LEN(grid); i++)
	{
		printf("#");
		for (j = 0; j < ARR_LEN(grid[0]); j++)
		{
		label:
			//printing enemy bullets
			if (!(flags & OVER)) {
				for (k = 0; k < 6; k++)
				{
					if (i == ebully[k] && j == ebullx[k] && ebully[k]) { printf("!"); j++; goto label; }
				}
			}
			//printing enemies
			if (!(flags & OVER)) {
				for (k = 0; k < ARR_LEN(enemy); k++) {
					if (i == enemy[k].y && j == enemy[k].x && enemy[k].h == 1) {
						if (pow(-1, move) == 1) { printf("%c", enemy[k].icon[0]); j++; goto label; }
						else { printf("%c", enemy[k].icon[1]); j++; goto label; }
					}
				}
			}
			//printing bars
			for (k = 0; k < ARR_LEN(bar); k++)
			{
				if (i == bar[k].Ly && j == bar[k].Lx) {
					if (bar[k].Lh <= barStartH && bar[k].Lh > barStartH / 2) { printf("I"); j++; goto label; }
					else if (bar[k].Lh <= barStartH / 2 && bar[k].Lh >= 1) { printf("i"); j++; goto label; }
				}
				else if (i == bar[k].Ry && j == bar[k].Rx) {
					if (bar[k].Rh <= barStartH && bar[k].Rh > barStartH / 2) { printf("I"); j++; goto label; }
					else if (bar[k].Rh <= barStartH / 2 && bar[k].Rh >= 1) { printf("i"); j++; goto label; }
				}
				else if (i == bar[k].TLy && j == bar[k].TLx) {
					if (bar[k].TLh <= barStartH && bar[k].TLh > barStartH / 2) { printf("&"); j++; goto label; }
					else if (bar[k].TLh <= barStartH / 2 && bar[k].TLh >= 1) { printf("o"); j++; goto label; }
				}
				else if (i == bar[k].TRy && j == bar[k].TRx) {
					if (bar[k].TRh <= barStartH && bar[k].TRh > barStartH / 2) { printf("&"); j++; goto label; }
					else if (bar[k].TRh <= barStartH / 2 && bar[k].TRh >= 1) { printf("o"); j++; goto label; }
				}
				else if (i == bar[k].Ty && j == bar[k].Tx) {
					if (bar[k].Th <= barStartH && bar[k].Th > barStartH / 2) { printf("="); j++; goto label; }
					else if (bar[k].Th <= barStartH / 2 && bar[k].Th >= 1) { printf("-"); j++; goto label; }
				}
			}
			if ((i == 0) | (j == ARR_LEN(grid[0]) - 1) | (i == ARR_LEN(grid) - 1)) printf("#"); //grid
			else if (i == y && j == x) printf("%c", icon); //you
			else if (!(flags&OVER) && i == bully && j == bullx) printf("|"); //your bull
			//print game over
			else if (flags & OVER && i == 4 && j == 9) {
				for (int z = 0; z < gotimer; z++) { printf("%c", go[z]); if (z > 0)j++; }
			}
			else if (flags & OVER && i == 6 && j == 14) { printf("PRESS R"); j += 6; }
			else printf(" ");
			//print the scores
			if (!(flags & OVER)) {
				if (!score) {
					if (i == 2 && j == ARR_LEN(grid[0]) - 1) printf("      S  10PTS");
					else if (i == 3 && j == ARR_LEN(grid[0]) - 1) printf("      ~  20PTS");
					else if (i == 4 && j == ARR_LEN(grid[0]) - 1) printf("      &  40PTS");
				}
			}

		}
		printf("\n");
	}

}

void sigame() {
	if (flags & OVER) { scene_(Over); icon = '%'; }
	if (!init) {
		init = 1;
		f_hs = fopen("si.komic", "r+");
		if (!f_hs) { f_hs = fopen("si.komic", "w+"); fprintf(f_hs, "%d", hs); }
		fscanf(f_hs, "%d", &hs);
		fclose(f_hs);
	}
	if (!dead) {
		frame++;

		if (_kbhit()) {
			ch = toupper(_getch());
			switch (ch) {
			case KEY_A: if (x > 0) { x--; } break;
			case KEY_D: if(x < ARR_LEN(grid[0]) - 1 - 1) { x++; } break;
			case KEY_SPACE:
				if (!(flags & SHOT)) {
					flags |= SHOT; bullx = x; bully = y;
				}
				break;
			case KEY_R: scene_(Intro); break;
			case KEY_P: scene_(Pause); break;
			case KEY_G: flags |= OVER; break;
			case KEY_ESC: exit(0); break;
				//case KEY_R: break;
				//case KEY_P: break;
			}
			printf("%c", ch);
		}

		
	}

	if (flags & SHOT) {
		bully--;
		if (bully <= 0) flags = flags & ~SHOT;
	}
	//
	drawGame();
	//bullet hits bars
	for (i = 0; i < ARR_LEN(bar); i++)
	{
		if (bullx == bar[i].Lx && bully == bar[i].Ly && bar[i].Lh > 0) {
			bar[i].Lh--;
			bully = 0;
		}
		if (bullx == bar[i].Rx && bully == bar[i].Ry && bar[i].Rh > 0) {
			bar[i].Rh--;
			bully = 0;
		}
		if (bullx == bar[i].TLx && bully == bar[i].TLy && bar[i].TLh > 0) {
			bar[i].TLh--;
			bully = 0;
		}
		if (bullx == bar[i].TRx && bully == bar[i].TRy && bar[i].TRh > 0) {
			bar[i].TRh--;
			bully = 0;
		}
		if (bullx == bar[i].Tx && bully == bar[i].Ty && bar[i].Th > 0) {
			bar[i].Th--;
			bully = 0;
		}
	}

	//bullet hits enemies
	//enemy
	for (i = 0; i < ARR_LEN(enemy); i++)
	{
		if (bullx == enemy[i].x && bully == enemy[i].y && enemy[i].h == 1) {
			enemy[i].h = 0;
			bully = 0;
			score += enemy[i].val;
		}
	}

	//moving enemies
	if (ENEMY_MOVE) {

		for (i = 0; i < ARR_LEN(enemy); i++)
		{
			enemy[i].x -= (int)pow(-1, spd);
			move++;
		}
	}
	//CRITICAL-- this is if frame % (n-spd) is 0, but frame is paused
	if (ENEMY_MOVE && dead) frame++;

	//enemy goes forward one/speeds up
	if ((enemy[10].x == ARR_LEN(grid[10]) - 1 - 1) | (enemy[0].x == 1)) {
		spd++; for (i = 0; i < ARR_LEN(enemy); i++) { enemy[i].y++; enemy[i].x -= (int)pow(-1, spd); }
	}

	//disabling ebulls and ebulls going down
	for (i = 0; i < ebullL; i++)
	{
		if (ebully[i] < ARR_LEN(grid) && ebully[i]) { if (frame % 2) ebully[i]++; }
		if (ebully[i] >= ARR_LEN(grid)) { ebulls--; ebully[i] = 0; }
	}

	//ebulls hits bars
	for (i = 0; i < ARR_LEN(bar); i++)
	{
		for (j = 0; j < ebullL; j++) {
			//  v -- -- --  ebull touches the bar -- -- -- --  v     vv the bar isnt dead 
			if (ebullx[j] == bar[i].Lx && ebully[j] == bar[i].Ly && bar[i].Lh > 0) {
				bar[i].Lh--;
				ebully[j] = ARR_LEN(grid);
			}
			if (ebullx[j] == bar[i].Rx && ebully[j] == bar[i].Ry && bar[i].Rh > 0) {
				bar[i].Rh--;
				ebully[j] = ARR_LEN(grid);
			}
			if (ebullx[j] == bar[i].TLx && ebully[j] == bar[i].TLy && bar[i].TLh > 0) {
				bar[i].TLh--;
				ebully[j] = ARR_LEN(grid);
			}
			if (ebullx[j] == bar[i].TRx && ebully[j] == bar[i].TRy && bar[i].TRh > 0) {
				bar[i].TRh--;
				ebully[j] = ARR_LEN(grid);
			}
			if (ebullx[j] == bar[i].Tx && ebully[j] == bar[i].Ty && bar[i].Th > 0) {
				bar[i].Th--;
				ebully[j] = ARR_LEN(grid);
			}
		}
	}

	//random enemy shoots a bullet at random
	int a = rand() % (35 * ARR_LEN(enemy));
	if ((a >= 0) && (a < ARR_LEN(enemy))) {
		if (enemy[a].h) { ebullx[ebulls] = enemy[a].x; ebully[ebulls] = enemy[a].y; ebulls++; a = -1; }
	}

	//ebull hits you
	for (i = 0; i < ebullL; i++)
	{
		if (ebullx[i] == x && ebully[i] == y)
		{
			dead = 1;
			ebully[i] = ARR_LEN(grid);
			if (flags & SHOT) bully = 0;
		}
	}

	//when invaders reach too low
	if (spd == 13-lowest_row) { flags |= OVER; }

	//checks for the lowest row
	lowest_row = 5;
	for (i = 0; i < 5; i++) {
		int a = 0;
		for (j = 11*i; j < 11+11*i; j++) {
			if (!enemy[j].h) a++;
		}
		if(a==11) lowest_row--;
	}


	// u die animation
	if (dead >= 1 && dead < 20) {
		if (dead == 1) { lives--; for (i = 0; i < ebulls; i++) { ebully[i] = ARR_LEN(grid); } }
		dead++;
		if (dead % 2) { icon = '#'; }
		else { icon = 'H'; }
	}
	if (dead == 20) {
		dead = 0; icon = '=';
		y = ARR_LEN(grid) - 3;
		x = 5;
	}

	if (!lives) { flags |= OVER; dead = 0; }

}

int main()
{
    SET_SEED(time(NULL));
    while (true) {
        Sleep(60);
        if (Scene == Intro) {
            if (!init) {
                init = 1;
                reset();
                system("cls");
                printf("\nSpace Invaders by alexa in C :--)\n\n");
                printf("Press Space to play\n");
            }
            ch = toupper(_getch());
            if (ch == KEY_SPACE) scene_(Game);
        }
        if (Scene == Game) {
            system("cls");
            //
            sigame();
        }
        if (Scene == Pause) {
            if (!init) {
                init = 1;
            }
            system("cls");
            drawGame();
            printf("\n\nGAME IS PAUSED   PRESS P TO UNPAUSE");
            ch = toupper(_getch());
            if (ch == KEY_P) scene_(Game);
        }
        if (Scene == Over) {
            if (!init) {
                init = 1;
            }
            system("cls");
            drawGame();
            if (_kbhit()) {
                ch = toupper(_getch());
                if (ch == KEY_R) scene_(Intro);
            }
        }
    }
}
