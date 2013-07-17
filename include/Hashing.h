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
#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__

#include <cstdint>
#include <ostream>
#include <map>
#include <vector>
#include "Chessboard.h"
#include "Board.h"

#ifdef _MSC_VER
#define U64(u) (u##ui64)
#else
#define U64(u) (u##ULL)
#endif

using namespace std;

class Node;

typedef pair<Board::UCIMove, Node *> MoveNode;

class Node {
public:
    ~Node();
    enum Status {
        TRESHOLD,
        MATE,
        STALEMATE,
        DRAW
    };
    SimplePos pos;
    /*
     * FIXME: only store the move !
     * OR : store as many info, then export only needed
     * legal_moves.size() = 1 when active side is playfor side
     * legal_moves contains all possible moves when playing
     * for opposite side
     */
    //vector<UCIMove> legal_moves;
    vector<MoveNode> legal_moves;
    //Polyglot "learn" field, uint32_t
    Status st;
    string to_string();
    static string to_string(Status s);
};

//map is internally ordered by key, ascending
class HashTable : public multimap<uint64_t, Node *>
{
public:

    static uint64_t hashFEN(string fenString);
    static uint64_t hashBoard(Chessboard *cb);
    ~HashTable();
    string to_string();
    //TODO: rename simplepos to FEN
    Node *findPos(SimplePos sp);
    void toPolyglot(ostream &os);
    static HashTable *fromPolyglot(istream &is);
private:
    static int pieceOffset(int kind, Board::Rank r, Board::File f);
    static uint64_t turnFromFEN(string side);
    static uint64_t piecesFromFEN(string pos);

    static const uint64_t Random64_[781];
};

#endif
