// MinesweeperAndBruteforce.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <cstdio>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <ctime>

using namespace std;

int gridx, gridy, mines;

int shift[8][2] = { //Instead of doing some awkward if()s for checks around current coords, use a matrix for shifting them
	{0,1}, {0,-1}, {1,0}, {-1,0}, //direct neighbour shifts
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
	if (!t.discovered) { cout << "#"; return; }
	if (t.flagged) { cout << "!"; return; }
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

void placeFlag(int x, int y) {

}

void discover(int x, int y) {
	grid[x][y].discovered = true;
	if (grid[x][y].nearMines) { return; }
	for (int i = 0; i < 8; i++) {
		if (inIntvl(shift[i][0] + x, 0, gridx - 1) && inIntvl(shift[i][1] + y, 0, gridy - 1)) {
			if (!grid [shift[i][0] + x] [shift[i][1] + y] .discovered) { discover(shift[i][0] + x, shift[i][1] + y); }
		}
	}
}

void discoverAll() {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			grid[i][j].discovered = true;
		}
	}
}

int main(){
	srand(5);
	gridx = 10, gridy = 10, mines = 12;
	clock_t timer = clock();
	initializeGrid(100000);//Starts to get slow at ~1 000 000, takes ~1.6 secs
	cout << "Generation time: " << float(clock() - timer) / float(CLOCKS_PER_SEC)<<" seconds.\n";
	printGrid();
	cout << endl;
	discover(5, 5);
	printGrid();
	cout << endl;
	discoverAll();
	printGrid();
    return 0;
}

