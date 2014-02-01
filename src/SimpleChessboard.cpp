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
#include <iostream>
#include <string>
#include <cstring>
#include <queue>
#include "SimpleChessboard.h"
#include "Movegen.h"

using namespace std;

namespace Board {

    const std::string PieceToChar(" PNBRQK  pnbrqk");

    InvalidFenException::InvalidFenException(string msg)
    {
        fenmsg = "Invalid fen string (" + msg + ")";
    }

    const char* InvalidFenException::what() throw()
    {
        return fenmsg.c_str();
    }

    Position::Position()
    {
        clear();
        /*init();*/
    }

    Position::~Position()
    {
        clear();
    }

    Color Position::side_to_move() const
    {
        return active_;
    }

    bool Position::empty(Square s) const
    {
        return board_[s] == NO_PIECE;
    }

    bool Position::takes(Square attacker, Square target) const
    {
        return (board_[target] != NO_PIECE &&
                color_of(board_[attacker]) != color_of(board_[target]));
    }

    Piece Position::piece_on(Square s) const
    {
        return board_[s];
    }

    Square Position::enpassant() const
    {
        return st_->enpassant;
    }

    void Position::init()
    {
        for (Square s = SQ_A1; s <= SQ_H8; ++s) {
            if (rank_of(s) == RANK_2)
                board_[s] = W_PAWN;
            else if (rank_of(s) == RANK_7)
                board_[s] = B_PAWN;
            else if (rank_of(s) == RANK_1 || rank_of(s) == RANK_8) {
                Color c = (rank_of(s) == RANK_1)?WHITE:BLACK;
                if (file_of(s) == FILE_A || file_of(s) == FILE_H)
                    board_[s] = make_piece(c, ROOK);
                else if (file_of(s) == FILE_B || file_of(s) == FILE_G)
                    board_[s] = make_piece(c, KNIGHT);
                else if (file_of(s) == FILE_C || file_of(s) == FILE_F)
                    board_[s] = make_piece(c, BISHOP);
                else if (file_of(s) == FILE_D)
                    board_[s] = make_piece(c, QUEEN);
                else
                    board_[s] = make_piece(c, KING);
            } else
                board_[s] = NO_PIECE;
        }
        st_ = &startState_;
    }

    void Position::clear()
    {
        StateInfo *prev = st_;
        StateInfo *tmp;
        /*Do not delete the first state, it's been statically allocated*/
        if (prev)
            prev = st_->prev;
        while (prev) {
            tmp = prev;
            prev = st_->prev;
            delete tmp;
        }
        /*Can't set 0 in whole position because of virtual functions*/
        std::memset(board_, 0, sizeof(board_));
        std::memset(&startState_, 0, sizeof(StateInfo));
        startState_.enpassant = SQ_NONE;
        st_ = &startState_;
        active_ = WHITE;
    }

    const string Position::pretty() const
    {
        StateInfo st = *st_;
        const string RED = "\e[31;1m";
        const string RESET = "\e[0m";
        ostringstream oss;
        oss << "  -----------------\n";
        for (Rank r = RANK_8; r >= RANK_1; --r)
            for (File f = FILE_A; f <= FILE_H; ++f) {
                Square s = make_square(r, f);
                Piece p = board_[s];
                if (f == FILE_A)
                    oss << rank_to_char(r) << " ";
                /*TODO*/
                oss << "|";
                oss << piece_to_string(p, true);
                if (f == FILE_H) {
                    oss << "| ";
                    if (r == RANK_8) {
                        oss << "todo : Piece taken.";
                        /*
                         *if (takenPieces_.empty())
                         *    oss << "No piece taken.";
                         *else
                         *    oss << "Pieces taken : ";
                         *for (list<Piece *>::iterator it = takenPieces_.begin(),
                         *        itEnd = takenPieces_.end();
                         *        it != itEnd; ++it) {
                         *    if (it != takenPieces_.begin())
                         *        oss << ", ";
                         *    (*it)->prettyPrint(oss);
                         *}
                         */
                    } else if (r == RANK_7) {
                        oss << "Fullmove clock : " << st.fullmoveClock;
                    } else if (r == RANK_6) {
                        oss << "Halfmove clock : " << st.halfmoveClock;
                    } else if (r == RANK_5) {
                        oss << color_to_string(active_);
                        oss << " to move.";
                    } else if (r == RANK_4) {
                        oss << "Enpassant : ";
                        if (is_ok(st.enpassant))
                            oss << square_to_string(st.enpassant);
                        else
                            oss << "<none>";
                    } else if (r == RANK_3) {
                        oss << "Castling : ";
                        if (st.castle & W_OO)
                            oss << "K";
                        if (st.castle & W_OOO)
                            oss << "Q";
                        if (st.castle & B_OO)
                            oss << "k";
                        if (st.castle & B_OOO)
                            oss << "q";
                    }
                    oss << "\n";
                    oss << "  -----------------\n";
                }
            }
        oss << "   a b c d e f g h\n\n";
        /*oss << prettyHistoryToString() << "\n";*/
        /*oss << historyToString() << "\n";*/
        return oss.str();
    }

    void Position::set(string fenString) throw(InvalidFenException)
    {
        clear();
        queue<string> infos;
        stringstream ss(fenString);
        string tmpInfo;
        while (getline(ss, tmpInfo, ' '))
            infos.push(tmpInfo);

        //FEN has 6 data fields
        if (infos.size() != 6)
            throw InvalidFenException("fen must have 6 fields");

        while (!infos.empty()) {
            string info = infos.front();
            switch (infos.size()) {
                case 6:
                    setPos(info);
                    break;
                case 5:
                    setSide(info);
                    break;
                case 4:
                    setCastle(info);
                    break;
                case 3:
                    setEP(info);
                    break;
                case 2:
                    setClock(info, startState_.halfmoveClock);
                    break;
                case 1:
                    setClock(info, startState_.fullmoveClock);
                    break;
                default:
                    break;

            }
            infos.pop();
        }
    }

    const string Position::fen() const {

        int emptyCnt;
        std::ostringstream ss;

        for (Rank rank = RANK_8; rank >= RANK_1; --rank)
        {
            for (File file = FILE_A; file <= FILE_H; ++file)
            {
                for (emptyCnt = 0; file <= FILE_H && empty(make_square(rank, file)); ++file)
                    ++emptyCnt;

                if (emptyCnt)
                    ss << emptyCnt;

                if (file <= FILE_H)
                    ss << PieceToChar[board_[make_square(rank, file)]];
            }

            if (rank > RANK_1)
                ss << '/';
        }

        ss << (active_ == WHITE ? " w " : " b ");

        /*Castling*/
        if (can_castle(W_OO))
            ss << 'K';

        if (can_castle(W_OOO))
            ss << 'Q';

        if (can_castle(B_OO))
            ss << 'k';

        if (can_castle(B_OOO))
            ss << 'q';

        if (!can_castle(WHITE) && !can_castle(BLACK))
            ss << '-';

        /*En passant*/
        ss << (!is_ok(st_->enpassant) ? " - " : " " + square_to_string(st_->enpassant) + " ");
        /*Clocks*/
        ss << st_->halfmoveClock << " " << st_->fullmoveClock;

        return ss.str();
    }


    /*---------------------/
     *                     /
     *Protected methods    /
     *                     /
     *--------------------*/
    void Position::setPos(string fenPos) throw(InvalidFenException)
    {
        queue<string> ranks;
        stringstream ss(fenPos);
        string rank;
        while (getline(ss, rank, '/'))
            ranks.push(rank);
        //Board has 8 data fields
        if (ranks.size() != 8)
            throw InvalidFenException("position must have 8 ranks");

        Rank r = RANK_8;
        File f = FILE_A;
        while (!ranks.empty()) {
            const char *crank = ranks.front().c_str();
            char c;
            while ((c = *crank++)) {
                Square s = make_square(r, f);
                if (!is_ok(s))
                    throw InvalidFenException("Current square is invalid");
                if (c >= '1' && c <= '8') {
                    //should rotate through files
                    f += File(c - '0');
                } else {
                    Piece p = piece_from_char(c);
                    if (p == NO_PIECE)
                        throw InvalidFenException("Unrecognize char in position");
                    board_[s] = p;
                    ++f;
                }
            }
            --r;
            f = FILE_A;
            ranks.pop();
        }
    }

    void Position::setSide(string fenSide) throw(InvalidFenException)
    {
        if (fenSide == "w")
            active_ = WHITE;
        else if (fenSide == "b")
            active_ = BLACK;
        else
            throw InvalidFenException("Can't determine active side");
    }

    void Position::setCastle(string fenCastle) throw(InvalidFenException)
    {
        if (fenCastle == "-")
            return;
        else {
            const char *ccastle = fenCastle.c_str();
            char c;
            int castle = 0;
            while ((c = *ccastle++)) {
                switch (c) {
                    case 'K':
                        castle |= W_OO;
                        break;
                    case 'Q':
                        castle |= W_OOO;
                        break;
                    case 'k':
                        castle |= B_OO;
                        break;
                    case 'q':
                        castle |= B_OOO;
                        break;
                    default:
                        throw InvalidFenException("Can't set castle");
                }
            }
            startState_.castle = castle;
        }
    }

    void Position::setEP(string fenEP) throw(InvalidFenException)
    {
        Square s = square_from_string(fenEP);
        if (fenEP == "-") {
            return;
        } else {
            if (s == SQ_NONE)
                throw InvalidFenException("Invalid enpassant square");
            startState_.enpassant = s;
        }
    }

    void Position::setClock(string fenClock, int &clock) throw(InvalidFenException)
    {
        if (fenClock == "-") {
            return;
        } else {
            try {
                clock = stoi(fenClock);
            } catch (...) {
                throw InvalidFenException("can't parse clock");
            }
        }
    }

}
