#ifndef __CHESSBOARD_H__
#define __CHESSBOARD_H__
#include <map>
#include <string>
#include <list>
#include "Board.h"

using namespace Board;
class Chessboard {
public:

    ~Chessboard();
    const string to_string();
    void uciApplyMove(string uciMove);
    void backtrackMove();


    static Chessboard *createChessboard();
    static Chessboard *createFromFEN(string fenString);
    static string exportToFEN(Chessboard board);

    static const string CHESS_STARTPOS;
    static const string ALAMOS_STARTPOS;
    static const string GARDNER_STARTPOS;
private:
    Chessboard();

    Square *squareFromString(string str);
    void applyMove(Square *from, Square *to);

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
};


#endif
