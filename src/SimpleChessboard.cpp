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
#include <queue>
#include <set>
#include <algorithm>
#include <cstring>
#include <cmath>
#include "SimpleChessboard.h"
#include "CompareMove.h"
#include "Hashing.h"
#include "Movegen.h"
#include "Output.h"

#define COMPUTE_CASTLING_ROOK(move, sqFrom, sqTo) \
    File rookFile = (move.to > move.from)?FILE_H:FILE_A;\
Rank rookRank = (active_ == WHITE)?RANK_1:RANK_8;\
sqFrom = make_square(rookRank, rookFile);\
sqTo = (move.to > move.from)?\
Square(move.to - 1):Square(move.to + 1);

using namespace std;

namespace Board {

    const std::string PieceToChar(" PNBRQK  pnbrqk");

    InvalidFenException::InvalidFenException(string msg)
    {
        fenmsg = "Invalid fen string (" + msg + ")";
    }

    InvalidMoveException::InvalidMoveException(string msg)
    {
        moveMsg = "Invalid move (" + msg + ")";
    }

    const char* InvalidFenException::what() const throw()
    {
        return fenmsg.c_str();
    }

    const char* InvalidMoveException::what() const throw()
    {
        return moveMsg.c_str();
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

    bool Position::attacked(Square s, Color c) const
    {
        std::set<Square> attackers = gen_attackers(c, s, *this);
        return !attackers.empty();
    }

    bool Position::takes(Square attacker, Square target) const
    {
        return (board_[target] != NO_PIECE &&
                color_of(board_[attacker]) != color_of(board_[target]));
    }

    std::set<Square> Position::pieces_squares(Color c) const
    {
        std::set<Square> squares;
        for (Square s = SQ_A1; s <= SQ_H8; ++s)
            if (board_[s] != NO_PIECE && color_of(board_[s]) == c)
                squares.insert(s);
        return squares;
    }

    Piece Position::piece_on(Square s) const
    {
        return board_[s];
    }

    Square Position::enpassant() const
    {
        return st_->enpassant;
    }

    Square Position::king(Color c) const
    {
        Piece king = make_piece(c, KING);
        for (Square s = SQ_A1; s <= SQ_H8; ++s)
            if (board_[s] == king)
                return s;
        assert(false && "No king on board !");
    }

    bool Position::canCastle(CastlingFlag f) const
    {
        return (st_->castle & f);
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
        st_->castle = (B_OO | B_OOO | W_OO | W_OOO);
    }

    void Position::clear()
    {
        for (Move m : moves_)
            delete m.state;
        moves_.clear();
        pgnMoves_.clear();
        /*Can't set 0 in whole position because of virtual functions*/
        std::memset(board_, 0, sizeof(board_));
        std::memset(&startState_, 0, sizeof(StateInfo));
        startState_.enpassant = SQ_NONE;
        st_ = &startState_;
        active_ = WHITE;
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

    bool Position::tryAndApplyMove(string &uciMove)
    {
        Move theMove;
        if (!getMoveFromUci(&theMove, uciMove))
            return false;
        return tryAndApplyMove(theMove);
    }

    bool Position::tryAndApplyMove(Move m)
    {
        try {
            string pgn = generatePGN(m);
            applyMove(m);
            pgnMoves_.push_back(pgn);
            return true;
        } catch (InvalidMoveException e) {
            return false;
        }
    }

    bool Position::tryMove(Move m)
    {
        try {
            applyMove(m);
            undoMove();
        } catch (InvalidMoveException e) {
            return false;
        }
        return true;
    }

    void Position::undoLastMove()
    {
        undoMove();
        pgnMoves_.pop_back();
    }

    void Position::undoMove()
    {
        if (moves_.empty())
            return;
        Move m = moves_.back();
        moves_.pop_back();
        StateInfo *mSI = m.state;
        StateInfo *prevSI = (moves_.empty())?&startState_:moves_.back().state;
        active_ = Color(!active_);

        /*Undo the main part of move*/
        board_[m.from] = board_[m.to];
        board_[m.to] = NO_PIECE;

        /*Restore captured piece (En passant is handled separately)*/
        if (m.captured != NO_KIND && m.type != ENPASSANT)
            board_[m.to] = make_piece(Color(!active_), m.captured);

        if (m.type == NORMAL) {
            /*
             * Do nothing : reverse double pawn push and reverse king/rook are
             * handled by reseting the StateInfo object.
             */
        } else if (m.type == PROMOTION) {
            /*Downgrade the promoted piece to pawn*/
            board_[m.from] = make_piece(active_, PAWN);
        } else if (m.type == ENPASSANT) {
            /*Restore pawn taken*/
            Square restore = (active_ == WHITE)?
                make_square(Rank(rank_of(m.to) - 1), file_of(m.to)):
                make_square(Rank(rank_of(m.to) + 1), file_of(m.to));
            board_[restore] = make_piece(Color(!active_), PAWN);
        } else if (m.type == CASTLING) {
            /*Move the rook (king's already handled)*/
            Square rookFrom, rookTo;
            COMPUTE_CASTLING_ROOK(m, rookFrom, rookTo);
            /*
             * Here from is to and to is from.
             * (eg: for OO, rookFrom is H1, rookTo is F1)
             */
            board_[rookFrom] = board_[rookTo];
            board_[rookTo] = NO_PIECE;
        }


        /*Restore previous state*/
        st_ = prevSI;
        if (mSI && mSI != &startState_)
            delete mSI;
    }

    const std::vector<Move> &Position::getMoves() const
    {
        return moves_;
    }

    std::string Position::getLastMove() const
    {
        if (moves_.empty())
            return "NOMOVES";
        Move last = moves_.back();
        return move_to_string(last);
    }

    string Position::pretty() const
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
                oss << "|";
                oss << piece_to_string(p, true);
                if (f == FILE_H) {
                    oss << "| ";
                    if (r == RANK_8) {
                        oss << "Pieces taken : ";
                        int count = 0;
                        for (Move m : moves_) {
                            if (m.captured != NO_KIND) {
                                if (count != 0)
                                    oss << ", ";
                                Color cTaken = (color_of(m.moving) == WHITE) ?
                                               BLACK : WHITE;
                                oss << piece_to_string(
                                        make_piece(cTaken, m.captured),
                                        true);
                                count++;
                            }
                        }
                        if (count == 0)
                            oss << "No piece taken.";
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
        oss << "   a b c d e f g h\n";
        oss << "Move history :" << moveHistory() << "\n";
        oss << "PGN history :" << pgn() << "\n\n";
        return oss.str();
    }

    string Position::moveHistory() const
    {
        std::ostringstream ss;
        for (Move m : moves_) {
            ss << " " << move_to_string(m);
        }
        return ss.str();
    }

    string Position::pgn() const
    {
        std::ostringstream ss;
        int printed = 0;
        int moveIndexFirst = st_->fullmoveClock -
                             (moves_.size() + (int)(active_ == WHITE))/2;
        int moveIndex = moveIndexFirst;
        //This is not related to halfmove clock
        int halfMove = 0;
        ss << moveIndex << ".";
        int notPar = moves_.size()%2;
        if ((active_ == WHITE)^notPar) {
            ss << "..";
            halfMove++;
        }
        ss << " ";

        for (string m : pgnMoves_) {
            if (moveIndex != moveIndexFirst && !(halfMove%2))
                ss << moveIndex << ". ";
            ss << m << " ";
            halfMove++;
            if (!(halfMove%2))
                moveIndex++;
            printed++;
            if (printed%10 == 0)
                ss << "\n";
        }
        return ss.str();
    }

    string Position::fen() const {

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

    string Position::signature() const
    {
        string retVal = "";
        for (Piece p : board_) {
            if (p != NO_PIECE)
                retVal += PieceToChar[p];
        }
        std::sort(retVal.begin(), retVal.end());
        return retVal;
    }

    uint64_t Position::hash() const
    {
        return HashTable::hashFEN(fen());
    }

    /*Return true if lhs < rhs*/
    bool Position::compareLines(const Line &lhs, const Line &rhs)
    {
        Move lhsM;
        if (!getMoveFromUci(&lhsM, lhs.firstMove()))
            Err::handle("Comparing illegal moves");
        Move rhsM;
        if (!getMoveFromUci(&rhsM, rhs.firstMove()))
            Err::handle("Comparing illegal moves");
        /*
         * First check the eval if they are too different.
         * (For example if the cp_threshold is 300 cp, then lines at -2.2 and +1.2
         * are equivalent, but the last one is better !)
         */
        float lhsEv = lhs.getEval();
        float rhsEv = rhs.getEval();
        Out::output("Comparing " + std::to_string(lhsEv) + " to "
                    + std::to_string(rhsEv) + "\n", 3);
        /*Set the limit to .5 eval*/
        if (abs(lhsEv - rhsEv) > 20) {
            return lhsEv > rhsEv;
        }

        /*Optionally add some restriction on the line*/
        return Options::getInstance().getMoveComparator()->compare(*this, lhsM, rhsM);
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
    uint16_t uciToPolyglot(const string &mv)
    {
        File f;
        Rank r;
        uint16_t encodedMove = 0x0;
        if (mv.size() != 4 && mv.size() != 5)
            Err::handle("Error parsing uci move");
        if (mv.size() == 5) {
            Piece promote = piece_from_char(mv[4]);
            switch (kind_of(promote)) {
                case KNIGHT:
                    encodedMove |= 0x1;
                    break;
                case BISHOP:
                    encodedMove |= 0x2;
                    break;
                case ROOK:
                    encodedMove |= 0x3;
                    break;
                case QUEEN:
                    encodedMove |= 0x4;
                    break;
                default:
                    break;
            }
            encodedMove <<= 3;
        }
        string from = mv.substr(0, 2);
        Square sqFrom = square_from_string(from);
        if (sqFrom == SQ_NONE)
            Err::handle("Error parsing uci move");
        r = rank_of(sqFrom);
        f = file_of(sqFrom);
        /*Board::squareFromString(from, &f, &r);*/
        //Rank in 1..8, go to 0..7
        encodedMove |= (uint16_t)r;
        encodedMove <<= 3;
        //File already on 0..7
        encodedMove |= (uint16_t)f;
        encodedMove <<= 3;

        string to = mv.substr(2, 2);
        Square sqTo = square_from_string(to);
        if (sqTo == SQ_NONE)
            Err::handle("Error parsing uci move");
        r = rank_of(sqTo);
        f = file_of(sqTo);
        /*Board::squareFromString(to, &f, &r);*/
        //Rank in 1..8, go to 0..7
        encodedMove |= (uint16_t)r;
        encodedMove <<= 3;
        //File already on 0..7
        encodedMove |= (uint16_t)f;
        //encodedMove <<= 3;
        Out::output("Move " + mv + " is " + std::to_string(encodedMove)
                + ", decimal\n", 3);
        return encodedMove;
    }

    string polyglotToUci(uint16_t mv)
    {
        File f;
        Rank r;
        string uci = "";
        f = (File)(mv & 0x7);
        mv >>= 3;
        r = (Rank)(mv & 0x7);
        mv >>= 3;
        //To
        //uci += std::to_string(to_char(f)) + std::to_string(to_char(r+1));
        uci += file_to_char(f);
        uci += rank_to_char(r);

        f = (File)(mv & 0x7);
        mv >>= 3;
        r = (Rank)(mv & 0x7);
        mv >>= 3;
        //From
        string from;
        from += file_to_char(f);
        from += rank_to_char(r);
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

    /*---------------------/
     *                     /
     * Protected methods   /
     *                     /
     *--------------------*/

    /**
     * This method generates the PGN for move m, it assumes the position is
     * the one *before* the move is done.
     */
    string Position::generatePGN(Move &m)
    {
        Square from = m.from;
        Piece p = board_[from];
        Square to = m.to;
        bool simSameRank = false;
        bool simSameFile = false;
        vector<Move> moves;
        for (Square s : getSimilarPieces(from)) {
            moves.clear();
            DISPATCH(moves, kind_of(p), gen_moves, s, *this);
            for (Move simMove : moves) {
                if (simMove.to == to) {
                    simSameRank |= (rank_of(simMove.from) == rank_of(from));
                    simSameFile |= (file_of(simMove.from) == file_of(from));
                }
            }
        }
        string pgn(1, kind_to_char(kind_of(p), false));
        if (simSameRank || kind_of(p) == PAWN)
            pgn += file_to_char(file_of(from));
        if (simSameFile)
            pgn += rank_to_char(rank_of(from));
        if (m.captured != NO_KIND)
            pgn += "x";
        pgn += square_to_string(to);
        if (m.type == PROMOTION)
            pgn += "=" + string(1, kind_to_char(m.promotion, true, true));
        return pgn;
    }

    set<Square> Position::getSimilarPieces(Square from)
    {
        std::set<Square> retVal;
        Piece p = piece_on(from);
        if (p == NO_PIECE)
            return retVal;
        for (Square s = SQ_A1; s <= SQ_H8; ++s) {
            if (s == from)
                continue;
            if (board_[s] == p)
                retVal.insert(s);
        }
        return retVal;
    }

    void Position::applyMove(Move m) throw (InvalidMoveException)
    {
        applyPseudoMove(m);
        if (kingInCheck(Color(!active_))) {
            undoMove();
            throw InvalidMoveException("King is in check after move");
        }
    }

    void Position::applyPseudoMove(Move m) throw (InvalidMoveException)
    {
        if (!is_ok(m.from) || !is_ok(m.to))
            throw InvalidMoveException("Square invalid");
        if (m.type == NO_TYPE)
            throw InvalidMoveException("No move type");
        Piece pFrom = board_[m.from];
        if (pFrom == NO_PIECE)
            throw InvalidMoveException("No piece to move");
        if (color_of(pFrom) != active_)
            throw InvalidMoveException("Moving opposite color piece");

        /* Build state infos after move */
        StateInfo *next = new StateInfo;
        std::memcpy(next, st_, sizeof(StateInfo));
        next->enpassant = SQ_NONE;
        next->halfmoveClock++;
        if (active_ == BLACK)
            next->fullmoveClock++;


        if (m.type == ENPASSANT) {
            /*Handle capture*/
            Square taken = (active_ == WHITE)?
                make_square(Rank(rank_of(m.to) - 1), file_of(m.to)):
                make_square(Rank(rank_of(m.to) + 1), file_of(m.to));
            board_[taken] = NO_PIECE;
        } else if (m.type == NORMAL) {
            /*Handle double pawn push*/
            if (kind_of(pFrom) == PAWN &&
                    abs(int(rank_of(m.to)) - rank_of(m.from)) == 2)
                next->enpassant = make_square(
                        (active_ == WHITE)?RANK_3:RANK_6,
                        file_of(m.from));
            /*Handle rook/king move and castling flag*/
            if (kind_of(pFrom) == KING)
                next->castle &= (active_ == WHITE)?(B_OO | B_OOO):(W_OO | W_OOO);
            else if (kind_of(pFrom) == ROOK) {
                if (active_ == WHITE) {
                    if (file_of(m.from) == FILE_A)
                        next->castle &= (W_OO | B_OOO | B_OO);
                    else if (file_of(m.from) == FILE_H)
                        next->castle &= (B_OO | W_OOO | B_OOO);
                } else {
                    if (file_of(m.from) == FILE_A)
                        next->castle &= (W_OO | W_OOO | B_OO);
                    else if (file_of(m.from) == FILE_H)
                        next->castle &= (W_OO | W_OOO | B_OOO);
                }
            }
        } else if (m.type == PROMOTION) {
            if (m.promotion == NO_KIND)
                throw InvalidMoveException("No promotion type for promotion");
            if (m.promotion < KNIGHT || m.promotion > QUEEN)
                throw InvalidMoveException("Invalid promotion type");
            /*Convert the pawn to promoted piece*/
            board_[m.from] = make_piece(active_, m.promotion);
        } else if (m.type == CASTLING) {
            if (kind_of(pFrom) != KING)
                throw InvalidMoveException("Castling with no king");
            CastlingFlag castleType = (m.to > m.from)?
                ((active_ == WHITE)?W_OO:B_OO):
                ((active_ == WHITE)?W_OOO:B_OOO);

            /*Double-check previous castling flag*/
            if (!(next->castle & castleType))
                throw InvalidMoveException("Castling flag invalid");

            /*Update castling flag*/
            next->castle &= (active_ == WHITE)?(B_OO | B_OOO):(W_OO | W_OOO);

            /*Move the rook (king's handled later)*/
            Square rookFrom, rookTo;
            COMPUTE_CASTLING_ROOK(m, rookFrom, rookTo);
            board_[rookTo] = board_[rookFrom];
            board_[rookFrom] = NO_PIECE;
        }

        /*Reset 50 moves rule*/
        if (kind_of(pFrom) == PAWN || m.captured != NO_KIND)
            next->halfmoveClock = 0;
        board_[m.to] = board_[m.from];
        board_[m.from] = NO_PIECE;

        m.state = next;
        st_ = next;
        moves_.push_back(m);
        active_ = Color(!active_);
    }

    bool Position::kingInCheck(Color c) const
    {
        Square ksq;
        Piece k = make_piece(c, KING);
        for (ksq = SQ_A1; ksq <= SQ_H8; ++ksq)
            if (board_[ksq] == k)
                break;
        return attacked(ksq, Color(!c));
    }

    bool Position::hasSufficientMaterial() const
    {
        list<Piece> allPieces;
        for (Piece p : board_)
            if (p != NO_PIECE)
                allPieces.push_back(p);

        if (allPieces.size() > 3)
            return true;
        else if (allPieces.size() == 2)
            return false;
        else {
            PieceKind notKing = NO_KIND;
            for (Piece p : allPieces)
                if (kind_of(p) != KING)
                    notKing = kind_of(p);
            return !(notKing == BISHOP || notKing == KNIGHT);
        }
    }

    bool Position::getMoveFromUci(Move *move, const std::string &mv)
    {
        Square from = sqFrom_from_uci(mv);
        if (!is_ok(from))
            return false;
        PieceKind k = kind_of(piece_on(from));
        if (k == NO_KIND)
            return false;
        vector<Move> legalMoves;
        DISPATCH(legalMoves, k, gen_moves, from, *this);
        move->from = SQ_NONE;
        for (Move m : legalMoves) {
            if (mv == move_to_string(m)) {
                *move = m;
                break;
            }
        }
        return move->from != SQ_NONE;
    }

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
                if (c >= '1' && c <= '8') {
                    //should rotate through files
                    f += File(c - '0');
                } else {
                    if (!is_ok(s))
                        throw InvalidFenException("Current square ("
                                                  + square_to_string(s)
                                                  + ") is invalid");
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
                        throw InvalidFenException("Can't set castle : '"
                                                  + string(1, c) + "'");
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
#undef COMPUTE_CASTLING_ROOK
