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

bool inIntvl(int num, int min, int max) {
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

/*
pos placeMine() {//Note: This tends to make horizontally clustered mines and can have difficulty when the empty tiles are few,
	//use shuffle-based method instead!
	int x, y;
	x = rand() % gridx; y = rand() % gridy;
	if (grid[x][y].hasMine) {
		while (grid[x][y].hasMine) {
			x++;
			if (x >= gridx) { x = 0; y++; }
			if (y >= gridy) { y = 0; }
		}
	}
	grid[x][y].hasMine = true;
	return pos(x, y);
}
*/

void initializeGrid(int shuffleIters) {
	int i, j;
	grid.resize(gridx);
	for (i = 0; i < gridx; i++) {
		grid[i].resize(gridy);
	}
	/*
	pos mine;
	for (i = 0; i < mines; i++) {
		mine = placeMine();
		for (j = 0; j < 8; j++) {
			if (inIntvl(shift[j][0] + mine.x, 0, gridx - 1) && inIntvl(shift[j][1] + mine.y, 0, gridy - 1)) {
				grid[shift[j][0] + mine.x][shift[j][1] + mine.y].nearMines++;
			}
		}
	}
	*/
	for (i = 0; i < mines; i++) {
		grid[i%gridx][i / gridx].hasMine = true;
	}
	for (i = 0; i < shuffleIters; i++) {
		// This method is faster than the 4xRand() one by ~11%!
		int swap1 = rand() % (gridx*gridy), swap2 = rand() % (gridx*gridy);
		swap(grid[swap1%gridx][swap1 / gridx], grid[swap2%gridx][swap2 / gridx]);
		
		//swap(grid[rand() % gridx][rand() % gridy], grid[rand() % gridx][rand() % gridy]); <-Slower!
	}

	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridy; j++) {
			if(grid[i][j].hasMine){
				for(int k = 0; k < 8; k++){
					if (inIntvl(shift[k][0] + i, 0, gridx - 1) && inIntvl(shift[k][1] + j, 0, gridy - 1)) {
						grid[shift[k][0] + i][shift[k][1] + j].nearMines++;
					}
				}
			}
		}
	}
	return;
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
	initializeGrid(100000);
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

