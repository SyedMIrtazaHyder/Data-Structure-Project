#pragma once
#ifndef PLAYER_H
#define PLAYER_H

#include <conio.h>

class Player {
	bool isWhite, isChecked, AI;
	std::list<Pieces*> pieces;
	//These are the pieces that have been moved/captured
	std::list<Pieces*> movedPieces;
	King* king;
public:
	int counter;

	void setPieces();

	void setAI(bool val);

	bool isAI();

	void setCheck(bool check);

	bool Checked();

	King* getKing();

	Player(bool isWhite);

	std::list<Pieces*>& getPieces();

	std::list<Pieces*>& getMovedPieces();

	void pushPiece(Pieces* pushed);

	void undo();

	void Captured(Pieces* piece);

	std::map<Pieces*, std::set<std::string>> LegalMovesInCheck();

	bool Promotion(Pieces* piece);
};

bool PlayerTurn(Player& A, Player& B, bool isWhite, int& WinorLose);
void AITurn(Player& AI, Player& A, std::map<Pieces*, std::set<std::string>>& movesInCheck, int& WinorLose);
void vsPlayerGame(bool AisAI, bool BisAI);
bool ThreefoldRepition(Pieces*,std::string);

#endif // !PLAYER_H
