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
#include "Line.h"

class HashTable;

class Chessboard {
public:
    friend class HashTable;

    ~Chessboard();
    const std::string to_string();
    //Reinit the board with another pos
    void reInitFromFEN(std::string fenString);
    //Return the pretty move
    int uciApplyMove(std::string uciMove);
    Board::Move getMoveFromUci(std::string uciMove);
    int uciApplyMoves(std::list<std::string> uciMoves);
    void undoMove();
    //Play the moves, backtrack them, then return
    //corresponding prettyMovesHistory
    const std::string tryUciMoves(const std::list<std::string> &moves, int limit = -1);

    /*Move comparators*/
    bool compareLines(Line *lhs, Line *rhs);

    Board::Side getActiveSide();

    const std::list<std::string> getUciMoves();

    const std::string exportToFEN();

    //Return true if board has sufficient material to not draw
    bool sufficientMaterial();

    static Chessboard *createChessboard();
    static Chessboard *createFromFEN(std::string fenString);

    static const std::string CHESS_STARTPOS;
    static const std::string ALAMOS_STARTPOS;
    static const std::string GARDNER_STARTPOS;
private:
    Chessboard();

    Board::Square *squareFromString(std::string str);
    int applyMove(Board::Move theMove);
    bool isValidMove(Board::Move theMove);
    const std::string getPrettyMove(Board::Move mv);
    const std::string prettyHistoryToString();
    const std::string historyToString();

    void clear();
    void posFromFEN(std::string pos);
    void sideFromFEN(std::string side);
    void castleFromFEN(std::string castle);
    void enpassantFromFEN(std::string ep);
    void halfmoveCkFromFEN(std::string clock);
    void fullmoveCkFromFEN(std::string clock);

    /*TODO restore*/
    /*MoveComparator *comparator_;*/

    std::list<Board::Piece *> takenPieces_;
    std::list<Board::Move> moveHistory_;
    std::list<std::string> prettyMoveHistory_;

    std::map<Board::File, std::map<Board::Rank, Board::Square*>> board_;
    Board::Side active_ = Board::Side::WHITE;
    //half moves since last pawn advance or capturing move
    int halfmoveClock_ = 0;
    //Number of moves. Incremented after each black move
    int fullmoveClock_ = 1;
    //enpassant square
    Board::Square *enpassant_ = NULL;

    int castle_ = 0x0;
};


#endif
