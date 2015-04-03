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
    unique_lock<mutex> lock(lock_);
    std::stack<Node *>::push(n);
    cond_.notify_one();
}

void NodeStack::push(std::vector<Node *> &nodes)
{
    unique_lock<mutex> lock(lock_);
    for (Node *n : nodes)
        std::stack<Node *>::push(n);
    cond_.notify_one();
}

Node *NodeStack::poptop()
{
    Node *n = nullptr;
    unique_lock<mutex> lock(lock_);
    while (empty()) {
        waitingWorkers_++;
        if (waitingWorkers_ == maxWorkers_) {
            cond_.notify_all();
            return nullptr;
        } else {
            cond_.wait(lock);
            waitingWorkers_--;
        }
    }
    n = top();
    pop();
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
                               ConcurrentMap<string, HashTable *> &oracle,
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
    vector<thread> threads(communicators.size());
    for (unsigned int i = 0; i < threads.size(); i++) {
        threads[i] = thread(OracleBuilder::exploreNode, std::ref(oracle),
                            std::ref(nodes), playFor, communicators[i]);
    }

    for (thread &t : threads) {
        t.join();
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
            + std::to_string(oracle[""]->size()) + ") : \n");
    Out::output(oracle[""]->to_string() + "\n", 2);
    Out::output("(size = " + std::to_string(oracle[""]->size()) + ") : \n", 2);

    return 0;
}




OracleFinder::OracleFinder(vector<int> &commIds) : Finder(commIds)
{
    string inputFilename = opt_.getInputFile();
    if (inputFilename.size() > 0) {
        oracleTables_[""] = HashTable::fromPolyglot(inputFilename);
    } else {
        Out::output("Creating new main empty table.\n", 2);
        oracleTables_[""] = new HashTable("");
    }
    for (const string &inFile : Utils::filesFromDir(opt_.getTableFolder(),
                                                    ".bin")) {
        const string &sign = Utils::signatureFromFilename(inFile);
        if (sign.length() == 0)
            Err::handle("Unable to determine table signature (" + inFile + ")");
        if (oracleTables_.count(sign))
            Err::handle("Loading twice a table for the same signature ("
                        + inFile + "/" + sign +")");
        string fileInDir = opt_.getTableFolder() + "/" + inFile;
        Out::output("Loading table \"" + fileInDir + "\" with signature \""
                    + sign + "\".\n", 2);
        oracleTables_[sign] = HashTable::fromPolyglot(fileInDir);
    }
}

OracleFinder::~OracleFinder()
{
    string outputFilename = opt_.getOutputFile();
    for (auto entry : oracleTables_) {
        if (entry.first == "") {
            if (outputFilename.length() > 0)
                entry.second->toPolyglot(outputFilename);
        } else {
            entry.second->autosave();
        }
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

void OracleBuilder::exploreNode(ConcurrentMap<string, HashTable *> &tables,
                                NodeStack &nodes, Color playFor, int commId)
{
    Position pos;
    Comm::UCICommunicatorPool &pool = Comm::UCICommunicatorPool::getInstance();
    Options &opt = Options::getInstance();
    HashTable *oracle = tables[""];
    pool.sendOption(commId, "MultiPV", to_string(opt.getMaxMoves()));
    //Main loop
    Node *current = nullptr;
    while ((current = nodes.poptop())) {
        string iterationOutput;
        const string currentPos = current->getPos();

        Line bestLine;
        /*Set the chessboard to current pos*/
        pos.set(currentPos);
        Color active = pos.side_to_move();
        string signature = pos.signature();
        uint64_t curHash = pos.hash();
        bool insertCopyInSignTable = false;

        /*Lookup in signature tables*/
        if (signature.length() <= opt.getMaxPiecesEnding()) {
            HashTable *table = nullptr;
            if (tables.count(signature) == 0) {
                string filename = opt.getTableFolder() + "/" + signature
                                  + ".autosave."
                                  + opt.getVariantAsString()
                                  + to_string(opt.getCutoffThreshold())
                                  + ".bin";
                table = new HashTable(filename);
                if (tables.findOrInsert(signature, table) != table) {
                    delete table;
                    table = tables[signature];
                }
            } else {
                table = tables[signature];
            }
            Node *s = nullptr;
            if ((s = table->findVal(curHash))) {
                current->updateStatus((Node::StatusFlag)
                                      (s->getStatus() | Node::SIGNATURE_TABLE));
                if (current->getStatus() & Node::THEM) {
                    OracleBuilder::displayNodeHistory(current);
                    Out::output("[Sign lookup] Iteration output for error :\n" + iterationOutput);
                    Err::handle("A node has gone from draw to mate, this is an error"
                                " until we decide on what to do, and if it's a bug"
                                " in the engine.");
                }
                if (oracle->findOrInsert(curHash, current) != current)
                    delete current;
                continue;
            } else {
                insertCopyInSignTable = true;
            }
        }

        /*Try to find the position and insert it if not found*/
        if (oracle->findOrInsert(curHash, current) != current) {
            Out::output(iterationOutput, "Position already in table.\n", 1);
            delete current;
            continue;
        }

        /* TODO rethink this, as we should probably remove the position if cut
         *if (OracleBuilder::cutNode(pos, current)) {
         *    Out::output("Node cut by user-defined function", 1);
         *    continue;
         *}
         */
        Out::output(iterationOutput, "[" + color_to_string(active)
                    + "] Proceed size : " + to_string(nodes.size())
                    + "\n");

        /*Clear cut*/
        if (!pos.hasSufficientMaterial()) {
            current->updateStatus(Node::DRAW);
            Out::output(iterationOutput, "[" + color_to_string(active)
                        + "] Insuficient material.\n", 2);
            //Proceed to next node...
            continue;
        }


        Out::output(iterationOutput, pos.pretty(), 2);

        /*Hit statistic*/
        __atomic_fetch_add(&OracleFinder::signStat_[signature], 1, __ATOMIC_SEQ_CST);

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
            Out::output(iterationOutput, "Push all lines : ", 2);
            for (Move m : all) {
                string uciMv = move_to_string(m);
                Out::output(iterationOutput, "+", 2);
                if (!pos.tryAndApplyMove(m))
                    Err::handle("Illegal move pushed ! (While proceeding against Node)");
                string fen = pos.fen();
                pos.undoLastMove();
                Node *next = new Node(current, fen, Node::PENDING);
                nodes.push(next);
                MoveNode move(uciMv, next);
                current->safeAddMove(move);
            }
            Out::output(iterationOutput, "\n", 2);
            continue;
        }

        /**********************************************/
        /*Here we are on a node with "playfor" to play*/
        /**********************************************/

        const vector<Line> &lines = pool.getResultLines(commId);

        /*Thinking according to the side the engine play for*/
        int moveTime = opt.getPlayforMovetime();

        Out::output(iterationOutput, "[" + color_to_string(active)
                    + "] Thinking... (" + to_string(moveTime) + ")\n", 1);

        string cmd = "go ";
        switch (opt.getSearchMode()) {
            case DEPTH:
                cmd += "depth " + to_string(opt.getSearchDepth());
                break;
            case MIXED://Intentional no-break
            /*TODO: implement mixed search ? likely not,
             * early multipv in stockfish*/
            case TIME:
            default:
                cmd += "movetime " + to_string(moveTime);
                break;
        }
        //Send go and wait for engine to finish thinking
        pool.sendAndWaitBestmove(commId, cmd);


        Out::output(iterationOutput, Utils::getPrettyLines(pos, lines), 2);

        bestLine = lines[0];
        bool skipThisNode = false;
        if (bestLine.empty()) {
            //STALEMATE
            current->updateStatus(Node::STALEMATE);
            Out::output(iterationOutput, "[" + color_to_string(active)
                        + "] Bestline is stalemate (cut)\n", 2);
            //Proceed to next node...
            skipThisNode = true;
        } else if (bestLine.isMat()) {
            Out::output(iterationOutput, "[" + color_to_string(active)
                        + "] Bestline is mate (cut)\n", 2);
            /*Eval is always negative if it's bad for us*/
            if (bestLine.getEval() < 0) {
//TODO : remove this, then push back previous pos in queue from table after cleaning out the table
                current->updateStatus((Node::StatusFlag)(Node::MATE | Node::THEM));
                Err::output("A node went from draw to loss, clearing table and"
                            " backtrackin");
                pool.clearHash(commId);
                Node *prevNode = const_cast<Node *>(current->getParents().back());
                Node *prevWhiteNode = const_cast<Node *>(prevNode->getParents().back());
                current->updateStatus((Node::StatusFlag)(Node::DRAW | Node::PENDING | Node::US));
                prevNode->updateStatus((Node::StatusFlag)
                                      (prevNode->getStatus() | Node::PENDING));
                prevWhiteNode->updateStatus((Node::StatusFlag)
                                      (prevWhiteNode->getStatus() | Node::PENDING));
#if 0
                //Remove both black and white prev pos
                //FIXME : right now we assume no node are currently looking up for
                //prevwhite node
                const Node *prevNode = current->getParents().back();
                string prevPos = prevNode->getPos();
                if (prevPos.length() > 8
                    && oracle->remove(HashTable::hashFEN(prevPos)) == 0)
                        continue;//Means that some other explorer took care of it
                const Node *prevWhiteNode = prevNode->getParents().back();
                //We don't need the black position anymore, and it won't be deleted
                //by any
                delete prevNode;
                pos.set(prevWhiteNode->getPos());
                string prevSignature = pos.signature();
                uint64_t prevHash = pos.hash();

                /*Lookup in signature tables*/
                if (prevSignature.length() <= opt.getMaxPiecesEnding()) {
                    if (tables[prevSignature]->remove(prevHash) > 0)
                        Out::output("Removed previous white pos from signature"
                                    " table (" + prevWhiteNode->getPos() + ")\n");
                }
                //Lookup in oracle
                if (oracle->remove(prevHash) > 0)
                    Out::output("Removed previous white pos from Oracle\n");
                //FIXME Voluntarily cast away the const qualifier, need some thinking
                const_cast<Node *>(prevWhiteNode)->reset();
                nodes.push(const_cast<Node *>(prevWhiteNode));
#endif
                continue;
#if 0
                Out::output(iterationOutput, "Error, Trying to clear table and "
                                             "ucinewgame on previous pos...\n");
                const Node *tmp = current->getParents().back()->getParents().back();
                Out::output(iterationOutput, "Trying prev fen : " + tmp->getPos());
                string cmdPos = "position fen " + tmp->getPos();
                pool.send(commId, cmdPos);
                pool.clearHash(commId);
                pool.sendAndWaitBestmove(commId, cmd);
                Out::output(iterationOutput, "Result lines :\n");
                Out::output(iterationOutput, Utils::getPrettyLines(pos, lines), 2);

                tmp = tmp->getParents().back()->getParents().back();
                Out::output(iterationOutput, "Trying prev fen : " + tmp->getPos());
                cmdPos = "position fen " + tmp->getPos();
                pool.send(commId, cmdPos);
                pool.sendAndWaitBestmove(commId, cmd);
                Out::output(iterationOutput, "Result lines :\n");
                Out::output(iterationOutput, Utils::getPrettyLines(pos, lines), 2);

                tmp = tmp->getParents().back()->getParents().back();
                Out::output(iterationOutput, "Trying prev fen : " + tmp->getPos());
                cmdPos = "position fen " + tmp->getPos();
                pool.send(commId, cmdPos);
                pool.sendAndWaitBestmove(commId, cmd);
                Out::output(iterationOutput, "Result lines :\n");
                Out::output(iterationOutput, Utils::getPrettyLines(pos, lines), 2);

                Out::output("Iteration output for error :\n" + iterationOutput);
                OracleBuilder::displayNodeHistory(current);
                Err::handle("A node has gone from draw to mate, this is an error"
                            " until we decide on what to do, and if it's a bug"
                            " in the engine.");
#endif
            } else {
                current->updateStatus((Node::StatusFlag)(Node::MATE | Node::US));
            }
            skipThisNode = true;
        } else if (fabs(bestLine.getEval()) > opt.getCutoffThreshold()) {
            Out::output(iterationOutput, "[" + color_to_string(active)
                        + "] Bestline is above threshold (cut)\n", 2);
            if (bestLine.getEval() < 0) {
                current->updateStatus((Node::StatusFlag)(Node::THRESHOLD | Node::THEM));
                Err::output("A node went from draw to loss, clearing table and"
                            " backtrackin");
                pool.clearHash(commId);
                Node *prevNode = const_cast<Node *>(current->getParents().back());
                Node *prevWhiteNode = const_cast<Node *>(prevNode->getParents().back());
                current->updateStatus((Node::StatusFlag)(Node::DRAW | Node::PENDING | Node::US));
                prevNode->updateStatus((Node::StatusFlag)
                                      (prevNode->getStatus() | Node::PENDING));
                prevWhiteNode->updateStatus((Node::StatusFlag)
                                      (prevWhiteNode->getStatus() | Node::PENDING));
#if 0
                //Remove both black and white prev pos
                //FIXME : right now we assume no node are currently looking up for
                //prevwhite node
                const Node *prevNode = current->getParents().back();
                string prevPos = prevNode->getPos();
                if (prevPos.length() > 8
                    && oracle->remove(HashTable::hashFEN(prevPos)) == 0)
                        continue;//Means that some other explorer took care of it
                const Node *prevWhiteNode = prevNode->getParents().back();
                //We don't need the black position anymore, and it won't be deleted
                //by any
                delete prevNode;
                pos.set(prevWhiteNode->getPos());
                string prevSignature = pos.signature();
                uint64_t prevHash = pos.hash();

                /*Lookup in signature tables*/
                if (prevSignature.length() <= opt.getMaxPiecesEnding()) {
                    if (tables[prevSignature]->remove(prevHash) > 0)
                        Out::output(iterationOutput, "Removed previous white pos from signature"
                                    " table (" + prevWhiteNode->getPos() + ")\n");
                }
                //Lookup in oracle
                if (oracle->remove(prevHash) > 0)
                    Out::output(iterationOutput, "Removed previous white pos from Oracle\n");
                //FIXME Voluntarily cast away the const qualifier, need some thinking
                const_cast<Node *>(prevWhiteNode)->reset();
                nodes.push(const_cast<Node *>(prevWhiteNode));
#endif
                continue;
#if 0
                Out::output(iterationOutput, "Error, Trying to clear table and "
                                             "ucinewgame on previous pos...\n");
                const Node *tmp = current->getParents().back()->getParents().back();
                Out::output(iterationOutput, "Trying prev fen : " + tmp->getPos());
                string cmdPos = "position fen " + tmp->getPos();
                pool.send(commId, cmdPos);
                pool.clearHash(commId);
                pool.sendAndWaitBestmove(commId, cmd);
                Out::output(iterationOutput, "Result lines :\n");
                Out::output(iterationOutput, Utils::getPrettyLines(pos, lines), 2);

                tmp = tmp->getParents().back()->getParents().back();
                Out::output(iterationOutput, "Trying prev fen : " + tmp->getPos());
                cmdPos = "position fen " + tmp->getPos();
                pool.send(commId, cmdPos);
                pool.sendAndWaitBestmove(commId, cmd);
                Out::output(iterationOutput, "Result lines :\n");
                Out::output(iterationOutput, Utils::getPrettyLines(pos, lines), 2);

                tmp = tmp->getParents().back()->getParents().back();
                Out::output(iterationOutput, "Trying prev fen : " + tmp->getPos());
                cmdPos = "position fen " + tmp->getPos();
                pool.send(commId, cmdPos);
                pool.sendAndWaitBestmove(commId, cmd);
                Out::output(iterationOutput, "Result lines :\n");
                Out::output(iterationOutput, Utils::getPrettyLines(pos, lines), 2);

#endif
                Out::output("Iteration output for error :\n" + iterationOutput);
                OracleBuilder::displayNodeHistory(current);
                Err::handle("A node has gone from draw to threshold, this is an error"
                            " until we decide on what to do, and if it's a bug"
                            " in the engine.");
            } else {
                current->updateStatus((Node::StatusFlag)(Node::THRESHOLD | Node::US));
            }
            skipThisNode = true;
        }

        /*Trick to avoid code duplicaton for inserting copy in table*/
        if (!skipThisNode) {
            current->updateStatus(Node::DRAW);
            /* If we are not in fullBuild mode, just insert a pending node in
             * table if the signature is low enough
             */
            if (!opt.fullBuild() &&
                signature.length() <= opt.getMaxPiecesEnding()) {
                if (oracle->findOrInsert(curHash, current) != current)
                    delete current;
                else
                    current->updateStatus((Node::StatusFlag)
                                          (current->getStatus() | Node::PENDING));
                continue;
            }
        }

        if (insertCopyInSignTable) {
            Node *cpy = current->lightCopy();
            if (tables[signature]->findOrInsert(curHash, cpy) != cpy)
                delete cpy;
        }

        if (skipThisNode)
            continue;

        /*If we are here, bestLine is "draw", and we should continue to explore*/
        vector<Line> playableLines;

        /*Select all the candidate lines (bestmove +- deviation)*/
        for (Line l : lines) {
            if (!(l.empty() || l.isMat())
                && (fabs(bestLine.getEval() - l.getEval())
                   <= opt.getBestmoveDeviation()
                   || l.getEval() >= -1))
                playableLines.push_back(l);
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
        for (auto rit = playableLines.rbegin();
             rit != playableLines.rend(); ++rit) {
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
            next = oracle->findVal(hashpos);
            if (next) {
                next->safeAddParent(current);
                break;
            }
        }

        /*No repetition found, sort the playable lines*/
        if (!next) {
            std::sort(playableLines.begin(), playableLines.end(),
                         [&pos](const Line &lhs, const Line &rhs)
                         {
                             return pos.compareLines(lhs, rhs);
                         });
            l = playableLines[0];
            mv = l.firstMove();
            pos.tryAndApplyMove(mv);
            string fenpos = pos.fen();
            pos.undoLastMove();

            //no next position in the table, push the node to stack
            next = new Node(current, fenpos, Node::PENDING);
            Out::output(iterationOutput, "[" + color_to_string(active)
                        + "] Pushed first line (" + mv + ") : " + fenpos + "\n", 2);
            nodes.push(next);
        }

        /*Whatever the move is, add it to our move list*/
        MoveNode move(mv, next);
        current->safeAddMove(move);
        Out::output(iterationOutput, "-----------------------\n", 1);
        /*Send the whole iteration output*/
        Out::output(iterationOutput);
    }
}

int OracleFinder::runFinderOnPosition(const Position &p,
                                      const list<string> &moves)
{
    /*Get the side from option*/
    playFor_ = (opt_.buildOracleForWhite()) ? WHITE : BLACK;


    return OracleBuilder::buildOracle(playFor_, oracleTables_, commIds_, p, moves);
}
