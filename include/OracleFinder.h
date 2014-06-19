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
#include <stack>
#include <unordered_map>
#include <pthread.h>
#include "Line.h"
#include "Finder.h"
#include "Hashing.h"

class NodeStack : private std::stack<Node *> {
    public:
        NodeStack(unsigned long workers);
        void push(Node *n);
        void push(std::vector<Node *> &nodes);
        Node *poptop();
        unsigned int size();
    private:
        pthread_mutex_t lock_ = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t cond_ = PTHREAD_COND_INITIALIZER;
        unsigned long waitingWorkers_ = 0;
        const unsigned long maxWorkers_;
};

namespace OracleBuilder {
    int buildOracle(Board::Color playFor,
                    HashTable *oracle,
                    const std::vector<int> &communicators,
                    const Board::Position &pos,
                    const std::list<std::string> &moves);
    void *exploreNode(void *args);
    void displayNodeHistory(const Node *start);
    bool cutNode(const Board::Position &pos, const Node *currentNode);
}

class OracleFinder : public Finder {
public:
    OracleFinder(std::vector<int> &commIds);
    virtual ~OracleFinder();
    static void dumpStat();

private:
    /*This should now create workers and handle termination*/
    int runFinderOnPosition(const Board::Position &pos,
                            const std::list<std::string> &moves);
    static void *exploreNode(void *args);
#if 0
    Board::LegalMoves getAllMoves();
#endif
    static HashTable *oracleTable_;
    /*
     *Node *rootNode_ = NULL;
     */
    /*static std::list<Node *> toProceed_;*/
    static NodeStack toProceed_;
    static std::map<std::string, int> signStat_;
};

#endif
