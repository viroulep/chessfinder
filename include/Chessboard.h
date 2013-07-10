/*
 * Matfinder, a program to help chess engines to find mat
 *
 * Copyright© 2013 Philippe Virouleau
 *
 * You can contact me at firstname.lastname@imag.fr
 * (Replace "firstname" and "lastname" with my actual names)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef __CHESSBOARD_H__
#define __CHESSBOARD_H__
#include <map>
#include <string>
#include <list>
#include "Board.h"

using namespace Board;
class Hashing;

class Chessboard {
public:
    friend class Hashing;

    ~Chessboard();
    const string to_string();
    //Return the pretty move
    int uciApplyMove(string uciMove);
    int uciApplyMoves(list<string> uciMoves);
    void undoMove();
    //Play the moves, backtrack them, then return
    //corresponding prettyMovesHistory
    const string tryUciMoves(const list<string> &moves, int limit = -1);

    const Side getActiveSide();

    const list<string> getUciMoves();

    const string exportToFEN(Chessboard &board);
    SimplePos getSimplePos();

    static Chessboard *createChessboard();
    static Chessboard *createFromFEN(string fenString);

    static const string CHESS_STARTPOS;
    static const string ALAMOS_STARTPOS;
    static const string GARDNER_STARTPOS;
private:
    Chessboard();

    Square *squareFromString(string str);
    int applyMove(Move theMove);
    bool isValidMove(Move theMove);
    const string getPrettyMove(Move mv);
    const string prettyHistoryToString();

    void posFromFEN(string pos);
    void sideFromFEN(string side);
    void castleFromFEN(string castle);
    void enpassantFromFEN(string ep);
    void halfmoveCkFromFEN(string clock);
    void fullmoveCkFromFEN(string clock);

    list<Piece *> takenPieces_;
    list<Move> moveHistory_;
    list<string> prettyMoveHistory_;

    map<Board::File, map<Board::Rank, Board::Square*>> board_;
    Side active_ = Side::WHITE;
    //half moves since last pawn advance or capturing move
    int halfmoveClock_ = 0;
    //Number of moves. Incremented after each black move
    int fullmoveClock_ = 1;
    //enpassant square
    Square *enpassant_ = NULL;

    int castle_ = 0x0;
};


#endif
