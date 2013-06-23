
#include "Board.h"
#include "Utils.h"

namespace Board {

    const string to_string(Board::Side theSide)
    {
        switch (theSide) {
            case WHITE:
                return "white";
            case BLACK:
                return "black";
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

    //Should only be called by Piece::moveTo
    void Square::changePiece(Piece *newPiece)
    {
        if (piece_)
            piece_->moveTo(NULL);
        piece_ = newPiece;
        //Do not piece.moveTo : a piece move to a square, not the opposite
    }


    const string Piece::to_string(Kind thePiece)
    {
        switch (thePiece) {
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

    const char Piece::to_char(Kind thePiece)
    {
        switch (thePiece) {
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
            default:
                return '\0';
        }
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
            deletedPieces.push_back(this);//drop
    }

    const string Piece::getPretty()
    {
        //TODO
        return "TODO";
    }

}
