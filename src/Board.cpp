#include <sstream>
#include "Board.h"
#include "Utils.h"

namespace Board {

    const string to_string(Board::Side theSide)
    {
        switch (theSide) {
            case WHITE:
                return "White";
            case BLACK:
                return "Black";
            default:
                return "undefined";
        }
    }

    const char to_char(File theFile)
    {
        return 'a' + theFile;
    }

    const char to_char(Rank theRank)
    {
        return '0' + theRank;
    }

    Square::Square(File file, Rank rank) :
        file_(file), rank_(rank)
    {
        if (rank_ < 1 || rank_ > 8)
            Utils::handleError("Invalid rank when creating square");
    }

    Square::Square(File file, Rank rank, Piece *piece) :
        file_(file), rank_(rank)
    {
        //Can be null
        piece_ = piece;
        Square(file, rank);
    }

    Square::~Square()
    {
        Utils::output("Deleting square\n", 5);
        if (piece_)
            delete piece_;
    }

    //Should only be called by Piece::moveTo
    void Square::changePiece(Piece *newPiece)
    {
        if (newPiece == piece_)
            return;
        Utils::output("Change to :\n", 4);
        if (newPiece)
            Utils::output(newPiece->to_string(), 4);
        else
            Utils::output("null", 4);
        Utils::output("\n", 4);

        if (newPiece && piece_)
            Utils::handleError("There is a already a piece on our square : "
                    + to_string());
        //handle by the chessboard
            //piece_->moveTo(NULL);
            
        //we moved a piece from the square
        piece_ = newPiece;

        //Do not newPiece.moveTo : a piece move to a square, not the opposite
    }

    Piece *Square::getPiece()
    {
        return piece_;
    }
    const File Square::getFile()
    {
        return file_;
    }

    const Rank Square::getRank()
    {
        return rank_;
    }

    const string Square::to_string()
    {
        string str;
        str += to_char(file_);
        str += to_char(rank_);
        return str;
    }

    const char Piece::to_char(Kind k)
    {
        switch (k) {
            case KNIGHT:
                return 'N';
            case BISHOP:
                return 'B';
            case ROOK:
                return 'R';
            case QUEEN:
                return 'Q';
            case KING:
                return 'K';
            case PAWN:
                return 'P';
            default:
                return ' ';
        }

    }

    const char Piece::to_uci(Kind k)
    {
        char c = to_char(k);
        c = c - 'A' + 'a';
        return c;
    }

    const string Piece::to_string()
    {
        switch (getKind()) {
            case PAWN:
                return "Pawn";
            case KNIGHT:
                return "Knight";
            case BISHOP:
                return "Bishop";
            case ROOK:
                return "Rook";
            case QUEEN:
                return "Queen";
            case KING:
                return "King";
            default:
                return "undefined";
        }
    }

    void Piece::prettyPrint(ostringstream &oss)
    {
        if (getColor() == Side::BLACK)
            oss << Utils::RED;
        oss << this->to_char();
        if (getColor() == Side::BLACK)
            oss << Utils::RESET;
    }

    const char Piece::to_char()
    {
        to_char(getKind());
    }

    const char Piece::to_pgn()
    {
        if (getKind() == PAWN)
            return '\0';
        else
            return to_char();
    }

    Piece::Piece(Kind kind, Side color) :
        kind_(kind), color_(color)
    {
    }

    Piece::Piece(Kind kind, Side color, Square *square) :
        kind_(kind), color_(color)
    {
        if (square)
            moveTo(square);
    }

    const Side Piece::getColor()
    {
        return color_;
    }

    const Piece::Kind Piece::getKind()
    {
        return (promoted_ == KING)?kind_:promoted_;
    }

    //Move the piece to square, even if illegal move
    void Piece::moveTo(Square *newSquare)
    {
        if (square_ == newSquare)
            return;
        if (square_)
            square_->changePiece(NULL);
        square_ = newSquare;
        if (square_)
            square_->changePiece(this);
        else
            Utils::output(string("Dropped : ") + to_char() + "\n", 4);
            //deletedPieces.push_back(this);//drop
    }

    void Piece::promoteTo(Kind kind)
    {
        if (promoted_ == KING)
            promoted_ = kind;
        else
            Utils::handleError("Piece is already promoted");
    }

    void Piece::unpromote()
    {
        promoted_ = KING;
    }

    Side getSideFromString(string sidestr)
    {
        if (sidestr == "w" || sidestr == "white")
            return WHITE;
        else if (sidestr == "b" || sidestr == "black")
            return BLACK;
        else
            Utils::handleError("Error parsing playfor side");
    }


}



