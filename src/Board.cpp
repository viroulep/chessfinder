#include <sstream>
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

    Square::~Square()
    {
        Utils::output("Deleting square\n", 5);
        if (piece_)
            delete piece_;
    }

    //Should only be called by Piece::moveTo
    void Square::changePiece(Piece *newPiece)
    {
        if (piece_)
            piece_->moveTo(NULL);
        piece_ = newPiece;
        //Do not piece.moveTo : a piece move to a square, not the opposite
    }

    Piece *Square::getPiece()
    {
        return piece_;
    }

    const string Piece::to_string()
    {
        switch (kind_) {
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

    const char Piece::to_char()
    {
        switch (kind_) {
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

    const char Piece::to_pgn()
    {
        if (kind_ == PAWN)
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

    Side Piece::getColor()
    {
        return color_;
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
            ;//TODO: by the GAME
            //deletedPieces.push_back(this);//drop
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

using namespace Board;
Chessboard::~Chessboard()
{
    Utils::output("Deleting chessboard\n", 5);
    //Free all the datum
    for (int f = A; f <= H; f++)
        for (Rank r = 1; r <= 8; r++)
            delete board_[(File)f][r];
    Utils::output("Deleting taken pieces\n", 5);
    while (!takenPieces.empty()) {
        Piece *p = takenPieces.top();
        delete p;
        takenPieces.pop();
    }
}

const string Chessboard::to_string()
{
    string RED="\e[31;1m";
    string RESET="\e[m";
    ostringstream oss;
    oss << "-----------------\n";
    for (Rank r = 8; r >= 1; r--)
        for (int f = A; f <= H; f++) {
            Square *sq = board_[(File)f][r];
            if (!sq)
                Utils::handleError("Chessboard not correctly initialized");
            Piece *p = sq->getPiece();
            if (p) {
                oss << '|';
                if (p->getColor() == BLACK)
                    oss << RED;
                oss << p->to_char();
                if (p->getColor() == BLACK)
                    oss << RESET;
            } else
                oss << "| ";
            if (f == H)
                oss << "|\n-----------------\n";
            
        }
    return oss.str();
}

Chessboard *Chessboard::createChessboard()
{
    Chessboard *cb = new Chessboard();
    cb->setupDefaultBoard();
    return cb;
}

Chessboard::Chessboard() : board_(), takenPieces()
{
    //need to initialize all the datas
    for (int f = A; f <= H; f++)
        for (Rank r = 1; r <= 8; r++)
            board_[(File)f][r] = new Square((File)f, r);
}

void Chessboard::setupDefaultBoard()
{
    for (Rank r = 8; r >= 1; r--)
        for (int f = A; f <= H; f++) {
            Square *sq = board_[(File)f][r];
            if (r == 7)
                new Piece(Board::Piece::Kind::PAWN, BLACK, sq);
            if (r == 2)
                new Piece(Board::Piece::Kind::PAWN, WHITE, sq);
            if ((f == A || f == H) && r == 1)
                new Piece(Board::Piece::Kind::ROOK, WHITE, sq);
            if ((f == A || f == H) && r == 8)
                new Piece(Board::Piece::Kind::ROOK, BLACK, sq);
            if ((f == B || f == G) && r == 1)
                new Piece(Board::Piece::Kind::KNIGHT, WHITE, sq);
            if ((f == B || f == G) && r == 8)
                new Piece(Board::Piece::Kind::KNIGHT, BLACK, sq);
            if ((f == C || f == F) && r == 1)
                new Piece(Board::Piece::Kind::BISHOP, WHITE, sq);
            if ((f == C || f == F) && r == 8)
                new Piece(Board::Piece::Kind::BISHOP, BLACK, sq);
            if (f == D && r == 1)
                new Piece(Board::Piece::Kind::QUEEN, WHITE, sq);
            if (f == D && r == 8)
                new Piece(Board::Piece::Kind::QUEEN, BLACK, sq);
            if (f == E && r == 1)
                new Piece(Board::Piece::Kind::KING, WHITE, sq);
            if (f == E && r == 8)
                new Piece(Board::Piece::Kind::KING, BLACK, sq);
            
        }
}



