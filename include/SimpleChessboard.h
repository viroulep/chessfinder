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
#ifndef __SIMPLECHESSBOARD_H__
#define __SIMPLECHESSBOARD_H__
#include <exception>
#include <vector>
#include <set>

#include "ChessboardTypes.h"
#include "Line.h"

namespace Board {
    /*TODO get rid of exceptions with retval*/

    extern const std::string PieceToChar;

    class InvalidFenException : public std::exception {
        public:
            InvalidFenException(std::string msg);
            virtual const char* what() const throw();
        private:
            std::string fenmsg;
    };

    class InvalidMoveException : public std::exception {
        public:
            InvalidMoveException(std::string msg);
            virtual const char* what() const throw();
        private:
            std::string moveMsg;
    };

    typedef struct StateInfo {
        Square enpassant = SQ_NONE;
        int castle = 0, halfmoveClock = 0, fullmoveClock = 0;
        /*PieceKind captured = NO_KIND;*/
    } StateInfo;

    typedef struct Move {
        Square from = SQ_NONE;
        Square to = SQ_NONE;
        MoveType type = NO_TYPE;
        Piece moving = NO_PIECE;
        PieceKind promotion = NO_KIND;
        PieceKind captured = NO_KIND;
        StateInfo *state = nullptr;
    } Move;

    class Position {
        public:
            Position();
            ~Position();
            Color side_to_move() const;
            bool empty(Square s) const;
            bool attacked(Square s, Color c) const;
            bool takes(Square attaker, Square target) const;
            bool kingInCheck(Color c) const;
            bool hasSufficientMaterial() const;
            std::set<Square> pieces_squares(Color c) const;
            Piece piece_on(Square s) const;
            Square enpassant() const;
            Square king(Color c) const;
            bool canCastle(CastlingFlag f) const;
            virtual void init();
            virtual void clear();
            void set(std::string fenString) throw(InvalidFenException);
            bool tryAndApplyMove(std::string &uciMove);
            bool tryAndApplyMove(Move m);
            bool tryMove(Move m);
            void undoLastMove();
            const std::vector<Move> &getMoves() const;
            std::string getLastMove() const;
            std::string pretty() const;
            std::string moveHistory() const;
            std::string pgn() const;
            std::string fen() const;
            std::string signature() const;
            uint64_t hash() const;
            bool compareLines(const Line &lhs, const Line &rhs);
        protected:
            /*A board is an array of 64 pieces (can be NO_PIECE)*/
            Piece board_[64];
            Color active_;
            StateInfo startState_;
            StateInfo *st_ = nullptr;
            std::vector<Move> moves_;
            std::vector<std::string> pgnMoves_;

            /*Generate pgn notation for last move*/
            std::string generatePGN(Move &m);

            std::set<Square> getSimilarPieces(Square from);

            void applyMove(Move m) throw(InvalidMoveException);
            void applyPseudoMove(Move m) throw(InvalidMoveException);
            void undoMove();

            bool getMoveFromUci(Move *move, const std::string &mv);

            void setPos(std::string fenPos) throw(InvalidFenException);
            void setSide(std::string fenSide) throw(InvalidFenException);
            void setCastle(std::string fenCastle) throw(InvalidFenException);
            void setEP(std::string fenEP) throw(InvalidFenException);
            void setClock(std::string fenClock, int &clock) throw(InvalidFenException);
            void setFMClock(std::string fenFM) throw(InvalidFenException);

            inline bool can_castle(CastlingFlag f) const { return st_->castle & f; }
            inline bool can_castle(Color c) const
            {
                return st_->castle & ((W_OO | W_OOO) << (2 * c));
            }
    };


    inline Piece piece_from_char(char p)
    {
        size_t idx = PieceToChar.find(p);
        return (idx != std::string::npos)?Piece(idx):NO_PIECE;
    }

    /*Return the uci formated move*/
    inline const std::string move_to_string(Move m)
    {
        std::string retVal;
        retVal += square_to_string(m.from) + square_to_string(m.to);
        if (m.type == PROMOTION)
            retVal += {kind_to_char(m.promotion, true, true)};
        return retVal;
    }

    /*Polyglot helpers*/
    uint16_t uciToPolyglot(const std::string &mv);
    std::string polyglotToUci(uint16_t mv);

}

#endif
