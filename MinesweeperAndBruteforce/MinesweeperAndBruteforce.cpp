// MinesweeperAndBruteforce.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"//MSVC required
#include <cstdio>//Printf
#include <iostream>//Cin and cout for easier IO
#include <vector>//Vector arrays
#include <cstdlib>
#include <ctime>//Mine seed and generation timer
#include <utility>

#include <windows.h> //Terminal clearing
#include <conio.h> //_getch() - direct character fetching from windows terminal





using namespace std;

int gridx, gridy, mines;

int shift[8][2] = { //Instead of doing some awkward if()s for checks around current coords, use a matrix for shifting them
	{0,1}, {0,-1}, {1,0}, {-1,0}, //Direct neighbour shifts: Right, left, down, up
	{1,1}, {1,-1}, {-1,1}, {-1,-1} }; //diagonal neighbour shifts


struct tile {
	bool hasMine = false, flagged = false, discovered = false;
	int nearMines = 0;
};

struct pos {
	int x, y;
	pos() { x = 0, y = 0; }
	pos(int _x, int _y) { x = _x; y = _y; }
};

vector< vector< tile > > grid;
vector< pos > flags;


template<typename type>
bool inIntvl(type num, type min, type max) {
	return num >= min && num <= max;
}

bool canShift(int x, int y, int shiftID) {
	return inIntvl( x + shift[shiftID][0], 0, gridx-1 ) && inIntvl( y + shift[shiftID][1], 0, gridy-1 );
}

bool canShift(pos xy, int shiftID) {
	return canShift(xy.x, xy.y, shiftID);
}

///<summary> Mod that works correctly with negative values.</summary>
template<typename type>
type modNeg(type a, type max) {
	if (a >= 0) return a % max;
	if (a<0 && a>-max) return max + a % max;
	return max - 1 + (a + 1) % max;
}



template<typename type>
type loopOverflow(type a, pair<type, type> minmax) {///@TODO is [min,max), should be [min,max]
	return minmax.first + modNeg((a), (minmax.second - minmax.first + 1));
}

int loopOverflow(int a, pos minmax) {
	return loopOverflow(a, make_pair(minmax.x, minmax.y));
}

///<summary> Places a mine/air in a non-occupied location and returns that location. <para />
///Do not use if the air/mine:total tile ratio is close to or higher than 0.5!</summary>
pos placePos(bool placeType = 1) {
	int x, y;
	x = rand() % gridx; y = rand() % gridy;
	while (grid[x][y].hasMine == placeType) {
		x = rand() % gridx; y = rand() % gridy;
	}
	grid[x][y].hasMine = placeType;
	return pos(x, y);
}

///<summary> Generates by placing mines in non-overlapping locations. <para />
///Do not use if the amount of mines is close to or higher than half the amount of total tiles! It will be slow!</summary>
void genPlaceMines(){
	int i;
	for (i = 0; i < mines; i++) {
		placePos(1);
	}
}

///<summary> Generates by placing air in non-overlapping locations. <para />
///Do not use if the amount of airs is close to or higher than half the amount of total tiles! It will be slow!</summary>
void genPlaceAir() {
	int i, j;

	//Fill the grid with mines instead of air.
	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridy; j++) {
			grid[i][j].hasMine = true;
		}
	}
	
	//Calculate amount of and place airs.
	int airs = gridx * gridy - mines;
	for (i = 0; i < airs; i++) {
		placePos(0);
	}
}

///<summary> Generates using shuffling - moves mines around. Can take a while, based on the iterations. <para />
///For example, 1 000 000 iterations took 1.6 seconds.</summary>
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
}

///<summary> Clears tile numeration.</summary>
void clearTileNumeration() {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			grid[i][j].nearMines = 0;
		}
	}
}

///<summary> Numerates (puts amount of near mines) all tiles.</summary>
void numerateTiles() {
	int i, j;
	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridy; j++) {
			if (grid[i][j].hasMine) {
				for (int k = 0; k < 8; k++) {
					if (canShift(i,j,k)) {
						grid[shift[k][0] + i][shift[k][1] + j].nearMines++;
					}
				}
			}
		}
	}
}

///<summary> Numerates all tiles, but the ones on the edges are aware of the mines on opposite edges.</summary>
void numerateTiles_looping() {

	int i, j;
	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridy; j++) {
			if (grid[i][j].hasMine) {
				for (int k = 0; k < 8; k++) {
					grid[modNeg(shift[k][0] + i, gridx)][modNeg(shift[k][1] + j, gridy)].nearMines++;
				}
			}
		}
	}
}

///<summary> Resizes the grid and generates it using one of the three methods, based on the ratio of mines to airs. </summary>
void generateGrid(int shuffleIters) {
	int i, j;
	grid.resize(gridx);
	for (i = 0; i < gridx; i++) {
		grid[i].resize(gridy);
	}
	
	float ratio = float(mines) / float(gridx*gridy);
	if (!inIntvl(ratio, 0.0f, 1.0f)) return;
	if (ratio < 0.3) { genPlaceMines(); }
	if (ratio > 0.7) { genPlaceAir(); }
	if (ratio > 0.3 && ratio < 0.7) { genShuffle(shuffleIters); }
	return;
}

pos findEmptyTile(pos startPos = pos(gridx/2,gridy/2)) {//Search is BFS, to find the one closest to the intended pos
	//Note: TILES NEED TO BE NUMERATED!
	//Note2: Finds tile with lowest mine count before shifting
	vector<pos> bfsl, oldBfsl;
	int i, j;
	vector< vector<bool> > explored;
	explored.resize(gridx);
	for (i = 0; i < gridx; i++) {
		explored[i].resize(gridy);
		for (j = 0; j < gridy; j++) {
			explored[i][j] = false;
		}
	}
	bfsl.push_back(startPos);
	while (!bfsl.empty()) {
		oldBfsl = bfsl;
		bfsl.clear();
		for (i = 0; i < oldBfsl.size(); i++) {
			explored[oldBfsl[i].x][oldBfsl[i].y] = true;
			if (grid[oldBfsl[i].x][oldBfsl[i].y].nearMines < grid[startPos.x][startPos.y].nearMines && !grid[oldBfsl[i].x][oldBfsl[i].y].hasMine) {
				startPos = oldBfsl[i];
			}
			for (j = 0; j < 4; j++) {
				if (canShift(oldBfsl[i], j) && !explored[oldBfsl[i].x+shift[j][0]][oldBfsl[i].y+shift[j][1]]) {
					bfsl.push_back( pos(oldBfsl[i].x + shift[j][0], oldBfsl[i].y + shift[j][1]) );
				}
			}
		}
	}
	return startPos;
}

///<summary>
///Shifts the grid around so that the cursor position always lands on a non-mined tile. <para />
///The shifting is done to the tile with the least amount of mines around it when the field is wrapped (tiled).
///</summary>
void shiftGrid(pos cursor) {
	clearTileNumeration();
	numerateTiles_looping();

	pos shiftTo = findEmptyTile(cursor);
	shiftTo.x -= cursor.x;
	shiftTo.y -= cursor.y;
	int i, j;
	vector< vector< tile > > shiftedGrid;
	shiftedGrid.resize(gridx);
	for (i = 0; i < gridx; i++) {
		shiftedGrid[i].resize(gridy);
	}
	for (i = 0; i < gridx; i++) {
		for (j = 0; j < gridy; j++) {
			shiftedGrid[i][j] = grid[modNeg(i + shiftTo.x,gridx)][modNeg(j + shiftTo.y,gridy)];
		}

	}

	grid = shiftedGrid;

	clearTileNumeration();
	numerateTiles();

}

///<summary> Prints a tile the way the user would see it. </summary>
void printTile(tile t) {
	if (t.flagged && !t.discovered) { cout << "!"; return; }
	if (!t.discovered) { cout << "#"; return; }
	if (t.hasMine) { cout << "@"; return; }
	if (!t.nearMines) { cout << " "; return; }
	cout << t.nearMines;
}

///<summary> Prints the entire grid, using printTile. </summary>
void printGrid() {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			printTile(grid[i][j]);
		}
		cout << endl;
	}
}

///<summary> Prints the grid with a 'highlight' (different char) under the cursor.</summary>
void printGridHighlight(pos cursor) {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			if (i == cursor.x && j == cursor.y) { cout << char(219); continue; };
			printTile(grid[i][j]);
		}
		cout << endl;
	}
}

///<summary> Prints the grid with warping around the cursor.</summary>
void printGridWarp(pos cursor) {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			if (j == cursor.y) { cout << " "; }
			printTile(grid[i][j]);
			if (j == cursor.y) { cout << " "; }
		}
		if (i == cursor.x - 1 || i == cursor.x ) { cout << endl; }
		cout << endl;
	}
}

///<summary> Prints the grid with warping and a crosshair around the cursor.</summary>
void printGridCrosshair(pos cursor) {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			if (j == cursor.y) {
				if (i == cursor.x) {
					cout << "-";
				}
				if (i != cursor.x) {
					cout << " ";
				}
			}
			printTile(grid[i][j]);
			if (j == cursor.y) {
				if (i == cursor.x) {
					cout << "-";
				}
				if (i != cursor.x) {
					cout << " ";
				}
			}
		}
		if (i == cursor.x - 1 || i == cursor.x) { 
			cout << endl;
			for (int j = 0; j < gridy; j++) {
				if (j == cursor.y + 1) { cout << "|"; break; }
				else {
					cout << " ";
				}
			}
		}

		cout << endl;
	}
}

///<summary> Prints the grid spaced and with a crosshair around the cursor.</summary>
void printGridCrosshair2(pos cursor) {

	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			if (j == cursor.y) {
				if (i == cursor.x) {
					cout << "-";
				}
				if (i != cursor.x) {
					cout << " ";
				}
			} else {
				cout << " ";
			}
			printTile(grid[i][j]);
			if (j == cursor.y) {
				if (i == cursor.x) {
					cout << "-";
				}
				if (i != cursor.x) {
					cout << " ";
				}
			} else {
				cout << " ";
			}
		}

		cout << endl;
		if (i == cursor.x - 1 || i == cursor.x) {
			
			for (int j = 0; j < gridy*3; j++) {
				if (j == cursor.y*3 + 1) { cout << "|"; break; }
				else {
					cout << " ";
				}
			}
		}
		cout << endl;
	}
}

///<summary> Places a flag and tells you if you've won (placed all falgs correctly).</summary>
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

///<summary> Like clicking - discovers tiles recursively, does not discover more if the tile is dangerous.</summary>
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

///<summary> Makes all tiles discovered.</summary>
void discoverAll() {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			grid[i][j].discovered = true;
		}
	}
}

///<summary> Hides (undiscovers) all tiles. </summary>
void hideAll() {
	for (int i = 0; i < gridx; i++) {
		for (int j = 0; j < gridy; j++) {
			grid[i][j].discovered = false;
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

///<summary> Lets the user play.</summary>
///<param name="printMethod"> The method to use for printing the grid. (0 - crosshair2, 1 - highlight, 2 - warp, 3 - crosshair)</param>
int interactiveMode(int printMethod=0) {
	cout << "Interactive mode - use the w, a, s and d keys to move a cursor around, space to click and q to toggle flags!\nPress any key to start! ";

	bool won=false;
	pos cursor(gridx/2,gridy/2);
	char action = _getch();

	while(!won){
		clearTerminal();
		switch(printMethod){
			case 0:
				printGridCrosshair2(cursor);
				break;
			case 1:
				printGridHighlight(cursor);
				break;
			case 2:
				printGridWarp(cursor);
				break;
			case 3:
				printGridCrosshair(cursor);
				break;
		}
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
	cout << "Enter grid dimensions: ";
	cin >> gridx >> gridy;
	cout << "Enter amount of mines: ";
	cin >> mines;
	clock_t timer = clock();
	generateGrid(100000);//Starts to get slow at ~1 000 000, takes ~1.6 secs
	cout << "Grid generation time: " << float(clock() - timer) / float(CLOCKS_PER_SEC) << " seconds.\n";
	cout << "User mode(1) or solver mode(0)?: ";
	int mode;
	cin >> mode;

	if(mode == 2){
		discoverAll();
		printGridCrosshair2(pos(5, 5));
		cout << "\n\n\nShifted:\n";

		shiftGrid(pos(5, 5));
		printGridCrosshair2(pos(5,5));
		cout << endl;
		return 2;
	}
	
	if (mode == 1) {
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
		return 1;
	}



    return 0;
}

