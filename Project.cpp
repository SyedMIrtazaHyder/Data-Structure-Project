﻿#include <iostream>
#include <list>
#include <stack>
#include <vector>
#include <algorithm>
#include <math.h>
#include <conio.h>
#include <set>
#include <map>
#include <cmath>
#include <queue>
#include <compare>

using namespace std;
class Pieces;
class Player;

int* decodePosition(string pos);
bool validPosition(string Space);
string toMove(int, int);
void displayBoard();
void AITurn(Player&, map<Pieces*, set<string>>&, int&);

struct Moves {
	int value;
	string move;

	Moves(string move, int value) : value(value), move(move) {}

	bool operator < (const Moves& alt) const {
		//cout << value << " & " << alt.value << endl;
		return this->value < alt.value;
	}

	bool operator <= (const Moves& alt) const {
		//cout << value << " & " << alt.value << endl;
		return this->value <= alt.value;
	}

	bool operator > (const Moves& alt) const {
		//cout << value << " & " << alt.value << endl;
		return this->value > alt.value;
	}

	Moves(): value(-1),move("NULL") {}
};
//call capture logic when a piece is captured like we did for enpassant
//all directions based on black side
//make capture Moves a priority Queue for AI
//Add the checkmate lose condition
//may use regex to make searching faster and easier

bool Enpassant = false;
Pieces* getEnPassant = NULL;
Pieces* checkEnPassant;
vector< vector<Pieces*> > board(8, vector<Pieces*>(8, NULL));
string kingToMove;//0 = white, 1 = black
int castling = 0;//-1 for queen Side, 1 for king side, 0 for not possible
string castlingMoves[4] = { "g8", "g1", "c8", "c1" };

//Module 1-Pieces
class Pieces {
public:
	int x, y, value;
	string name;
	stack<int> prevX;
	stack<int> prevY;
	set<string> possibleMoves;//listing all the possible moves possible by a certain piece
	set<string> captureMoves;//lists all the possible pieces current piece can capture
	priority_queue<Moves> bestMoves;
	Pieces() :x(0), y(0), value(0), name("NULL") {}
	Pieces(string pos, string name, int value) :x(pos[0] - 'a'), y(pos[1] - '1'), value(value), name(name) {
		board[y][x] = this;

		//pushes original position of piece onto stack.
		prevX.push(x);
		prevY.push(y);
	}
	//string validCastling[4] = {"g8", "g1", "c8", "c1"};

	bool isWhite() {
		return name[0] == 'w';
	}

	bool isOpponent(Pieces &Opp) {
		return name[0] != Opp.name[0];
	}

	void resetMoves() {
		while (!bestMoves.empty())
			bestMoves.pop();
		possibleMoves.clear();
		captureMoves.clear();
	}

	bool validMoves(string pos) {
		checkEnPassant = getEnPassant;
		pseudoLegalMoves();//EnPassant decided after this line
		cout << "Moves: ";
		for (string move : this->possibleMoves)
			cout << move << " ";

		cout << "\nCaptures: ";
		for (string move : this->captureMoves)
			cout << move << " ";

		cout << endl;
		return find(possibleMoves.begin(), possibleMoves.end(), pos) != possibleMoves.end();
	}

	void move(string pos) {
		//Pushes the new position of the piece onto the stack.
		//Pieces* checkEnPassant = getEnPassant;
		//for (string move : this->possibleMoves)
		//	cout << move << " ";

		if (getEnPassant != NULL && getEnPassant == this && (//for enPassant
			(toMove(x, y + 2) != pos && this->isWhite()) ||
			(toMove(x, y - 2) != pos && this->name[0] == 'b')
			)) {
			getEnPassant = NULL;
		}

		if (checkEnPassant == getEnPassant && getEnPassant != NULL)//may have sm bugs but idk not gonna deep test dis
		{
			if ((pos == toMove(getEnPassant->x, getEnPassant->y - 1) && getEnPassant->isWhite()) ||
				(pos == toMove(getEnPassant->x, getEnPassant->y + 1) && getEnPassant->name[0] == 'b'))
			{
				Pieces* capturedPiece = board[getEnPassant->y][getEnPassant->x];
				capturedPiece->name = "\0";
			}
			getEnPassant = NULL;

		}

		if (castling != 0){// && isCastlingMove(move)) {//code this new function
			if (find(castlingMoves, castlingMoves + 4, pos) == castlingMoves + 4)//failsafe for invalid castling moves
				castling = 0;

			if (castling == 1)//king side castle
			{
				castling = 0;//need to reset to prevent rook to make infinte loop when it calls move function again
				board[this->y][7]->move(toMove(5, this->y));
			}
			else if (castling == -1) {//queen side castle
				castling = 0;
				board[this->y][0]->move(toMove(3, this->y));
			}
		}


		if (board[pos[1] - '1'][pos[0] - 'a'] != NULL) {
			Pieces* capturedPiece = board[pos[1] - '1'][pos[0] - 'a'];
			capturedPiece->name = "\0";
		}

		//system("pause");
		prevX.push(x);
		prevY.push(y);


		board[y][x] = NULL;
		x = pos[0] - 'a';
		y = pos[1] - '1';
		board[y][x] = this;

	}

	void undoMove()
	{
		int oldx = prevX.top();
		int oldy = prevY.top();

		board[y][x] = NULL;
		x = oldx;
		y = oldy;
		board[y][x] = this;

		prevX.pop();
		prevY.pop();
	}

	bool isProtected();

	bool verticalPin() {//so piece cannot move right or left
		//searching a whole file (up and down the piece)
		bool kingInFile = false;
		bool threatInFile = false;

		int file = this->x;
		int rank = this->y;
		for (int i = rank - 1; i >= 0; i--)
			if (board[i][file] != NULL) {
				if (!board[i][file]->isOpponent(*this)) {//checking if king in file
					if (board[i][file]->name[1] == 'K')
						kingInFile = true;
					break;
				}
				else {
					if (!board[i][file]->name[1] == 'Q' || board[i][file]->name[1] == 'R')
						threatInFile = true;
					break;
				}
			}

		for (int i = rank + 1; i < 8; i++)//finding king
			if (board[i][file] != NULL) {
				if (!board[i][file]->isOpponent(*this)) {//checking if king in file
					if (board[i][file]->name[1] == 'K')
						kingInFile = true;
					break;//if any other white piece in way still should break
				}
				else {
					if (board[i][file]->name[1] == 'Q' || board[i][file]->name[1] == 'R')
						threatInFile = true;
					break;
				}
			}

		return kingInFile && threatInFile;
	}

	bool horizontalPin() {//have Enpassanat variation
		//searching a whole rank (left and right of the pieces)
		bool kingInRank = false;
		bool threatInRank = false;

		int rank = this->y;
		int file = this->x;
		for (int i = file + 1; i < 8; i++)
			if (board[rank][i] != NULL) {
				string pieceName = board[rank][i]->name;
				if (pieceName[0] == this->name[0])//checking if king in file
				{
					if (pieceName[1] == 'K')
						kingInRank = true;
					break;
				}

				else {
					if (pieceName[1] == 'Q' || pieceName[1] == 'R')
						threatInRank = true;
					break;
				}
			}

		for (int i = file - 1; i >= 0; i--)
			if (board[rank][i] != NULL) {
				string pieceName = board[rank][i]->name;
				if (pieceName[0] == this->name[0])//checking if king in file immediately
				{
					if (pieceName[1] == 'K')
						kingInRank = true;
					break;
				}

				else {
					if (pieceName[1] == 'Q' || pieceName[1] == 'R')
						threatInRank = true;
					break;
				}
			}
		return kingInRank && threatInRank;
	}

	bool rightDiagonalPin() {
		bool diagonalThreat = false;
		bool kingInDiagonal = false;

		int rank = this->y;
		int file = this->x;

		//NE -> SW
		int i = file + 1; int j = rank + 1;
		for (; i < 8 && j < 8; i++, j++) //NE
			if (board[j][i] != NULL) {
				string pieceName = board[j][i]->name;
				if (pieceName[0] == this->name[0])//checking if king in file immediately
				{
					if (pieceName[1] == 'K')
						kingInDiagonal = true;
					break;
				}

				else {
					if (pieceName[1] == 'Q' || pieceName[1] == 'B')
						diagonalThreat = true;
					break;
				}
			}

		i = file - 1; j = rank - 1;
		for (; i >= 0 && j >= 0; i--, j--)//SW
			if (board[j][i] != NULL) {
				string pieceName = board[j][i]->name;
				if (pieceName[0] == this->name[0])//checking if king in file immediately
				{
					if (pieceName[1] == 'K')
						kingInDiagonal = true;
					break;
				}

				else {
					if (pieceName[1] == 'Q' || pieceName[1] == 'B')
						diagonalThreat = true;
					break;
				}
			}
		return diagonalThreat && kingInDiagonal;
	}

	bool leftDiagonalPin() {
		bool diagonalThreat = false;
		bool kingInDiagonal = false;
		int rank = this->y;
		int file = this->x;

		//NW -> SE
		int i = file - 1; int j = rank + 1;
		for (; i >= 0 && j < 8; i--, j++)//NW
			if (board[j][i] != NULL) {
				string pieceName = board[j][i]->name;
				if (pieceName[0] == this->name[0])//checking if king in file immediately
				{
					if (pieceName[1] == 'K')
						kingInDiagonal = true;
					break;
				}
				else {
					if (pieceName[1] == 'Q' || pieceName[1] == 'B')
						diagonalThreat = true;
					break;
				}
			}

		i = file + 1; j = rank - 1;
		for (; i < 8 && j >= 0; i++, j--)//SE
			if (board[j][i] != NULL) {
				string pieceName = board[j][i]->name;
				if (pieceName[0] == this->name[0])//checking if king in file immediately
				{
					if (pieceName[1] == 'K')
						kingInDiagonal = true;
					break;
				}

				else {
					if (pieceName[1] == 'Q' || pieceName[1] == 'B')
						diagonalThreat = true;
					break;
				}
			}
		return diagonalThreat && kingInDiagonal;
	}

	virtual set<string> pseudoLegalMoves() = 0;
	virtual Moves BM() = 0; //BestMove
};

class Pawn :public Pieces {
public:
	Pawn(string pos, string color) :Pieces(pos, color + "P", 45) {}
	//list<string> possibleMoves;//will move this into pieces later as a set, as two pieces can never have the same start pos but can have same end pos

	set<string> pseudoLegalMoves() {//generating a rough lookup-table
		resetMoves();
		//for white
		bool Hpin = horizontalPin();
		bool Vpin = verticalPin();
		bool LDpin = leftDiagonalPin();
		bool RDpin = rightDiagonalPin();

		if (isWhite()) {
			if (x > 0 && y < 7 && board[y + 1][x - 1] != NULL && board[y + 1][x - 1]->name[0] == 'b' && !Vpin && !Hpin && !RDpin)//capture left
			{
				possibleMoves.insert(toMove(x - 1, y + 1));
				captureMoves.insert(toMove(x - 1, y + 1));
			}

			if (x < 7 && y < 7 && board[y + 1][x + 1] != NULL && board[y + 1][x + 1]->name[0] == 'b' && !Vpin && !Hpin && !LDpin)//capture right
			{
				possibleMoves.insert(toMove(x + 1, y + 1));
				captureMoves.insert(toMove(x + 1, y + 1));
			}

			if (this->y == 1 && board[y + 2][x] == NULL && board[y + 1][x] == NULL && !Hpin && !LDpin && !RDpin)//double push so chance to enpassant
			{
				possibleMoves.insert(toMove(x, y + 2));
				if ((x > 0 && board[y + 2][x - 1] != NULL && board[y + 2][x - 1]->name == "bP") ||
					(x < 7 && board[y + 2][x + 1] != NULL && board[y + 2][x + 1]->name == "bP") //checking if pawn on right or left
					)//if this is the move made then update enPassant var
					getEnPassant = this;
			}

			if (y < 7 && board[y + 1][x] == NULL && !Hpin && !LDpin && !RDpin)//single push l8r with promotion
				possibleMoves.insert(toMove(x, y + 1));//check for promotion later

			if (y == 4 && !Vpin && !Hpin) {//enPassant baybee, need to check for both horizontal and vertical pins smfh

				Pieces* west = NULL;
				Pieces* east = NULL;
				if (x > 0)
					west = board[y][x - 1];
				if (x < 7)
					east = board[y][x + 1];

				if (west != NULL && west == getEnPassant && !RDpin)
				{
					possibleMoves.insert(toMove(x - 1, y + 1));
					captureMoves.insert(toMove(x - 1, y));
				}
				if (east != NULL && east == getEnPassant && !LDpin)
				{
					possibleMoves.insert(toMove(x + 1, y + 1));
					captureMoves.insert(toMove(x + 1, y));
				}

			}
		}
		else {
			if (x > 0 && y > 0 && board[y - 1][x - 1] != NULL && board[y - 1][x - 1]->isWhite() && !Vpin && !Hpin && !LDpin)//capture left
			{
				possibleMoves.insert(toMove(x - 1, y - 1));
				captureMoves.insert(toMove(x - 1, y - 1));
			}

			if (x < 7 && y > 0 && board[y - 1][x + 1] != NULL && board[y - 1][x + 1]->isWhite() && !Vpin && !Hpin && !RDpin)//capture right
			{
				possibleMoves.insert(toMove(x + 1, y - 1));
				captureMoves.insert(toMove(x + 1, y - 1));
			}

			if (this->y == 6 && board[y - 2][x] == NULL && board[y - 1][x] == NULL && !Hpin && !LDpin && !RDpin) {//Double push
				possibleMoves.insert(toMove(x, y - 2));
				if (
					(x > 0 && board[y - 2][x - 1] != NULL && board[y - 2][x - 1]->name == "wP") ||
					(x < 7 && board[y - 2][x + 1] != NULL && board[y - 2][x + 1]->name == "wP")
					)//checking if pawn can be enpassanted
					getEnPassant = this;
			}

			if (y > 0 && board[y - 1][x] == NULL && !Hpin && !LDpin && !RDpin)//single push
				possibleMoves.insert(toMove(x, y - 1));//promotion logic exists so will make AI most likely only choose to promote queen ???

			if (y == 3 && !Hpin && !Vpin) {//lmao get enPassanted 
				Pieces* west = NULL;
				Pieces* east = NULL;
				if (x > 0)
					west = board[y][x - 1];
				if (x < 7)
					east = board[y][x + 1];;

				if (west != NULL && west == getEnPassant && !LDpin)
				{
					possibleMoves.insert(toMove(x - 1, y - 1));
					captureMoves.insert(toMove(x - 1, y));
				}
				if (east != NULL && east == getEnPassant && !RDpin)
				{
					possibleMoves.insert(toMove(x + 1, y - 1));
					captureMoves.insert(toMove(x + 1, y));
				}

			}
		}
		return possibleMoves;
	}

	Moves BM() {
		pseudoLegalMoves();
		if (possibleMoves.empty()) {
			return Moves("NULL", -1);
		}
		for (string moves : possibleMoves) {
			int* pos = decodePosition(moves);
			int attackX = pos[0], attackY = pos[1];
			if (this->isWhite())
				attackY += 1;
			else
				attackY -= 1;
			//if (board[attackY][attackX]->value >= this->value)//pawn can capture other pawn
			if (prevY.size() > 1 && (
				abs(this->y - 7) == 1 || abs(this->y - 0) == 1))//prefer promotion move
				bestMoves.push(*(new Moves(moves, 500)));

			if (attackX < 7 && attackY < 7 && attackY > 0 && board[attackY][attackX + 1] != NULL) { //pawn protect and attack right variation
				if (!board[attackY][attackX + 1]->isOpponent(*this)) {
					int val = 10;
					if ((attackY == 4 || attackY == 5) && (attackX == 4 || attackX == 5))
						val += 5;
					bestMoves.push(*(new Moves(moves, val)));
				}
				else//attacking piece
					bestMoves.push(*(new Moves(moves, board[attackY][attackX + 1]->value)));
			}

			if (attackX > 0 && attackY < 7 && attackY > 0 && board[attackY][attackX - 1] != NULL) { //pawn protect and attack right variation
				if (!board[attackY][attackX - 1]->isOpponent(*this)) {
					int val = 10;
					if ((attackY == 4 || attackY == 5) && (attackX == 4 || attackX == 5))
						val += 5;
					bestMoves.push(*(new Moves(moves, val)));
				}
				else//attacking piece
					bestMoves.push(*(new Moves(moves, board[attackY][attackX - 1]->value)));
			}

			else
				bestMoves.push(*(new Moves(*possibleMoves.begin(), 6)));//so at least there is 1 element in priority queue
		}

		for (string moves : captureMoves) {
			int* pos = decodePosition(moves);
			int attackX = pos[0], attackY = pos[1];
			bestMoves.push(*(new Moves(moves, board[attackY][attackX]->value)));
		}
		return bestMoves.top();
	}
};

class Knight :public Pieces {
public:
	Knight(string pos, string color) :Pieces(pos, color + "N", 55) {}

	//if horse pinned it cannot moved anywhere
	set<string> pseudoLegalMoves() {
		resetMoves();
		//moving like an octopus smh
		//--|
		bool pin = horizontalPin() && verticalPin() && leftDiagonalPin() && rightDiagonalPin();
		if (pin)//return empty set as horse has no valid moves
			return possibleMoves;

		if (this->x + 2 < 8 && this->y + 1 < 8 && (board[this->y + 1][this->x + 2] == NULL || board[this->y + 1][this->x + 2]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x + 2, this->y + 1));
			if (board[this->y + 1][this->x + 2] != NULL && board[this->y + 1][this->x + 2]->isOpponent(*this))
				captureMoves.insert(toMove(this->x + 2, this->y + 1));
		}
		if (this->x + 2 < 8 && this->y - 1 >= 0 && (board[this->y - 1][this->x + 2] == NULL || board[this->y - 1][this->x + 2]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x + 2, this->y - 1));
			if (board[this->y - 1][this->x + 2] != NULL && board[this->y - 1][this->x + 2]->isOpponent(*this))
				captureMoves.insert(toMove(this->x + 2, this->y - 1));

		}

		//|--
		if (this->x - 2 >= 0 && this->y + 1 < 8 && (board[this->y + 1][this->x - 2] == NULL || board[this->y + 1][this->x - 2]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x - 2, this->y + 1));
			if (board[this->y + 1][this->x - 2] != NULL && board[this->y + 1][this->x - 2]->isOpponent(*this))
				captureMoves.insert(toMove(this->x - 2, this->y + 1));
		}
		if (this->x - 2 >= 0 && this->y - 1 >= 0 && (board[this->y - 1][this->x - 2] == NULL || board[this->y - 1][this->x - 2]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x - 2, this->y - 1));
			if (board[this->y - 1][this->x - 2] != NULL && board[this->y - 1][this->x - 2]->isOpponent(*this))
				captureMoves.insert(toMove(this->x - 2, this->y - 1));
		}

		// |
		//-|
		// |
		if (this->x + 1 < 8 && this->y + 2 < 8 && (board[this->y + 2][this->x + 1] == NULL || board[this->y + 2][this->x + 1]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x + 1, this->y + 2));
			if (board[this->y + 2][this->x + 1] != NULL && board[this->y + 2][this->x + 1]->isOpponent(*this))
				captureMoves.insert(toMove(this->x + 1, this->y + 2));
		}
		if (this->x + 1 < 8 && this->y - 2 >= 0 && (board[this->y - 2][this->x + 1] == NULL || board[this->y - 2][this->x + 1]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x + 1, this->y - 2));
			if (board[this->y - 2][this->x + 1] != NULL && board[this->y - 2][this->x + 1]->isOpponent(*this))
				captureMoves.insert(toMove(this->x + 1, this->y - 2));
		}

		//|
		//|-
		//|
		if (this->x - 1 >= 0 && this->y + 2 < 8 && (board[this->y + 2][this->x - 1] == NULL || board[this->y + 2][this->x - 1]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x - 1, this->y + 2));
			if (board[this->y + 2][this->x - 1] != NULL && board[this->y + 2][this->x - 1]->isOpponent(*this))
				captureMoves.insert(toMove(this->x - 1, this->y + 2));
		}
		if (this->x - 1 >= 0 && this->y - 2 >= 0 && (board[this->y - 2][this->x - 1] == NULL || board[this->y - 2][this->x - 1]->isOpponent(*this)))
		{
			possibleMoves.insert(toMove(this->x - 1, this->y - 2));
			if (board[this->y - 2][this->x - 1] != NULL && board[this->y - 2][this->x - 1]->isOpponent(*this))
				captureMoves.insert(toMove(this->x - 1, this->y - 2));
		}

		return possibleMoves;
	}

	Moves BM() {
		pseudoLegalMoves();
		if (possibleMoves.empty()) {
			return Moves("NULL", -1);
		}
		for (string moves : captureMoves) {
			int* pos = decodePosition(moves);
			int attackX = pos[0], attackY = pos[1];
			bestMoves.push(*(new Moves(moves, board[attackY][attackX]->value)));
			possibleMoves.erase(possibleMoves.find(moves));
		}

		priority_queue<Moves> temp = bestMoves;//as bestMoves cleared due to reset function
		const set<string> copy = possibleMoves;
		for (string moves : copy) {
			move(moves);//future attacks
			pseudoLegalMoves();
			int val = 15;
			if (this->x == 0 || this->x == 7 || this->y == 0 || this->y == 7)//reducing chances of going to the rim
				val -= 15;
			temp.push(*(new Moves(moves, captureMoves.size() * 6 + val)));
			undoMove();
		}

		vector<Moves> v;
		for (int i = 0; i < 3 && !temp.empty(); i++) {
			if (v.size() > 0 && abs(v[0].value - temp.top().value) > 15)
				break;
			v.push_back(temp.top());
			temp.pop();
		}

		srand(unsigned(time(NULL)));
		int pos = rand() % v.size();
		return v[pos];
	}
};

class Bishop :public Pieces {
public:
	Bishop(string pos, string color) :Pieces(pos, color + "B", 55) {}

	int rays(bool isNorth, bool isEast) {//lasers generator
		int skewer = 10;
		int dirX, dirY;
		if (isNorth)
			dirY = 1;
		else
			dirY = -1;

		if (isEast)
			dirX = 1;
		else
			dirX = -1;

		int i,j;
		for (i = this->y + dirY, j = this->x + dirX; i >= 0 && i < 8 && j < 8 && j >= 0; i+=dirY, j+=dirX)//se
		{
			if (board[i][j] != NULL)
			{
				if (board[i][j]->isOpponent(*this))
				{
					switch (board[i][j]->name[1]) {
					case 'K':
						skewer += 60;
						break;
					case 'Q':
						skewer += 85;
						break;
					case 'R':
						skewer += 53;
						break;
					case 'B':
						skewer -= 30;
						break;
					case 'N':
						skewer += 35;
						break;

					case 'P':
						skewer -= 30;
						break;
					}
				}

				else
					skewer -= 60;

				if (abs(i - this->y) == 1 && abs(j - this->x) == 1 && board[i][j]->isOpponent(*this)
					&& board[i][j]->name[1] == 'K' || board[i][j]->name[1] == 'Q')//if right next to the piece move not so good
					skewer -= 15;
			}
		}

		return skewer;
	}

	set<string> pseudoLegalMoves() {
		bool Hpin = horizontalPin();
		bool Vpin = verticalPin();
		bool LDpin = leftDiagonalPin();
		bool RDpin = rightDiagonalPin();

		int i = 0; int j = 0;
		resetMoves();

		//if bishop binned vertically or horizontally then it cannot move
		if (Hpin || Vpin)
			return possibleMoves;

		for (i = this->y - 1, j = this->x - 1; i >= 0 && j >= 0 && !LDpin; i--, j--) {//sw
			if (board[i][j] != NULL)
			{
				if (board[i][j]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
					captureMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
				}
				break;
			}
			possibleMoves.insert(toMove(j, i));
		}

		for (i = this->y - 1, j = this->x + 1; i >= 0 && j < 8 && !RDpin; i--, j++)//se
		{
			if (board[i][j] != NULL)
			{
				if (board[i][j]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
					captureMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
				}
				break;
			}
			possibleMoves.insert(toMove(j, i));

		}

		for (i = this->y + 1, j = this->x - 1; i < 8 && j >= 0 && !RDpin; i++, j--)//nw
		{
			if (board[i][j] != NULL)
			{
				if (board[i][j]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
					captureMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
				}
				break;
			}
			possibleMoves.insert(toMove(j, i));

		}

		for (i = this->y + 1, j = this->x + 1; i < 8 && j < 8 && !LDpin; i++, j++)//ne
		{
			if (board[i][j] != NULL)
			{
				if (board[i][j]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
					captureMoves.insert(toMove(j, i));//adding the possible capture/attacked piece
				}
				break;
			}
			possibleMoves.insert(toMove(j, i));

		}

		return possibleMoves;
	}

	Moves BM() {
		pseudoLegalMoves();
		if (possibleMoves.empty()) {
			return Moves("NULL", -1);
		}
		for (string moves : captureMoves) {
			int* pos = decodePosition(moves);
			int attackX = pos[0], attackY = pos[1];
			bestMoves.push(*(new Moves(moves, board[attackY][attackX]->value)));
			possibleMoves.erase(possibleMoves.find(moves));
		}
		const set<string> copy = possibleMoves;
		for (string moves : copy) { //laser moves --- to pin pieces
			move(moves);//future attacks
			bestMoves.push(*(new Moves(moves, (rays(0, 0) + rays(0, 1) + rays(1, 0) + rays(1, 1))/3)));//average attack of diagonal
			undoMove();
		}
		vector<Moves> v;
		for (int i = 0; i < 3 && !bestMoves.empty(); i++) {
			if (v.size() > 0 && abs(v[0].value - bestMoves.top().value) > 15)
				break;
			v.push_back(bestMoves.top());
			bestMoves.pop();
		}

		srand(unsigned(time(NULL)));
		int pos = rand() % v.size();
		return v[pos];

	}
};

class Rook :public Pieces {
public:
	Rook(string pos, string color) :Pieces(pos, color + "R", 70) {}
	//sliding piece so need to just check for obstacles

	int lasers(bool isVertical, bool increment) {//lasers generator
		int skewer = 10;
		int dirX, dirY;
		if (isVertical)
		{
			dirX = 0;
			dirY = pow(-1, increment + 1);
		}
		else
		{
			dirY = 0;
			dirX = pow(-1, increment + 1);
		}
	

		int i, j;
		for (i = this->y + dirY, j = this->x + dirX; i >= 0 && i < 8 && j < 8 && j >= 0; i += dirY, j += dirX)//se
		{
			if (board[i][j] != NULL)
			{
				if (board[i][j]->isOpponent(*this))
				{
					switch (board[i][j]->name[1]) {
					case 'K':
						skewer += 80;
						break;
					case 'Q':
						skewer += 65;
						break;
					case 'R':
						skewer -= 60;
						break;
					case 'B':
						skewer += 55;
						break;
					case 'N':
						skewer += 55;
						break;

					case 'P':
						skewer += 5;
						break;
					}
				}

				else
					skewer -= 60;

				if (abs(i - this->y) <= 1 && abs(j - this->x) <= 1 && board[i][j]->isOpponent(*this)
					&& board[i][j]->name[1] == 'K' || board[i][j]->name[1] == 'Q')
					skewer -= 15;
			}

			else {
				if (i < 7 && j < 7 && i > 0 && j > 0) {
					float mul = 0;
					if (i < 6 && j < 6 && i > 1 && j > 1)
						mul = 1;
					if (board[i + 1][j + 1] == NULL)
						mul += 0.5;
					if (board[i + 1][j - 1] == NULL)
						mul += 0.5;
					if (board[i - 1][j + 1] == NULL)
						mul += 0.5;;
					if (board[i - 1][j - 1] == NULL)
						mul += 0.5;;
					if (board[i][j + 1] == NULL)
						mul += 0.5;;
					if (board[i][j - 1] == NULL)
						mul += 0.5;;
					if (board[i + 1][j] == NULL)
						mul += 0.5;;
					if (board[i - 1][j] == NULL)
						mul += 0.5;
					skewer += (int) mul;
				}

			}
		}
		return skewer;
	}

	set<string> pseudoLegalMoves() {
		bool Hpin = horizontalPin();
		bool Vpin = verticalPin();
		bool LDpin = leftDiagonalPin();
		bool RDpin = rightDiagonalPin();

		resetMoves();

		if (LDpin || RDpin)
			return possibleMoves;

		for (int i = this->y - 1; i >= 0 && !Hpin; i--)//north
		{
			if (board[i][this->x] != NULL)
			{
				if (board[i][this->x]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(this->x, i));//adding the possible capture/attacked piece
					captureMoves.insert(toMove(this->x, i));//adding the possible capture/attacked piece
				}
				break;
			}
			possibleMoves.insert(toMove(this->x, i));
		}

		for (int i = this->y + 1; i < 8 && !Hpin; i++)//south
		{
			if (board[i][this->x] != NULL)//going till it is blocked
			{
				if (board[i][this->x]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(this->x, i));//adding the possible capture/attacked piece
					captureMoves.insert(toMove(this->x, i));//adding the possible capture/attacked piece
				}
				break;
			}
			possibleMoves.insert(toMove(this->x, i));
		}


		for (int j = this->x - 1; j >= 0 && !Vpin; j--)//west
		{
			if (board[this->y][j] != NULL)
			{
				if (board[this->y][j]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(j, this->y));
					captureMoves.insert(toMove(j, this->y));//adding the possible capture/attacked piece
				}
				break;
			}
			possibleMoves.insert(toMove(j, this->y));
		}

		for (int j = this->x + 1; j < 8 && !Vpin; j++)//east
		{
			if (board[this->y][j] != NULL)
			{
				if (board[this->y][j]->isOpponent(*this))
				{
					possibleMoves.insert(toMove(j, this->y));
					captureMoves.insert(toMove(j, this->y));//adding the possible capture/attacked piece
				}
				break;
			}

			possibleMoves.insert(toMove(j, this->y));
		}

		return possibleMoves;
	}

	Moves BM() {
		pseudoLegalMoves();
		if (possibleMoves.empty()) {
			return Moves("NULL", -1);
		}
		for (string moves : captureMoves) {
			int* pos = decodePosition(moves);
			int attackX = pos[0], attackY = pos[1];
			bestMoves.push(*(new Moves(moves, board[attackY][attackX]->value)));
			possibleMoves.erase(possibleMoves.find(moves));
		}
		const set<string> copy = possibleMoves;
		for (string moves : copy) { //laser moves --- to pin pieces
			move(moves);//future attacks
			bestMoves.push(*(new Moves(moves, (lasers(0, 0) + lasers(0, 1) + lasers(1, 0) + lasers(1, 1)) / 3)));//average attack of diagonal
			undoMove();
		}

		vector<Moves> v;
		for (int i = 0; i < 3 && !bestMoves.empty(); i++) {
			if (v.size() > 0 && abs(v[0].value - bestMoves.top().value) > 15)
				break;
			v.push_back(bestMoves.top());
			bestMoves.pop();
		}

		srand(unsigned(time(NULL)));
		int pos = rand() % v.size();
		return v[pos];
		}
};

class Queen :public Pieces {
public:
	Queen(string pos, string color) :Pieces(pos, color + "Q", 100) {}

	set<string> pseudoLegalMoves() {
		resetMoves();
		Bishop bisMoves = *((Bishop*)this);
		Rook rookMoves = *((Rook*)this);

		set<string> diagonals = bisMoves.pseudoLegalMoves();//downcasting piece as bishop and storing its possible moves
		possibleMoves = rookMoves.pseudoLegalMoves();//downcasting piece as rook and storing its possible moves
		set<string> diagonalCapture = bisMoves.captureMoves;
		captureMoves = rookMoves.captureMoves;

		//both above functions clear the possibleMoves list by default so hence need to save in seperate variables

		for (string potentialPos : diagonals)
			possibleMoves.insert(potentialPos);
		for (string potentialCapture : diagonalCapture)
			captureMoves.insert(potentialCapture);
		return possibleMoves;
	}

	Moves BM() {
		pseudoLegalMoves();
		if (possibleMoves.empty()) {
			return Moves("NULL", -1);
		}
		Bishop tempB = *((Bishop*)this);
		Rook tempR = *((Rook*)this);

		bestMoves.push(tempB.BM());
		bestMoves.push(tempR.BM());
		//bestMoves.push(*(new Moves(*possibleMoves.begin(), 0)));

		board[this->y][this->x] = this;
		return bestMoves.top();
	}
};

bool Pieces::isProtected() {//mainly for king capturing opposing/checking pieces
	//diagonals
	//chaging color to get pieces of same color using capture space
	bool safe = false;
	if (this->isWhite())
		this->name[0] = 'b';
	else
		this->name[0] = 'w';

	Queen capturedThreat = *((Queen*)this);
	capturedThreat.pseudoLegalMoves();
	//capturedThreat->pseudoLegalMoves();//generating captures for if the piece was the opposite color queen
	/*if (captureMoves.empty())
		return false;
	//cout << "Protected by: ";
	*/
	for (string pos : capturedThreat.captureMoves) {//ensure that only same color pieces in this list TESTING REQUIRED
		int* move = decodePosition(pos);
		string pieceName = board[move[1]][move[0]]->name;
		if ((move[1] == this->y || move[0] == this->x) &&//Protected vertically or horizontally
			(pieceName[1] == 'R' ||
				pieceName[1] == 'Q'))
			safe = true;

		else if (pieceName[1] == 'B' || pieceName[1] == 'Q')//otherwise protected by long diagonal
			safe = true;

		else if (pieceName[1] == 'P') {//pawn protection
			if (pieceName[0] == 'w' &&
				move[1] == this->y - 1 && (move[0] == this->x - 1 || move[0] == this->x + 1))
				safe = true;
			else if (move[1] == this->y + 1 && (move[0] == this->x - 1 || move[0] == this->x + 1))
				safe = true;
		}

		else if (pieceName[1] == 'K')//King protection
		{
			if ((move[0] == this->x - 1 || move[0] == this->x || move[0] == this->x + 1) &&
				(move[1] == this->y - 1 || move[1] == this->y || move[0] == this->y + 1))
				safe = true;
		}
	}

	Knight _capturedLThreat = *((Knight*)this);
	_capturedLThreat.pseudoLegalMoves();
	for (string captures : _capturedLThreat.captureMoves) {
		int* Lpos = decodePosition(captures);
		if (board[Lpos[1]][Lpos[0]]->name[1] == 'N')//capture only gives us same color pieces as the protected piece on the L position
			safe = true;
	}

	//resetting kings color
	if (this->isWhite())
		this->name[0] = 'b';
	else
		this->name[0] = 'w';

	//cout << endl;
	return safe;
}

class King :public Pieces {
	Player* opponent;
	//vector<string> castlingMoves(2, "0");
public:
	list<string> checks;
	King(string pos, string color) :Pieces(pos, color + "K", 999), opponent(NULL) {}

	void setOpponent(Player& opp);

	bool isChecked() {
		list<string> checked = checkedList();
		return !checked.empty();
	}

	list<string> checkedList() {//logic for checking if piece checked
		checks.clear();
		Rook LinearThreats = *((Rook*)this);
		Bishop DiagonalThreats = *((Bishop*)this);
		Knight LThreats = *((Knight*)this);
		Pawn pawnThreats = *((Pawn*)this);

		LinearThreats.pseudoLegalMoves();//this covers all horizontal and vertical threats the king may face
		LThreats.pseudoLegalMoves();//this covers Knight threats
		DiagonalThreats.pseudoLegalMoves();//covers diagonals
		pawnThreats.pseudoLegalMoves();//covers smaller siagonal with a pawn

		//merging all the threats
		for (string threat : LinearThreats.captureMoves) {
			int* pos = decodePosition(threat);	
			Pieces* opPiece = board[pos[1]][pos[0]];
			if(opPiece->isOpponent(*this) &&//linear check
				(opPiece->name[1] == 'R' ||
				opPiece->name[1] == 'Q')
				)
				checks.push_back(threat);
		}

		for (string threat : DiagonalThreats.captureMoves) {
			int* pos = decodePosition(threat);
			Pieces* opPiece = board[pos[1]][pos[0]];
			if (opPiece->isOpponent(*this) &&//diagonal check
				(opPiece->name[1] == 'B' ||
				opPiece->name[1] == 'Q')
				)
				checks.push_back(threat);
		}

		for (string threat : pawnThreats.captureMoves) {
			int* pos = decodePosition(threat);
			Pieces* opPiece = board[pos[1]][pos[0]];
			if (opPiece->isOpponent(*this) && opPiece->name[1] == 'P') //Checking for Pawn
				checks.push_back(threat);
		}

		for (string threat : LThreats.captureMoves) {
			int* pos = decodePosition(threat);
			Pieces* opPiece = board[pos[1]][pos[0]];
			if (opPiece->isOpponent(*this) && opPiece->name[1] == 'N') //Checking for Knight 
				checks.push_back(threat);
		}

		for (string threat : checks) {
			cout << threat << " ";
		}
		cout << endl;
		return checks;

	}

	set<string> pseudoLegalMoves(){
		resetMoves();
		bool castlingFlag = false;
		castling = 0;
		//N
		if (y > 0 && (board[this->y - 1][this->x] == NULL || board[this->y - 1][this->x]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x, this->y - 1));
		//S
		if (y < 7 && (board[this->y + 1][this->x] == NULL || board[this->y + 1][this->x]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x, this->y + 1));
		//E
		if (x < 7 && (board[this->y][this->x + 1] == NULL || board[this->y][this->x + 1]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x + 1, this->y));
		//W
		if (x > 0 && (board[this->y][this->x - 1] == NULL || board[this->y][this->x - 1]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x - 1, this->y));
		//NW
		if (y > 0 && x > 0 && (board[this->y - 1][this->x - 1] == NULL || board[this->y - 1][this->x - 1]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x - 1, this->y - 1));
		//SW
		if (y < 7 && x > 0 && (board[this->y + 1][this->x - 1] == NULL || board[this->y + 1][this->x - 1]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x - 1, this->y + 1));
		//SE
		if (y < 7 && x < 7 && (board[this->y + 1][this->x + 1] == NULL || board[this->y + 1][this->x + 1]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x + 1, this->y + 1));
		//NE
		if (y > 0 && x < 7 && (board[this->y - 1][this->x + 1] == NULL || board[this->y - 1][this->x + 1]->isOpponent(*this)))
			possibleMoves.insert(toMove(this->x + 1, this->y - 1));

		//checking for castling
		// add color of rook as well just to prevent any bugs in custom positions
		//Queen side castling hence west
		if (board[this->y][0] != NULL &&//checking if piece in that location exists
			board[this->y][0]->name[1] == 'R' && board[this->y][0]->prevX.size() == 1 &&//the square has same coloured unmoved Rook
			board[this->y][1] == NULL && board[this->y][2] == NULL && board[this->y][3] == NULL &&//empty spaces between left rook and king, filter out by legal moves
			this->prevX.size() == 1 && this->x == 4 && !isChecked())//king has not moved from its own position
		{
			possibleMoves.insert(toMove(this->x - 2, this->y));//Castling possible, write castling logic below
			castlingFlag = true;
		}

		//King side castling hence east
		if (board[this->y][7] != NULL &&//checking if piece in that location exists
			board[this->y][7]->name[1] == 'R' && board[this->y][7]->prevX.size() == 1 &&//the square has same coloured unmoved Rook
			board[this->y][6] == NULL && //automatically gets catered
			this->prevX.size() == 1 && this->x == 4 && !isChecked())//king has not moved from its own position
		{
			possibleMoves.insert(toMove(this->x + 2, this->y));//Castling possible, write castling logic below
			castlingFlag = true;
		}

		if (kingToMove != this->name)
			return possibleMoves;

		if (kingToMove == this->name)//filtering so king only has legal moves.
		{
			legalMoves();
			set<string> tempCopy(possibleMoves);
			for (string moves : possibleMoves) {
				int* coord = decodePosition(moves);
				if (board[coord[1]][coord[0]] != NULL && board[coord[1]][coord[0]]->isOpponent(*this))
					if (!board[coord[1]][coord[0]]->isProtected())
						captureMoves.insert(moves);
					else
						tempCopy.erase(tempCopy.find(moves));
			}
			possibleMoves = tempCopy;
		}

		//castling checks
		if (castlingFlag) {//need to revamp logic here as this should only generate moves not do them
			if (possibleMoves.find(toMove(5, this->y)) != possibleMoves.end() && possibleMoves.find(toMove(6, this->y)) != possibleMoves.end())//king side castling only possible if no rays at the kings final and initial position
				castling = 1;
				//board[this->y][7]->move(toMove(5, this->y));//put this logic somewhere else
			else {
				set<string>::iterator cond2 = possibleMoves.find(toMove(6, this->y));
				if (cond2 != possibleMoves.end())
					possibleMoves.erase(cond2);
			}
			if (possibleMoves.find(toMove(3, this->y)) != possibleMoves.end() && possibleMoves.find(toMove(2, this->y)) != possibleMoves.end())//same as above
				castling = -1;
				//board[this->y][0]->move(toMove(3, this->y));

			else{//otherwise removing the moves from the legal moves entirely
				set<string>::iterator cond1 = possibleMoves.find(toMove(2, this->y));
				if (cond1 != possibleMoves.end())
					possibleMoves.erase(cond1);
			}
		}
		
		return possibleMoves;
	}//change return to legalMoves function

	void legalMoves();

	Moves BM() {
		pseudoLegalMoves();
		if (possibleMoves.empty()) {
			return Moves("NULL", -1);
		}
		for (string moves : possibleMoves) {
			int* pos = decodePosition(moves);
			int attackX = pos[0], attackY = pos[1];
			//if (board[attackY][attackX]->value >= this->value)//pawn can capture other pawn
			if (prevY.size() == 1 && (find(castlingMoves, castlingMoves + 4, moves) != castlingMoves + 4))//prefer castling at first possible moment
				bestMoves.push(*(new Moves(moves, 50)));

			if (//if in the open try to take cover
				(attackX < 7 && board[attackY][attackX + 1] != NULL && !board[attackY][attackX + 1]->isOpponent(*this)) || //protect fellow pieces
				(attackX > 0 && board[attackY][attackX - 1] != NULL && !board[attackY][attackX - 1]->isOpponent(*this))) {
				int val = 6;
				if ((this->isWhite() && (attackY == 0 || attackY == 1)) ||
					(this->name[0] == 'b' && (attackY == 6 || attackY == 7)))
					val += 6;
				bestMoves.push(*(new Moves(moves, val)));
			}

			else
				bestMoves.push(*(new Moves(*possibleMoves.begin(), 0)));
		}
		for (string moves : captureMoves) {
			int* pos = decodePosition(moves);
			int attackX = pos[0], attackY = pos[1];
			bestMoves.push(*(new Moves(moves, board[attackY][attackX]->value)));
		}

		//adding slight randomness
		vector<Moves> v;
		for (int i = 0; i < 3 && !bestMoves.empty(); i++){
			v.push_back(bestMoves.top());
			bestMoves.pop();
		}

		srand(unsigned(time(NULL)));
		int pos = rand()%v.size();
		return v[pos];
	}
};

//Module 2-Player
class Player {
	bool isWhite, isChecked, AI;
	list<Pieces*> pieces;
	//These are the pieces that have been moved/captured
	stack<Pieces*> movedPieces;
	King* king;
	int counter;
public:
	void setPieces() {
		for (auto i = pieces.begin(); i != pieces.end();) {
			if ((*i)->name == "\0")
				pieces.erase(i++);
			else
				++i;
		}
	}

	void setAI(bool val) {
		AI = val;
	}

	bool isAI() {
		return AI;
	}

	void setCheck(bool check){
		isChecked = check;
	}

	bool Checked() {
		return isChecked;
	}

	King* getKing() {
		return king;
	}

	Player(bool isWhite) : isWhite(isWhite), isChecked(false), AI(false) {
		//The counter keeps track of how many undo's the player can do
		counter = 0;

		//Adding a buffer element so that the stack does not give errors
		movedPieces.push(NULL);
		//color only needed for king assignment
		if (isWhite)
		{
			king = new King("e1", "w");
		}
		else
		{
			king = new King("e8", "b");

		}
		pieces.push_back(king);
		//placing pawns
		for (char i = 'a'; i < 'i'; i++)
		{
			if (isWhite)
				pieces.push_back(new Pawn(string(1, i) + "2", "w"));
			else
				pieces.push_back(new Pawn(string(1, i) + "7", "b"));
		}

		if (isWhite) {
			pieces.push_back(new Queen("d1", "w"));
			pieces.push_back(new Bishop("c1", "w"));
			pieces.push_back(new Bishop("f1", "w"));
			pieces.push_back(new Knight("g1", "w"));
			pieces.push_back(new Knight("b1", "w"));
			pieces.push_back(new Rook("h1", "w"));
			pieces.push_back(new Rook("a1", "w"));
		}

		else {
			pieces.push_back(new Queen("d8", "b"));
			pieces.push_back(new Bishop("c8", "b"));
			pieces.push_back(new Bishop("f8", "b"));
			pieces.push_back(new Knight("g8", "b"));
			pieces.push_back(new Knight("b8", "b"));
			pieces.push_back(new Rook("a8", "b"));
			pieces.push_back(new Rook("h8", "b"));
		}

		//>>>>Add pieces here<<<<

		//DO NOT DEFINE KING HERE AS IT IS BEING IMPLICITY MADE WHEN PLAYER CREATED
		/*if (isWhite) {
			//Checkmate
			//pieces.push_back(new Rook("a7", "w"));
			//pieces.push_back(new Queen("d4", "w"));
			//pieces.push_back(new Pawn("f2", "w"));
			//pieces.push_back(new Pawn("c5", "w"));
			//Pins
			pieces.push_back(new Queen("c1", "w"));
			pieces.push_back(new Bishop("g4", "w"));
			//pieces.push_back(new Bishop("g5", "w"));
		}
		else {
			//checkMate
			//pieces.push_back(new Pawn("e4", "b"));
			//pieces.push_back(new Pawn("b7", "b"));
			//pieces.push_back(new Rook("a2", "b"));
			//pieces.push_back(new Rook("h8", "b"));
			//pieces.push_back(new Knight("g3", "b"));
			//Pins
			//pieces.push_back(new Queen("e7", "b"));
			//pieces.push_back(new Pawn("g2", "b"));
			//pieces.push_back(new Bishop("g5", "b"));
			pieces.push_back(new Queen("h3", "b"));
			//pieces.push_back(new Bishop("a3", "b"));
			//pieces.push_back(new Bishop("a4", "b"));
		}*/
	}

	list<Pieces*>& getPieces() {
		return pieces;
	}

	stack<Pieces*>& getMovedPieces()
	{
		return movedPieces;
	}

	void pushPiece(Pieces* pushed) {
		if (counter < 3)
		{
			counter++;
		}
		movedPieces.push(pushed);
	}

	void undo() {
		if (movedPieces.top() == NULL && !movedPieces.empty())
		{
			movedPieces.pop();
			//>>>>problem when returning the latest of the captured pieces<<<,
			//movedPieces.pop();//popping twice to retrive the captured piece as well
		}

		else if (movedPieces.size() > 1 && counter > 0)
		{
			movedPieces.top()->undoMove();
			movedPieces.pop();

			counter--;
		}
	}

	void Captured(Pieces* piece)
	{
		piece->prevX.push(piece->x);
		piece->prevY.push(piece->y);

		this->pushPiece(piece);
	}

	map<Pieces*, set<string>> LegalMovesInCheck() {//>>>>Avoding checkmate logic here, user only allowed to make these moves
		//atm just generating the kingMoves
		map<Pieces*, set<string>> movesInCheck;
		kingToMove = king->name;
		king->pseudoLegalMoves();
		if (!king->possibleMoves.empty())
			movesInCheck[king] = king->possibleMoves;//for avoiding general checks
		list<string> danger = king->checkedList();//getting positions where piece is checked from
		int* attackerPos = decodePosition(*danger.begin());
		Pieces* attackerPiece = board[attackerPos[1]][attackerPos[0]];
		if (danger.size() > 1)//for double check, can only move king
			return movesInCheck;

		/*for (string kingMove : king->possibleMoves)values already in unordered set
			danger.push_back(kingMove);*/

		//adding empty spaces where we can block forseen check
		if (attackerPos[0] == king->x)// so we know that opposing piece has vertical or horizontal ray
		{
			int mul = 1;
			if (attackerPos[1] > king->y)
				mul *= -1;
			for (int i = attackerPos[1] + mul; i != king->y; i += mul)
				danger.push_back(toMove(king->x, i));
		}

		else if (attackerPos[1] == king->y) {
			int mul = 1;
			if (attackerPos[0] > king->x)
				mul *= -1;
			for (int i = attackerPos[0] + mul; i != king->x; i += mul)
				danger.push_back(toMove(i, king->y));
		}

		else if (attackerPiece->name[1] != 'N' || attackerPiece->name[1] != 'P')//Hence if both of these eliminated then only diagonal check possible
		{
			int mulEW = 1, mulNS = 1;
			if (attackerPos[0] > king->x)
				mulEW *= -1;
			if (attackerPos[1] > king->y)
				mulNS *= -1;

			int i = attackerPos[0] + mulEW, j = attackerPos[1] + mulNS;
			for (; king->x != i && king->y != j; i += mulEW, j += mulNS) {
				danger.push_back(toMove(i, j));
			}
		}
		
		//generating all possible moves for all pieces and comparing it with moves in danger
		for (Pieces* p : pieces) {
			if (p == NULL)
				continue;
			if (p->name[1] == 'K')
				continue;
			p->pseudoLegalMoves();
			set<string> blockingMoves;
			for (string threatSpace : danger) {
				if (p->possibleMoves.find(threatSpace) != p->possibleMoves.end())
					blockingMoves.insert(threatSpace);
			}
			if (!blockingMoves.empty())
				movesInCheck[p] = blockingMoves;
		}

		return movesInCheck;
		//call function to get captured Pieces in each case
		//capturing the checking piece
		//if (attackerPiece->name[1] == 'N' || attackerPiece->name[1] == 'P')//checked by knight, pawn checked cannot be blocked so need to capture or move king
		//blocking check and not possible if checked by knight, pawn checked cannot be blocked, and king cannot give check
		


	}

	void Promotion(Pieces* piece)
	{
		if (piece->name[1] == 'P' && (piece->y == 0 || piece->y == 7))
		{
			char choice = ' ';
			string piecePosition = "Z0";
			piecePosition[0] = piece->x + 'a';
			piecePosition[1] = piece->y + '1';

			Captured(piece);

			if (isAI()) {
				pieces.push_back(new Queen(piecePosition, string(1, piece->name[0])));
				counter = 0;
				return;
			}

			cout << "\n\n";
			cout << "What would you like to promote your pawn into?\n";
			cout << "N for Knight - Q for Queen - B for Bishop - R for Rook\n" << "choice: ";
			while (choice != 'N' && choice != 'Q' && choice != 'B' && choice != 'R')
				cin >> choice;

			switch (choice)
			{
			case 'N':
				pieces.push_back(new Knight(piecePosition, string(1, piece->name[0])));
				break;
			case 'Q':
				pieces.push_back(new Queen(piecePosition, string(1, piece->name[0])));
				break;
			case 'B':
				pieces.push_back(new Bishop(piecePosition, string(1, piece->name[0])));
				break;
			case 'R':
				pieces.push_back(new Knight(piecePosition, string(1, piece->name[0])));
				break;
			}

			//Cannot undo a Promotion
			counter = 0;
		}
	}
};

//>>>>Forwading for the king functions<<<<
void King::setOpponent(Player& opp) {
	opponent = &opp;
}

void King::legalMoves() {//>>>Can be made more efficient by only going through the king moves, check isin function or smthn<<<
	//possibleMoves = pseudoLegalMoves(toMove(this->x, this->y));
	board[this->y][this->x] = NULL;//so the king is no longer in the "ray" of the attacking piece
	for (Pieces* piece : opponent->getPieces()) {//Error here ://////
		//PAWN HAS DIFFERENT ATTACK PATTERN
		if (piece->name[1] == 'P') {
			piece->resetMoves();
			if (piece->isWhite()) {
				piece->possibleMoves.insert(toMove(piece->x + 1, piece->y + 1));//can generate illegal moves as we do not care for these moves
				piece->possibleMoves.insert(toMove(piece->x - 1, piece->y + 1));
			}

			else {//black Pawn
				piece->possibleMoves.insert(toMove(piece->x + 1, piece->y - 1));//can generate illegal moves as we do not care for these moves
				piece->possibleMoves.insert(toMove(piece->x - 1, piece->y - 1));
			}
		}
		else
			piece->pseudoLegalMoves();//setting the pieces pseudoLegalMoves unordered set
		for (string move : piece->possibleMoves)
			if (possibleMoves.find(move) != possibleMoves.end())
				possibleMoves.erase(possibleMoves.find(move));
	}
	board[this->y][this->x] = this;
}

//Module 3-Board
void displayBoard() {
	cout << "   ";
	for (int i = 0; i < 8; i++)
		cout << "____ ";
	cout << endl;
	for (int i = 0; i < 8; i++) {
		cout << i + 1 << " ";
		for (int j = 0; j < 8; j++) {
			cout << "| ";
			if (board[i][j] != NULL)
				cout << board[i][j]->name;
			else
				cout << "..";
			cout << " ";
		}
		cout << "|\n  ";
		for (int j = 0; j < 8; j++) {
			cout << "|____";
		}
		cout << "|" << endl;
	}
	cout << "  ";
	for (char a = 'a'; a < 'i'; a++) {
		cout << "  " << a << "  ";
	}
	cout << endl;
}

//Used to decode the position of the spaces from string to integers
int* decodePosition(string pos) {
	int* position = new int[2];
	position[0] = pos[0] - 'a';
	position[1] = pos[1] - '1';

	return position;
}

string toMove(int x, int y) {
	return string(1, (char)('a' + x)) + string(1, (char)('1' + y));
}

//Input validation function
bool isvalid(bool isWhite, string space)
{
	if (board[space[1] - '1'][space[0] - 'a'] == NULL)
	{
		cout << "That is not a selectable Piece." << endl;
		return false;
	}

	else if ((board[space[1] - '1'][space[0] - 'a']->isWhite() && !isWhite) || (board[space[1] - '1'][space[0] - 'a']->name[0] == 'b' && isWhite))
	{
		cout << "You cannot control those pieces." << endl;
		return false;
	}

	else if (space[1] > '8' || space[0] > 'h')
	{
		cout << "Out of bounds" << endl;
		return false;
	}

	return true;
}

//This function checks if the user's input is validated
bool validPosition(string Space)
{
	try
	{
		board.at(Space[1] - '1').at(Space[0] - 'a');
		return true;
	}
	catch (...)
	{
		return false;
	}

}

//This boolean will decide whether to skip the next player's turn or not.
bool PlayerTurn(Player& A, Player& B, bool isWhite, int& WinorLose)
{	//Player A is the current player
	//need info on the opponent king so we can prevent their move generation
	kingToMove = A.getKing()->name;
	string p1, p2;
	bool CheckRollback = false;
	displayBoard();
	map<Pieces*, set<string>> movesInCheck;
	Pieces* pieceSelected = NULL;
	A.setPieces();

	cout << A.getPieces().size() << endl;
	if (A.getPieces().size() == 1 && B.getPieces().size() == 1) {
		cout << "Stalemate\n";
		WinorLose = 3;
		return 1;
	}

	if (A.getKing()->isChecked()) {
		movesInCheck = A.LegalMovesInCheck();
		if (movesInCheck.empty())
		{
			cout << "CheckMate...\n";
			if (isWhite)
				WinorLose = 2;
			else
				WinorLose = 1;
			return 1;
		}

		//add player losing logic here
		else {
			A.setCheck(true);
			cout << "Check!!!\n";
			for (auto i = movesInCheck.begin(); i != movesInCheck.end(); i++) {
				cout << "Piece: " << i->first->name << endl << "Moves: ";
				for (auto j = i->second.begin(); j != i->second.end(); j++)
					cout << *j << " ";
				cout << endl;
			}
		}
	}

	if (A.isAI()) {
		cout << "AI making move...\n";
		system("pause");
		AITurn(A, movesInCheck, WinorLose);
		return 0;
	}

	cout << "What would you like to do?\n" << "1.Make a move" << endl << "2.Undo a Move" << endl << "3.Give up" << endl << endl << "Your choice: ";
	do
	{
		cin >> p1;
	} while (p1 != "1" && p1 != "2" && p1 != "3");

	if (p1 == "3")
	{
		if (isWhite)
			WinorLose = 2;
		else
			WinorLose = 1;
		return 1;
	}


	if (p1 == "2")
	{
		system("CLS");
		if (A.getMovedPieces().size() > 2 && B.getMovedPieces().size() > 2)
		{
			//Undo because of Player
			A.undo();
			B.undo();


			//Undo for the Opponent
			A.undo();
			B.undo();

			A.setCheck(false);
		}

		else
			cout << "No more Undo's Possible" << endl << endl;

		return 1;
	}

	//Loop for the player
	while (true)
	{
		//User's turn
		do
		{
			cout << "Enter current position of piece: ";
			cin >> p1;
			
			if (validPosition(p1) && A.Checked() && !movesInCheck.empty()) {
				int* boardPos = decodePosition(p1);
				if (movesInCheck.find(board[boardPos[1]][boardPos[0]]) == movesInCheck.end())//only move valid piece in check
				{
					cout << "Invalid Move, you are currently in check" << endl;
					p1 = "999";
				}
			}
		} while (!validPosition(p1));

		//This checks if the piece chosen is actually the player's
		if (!isvalid(isWhite, p1))
			continue;

		int* selectedPiecePos = decodePosition(p1);
		pieceSelected = board[selectedPiecePos[1]][selectedPiecePos[0]];

		
		cout << "Enter move: ";
		cin >> p2;
		if (validPosition(p2) && A.Checked() && !movesInCheck.empty()) {
			int* boardPos = decodePosition(p2);
			if (movesInCheck[pieceSelected].find(p2) == movesInCheck[pieceSelected].end())//only play valid move in check
			{
				cout << "Invalid Move, you are currently in check" << endl;
				continue;
			}
		}


		if (board[p1[1] - '1'][p1[0] - 'a']->validMoves(p2))//, board[p1[1] - '1'][p1[0] - 'a']->name))
		{
			//Pushing the enemy piece onto the Undo Stack
			if (board[p2[1] - '1'][p2[0] - 'a'] != NULL)
				B.Captured(board[p2[1] - '1'][p2[0] - 'a']);

			//Adding in NULL to make sure Undo's work properly
			else
				B.pushPiece(NULL);
			Enpassant = false;


			//Pushing previous Piece and moving it
			A.pushPiece(board[p1[1] - '1'][p1[0] - 'a']);

			//Moving the piece on the board
			board[p1[1] - '1'][p1[0] - 'a']->move(p2);

			//Promotion in case it happens
			A.Promotion(board[p2[1] - '1'][p2[0] - 'a']);
			A.setCheck(false);
		}
		else
		{
			cout << "Invalid Move" << endl;
			continue;
		}
		system("pause");
		system("CLS");
		break;
	}

	return 0;
}

void AITurn(Player& AI, map<Pieces*, set<string>>& inCheck, int& WinorLose)
{
	list<Pieces*> allPieces = AI.getPieces();
	Pieces* pieceToMove = NULL;
	Moves MoveToPlay;
	multimap<Moves, Pieces*, greater<Moves>> movepool;

	if (AI.Checked()) {
		pieceToMove = AI.getKing();
		movepool.insert(pair<Moves, Pieces*>(pieceToMove->BM(), pieceToMove));
	}

	else {
		for (Pieces* piece : allPieces) {
			Moves pieceMove = piece->BM();
			if (MoveToPlay <= pieceMove && pieceMove.value != -1)
			{
				movepool.insert(pair<Moves, Pieces*>(pieceMove, piece));
				MoveToPlay = pieceMove;
			}
		}
	}

	if (movepool.empty()) {
		if (AI.Checked()) {
			if (AI.getKing()->isWhite())
				WinorLose = 2;
			else
				WinorLose = 1;
		}
		else
			WinorLose = 3;
		return;
	}
	AI.setCheck(false);
	srand(unsigned(time(NULL)));

	int pos = 1;
	multimap<Moves, Pieces*>::iterator prev = movepool.begin();
	for (multimap<Moves, Pieces*>::iterator i = ++movepool.begin(); i != movepool.end(); i++, pos++) {
		if (prev->first.value - i->first.value < 10)
			prev++;
		else
			break;
	}

	pos = rand() % pos;


	multimap<Moves, Pieces*>::iterator it = movepool.begin();
	advance(it, pos);
	MoveToPlay = (*it).first;
	pieceToMove = (*it).second;
	pieceToMove->move(MoveToPlay.move);
	int* decodedPos = decodePosition(MoveToPlay.move);
	AI.Promotion(board[decodedPos[1]][decodedPos[0]]);
}

void vsPlayerGame(bool AisAI, bool BisAI)
{
	system("CLS");
	bool isWhite = 1;// rand() % 2;
	bool CheckRollback = false;
	bool skipPlayer = false;
	int WinorLose = 0;
	//setting new players
	Player A(isWhite);
	Player B(!isWhite);

	if (AisAI)
		A.setAI(true);
	if (BisAI)
		B.setAI(true);

	//>>>>Setting player's kings with the data they require<<<<
	A.getKing()->setOpponent(B);
	B.getKing()->setOpponent(A);//king needs opponent pieces potential moves
	//>>><<<<
	if (isWhite)
		cout << "Player 1 will Control White and Player 2 will Control Black." << endl;
	else
		cout << "Player 1 will Control Black and Player 2 will Control White." << endl;


	//Main gameplay loop is here
	do {

		if (!skipPlayer)
		{
			cout << "Player 1 Turn: " << endl;
			skipPlayer = PlayerTurn(A, B, isWhite, WinorLose);
		}
		else
			skipPlayer = false;

		if (!skipPlayer)
		{
			cout << "Player 2 Turn: " << endl;
			skipPlayer = PlayerTurn(B, A, !isWhite, WinorLose);
		}
		else
			skipPlayer = false;

	} while (WinorLose == 0);

	if (WinorLose == 1)
		cout << endl << "The White Player has won the game!";
	else if (WinorLose == 2)
		cout << endl << "The Black Player has won the game!";
	else
		cout << endl << "Stalemate";

	_getch();
}

int main() {
	srand(unsigned(time(NULL)));

	while (true)
	{
		//Clearing the board for the next game
		for (int i = 0; i < 8; i++)
			for (int j = 0; j < 8; j++)
				board[i][j] = NULL;

		//Clearing the screen of clutter
		system("CLS");

		int choice = 0;
		cout << "\t\t         CHESS" << endl;
		cout << "\t\tWhat kind of game would you like to play?" << endl;
		cout << "\t\t1. vs Bot" << endl << "\t\t2. vs Player" << endl << "\t\t3. Bot Vs Bot" << endl << "\t\t4. Quit" << endl;
		cout << "\t\tYour choice: ";
		while (choice < 1 || choice > 4)
			cin >> choice;

		switch (choice)
		{
		case 1:
			vsPlayerGame(false, true);
			break;
		case 2:
			vsPlayerGame(false, false);
			break;
		case 3:
			vsPlayerGame(true, true);
			break;
		case 4:
			break;
		}

		if (choice == 4)
			break;
	}
	return 0;
}