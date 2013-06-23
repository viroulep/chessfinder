#ifndef __BOARD_H__
#define __BOARD_H__
#include <string>
#include <map>
#include <list>

using namespace std;

namespace Board {
    enum Side {
        WHITE,
        BLACK
    };
    enum File {
        A, B, C, D,
        E, F, G, H
    };
    typedef unsigned int Rank;

    static const string to_string(Board::Side theSide);
    static const char to_char(File theFile);
    static const char to_char(Rank theRank);

    class Piece;
    class Square { 
    public:
        Square(File file, Rank rank);
        Square(File file, Rank rank, Piece *piece);
        void changePiece(Piece *newPiece);

    private:
        const File file_;
        const Rank rank_;
        Piece *piece_ = NULL;
    };


    class Piece { 
    public:
        enum Kind {
            PAWN,
            KNIGHT,
            BISHOP,
            ROOK,
            QUEEN,
            KING
        };
        static const string to_string(Kind thePiece);
        static const char to_char(Kind thePiece);

        Piece(Kind kind, Side color);
        Piece(Kind kind, Side color, Square *square);
        //Move the piece to square, even if illegal move
        void moveTo(Square *newSquare);
        const string getPretty();
    private:
        const Kind kind_;
        const Side color_;
        Square *square_ = NULL;
    };

    static list<Piece *> deletedPieces;

}

typedef map<Board::File, map<Board::Rank, Board::Square>> Chessboard;

#endif
