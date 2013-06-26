#ifndef __BOARD_H__
#define __BOARD_H__
#include <string>
#include <sstream>

using namespace std;


namespace Board {
    static const int WKCASTLE = 0x0001;
    static const int WQCASTLE = 0x0010;
    static const int BKCASTLE = 0x0100;
    static const int BQCASTLE = 0x1000;

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
        const File getFile();
        const Rank getRank();
        const string to_string();

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
        static const char to_char(Kind k);
        const string to_string();
        void prettyPrint(ostringstream &oss);
        const char to_char();
        //Useful because we don't want to print pawns 'P' in pgn
        const char to_pgn();

        Piece(Kind kind, Side color);
        Piece(Kind kind, Side color, Square *square);
        const Side getColor();
        const Kind getKind();
        //Move the piece to square, even if illegal move
        void moveTo(Square *newSquare);
        void promoteTo(Kind kind);
        void unpromote();
    private:
        const Kind kind_;
        const Side color_;
        Kind promoted_ = KING;
        Square *square_ = NULL;
    };

    typedef struct move_s {
        //Board state
        Square *from = NULL;
        Square *to = NULL;
        Square *enpassantSquare = NULL;
        int halfMoveCk = 0;
        //some help
        bool takePiece = false;
        //if promoteTo != KING : promotion
        Piece::Kind promoteTo = Piece::Kind::KING;
    } Move;
    Side getSideFromString(string sidestr);
}



#endif
