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
#include <queue>
#include <cstdlib>
#include <cmath>
#include "Chessboard.h"
#include "Line.h"
#include "Utils.h"
#include "MatfinderOptions.h"
#include "Output.h"

using namespace Board;
Chessboard::~Chessboard()
{
    Out::output("Deleting chessboard\n", 5);
    //Free all the data
    for (int f = A; f <= H; f++)
        for (Rank r = 1; r <= 8; r++)
            delete board_[(File)f][r];
    Out::output("Deleting taken pieces\n", 5);
    while (!takenPieces_.empty()) {
        Piece *p = takenPieces_.back();
        delete p;
        takenPieces_.pop_back();
    }
}

const string Chessboard::to_string()
{
    ostringstream oss;
    oss << "  -----------------\n";
    for (Rank r = 8; r >= 1; r--)
        for (int f = A; f <= H; f++) {
            Square *sq = board_[(File)f][r];
            if (!sq)
                Err::handle("Chessboard not correctly initialized");
            Out::output("Square ok\n", 5);
            if (f == A)
                oss << r << " ";
            Piece *p = sq->getPiece();
            if (p) {
                oss << '|';
                p->prettyPrint(oss);
            } else
                oss << "| ";
            if (f == H) {
                if (r == 8) {
                    oss << "| ";
                    if (takenPieces_.empty())
                        oss << "No piece taken.";
                    else
                        oss << "Pieces taken : ";
                    for (list<Piece *>::iterator it = takenPieces_.begin(),
                            itEnd = takenPieces_.end();
                            it != itEnd; ++it) {
                        if (it != takenPieces_.begin())
                            oss << ", ";
                        (*it)->prettyPrint(oss);
                    }
                    oss << "\n";
                } else if (r == 7) {
                    oss << "| Fullmove clock : " << fullmoveClock_ << "\n";
                } else if (r == 6) {
                    oss << "| Halfmove clock : " << halfmoveClock_ << "\n";
                } else if (r == 5) {
                    oss << "| ";
                    if (active_ == Side::BLACK)
                        oss << Utils::RED;
                    oss << Board::to_string(active_);
                    if (active_ == Side::BLACK)
                        oss << Utils::RESET;
                    oss << " to move.\n";
                } else if (r == 4) {
                    oss << "| Enpassant : ";
                    if (enpassant_)
                        oss << enpassant_->to_string();
                    else
                        oss << "<none>";
                    oss << "\n";
                } else if (r == 3) {
                    oss << "| Castling : ";
                    if (castle_ & WKCASTLE)
                        oss << "K";
                    if (castle_ & WQCASTLE)
                        oss << "Q";
                    if (castle_ & BKCASTLE)
                        oss << "k";
                    if (castle_ & BQCASTLE)
                        oss << "q";
                    oss << "\n";
                } else {
                    oss << "|\n";
                }
                oss << "  -----------------\n";
            }
        }
    oss << "   a b c d e f g h\n\n";
    oss << prettyHistoryToString() << "\n";
    oss << historyToString() << "\n";
    return oss.str();
}

void Chessboard::reInitFromFEN(string fenString)
{
    //TODO: replace handleError by throw runtime_error(error)
    queue<string> infos;
    stringstream ss(fenString);
    string tmpInfo;
    while (getline(ss, tmpInfo, ' '))
        infos.push(tmpInfo);

    //FEN has 6 data fields
    if (infos.size() != 6)
        Err::handle("Invalid input fen : must have 6 field");

    clear();
    while (!infos.empty()) {
        string info = infos.front();
        switch (infos.size()) {
            case 6:
                posFromFEN(info);
                break;
            case 5:
                sideFromFEN(info);
                break;
            case 4:
                castleFromFEN(info);
                break;
            case 3:
                enpassantFromFEN(info);
                break;
            case 2:
                halfmoveCkFromFEN(info);
                break;
            case 1:
                fullmoveCkFromFEN(info);
                break;
            default:
                break;

        }
        infos.pop();
    }
}

int Chessboard::uciApplyMove(string uciMove)
{
    return applyMove(getMoveFromUci(uciMove));
}

Move Chessboard::getMoveFromUci(string uciMove)
{
    if (uciMove.size() < 4 || uciMove.size() > 5)
        Err::handle("Error parsing uci move");
    Square *from = squareFromString(uciMove.substr(0, 2));
    Square *to = squareFromString(uciMove.substr(2, 2));
    Move mv;
    mv.from = from;
    mv.to = to;
    mv.halfMoveCk = halfmoveClock_;
    mv.enpassantSquare = enpassant_;
    if (to->getPiece())
        mv.takePiece = true;
    if (uciMove.size() == 5)
        mv.promoteTo = Board::promotionFromChar(uciMove[4]);
    return mv;
}

int Chessboard::uciApplyMoves(list<string> uciMoves)
{
    int ret = 0;
    for (list<string>::iterator it = uciMoves.begin(), itEnd = uciMoves.end();
            it != itEnd; ++it)
        ret &= uciApplyMove(*it);
    return ret;
}

void Chessboard::undoMove()
{
    if (moveHistory_.empty()) {
        Out::output("Move history is empty\n");
        return;
    }
    Move lastMove = moveHistory_.back();
    moveHistory_.pop_back();
    Out::output("Backtrack : " + prettyMoveHistory_.back() + "\n", 3);
    prettyMoveHistory_.pop_back();
    Square *from = lastMove.to;
    Square *to = lastMove.from;

    if (!from || !to)
        Err::handle("Unexpected error : cannot undo move.");

    Piece *toMove = from->getPiece();
    if (!toMove)
        Err::handle("Unexpected error : cannot undo move.");

    //Re-init enpassant
    enpassant_ = lastMove.enpassantSquare;

    //Restore hmClock
    halfmoveClock_ = lastMove.halfMoveCk;
    //Restore fullmoveCk
    if (active_ == Side::WHITE)
        fullmoveClock_--;


    //Actually handle move back
    if (lastMove.promoteTo != Piece::Kind::KING)
        toMove->unpromote();
    toMove->moveTo(to);

    if (lastMove.takePiece) {
        Piece *taken = takenPieces_.back();
        if (enpassant_ == from && taken->getKind() == Piece::Kind::PAWN) {
            taken->moveTo(board_[from->getFile()][to->getRank()]);
        } else {
            if (enpassant_ == from)
                Out::output("[WARNING] Undoing weird case : restoring"\
                        "non-pawn piece on 'enpassant' square. ("
                        + taken->to_string() + " taken by "
                        + toMove->to_string() + "\n");
            taken->moveTo(from);
        }
        takenPieces_.pop_back();
    }

    //Opposite side to move
    active_ = (Side)!active_;

    if (toMove->getKind() != Piece::Kind::KING)
        return ;

    //handle castling back
    if (to == board_[E][1]) {
        if (from == board_[G][1])
            board_[F][1]->getPiece()->moveTo(board_[H][1]);
        else if (from == board_[C][1])
            board_[D][1]->getPiece()->moveTo(board_[A][1]);
    } else if (to == board_[E][8]) {
        if (from == board_[G][8])
            board_[F][8]->getPiece()->moveTo(board_[H][8]);
        else if (from == board_[C][8])
            board_[D][8]->getPiece()->moveTo(board_[A][8]);
    }
}

const string Chessboard::tryUciMoves(const list<string> &moves, int limit)
{
    ostringstream oss;
    Move mv;
    int moveClock = fullmoveClock_;
    int halfMove = 0;
    int movePlayed = 0;
    int stop = (limit == -1)?moves.size():limit;
    oss << moveClock << ".";
    if ((int)active_) {
        oss << "..";
        halfMove++;
    }
    oss << " ";
    for (list<string>::const_iterator it = moves.begin(), itEnd = moves.end();
            it != itEnd && movePlayed < stop; ++it) {
        if (fullmoveClock_ != moveClock && !(halfMove%2))
            oss << fullmoveClock_ << ". ";
        uciApplyMove(*it);
        oss << prettyMoveHistory_.back() << " ";
        halfMove++;
        movePlayed++;
    }
    for (int i = 0; i < movePlayed; i++)
        undoMove();
    if (movePlayed < (int) moves.size())
        oss << "[...moves...]";
    /*
     *for (list<string>::const_iterator it = moves.begin(), itEnd = moves.end();
     *        it != itEnd; ++it)
     *    undoMove();
     */
    return oss.str();
}

/*Return true if lhs < rhs*/
bool Chessboard::compareLines(Line *lhs, Line *rhs)
{
    /*Move lhsM = getMoveFromUci(lhs->firstMove());*/
    /*Move rhsM = getMoveFromUci(rhs->firstMove());*/
    /*
     * First check the eval if they are too different.
     * (For example if the cp_treshold is 300 cp, then lines at -2.2 and +1.2
     * are equivalent, but the last one is better !)
     */
    float lhsEv = lhs->getEval();
    float rhsEv = rhs->getEval();
    Out::output("Comparing " + std::to_string(lhsEv) + " to "
            + std::to_string(rhsEv) + "\n", 3);
    /*Set the limit to .5 eval*/
    if (abs(lhsEv - rhsEv) > 20) {
        return lhsEv > rhsEv;
    }

    /*Optionally add some restriction on the line*/
    /*TODO restore*/
    /*return comparator_->compare(lhsM, rhsM);*/
    return true;
}


Side Chessboard::getActiveSide()
{
    return active_;
}

const list<string> Chessboard::getUciMoves()
{
    ostringstream oss;
    string move;
    list<string> moves;
    for (list<Move>::iterator it = moveHistory_.begin(),
            itEnd = moveHistory_.end(); it != itEnd; ++it) {
        move = "";
        move += it->from->to_string() + it->to->to_string();
        if (it->promoteTo != Piece::Kind::KING)
            move += Piece::to_uci(it->promoteTo);
        moves.push_back(move);
    }
    return moves;
}

const string Chessboard::exportToFEN()
{
    SimplePos sp;
    int pad = 0;
    for (Rank r = 8; r >= 1; r--)
        for (int f = A; f <= H; f++) {
            Square *sq = board_[(File)f][r];
            Piece *p = sq->getPiece();
            if (p) {
                if (pad > 0) {
                    sp += std::to_string(pad);
                    pad = 0;
                }
                Board::Side c = p->getColor();
                switch (p->getKind()) {
                    case Piece::Kind::QUEEN:
                        if (c == Board::Side::WHITE)
                            sp += "Q";
                        else
                            sp += "q";
                        break;
                    case Piece::Kind::ROOK:
                        if (c == Board::Side::WHITE)
                            sp += "R";
                        else
                            sp += "r";
                        break;
                    case Piece::Kind::BISHOP:
                        if (c == Board::Side::WHITE)
                            sp += "B";
                        else
                            sp += "b";
                        break;
                    case Piece::Kind::KNIGHT:
                        if (c == Board::Side::WHITE)
                            sp += "N";
                        else
                            sp += "n";
                        break;
                    case Piece::Kind::PAWN:
                        if (c == Board::Side::WHITE)
                            sp += "P";
                        else
                            sp += "p";
                        break;
                    case Piece::Kind::KING:
                        if (c == Board::Side::WHITE)
                            sp += "K";
                        else
                            sp += "k";
                        break;
                }
            } else {
                pad++;
            }
            if (f == H) {
                if (pad > 0)
                    sp += std::to_string(pad);
                if (r != 1) {
                    pad = 0;
                    sp += "/";
                }
            }
        }
    sp += " ";
    if (active_ == Board::Side::WHITE)
        sp += "w";
    else
        sp += "b";
    sp += " ";
    string castleString = "";
    if (castle_ & WKCASTLE)
        castleString += "K";
    if (castle_ & WQCASTLE)
        castleString += "Q";
    if (castle_ & BKCASTLE)
        castleString += "k";
    if (castle_ & BQCASTLE)
        castleString += "q";
    if (castleString == "")
        castleString = "-";
    sp += castleString;
    sp += " ";
    if (enpassant_)
        sp += enpassant_->to_string();
    else
        sp += "-";
    /*if (removeClock) {*/
        /*
         *if (halfmoveClock_ > 47)
         *    sp += " " + std::to_string(halfmoveClock_);
         *else
         *    sp += " " + std::to_string(halfmoveClock_ + 47);
         */
        /*sp += " -";*/
        /*sp += " 50 1";*/
    /*} else {*/
    sp += " " + std::to_string(halfmoveClock_);
    sp += " " + std::to_string(fullmoveClock_);
    /*}*/
    return sp;
}

bool Chessboard::sufficientMaterial()
{
    list<Piece *> allPieces;
    for (Rank r = 8; r >= 1; r--)
        for (int f = A; f <= H; f++) {
            Square *sq = board_[(File)f][r];
            Piece *p = sq->getPiece();
            if (p)
                allPieces.push_back(p);
        }
    if (allPieces.size() > 3)
        return true;
    else if (allPieces.size() == 2)
        return false;
    else {
        Piece *notKing;
        for (auto it = allPieces.begin(), itEnd = allPieces.end();
                it != itEnd; ++it)
            if ((*it)->getKind() != Piece::KING)
                notKing = (*it);
        if (!notKing) {
            Out::output("Warning : 3 kings on the board ?!\n");
            return false;
        } else {
            return !(notKing->getKind() == Piece::BISHOP
                    || notKing->getKind() == Piece::KNIGHT);
        }
    }
}

Chessboard *Chessboard::createChessboard()
{
    return createFromFEN(CHESS_STARTPOS);
}

Chessboard *Chessboard::createFromFEN(string fenString)
{
    //TODO: replace handleError by throw runtime_error(error)
    queue<string> infos;
    stringstream ss(fenString);
    string tmpInfo;
    while (getline(ss, tmpInfo, ' '))
        infos.push(tmpInfo);

    //FEN has 6 data fields
    if (infos.size() != 6)
        Err::handle("Invalid input fen : must have 6 field");

    Chessboard *cb = new Chessboard();

    while (!infos.empty()) {
        string info = infos.front();
        switch (infos.size()) {
            case 6:
                cb->posFromFEN(info);
                break;
            case 5:
                cb->sideFromFEN(info);
                break;
            case 4:
                cb->castleFromFEN(info);
                break;
            case 3:
                cb->enpassantFromFEN(info);
                break;
            case 2:
                cb->halfmoveCkFromFEN(info);
                break;
            case 1:
                cb->fullmoveCkFromFEN(info);
                break;
            default:
                break;

        }
        infos.pop();
    }
    return cb;
}

/*Chessboard::Chessboard() : comparator_(MatfinderOptions::getMoveComparator()),*/
Chessboard::Chessboard() : takenPieces_(), board_()
{
    //need to initialize all the datas
    for (int f = A; f <= H; f++)
        for (Rank r = 1; r <= 8; r++)
            board_[(File)f][r] = new Square((File)f, r);
}

Square *Chessboard::squareFromString(string str)
{
    if (str.size() != 2)
        Err::handle("Error parsing square string : " + str);
    char file = str[0];
    char rank = str[1];
    if (file < 'a' || file > 'h')
        Err::handle("Error parsing square string : " + str);
    File f = (File)(file - 'a');
    if (rank < '1' || rank > '8')
        Err::handle("Error parsing square string : " + str);
    Rank r = (rank - '0');
    Out::output(string("Returning square ") + to_char(f) + to_char(r) + "\n", 4);
    return board_[f][r];
}

int Chessboard::applyMove(Move theMove)
{
    //should take "Move"
    if (!isValidMove(theMove))
        return 1;

    Square *from = theMove.from;
    Square *to = theMove.to;
    Piece *toMove = from->getPiece();
    Piece *toTake;

    //Handle piece taken, if any
    if (enpassant_ == to && toMove->getKind() == Piece::Kind::PAWN)
        toTake = board_[to->getFile()][from->getRank()]->getPiece();
    else
        toTake = to->getPiece();

    enpassant_ = NULL;

    if (toTake) {
        toTake->moveTo(NULL);
        halfmoveClock_ = 0;
        theMove.takePiece = true;
        takenPieces_.push_back(toTake);
    } else
        halfmoveClock_++;

    if (toMove->getKind() == Piece::Kind::PAWN) {
        halfmoveClock_ = 0;
        //Update enpassant square
        if (to->getRank() - (int)from->getRank() == 2)
            enpassant_ = board_[from->getFile()][from->getRank() + 1];
        else if ((int) to->getRank() - (int)from->getRank() == -2)
            enpassant_ = board_[from->getFile()][from->getRank() - 1];
    }

    //Update the full clock
    if (active_ == Side::BLACK)
        fullmoveClock_++;

    //Actually move the piece
    moveHistory_.push_back(theMove);
    prettyMoveHistory_.push_back(getPrettyMove(theMove));
    if (theMove.promoteTo != Piece::Kind::KING)
        toMove->promoteTo(theMove.promoteTo);
    toMove->moveTo(to);

    //Opposite side to move
    active_ = (Side)!active_;

    if (toMove->getKind() != Piece::Kind::KING)
        return 0;

    //Handle castling : assume castling has been verified by isValidMove...
    if (from == board_[E][1]) {
        if (to == board_[G][1])
            board_[H][1]->getPiece()->moveTo(board_[F][1]);
        else if (to == board_[C][1])
            board_[A][1]->getPiece()->moveTo(board_[D][1]);
    } else if (from == (Square *)board_[E][8]) {
        if (to == board_[G][8])
            board_[H][8]->getPiece()->moveTo(board_[F][8]);
        else if (to == board_[C][8])
            board_[A][8]->getPiece()->moveTo(board_[D][8]);
    }


    return 0;
}

const string Chessboard::getPrettyMove(Move mv)
{
    string prettyMove("");
    Square *from = mv.from;
    Square *to = mv.to;
    if (!from || !to)
        return "";
    //Handle castling...
    //King-side castle
    if ((from == board_[E][1] && to == board_[G][1]) ||
            (from == board_[E][8] && to == board_[G][8]))
        return "O-O";
    else if ((from == board_[E][1] && to == board_[C][1]) ||
            (from == board_[E][8] && to == board_[C][8]))
        return "O-O-O";
    Piece *p = from->getPiece();
    if (!p)
        return "";
    if (p->to_pgn())
        prettyMove += p->to_pgn();
    if (p->getKind() == Piece::Kind::PAWN && mv.takePiece)
        prettyMove += to_char(from->getFile());
    if (mv.takePiece)
        prettyMove += "x";
    prettyMove += to->to_string();
    if (mv.promoteTo != Piece::Kind::KING)
        prettyMove += string("=") + Piece::to_char(mv.promoteTo);
    Out::output("Returning pretty move : " + prettyMove + "\n", 4);
    return prettyMove;
}

const string Chessboard::prettyHistoryToString()
{
    int printed = 0;
    int moveIndexFirst = fullmoveClock_ -
        (prettyMoveHistory_.size() + !(int)active_)/2;
    int moveIndex = moveIndexFirst;
    //This is not related to halfmove clock
    int halfMove = 0;
    ostringstream oss;
    oss << moveIndex << ".";
    int notPar = prettyMoveHistory_.size()%2;
    if ((int)active_^notPar) {
        oss << "..";
        halfMove++;
    }
    oss << " ";

    for (list<string>::iterator it = prettyMoveHistory_.begin(),
            itEnd = prettyMoveHistory_.end();
            it != itEnd; ++it) {
        if (moveIndex != moveIndexFirst && !(halfMove%2))
            oss << moveIndex << ". ";
        oss << (*it) << " ";
        halfMove++;
        if (!(halfMove%2))
            moveIndex++;
        printed++;
        if (printed%10 == 0)
            oss << "\n";
    }
    return oss.str();
}

const string Chessboard::historyToString()
{
    ostringstream oss;
    int printed = 0;
    for (list<Move>::iterator it = moveHistory_.begin(),
            itEnd = moveHistory_.end();
            it != itEnd; ++it) {
        oss << it->from->to_string() << it->to->to_string() << " ";
        printed++;
        if (printed%10 == 0)
            oss << "\n";
    }
    return oss.str();
}

bool Chessboard::isValidMove(Move theMove)
{
    Square *from = theMove.from;
    Square *to = theMove.to;
    if (!from || !to || from == to) {
        Out::output("/!\\Invalid move : invalid squares\n");
        return false;
    }
    Piece *toMove = from->getPiece();
    if (!toMove) {
        Out::output("/!\\Invalid move : no piece to move\n");
        Out::output("/!\\\tmove : " + from->to_string()
                + to->to_string() + "\n");
        return false;
    }
    //TODO (maybe) : check for move validity
    return true;
}

void Chessboard::clear()
{
    //Free all the data
    Out::output("Deleting pieces\n", 5);
    for (int f = A; f <= H; f++)
        for (Rank r = 1; r <= 8; r++) {
            Piece *p = board_[(File)f][r]->getPiece();
            board_[(File)f][r]->changePiece(NULL);
            if (p)
                delete p;
        }
    Out::output("Deleting taken pieces\n", 5);
    while (!takenPieces_.empty()) {
        Piece *p = takenPieces_.back();
        delete p;
        takenPieces_.pop_back();
    }
    enpassant_ = NULL;
    halfmoveClock_ = 0;
    fullmoveClock_ = 1;
    active_ = Side::WHITE;
    castle_ = 0x0;
}

void Chessboard::posFromFEN(string pos)
{
    queue<string> ranks;
    stringstream ss(pos);
    string rank;
    while (getline(ss, rank, '/'))
        ranks.push(rank);
    //Board has 8 data fields
    if (ranks.size() != 8)
        Err::handle("Invalid input pos : must have 8 ranks");

    Rank r = 8;
    File f = A;
    while (!ranks.empty()) {
        Out::output("Rank : " + std::to_string(r) + "\n", 4);
        const char *crank = ranks.front().c_str();
        char c;
        while ((c = *crank++)) {
            Square *sq = board_[f][r];
            switch (c) {
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                    //should rotate through files
                    f = (File)(f + c - '0');
                    break;
                case 'p':
                    new Piece(Board::Piece::Kind::PAWN, BLACK, sq);
                    f = (File)(f + 1);
                    break;
                case 'P':
                    new Piece(Board::Piece::Kind::PAWN, WHITE, sq);
                    f = (File)(f + 1);
                    break;
                case 'r':
                    new Piece(Board::Piece::Kind::ROOK, BLACK, sq);
                    f = (File)(f + 1);
                    break;
                case 'R':
                    new Piece(Board::Piece::Kind::ROOK, WHITE, sq);
                    f = (File)(f + 1);
                    break;
                case 'n':
                    new Piece(Board::Piece::Kind::KNIGHT, BLACK, sq);
                    f = (File)(f + 1);
                    break;
                case 'N':
                    new Piece(Board::Piece::Kind::KNIGHT, WHITE, sq);
                    f = (File)(f + 1);
                    break;
                case 'b':
                    new Piece(Board::Piece::Kind::BISHOP, BLACK, sq);
                    f = (File)(f + 1);
                    break;
                case 'B':
                    new Piece(Board::Piece::Kind::BISHOP, WHITE, sq);
                    f = (File)(f + 1);
                    break;
                case 'q':
                    new Piece(Board::Piece::Kind::QUEEN, BLACK, sq);
                    f = (File)(f + 1);
                    break;
                case 'Q':
                    new Piece(Board::Piece::Kind::QUEEN, WHITE, sq);
                    f = (File)(f + 1);
                    break;
                case 'k':
                    new Piece(Board::Piece::Kind::KING, BLACK, sq);
                    f = (File)(f + 1);
                    break;
                case 'K':
                    new Piece(Board::Piece::Kind::KING, WHITE, sq);
                    f = (File)(f + 1);
                    break;
                default:
                    f = (File)(f + 1);
                    Err::handle("Unrecognize char in position");
                    break;
            }
            f = (File)(f%8);
        }
        r--;
        ranks.pop();
    }

}

void Chessboard::sideFromFEN(string side)
{
    if (side == "w")
        active_ = Side::WHITE;
    else if (side == "b")
        active_ = Side::BLACK;
    else
        Err::handle("Invalid input fen : can't determine active side");
}

void Chessboard::castleFromFEN(string castle)
{
    if (castle == "-")
        return;
    else {
        const char *ccastle = castle.c_str();
        char c;
        while ((c = *ccastle++)) {
            switch (c) {
                case 'K':
                    castle_ |= WKCASTLE;
                    break;
                case 'Q':
                    castle_ |= WQCASTLE;
                    break;
                case 'k':
                    castle_ |= BKCASTLE;
                    break;
                case 'q':
                    castle_ |= BQCASTLE;
                    break;
                default:
                    Err::handle("Invalid input fen : can't set castle");
            }
        }
    }
}

void Chessboard::enpassantFromFEN(string ep)
{
    if (ep == "-")
        return;
    else
        enpassant_ = squareFromString(ep);
}

void Chessboard::halfmoveCkFromFEN(string clock)
{
    if (clock == "-")
        return;
    else {
        try {
            halfmoveClock_ = stoi(clock);
        } catch (...) {
            Err::handle(
                    "Invalid input fen : can't parse halfmove clock");
        }
    }
}

void Chessboard::fullmoveCkFromFEN(string clock)
{
    if (clock == "-")
        return;
    else {
        try {
            fullmoveClock_ = stoi(clock);
        } catch (...) {
            Err::handle(
                    "Invalid input fen : can't parse fullmove clock");
        }
    }
}

const string Chessboard::CHESS_STARTPOS =
                "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
const string Chessboard::ALAMOS_STARTPOS =
                "8/1rnqknr1/1pppppp1/8/8/1PPPPPP1/1RNQKNR1/8 w - - 0 1";
const string Chessboard::GARDNER_STARTPOS =
                "8/8/1rnbqk2/1ppppp2/8/1PPPPP2/1RNBQK2/8 w - - 0 1";

