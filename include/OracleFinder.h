/*
 * Oraclefinder, a program to find "perfect" chess game
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
#ifndef __ORACLEFINDER_H__
#define __ORACLEFINDER_H__

#include <string>
#include <vector>
#include <list>
#include <vector>
#include <unordered_map>
#include "Line.h"
#include "Finder.h"
#include "Hashing.h"


class OracleFinder : public Finder {
public:
    OracleFinder(int comm);
    virtual ~OracleFinder();
    static void dumpStat();

private:
    int runFinderOnPosition(const Board::Position &pos,
                            const std::list<std::string> &moves);
    void getLines(const std::vector<Line> all, std::vector<Line> &balanced,
                  std::vector<Line> &unbalanced);
#if 0
    Board::LegalMoves getAllMoves();
#endif
    void proceedAgainstNode(Board::Position &pos, Node *againstNode);
    void proceedUnbalancedLines(Board::Position &pos,
                                std::vector<Line> &unbalanced);
    void pushAllLines(std::vector<Line &> lines, Node *currentNode);
    bool cutNode(const Board::Position &pos, const Node *currentNode);
    HashTable *oracleTable_;
    /*
     *Node *rootNode_ = NULL;
     */
    std::list<Node *> toProceed_;
    static std::map<std::string, int> signStat_;
};

#endif
