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

#include <iostream>
#include <sstream>
#include <cmath>
#include <array>
#include <queue>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <ctime>

#include "Finder.h"
#include "ChessboardTypes.h"
#include "OracleFinder.h"
#include "Stream.h"
#include "Utils.h"
#include "Output.h"
#include "Hashing.h"
#include "Movegen.h"

using namespace std;
using namespace Board;


map<string, int> OracleFinder::signStat_;


NodeStack::NodeStack(unsigned long workers) : maxWorkers_(workers)
{}

void NodeStack::push(Node *n)
{
    pthread_mutex_lock(&lock_);
    std::stack<Node *>::push(n);
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&lock_);
}

void NodeStack::push(std::vector<Node *> &nodes)
{
    pthread_mutex_lock(&lock_);
    for (Node *n : nodes)
        std::stack<Node *>::push(n);
    pthread_cond_signal(&cond_);
    pthread_mutex_unlock(&lock_);
}

Node *NodeStack::poptop()
{
    Node *n = nullptr;
    pthread_mutex_lock(&lock_);
    while (empty()) {
        waitingWorkers_++;
        if (waitingWorkers_ == maxWorkers_) {
            pthread_cond_broadcast(&cond_);
            pthread_mutex_unlock(&lock_);
            return nullptr;
        } else {
            pthread_cond_wait(&cond_, &lock_);
            waitingWorkers_--;
        }
    }
    n = top();
    pop();
    pthread_mutex_unlock(&lock_);
    return n;
}

unsigned int NodeStack::size()
{
    return std::stack<Node *>::size();
}

void OracleBuilder::displayNodeHistory(const Node *start)
{
    const Node *cur = start;
    /*
     * I guess 30 positions are enough to display, since we display this message
     * as soon as we detect an inversion in evaluation.
     */
    int limit = 30;
    int i = 0;
    Out::output("Displaying node history for " + start->getPos()
                + " (reverse order)\n");
    /*TODO think about what to do if multiple parent*/
    while (cur && i < limit) {
        Out::output(cur->to_string() + "\n");
        cur = cur->getParents().back();
        i++;
    }
}

bool OracleBuilder::cutNode(const Position &, const Node *)
{
    /*TODO evaluate if we should process this node or not, according to
     * the chessboard state.*/
    return false;
}

int OracleBuilder::buildOracle(Board::Color playFor,
                               map<string, HashTable *> *oracle,
                               const vector<int> &communicators,
                               const Position &p,
                               const list<string> &moves)
{
    Position pos;
    /*int commId = commIds_.front();*/
    pos.set(p.fen());

    /*Get the side from option*/
    /*playFor_ = (opt_.buildOracleForWhite()) ? WHITE : BLACK;*/

    for (string mv : moves) {
        pos.tryAndApplyMove(mv);
    }
    string startingFen = pos.fen();
    pos.clear();
    pos.set(startingFen);


    Out::output("Building an oracle for : " + color_to_string(playFor) + "\n");
    Out::output("Starting board is :\n" + pos.pretty() + "\n");

/*
 *    Out::output("Doing some basic evaluation on submitted position...\n");
 *
 *    sendCurrentPositionToEngine();
 *    lines_.assign(maxMoves, Line::emptyLine);
 *    sendToEngine("go movetime "
 *            + to_string(MatfinderOptions::getPlayforMovetime()));
 *    waitBestmove();
 *    Out::output("Evaluation is :\n");
 *    Out::output(getPrettyLines());
 *    lines_.clear();
 */

    NodeStack nodes(communicators.size());
    string initFen = pos.fen();
    Node *init = new Node(nullptr, initFen, Node::PENDING);
    Node *rootNode_ = init;
    //depth-first
    nodes.push(rootNode_);
    /*vector<pthread_t> threads;*/
    vector<explorerArgs *> argsVect;
    for (unsigned i = 0; i < communicators.size(); i++) {
        explorerArgs *args = (explorerArgs *)malloc(sizeof(explorerArgs));
        argsVect.push_back(args);
        args->commdId = communicators[i];
        args->tables = oracle;
        args->stack = &nodes;
        args->playFor = playFor;
        Err::handle(pthread_create(&(args->th),
                                   nullptr, exploreNode, args));
    }

    for (explorerArgs *args : argsVect) {
        Err::handle(pthread_join(args->th, nullptr));
        free(args);
    }

    /*
     *TODO we still need to do some cleaning in the table :
     *    - if all moves after a draw are mate or tresh, then this line is not
     *    a draw and should be updated.
     *    - what to do with threshold/mate node ? Matfinder has to close
     *    these lines
     */
    //Display info at the end of computation
    Out::output("[End] Finder is done. Starting board was : \n");
    pos.set(initFen);
    Out::output(pos.pretty() + "\n");


    Out::output("Hashtable size = "
            + std::to_string((*oracle)[""]->hash_size()) + ") : \n");
    Out::output((*oracle)[""]->to_string() + "\n", 2);
    Out::output("(size = " + std::to_string((*oracle)[""]->hash_size()) + ") : \n", 2);

    return 0;
}




OracleFinder::OracleFinder(vector<int> &commIds) : Finder(commIds)
{
    //engine_side_ = cb_->getActiveSide();
    //engine_play_for_ = MatfinderOptions::getPlayFor();
    string inputFilename = opt_.getInputFile();
    if (inputFilename.size() > 0) {
        Out::output("Loading main table from " + inputFilename + ".\n", 2);
        ifstream inputFile(inputFilename, ios::binary);
        if (!inputFile.good())
            Err::handle("Unable to load table from file "
                        + inputFilename);
        oracleTables_[""] = HashTable::fromPolyglot(inputFile);
    } else {
        Out::output("Creating new main empty table.\n", 2);
        oracleTables_[""] = new HashTable();
    }
    for (auto pair : opt_.getInputTables()) {
        const string &sign = pair.first;
        const string &inFile = pair.second;
        if (oracleTables_.count(sign))
            Err::handle("Loading twice a table for the same signature ("
                        + sign +")");
        Out::output("Loading table from " + inFile + ".\n", 2);
        ifstream inputFile(inFile, ios::binary);
        if (!inputFile.good())
            Err::handle("Unable to load signature table \"" + sign
                        + "\" from file " + inFile);
        oracleTables_[sign] = HashTable::fromPolyglot(inputFile);
    }
}

OracleFinder::~OracleFinder()
{
    string outputFilename = opt_.getOutputFile();
    if (outputFilename.size() > 0) {
        Out::output("Saving table to " + outputFilename + ".\n", 2);
        ofstream outputFile(outputFilename, ios::binary);
        if (!outputFile.good())
            Out::output("Unable to save table to file "
                    + outputFilename + "\n");
        else
            oracleTables_[""]->toPolyglot(outputFile);
    }
    for (auto entry : oracleTables_) {
        delete entry.second;
    }
    dumpStat();
}

void OracleFinder::dumpStat()
{
    Out::output("Materiel signature hit statistics :\n", 2);
    for (auto elem : signStat_) {
        Out::output(elem.first + " : " + to_string(elem.second) + "\n", 2);
    }
    if (signStat_.size() == 0)
        Out::output("No hit...\n", 2);
}

void *OracleBuilder::exploreNode(void *args)
{
    Position pos;
    Comm::UCICommunicatorPool &pool = Comm::UCICommunicatorPool::getInstance();
    Options &opt = Options::getInstance();
    explorerArgs *all = (explorerArgs *)args;
    Board::Color playFor = all->playFor;
    int commId = all->commdId;
    map<string, HashTable *> *tables = all->tables;
    HashTable *oracle = (*tables)[""];
    NodeStack *nodes = all->stack;
    Err::handle("Args unpacking", !(oracle && nodes));
    pool.sendOption(commId, "MultiPV", to_string(opt.getMaxMoves()));
    //Main loop
    Node *current = nullptr;
    while ((current = nodes->poptop())) {
        const string currentPos = current->getPos();

        Line bestLine;
        /*Set the chessboard to current pos*/
        pos.set(currentPos);
        Color active = pos.side_to_move();
        string signature = pos.signature();
        uint64_t curHash = pos.hash();

        /*Check we are not computing an already existing position*/
        if (tables->find(signature) != tables->end()) {
            HashTable *signTable = (*tables)[signature];
            Node *s = nullptr;
            if ((s = signTable->findPos(curHash))) {
                current->updateStatus((Node::StatusFlag)
                                      (s->getStatus() | Node::SIGNATURE_TABLE));
                if (oracle->findOrInsert(curHash, current) != current)
                    delete current;
                continue;
            }
        }
        /*Try to find the position and insert it if not found*/
        if (oracle->findOrInsert(curHash, current) != current) {
            Out::output("Position already in table.\n", 1);
            delete current;
            continue;
        }

        /* TODO rethink this, as we should probably remove the position if cut
         *if (OracleBuilder::cutNode(pos, current)) {
         *    Out::output("Node cut by user-defined function", 1);
         *    continue;
         *}
         */
        if (nodes->size() % 10000 == 0)
            Out::output("[" + color_to_string(active) + "] Proceed size : "
                        + to_string(nodes->size()) + "\n");

        /*Clear cut*/
        if (!pos.hasSufficientMaterial()) {
            current->updateStatus(Node::DRAW);
            Out::output("[" + color_to_string(active)
                    + "] Insuficient material.\n", 2);
            //Proceed to next node...
            continue;
        }


        Out::output(pos.pretty(), 2);
        int hit = OracleFinder::signStat_[signature];
        OracleFinder::signStat_[signature] = ++hit;
        string position = "position fen ";
        position += pos.fen();
        pool.send(commId, position);

        if (playFor != active) {
            /*We are on a node where the opponent has to play*/
            current->updateStatus(Node::AGAINST);
            /*proceedAgainstNode(pos, current);*/
            vector<Move> all = gen_all(pos);
            /*
             * Push all node for the side we "play for".
             * eg: if we are building an oracle for white, we need to push all
             * possible white positions when computing a black node.
             */
            Out::output("Push all lines : ", 2);
            for (Move m : all) {
                string uciMv = move_to_string(m);
                Out::output("+", 2);
                if (!pos.tryAndApplyMove(m))
                    Err::handle("Illegal move pushed ! (While proceeding against Node)");
                string fen = pos.fen();
                pos.undoLastMove();
                Node *next = new Node(current, fen, Node::PENDING);
                nodes->push(next);
                MoveNode move(uciMv, next);
                current->safeAddMove(move);
            }
            Out::output("\n", 2);
            continue;
        }

        /**********************************************/
        /*Here we are on a node with "playfor" to play*/
        /**********************************************/

        const vector<Line> &lines = pool.getResultLines(commId);

        /*Thinking according to the side the engine play for*/
        int moveTime = opt.getPlayforMovetime();

        Out::output("[" + color_to_string(active) + "] Thinking... ("
                    + to_string(moveTime) + ")\n", 1);

        string cmd = "go ";
        switch (opt.getSearchMode()) {
            case DEPTH:
                cmd += "depth " + to_string(opt.getSearchDepth());
                break;
            case MIXED://Intentional no-break TODO: implement mixed search ?
            case TIME:
            default:
                cmd += "movetime " + to_string(moveTime);
                break;
        }
        //Send go and wait for engine to finish thinking
        pool.sendAndWaitBestmove(commId, cmd);


        Out::output(Utils::getPrettyLines(pos, lines), 2);

        bestLine = lines[0];
        if (bestLine.empty()) {
            //STALEMATE
            current->updateStatus(Node::STALEMATE);
            Out::output("[" + color_to_string(active)
                        + "] Bestline is stalemate (cut)\n", 2);
            //Proceed to next node...
            continue;
        } else if (bestLine.isMat()) {
            Out::output("[" + color_to_string(active)
                        + "] Bestline is mate (cut)\n", 2);
            /*Eval is always negative if it's bad for us*/
            if (bestLine.getEval() < 0) {
                current->updateStatus((Node::StatusFlag)(Node::MATE | Node::THEM));
                OracleBuilder::displayNodeHistory(current);
                Err::handle("A node has gone from draw to mate, this is an error"
                            " until we decide on what to do, and if it's a bug"
                            " in the engine.");
            } else {
                current->updateStatus((Node::StatusFlag)(Node::MATE | Node::US));
            }
            continue;
        } else if (fabs(bestLine.getEval()) > opt.getCutoffThreshold()) {
            Out::output("[" + color_to_string(active)
                        + "] Bestline is above threshold (cut)\n", 2);
            if (bestLine.getEval() < 0) {
                current->updateStatus((Node::StatusFlag)(Node::THRESHOLD | Node::THEM));
                OracleBuilder::displayNodeHistory(current);
                Err::handle("A node has gone from draw to threshold, this is an error"
                            " until we decide on what to do, and if it's a bug"
                            " in the engine.");
            } else {
                current->updateStatus((Node::StatusFlag)(Node::THRESHOLD | Node::US));
            }
            continue;
        }

        /*If we are here, bestLine is draw, and we should continue to explore*/
        vector<Line> draw;
        vector<Line> cut;

        /*Split draw and unbalanced lines*/
        for (Line l : lines) {
            if (l.empty())
                continue;
            int limit = opt.getCutoffThreshold();
            if (l.isMat())
                cut.push_back(l);
            else if (fabs(l.getEval()) <= limit)
                draw.push_back(l);
            else
                cut.push_back(l);
        }

        current->updateStatus(Node::DRAW);

        /*
         *Add all the imbalanced line to the hashtable (either mat or "threshold")
         *(save some iterations in main loop : we could also push all the
         *unbalanced lines and see...)
         */
        for (Line l : cut) {
            Node::StatusFlag s = Node::THEM;
            if (l.isMat())
                s = (Node::StatusFlag)(s | Node::MATE);
            else
                s = (Node::StatusFlag)(s | Node::THRESHOLD);
            Node *toAdd = new Node(current, pos.fen(), s);
            string next = l.firstMove();
            if (!pos.tryAndApplyMove(next))
                Err::handle("Illegal move pushed ! (In an unbalanced line)");
            uint64_t hash = pos.hash();
            pos.undoLastMove();
            pair<uint64_t, Node *> p(hash, toAdd);
            /*If a position if found, do not insert node and delete it*/
            if (oracle->findOrInsert(hash, toAdd) != toAdd)
                delete toAdd;
        }



        /*
         *Then proceed the balanced lines according to the side the engine
         *play for.
         */
        if (playFor != active)
            Err::handle("Current side should be the engine plays for");

        /*
         * General Idea : try to force repetition by trying to find a playable
         * position in the hashtable. If not sort the playable moves
         * according to a user defined comparator
         */
        Node *next = NULL;
        /*The elected move*/
        string mv;
        /*Try to find a position in the table*/
        /* There must be a reason to go trough it backward, but I can't
         * remember it right now.
         */
        Line l;
        for (auto rit = draw.rbegin(); rit != draw.rend(); ++rit) {
            l = *rit;
            mv = l.firstMove();
            if (!pos.tryAndApplyMove(mv)) {
                Err::output(pos.pretty());
                Err::output("Move : " + mv);
                Err::handle("Illegal move while proceeding a draw node");
            }
            /*This is the next pos*/
            uint64_t hashpos = pos.hash();
            pos.undoLastMove();
            //Jean Louis' idea to force finding positions in oracle
            next = oracle->findPos(hashpos);
            if (next) {
                next->safeAddParent(current);
                break;
            }
        }
        /*FIXME: what if positive eval ?*/
        /*Maybe just continue, but then we should register that mate or
         * threshold is winning or losing*/
        if (!next) {
            std::sort(draw.begin(), draw.end(),
                         [&pos](const Line &lhs, const Line &rhs)
                         {
                             return pos.compareLines(lhs, rhs);
                         });
            l = draw[0];
            mv = l.firstMove();
            pos.tryAndApplyMove(mv);
            string fenpos = pos.fen();
            pos.undoLastMove();

            //no next position in the table, push the node to stack
            next = new Node(current, fenpos, Node::PENDING);
            Out::output("[" + color_to_string(active)
                    + "] Pushed first line (" + mv + ") : " + fenpos + "\n", 2);
            nodes->push(next);
        }
        /*Whatever the move is, add it to our move list*/
        MoveNode move(mv, next);
        current->safeAddMove(move);
        Out::output("-----------------------\n", 1);

    }
    return 0;
}

int OracleFinder::runFinderOnPosition(const Position &p,
                                      const list<string> &moves)
{
    using namespace OracleBuilder;

    /*Get the side from option*/
    playFor_ = (opt_.buildOracleForWhite()) ? WHITE : BLACK;


    return buildOracle(playFor_, &oracleTables_, commIds_, p, moves);
}
