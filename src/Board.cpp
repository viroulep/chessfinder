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
#include <sstream>
#include "Board.h"
#include "Utils.h"
#include "Output.h"

using namespace std;

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

    char to_char(File theFile)
    {
        return 'a' + theFile;
    }

    char to_char(Rank theRank)
    {
        return '0' + theRank;
    }

    Square::Square(File file, Rank rank) :
        file_(file), rank_(rank)
    {
        if (rank_ < 1 || rank_ > 8)
            Err::handle("Invalid rank when creating square");
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
        Out::output("Deleting square " + to_string() + "\n", 5);
        if (piece_)
            delete piece_;
    }

    //Should only be called by Piece::moveTo
    void Square::changePiece(Piece *newPiece)
    {
        if (newPiece == piece_)
            return;
        Out::output("Change to :\n", 4);
        if (newPiece)
            Out::output(newPiece->to_string(), 4);
        else
            Out::output("null", 4);
        Out::output("\n", 4);

        if (newPiece && piece_)
            Err::handle("There is a already a piece on our square : "
                    + to_string() + " when moving " + newPiece->to_string()
                    + " on " + piece_->to_string());
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
    File Square::getFile()
    {
        return file_;
    }

    Rank Square::getRank()
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

    char Piece::to_char(Kind k)
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

    char Piece::to_uci(Kind k)
    {
        char c = to_char(k);
        c = c - 'A' + 'a';
        return c;
    }

    const string Piece::to_string()
    {
        string ret;
        switch (getKind()) {
            case PAWN:
                ret = "Pawn";
                break;
            case KNIGHT:
                ret = "Knight";
                break;
            case BISHOP:
                ret = "Bishop";
                break;
            case ROOK:
                ret = "Rook";
                break;
            case QUEEN:
                ret = "Queen";
                break;
            case KING:
                ret = "King";
                break;
            default:
                ret = "undefined";
                break;
        }
        ret += "(" + Board::to_string(color_) + ")";
        if (square_)
            ret += "[" + square_->to_string() + "]";
        return ret;
    }

    void Piece::prettyPrint(ostringstream &oss)
    {
        if (getColor() == Side::BLACK)
            oss << Utils::RED;
        oss << this->to_char();
        if (getColor() == Side::BLACK)
            oss << Utils::RESET;
    }

    char Piece::to_char()
    {
        return to_char(getKind());
    }

    char Piece::to_pgn()
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

    Piece::~Piece()
    {
        Out::output("Deleting piece\n", 5);
    }

    Side Piece::getColor()
    {
        return color_;
    }

    Piece::Kind Piece::getKind()
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
            Out::output(string("Dropped : ") + to_string() + "\n", 4);
            //deletedPieces.push_back(this);//drop
    }

    void Piece::promoteTo(Kind kind)
    {
        if (promoted_ == KING)
            promoted_ = kind;
        else
            Err::handle("Piece is already promoted");
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
            Err::handle("Error parsing playfor side");
        return WHITE;
    }

    bool checkMove(string mv)
    {
        if ( mv.size() < 4 || mv.size() > 5
                || mv[0] > 'h' || mv[0] < 'a'
                || mv[1] > '8' || mv[1] < '1'
                || mv[2] > 'h' || mv[2] < 'a'
                || mv[3] > '8' || mv[3] < '1')
            return false;
        else if (mv.size() == 5 && (
                    mv[4] != 'q' || mv[4] != 'n'
                    || mv[4] != 'b' || mv[4] != 'r'
                    ))
            return false;
        else
            return true;
    }

    Piece::Kind promotionFromChar(char p)
    {
        switch (p) {
            case 'q':
                return Piece::Kind::QUEEN;
                break;
            case 'n':
                return Piece::Kind::KNIGHT;
                break;
            case 'b':
                return Piece::Kind::BISHOP;
                break;
            case 'r':
                return Piece::Kind::ROOK;
                break;
            default:
                Err::handle("Invalid promotion");
        }
        return Piece::Kind::QUEEN;
    }

    void squareFromString(string str, File *f, Rank *r)
    {
        if (str.size() != 2)
            Err::handle("Error parsing square string : " + str);
        char file = str[0];
        char rank = str[1];
        if (file < 'a' || file > 'h')
            Err::handle("Error parsing square string : " + str);
        (*f) = (File)(file - 'a');
        if (rank < '1' || rank > '8')
            Err::handle("Error parsing square string : " + str);
        (*r) = (rank - '0');
        Out::output(string("Returning square ") + to_char(*f)
                + to_char(*r) + "\n", 4);
    }

    /*
     *bits                meaning
     *===================================
     *0,1,2               to file
     *3,4,5               to row
     *6,7,8               from file
     *9,10,11             from row
     *12,13,14            promotion piece
     *"promotion piece" is encoded as follows
     *none       0
     *knight     1
     *bishop     2
     *rook       3
     *queen      4
     */
    uint16_t uciToPolyglotb(UCIMove &mv)
    {
        File f;
        Rank r;
        uint16_t encodedMove = 0x0;
        if (mv.size() != 4 && mv.size() != 5)
            Err::handle("Error parsing uci move");
        if (mv.size() == 5) {
            Piece::Kind promote = promotionFromChar(mv[4]);
            switch (promote) {
                case Piece::KNIGHT:
                    encodedMove |= 0x1;
                    break;
                case Piece::BISHOP:
                    encodedMove |= 0x2;
                    break;
                case Piece::ROOK:
                    encodedMove |= 0x3;
                    break;
                case Piece::QUEEN:
                    encodedMove |= 0x4;
                    break;
                default:
                    break;
            }
            encodedMove <<= 3;
        }
        string from = mv.substr(0, 2);
        Board::squareFromString(from, &f, &r);
        //Rank in 1..8, go to 0..7
        encodedMove |= (r - 1);
        encodedMove <<= 3;
        //File already on 0..7
        encodedMove |= (uint16_t)f;
        encodedMove <<= 3;

        string to = mv.substr(2, 2);
        Board::squareFromString(to, &f, &r);
        //Rank in 1..8, go to 0..7
        encodedMove |= (r - 1);
        encodedMove <<= 3;
        //File already on 0..7
        encodedMove |= (uint16_t)f;
        //encodedMove <<= 3;
        Out::output("Move " + mv + " is " + std::to_string(encodedMove)
                + ", decimal\n", 3);
        return encodedMove;
    }

    UCIMove polyglotToUcib(uint16_t mv)
    {
        File f;
        Rank r;
        UCIMove uci = "";
        f = (File)(mv & 0x7);
        mv >>= 3;
        r = mv & 0x7;
        mv >>= 3;
        //To
        //uci += std::to_string(to_char(f)) + std::to_string(to_char(r+1));
        uci += to_char(f);
        uci += to_char(r + 1);

        f = (File)(mv & 0x7);
        mv >>= 3;
        r = mv & 0x7;
        mv >>= 3;
        //From
        string from;
        from += to_char(f);
        from += to_char(r + 1);
        uci = from + uci;

        switch (mv) {
            case 1:
                uci += "n";
                break;
            case 2:
                uci += "b";
                break;
            case 3:
                uci += "r";
                break;
            case 4:
                uci += "q";
                break;
            default:
                break;
        }
        Out::output(std::to_string(mv) + ", decimal is move "
                + uci + "\n", 3);
        return uci;
    }
}



