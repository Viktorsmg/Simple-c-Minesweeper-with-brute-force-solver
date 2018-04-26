// MinesweeperAndBruteforce.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"//MSVC required
#include <cstdio>//Printf
#include <iostream>//Cin and cout for easier IO
#include <vector>//Vector arrays
#include <cstdlib>
#include <ctime>//Mine seed and generation timer


#include <windows.h> //Terminal clearing
#include <conio.h> //_getch() - direct character fetching from windows terminal




using namespace std;

int gridx, gridy, mines;

int shift[8][2] = { //Instead of doing some awkward if()s for checks around current coords, use a matrix for shifting them
	{0,1}, {0,-1}, {1,0}, {-1,0}, //Direct neighbour shifts: Right, left, down, up
	{1,1}, {1,-1}, {-1,1}, {-1,-1} }; //diagonal neighbour shifts

template<typename type>
bool inIntvl(type num, type min, type max) {
	return num >= min && num <= max;
}

struct tile {
	bool hasMine=false, flagged=false, discovered=false;
	int nearMines=0;
};

struct pos {
	int x, y;
	pos() { x = 0, y = 0; }
	pos(int _x, int _y) { x = _x; y = _y; }
};

vector< vector< tile > > grid;
vector< pos > flags;


pos placePos(bool placeType = 1) {// placeType - place mine(1) or air(0) IE what should hasMine be
	//Do not use when the ratios are not in your favor! Like placing mines when there's very few empty spots, etc.
	int x, y;
	x = rand() % gridx; y = rand() % gridy;
	while (grid[x][y].hasMine == placeType) {
		x = rand() % gridx; y = rand() % gridy;
	}
	grid[x][y].hasMine = placeType;
	return pos(x, y);
}


void genPlaceMines(){
	int i, j;
	pos mine;
	for (i = 0; i < mines; i++) {
		mine = placePos(1);
		for (j = 0; j < 8; j++) {
			if (inIntvl(shift[j][0] + mine.x, 0, gridx - 1) && inIntvl(shift[j][1] + mine.y, 0, gridy - 1)) {
				grid[shift[j][0] + mine.x][shift[j][1] + mine.y].nearMines++;
			}
		}
	}
}

void genPlaceAir() {
	int i, j;

	//Fill the grid with mines instead of air.
	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridy; j++) {
			grid[i][j].hasMine = true;
			if ((i == 0 || i == (gridx - 1)) && (j == 0 || j == (gridy - 1))) {//Is this a corner tile? They have only 3 near mines.
				grid[i][j].nearMines = 3;//This is to remedy future reduction of mine counts when placing air.
				continue;
			}
			if ((i == 0 || i == (gridx - 1)) || (j == 0 || j == (gridy - 1))) {//Edge tiles have 5 mines; we continued from the last if,
				grid[i][j].nearMines = 5;//so it can't be that BOTH statements are true.
				continue;
			}
			grid[i][j].nearMines = 9;
		}
	}

	pos air;
	int airs = gridx * gridy - mines;
	for (i = 0; i < airs; i++) {
		air = placePos(0);
		for (j = 0; j < 8; j++) {
			if (inIntvl(shift[j][0] + air.x, 0, gridx - 1) && inIntvl(shift[j][1] + air.y, 0, gridy - 1)) {
				grid[shift[j][0] + air.x][shift[j][1] + air.y].nearMines--;
			}
		}
	}
}

void genShuffle(int shuffleIters){

	int i, j;

	//Place a line of mines
	for (i = 0; i < mines; i++) {
		grid[i%gridx][i / gridx].hasMine = true;
	}

	//Shuffle
	for (i = 0; i < shuffleIters; i++) {
		// This method is faster than the 4xRand() one by ~11%!
		int swap1 = rand() % (gridx*gridy), swap2 = rand() % (gridx*gridy);
		swap(grid[swap1 % gridx][swap1 / gridx], grid[swap2 % gridx][swap2 / gridx]);

		//swap(grid[rand() % gridx][rand() % gridy], grid[rand() % gridx][rand() % gridy]); <-Slower!
	}

	//Add mine counts to tiles
	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridy; j++) {
			if (grid[i][j].hasMine) {
				for (int k = 0; k < 8; k++) {
					if (inIntvl(shift[k][0] + i, 0, gridx - 1) && inIntvl(shift[k][1] + j, 0, gridy - 1)) {
						grid[shift[k][0] + i][shift[k][1] + j].nearMines++;
					}
				}
			}
		}
	}
}

void initializeGrid(int shuffleIters) {
	int i, j;
	grid.resize(gridx);
	for (i = 0; i < gridx; i++) {
		grid[i].resize(gridy);
	}
	
	float ratio = mines / (gridx*gridy);
	if (!inIntvl(ratio, 0.0f, 1.0f)) return;
	if (ratio < 0.3) { genPlaceMines(); return; }
	if (ratio > 0.7) { genPlaceAir(); return; }
	genShuffle(shuffleIters); return;
}

void printTile(tile t) {
	if (t.flagged && !t.discovered) { cout << "!"; return; }
	if (!t.discovered) { cout << "#"; return; }
	if (t.hasMine) { cout << "@"; return; }
	if (!t.nearMines) { cout << " "; return; }
	cout << t.nearMines;
}

void printGrid() {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			printTile(grid[i][j]);
		}
		cout << endl;
	}
}

void printGrid(pos cursor) {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			if (i == cursor.x && j == cursor.y) { cout << char(219); continue; };
			printTile(grid[i][j]);
		}
		cout << endl;
	}
}

bool placeFlag(int x, int y) {
	if (grid[x][y].discovered) return false;
	grid[x][y].flagged = !grid[x][y].flagged;
	int i, j;
	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridx; j++) {
			if (grid[i][j].flagged != grid[i][j].hasMine) return false;
		}
	}
	return true;
}

bool placeFlag(pos xy) { return placeFlag(xy.x, xy.y); }

void discover(int x, int y) {
	grid[x][y].discovered = true;
	if (grid[x][y].nearMines) { return; }
	for (int i = 0; i < 8; i++) {
		if (inIntvl(shift[i][0] + x, 0, gridx - 1) && inIntvl(shift[i][1] + y, 0, gridy - 1)) {
			if (!grid [shift[i][0] + x] [shift[i][1] + y] .discovered) { discover(shift[i][0] + x, shift[i][1] + y); }
		}
	}
}

void discover(pos xy) { discover(xy.x, xy.y); }

void discoverAll() {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			grid[i][j].discovered = true;
		}
	}
}

void clearTerminal() {
	system("cls");
	/*
	for (int i = 0; i < 30; i++) {
		cout << endl;
	}
	*/
}

int interactiveMode() {
	cout << "Interactive mode - use the w, a, s and d keys to move a cursor around, space to click and q to toggle flags!";

	bool won=false;
	pos cursor(gridx/2,gridy/2);
	char action = 5;

	while(!won){
		printf("\n(%i)[%c]", int(action), action);
		clearTerminal();
		printGrid(cursor);
		action = _getch();
		switch (action) {
			case 'w':
			case 'W':
				action = 3;
				break;

			case 's':
			case 'S':
				action = 2;
				break;

			case 'd':
			case 'D':
				action = 0;
				break;

			case 'a':
			case 'A':
				action = 1;
				break;
		}

		if (inIntvl(int(action), 0, 3)) {
			if (inIntvl(cursor.x + shift[action][0], 0, gridx - 1) && inIntvl(cursor.y + shift[action][1], 0, gridy - 1)) {
				cursor.x += shift[action][0];
				cursor.y += shift[action][1];
			}
			continue;
		}

		switch (action) {
			case ' ':
				discover(cursor);
				if (grid[cursor.x][cursor.y].hasMine) return -1;
				break;
			case 'q':
				won = placeFlag(cursor);
				break;
		}

	}
	return 1;
}

int main(){
	srand(5);
	gridx = 10, gridy = 10, mines = 12;
	clock_t timer = clock();
	initializeGrid(100000);//Starts to get slow at ~1 000 000, takes ~1.6 secs
	cout << "Generation time: " << float(clock() - timer) / float(CLOCKS_PER_SEC) << " seconds.\n";
	/*
	printGrid();
	cout << endl;
	discover(5, 5);
	printGrid();
	cout << endl;
	discoverAll();
	printGrid();
	*/

	int result = interactiveMode();
	switch (result) {
		case -1:
			cout << "You lost!";
			break;
		case 1:
			cout << "You won!";
			break;
		default:
			cout << "Some sort of error has occured...";
			break;
	}
    return 0;
}

