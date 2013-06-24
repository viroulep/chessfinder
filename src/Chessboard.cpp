#include <sstream>
#include <queue>
#include "Chessboard.h"
#include "Utils.h"

using namespace Board;
Chessboard::~Chessboard()
{
    Utils::output("Deleting chessboard\n", 5);
    //Free all the datum
    for (int f = A; f <= H; f++)
        for (Rank r = 1; r <= 8; r++)
            delete board_[(File)f][r];
    Utils::output("Deleting taken pieces\n", 5);
    while (!takenPieces_.empty()) {
        Piece *p = takenPieces_.back();
        delete p;
        takenPieces_.pop_back();
    }
}

const string Chessboard::to_string()
{
    string RED="\e[31;1m";
    string RESET="\e[0m";
    ostringstream oss;
    oss << "-----------------\n";
    for (Rank r = 8; r >= 1; r--)
        for (int f = A; f <= H; f++) {
            Square *sq = board_[(File)f][r];
            if (!sq)
                Utils::handleError("Chessboard not correctly initialized");
            Utils::output("Square ok\n", 5);
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

void Chessboard::uciApplyMove(string uciMove)
{
    if (uciMove.size() < 4)
        Utils::handleError("Error parsing uci move");
    Square *from = squareFromString(uciMove.substr(0, 2));
    Square *to = squareFromString(uciMove.substr(2, 2));
    //TODO: promote = substr(4,1);
    applyMove(from, to);
}

void Chessboard::backtrackMove()
{
    //TODO
}

Chessboard *Chessboard::createChessboard()
{
    return createFromFEN(CHESS_STARTPOS);
}

Chessboard *Chessboard::createFromFEN(string fenString)
{

    queue<string> infos;
    stringstream ss(fenString);
    string tmpInfo;
    while (getline(ss, tmpInfo, ' '))
        infos.push(tmpInfo);

    //FEN has 6 data fields
    if (infos.size() != 6)
        Utils::handleError("Invalid input fen : must have 6 field");

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

Chessboard::Chessboard() : board_(), takenPieces_()
{
    //need to initialize all the datas
    for (int f = A; f <= H; f++)
        for (Rank r = 1; r <= 8; r++)
            board_[(File)f][r] = new Square((File)f, r);
}

Square *Chessboard::squareFromString(string str)
{
    //TODO
}

void Chessboard::applyMove(Square *from, Square *to)
{
    //TODO
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
        Utils::handleError("Invalid input pos : must have 8 ranks");

    Rank r = 8;
    File f = A;
    while (!ranks.empty()) {
        Utils::output("Rank : " + std::to_string(r) + "\n", 4);
        const char *crank = ranks.front().c_str();
        char c;
        while (c = *crank++) {
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
                    Utils::handleError("Unrecognize char in position");
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
        Utils::handleError("Invalid input fen : can't determine active side");
}

void Chessboard::castleFromFEN(string castle)
{
    if (castle == "-")
        return;
    else
        ;//TODO
}

void Chessboard::enpassantFromFEN(string ep)
{
    if (ep == "-")
        return;
    else
        ;//TODO
}

void Chessboard::halfmoveCkFromFEN(string clock)
{
    if (clock == "-")
        return;
    else {
        try {
            halfmoveClock_ = stoi(clock);
        } catch (...) {
            Utils::handleError(
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
            Utils::handleError(
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

