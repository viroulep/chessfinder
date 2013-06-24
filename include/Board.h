#ifndef __BOARD_H__
#define __BOARD_H__
#include <string>

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

    const string to_string(Board::Side theSide);
    const char to_char(File theFile);
    const char to_char(Rank theRank);

    class Piece;
    class Square { 
    public:
        Square(File file, Rank rank);
        Square(File file, Rank rank, Piece *piece);
        ~Square();
        void changePiece(Piece *newPiece);
        Piece *getPiece();

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
        const string to_string();
        const char to_char();
        //Useful because we don't want to print pawns 'P' in pgn
        const char to_pgn();

        Piece(Kind kind, Side color);
        Piece(Kind kind, Side color, Square *square);
        Side getColor();
        //Move the piece to square, even if illegal move
        void moveTo(Square *newSquare);
    private:
        const Kind kind_;
        const Side color_;
        Square *square_ = NULL;
    };

    typedef struct move_s {
        Square *from = NULL;
        Square *to = NULL;
        bool takePiece = false;
    } Move;
    Side getSideFromString(string sidestr);
}



#endif
