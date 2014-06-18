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
    private:
        pthread_mutex_t lock_ = PTHREAD_MUTEX_INITIALIZER;
        pthread_cond_t cond_ = PTHREAD_COND_INITIALIZER;
        unsigned long waitingWorkers_ = 0;
        const unsigned long maxWorkers_;
};

class OracleFinder : public Finder {
public:
    OracleFinder(std::vector<int> &commIds);
    virtual ~OracleFinder();
    static void dumpStat();

private:
    /*This should now create workers and handle termination*/
    int runFinderOnPosition(const Board::Position &pos,
                            const std::list<std::string> &moves);
    /*TODO*/
/*
 *Idea :
 *    - workers get work from toProceed_ via getSafeNode
 *    - toproceed should have safeInsert (+signal) and safeGet
 *    - workers does somthg, if safeGet = null && sleeping workers = N-1 then quit
 *        else cond.wait
 */
    static void *exploreNode(void *args);
    static void getLines(const std::vector<Line> all, std::vector<Line> &balanced,
                         std::vector<Line> &unbalanced);
#if 0
    Board::LegalMoves getAllMoves();
#endif
    static void proceedAgainstNode(Board::Position &pos, Node *againstNode);
    static void proceedUnbalancedLines(Board::Position &pos,
                                       const Node *cur,
                                       std::vector<Line> &unbalanced);
    static void pushAllLines(std::vector<Line &> lines, Node *currentNode);
    static bool cutNode(const Board::Position &pos, const Node *currentNode);
    static void displayNodeHistory(const Node *start);
    static void push_node(Node *n);
    static void push_nodes(std::vector<Node *> &nodes);
    static Node *pop_node();
    static HashTable *oracleTable_;
    /*
     *Node *rootNode_ = NULL;
     */
    static std::list<Node *> toProceed_;
    static std::map<std::string, int> signStat_;
    static pthread_cond_t stackCond_;
    static pthread_mutex_t stackLock_;
    static unsigned long waitingWorkers_;
};

#endif
