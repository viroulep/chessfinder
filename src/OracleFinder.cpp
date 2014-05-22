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


std::map<std::string, int> OracleFinder::signStat_;

OracleFinder::OracleFinder(int comm) : Finder(comm)
{
    //engine_side_ = cb_->getActiveSide();
    //engine_play_for_ = MatfinderOptions::getPlayFor();
    string inputFilename = opt_.getInputFile();
    if (inputFilename.size() > 0) {
        Out::output("Loading table from " + inputFilename + ".\n", 2);
        ifstream inputFile(inputFilename, ios::binary);
        if (!inputFile.good())
            Err::handle("Unable to load table from file "
                        + inputFilename);
        oracleTable_ = HashTable::fromPolyglot(inputFile);
    } else {
        Out::output("Creating new empty table.\n", 2);
        oracleTable_ = new HashTable();
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
            oracleTable_->toPolyglot(outputFile);
    }
    delete oracleTable_;
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

int OracleFinder::runFinderOnPosition(const Position &p,
                                      const list<string> &moves)
{
    Position pos;
    pos.set(p.fen());

    /*Get the side from option*/
    playFor_ = (opt_.buildOracleForWhite()) ? WHITE : BLACK;

    for (string mv : moves) {
        pos.tryAndApplyMove(mv);
    }
    string startingFen = pos.fen();
    pos.clear();
    pos.set(startingFen);

    pool_.sendOption(commId_, "MultiPV", to_string(opt_.getMaxMoves()));

    Out::output("Building an oracle for : " + color_to_string(playFor_) + "\n");
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


    Node *init = new Node(nullptr);
    Node *rootNode_ = init;
    init->pos = pos.fen();
    string initFen = init->pos;
    //depth-first
    toProceed_.push_front(rootNode_);

    //Main loop
    while (toProceed_.size() != 0) {
        Node *current = toProceed_.front();
        toProceed_.pop_front();
        /*Check we are not computing an already existing position*/
        /*FIXME here we should "findorinsert" to be sure to avoid duplicate processing*/
        if (oracleTable_->findPos(current->pos)) {
            Out::output("Position already in table.\n", 1);
            delete current;
            continue;
        }
        if (cutNode(pos, current)) {
            Out::output("Node cut by user-defined function", 1);
            continue;
        }

        Line bestLine;
        /*Set the chessboard to current pos*/
        pos.set(current->pos);
        Color active = pos.side_to_move();

        Out::output("[" + color_to_string(active) + "] Proceed size : "
                    + to_string(toProceed_.size()) + "\n");

        /*Register current pos in the table*/
        uint64_t curHash = pos.hash();
        pair<uint64_t, Node *> p(curHash, current);
        oracleTable_->insert(p);

        /*Clear cut*/
        if (!pos.hasSufficientMaterial()) {
            current->st = Node::DRAW;
            Out::output("[" + color_to_string(active)
                    + "] Insuficient material.\n", 2);
            //Proceed to next node...
            continue;
        }


        Out::output(pos.pretty(), 2);
        string signature = pos.signature();
        int hit = signStat_[signature];
        signStat_[signature] = ++hit;
        sendPositionToEngine(pos);

        if (playFor_ != active) {
            /*We are on a node where the opponent has to play*/
            current->st = Node::AGAINST;
            proceedAgainstNode(pos, current);
            continue;
        }

        /**********************************************/
        /*Here we are on a node with "playfor" to play*/
        /**********************************************/

        const vector<Line> &lines = pool_.getResultLines(commId_);

        /*Thinking according to the side the engine play for*/
        int moveTime = opt_.getPlayforMovetime();

        Out::output("[" + color_to_string(active) + "] Thinking... ("
                    + to_string(moveTime) + ")\n", 1);

        string cmd = "go ";
        switch (opt_.getSearchMode()) {
            case DEPTH:
                cmd += "depth " + to_string(opt_.getSearchDepth());
                break;
            case MIXED://Intentional no-break TODO: implement mixed search ?
            case TIME:
            default:
                cmd += "movetime " + to_string(moveTime);
                break;
        }
        //Send go and wait for engine to finish thinking
        pool_.sendAndWaitBestmove(commId_, cmd);


        Out::output(getPrettyLines(pos, lines), 2);

        bestLine = lines[0];
        if (bestLine.empty()) {
            //STALEMATE
            current->st = Node::STALEMATE;
            Out::output("[" + color_to_string(active)
                        + "] Bestline is stalemate (cut)\n", 2);
            //Proceed to next node...
            continue;
        } else if (bestLine.isMat()) {
            current->st = Node::MATE_US;
            Out::output("[" + color_to_string(active)
                        + "] Bestline is mate (cut)\n", 2);
            /*Eval is always negative if it's bad for us*/
            if (bestLine.getEval() < 0) {
                current->st = Node::MATE_THEM;
                displayNodeHistory(current);
                Err::handle("A node has gone from draw to mate, this is an error"
                            " until we decide on what to do, and if it's a bug"
                            " in the engine.");
            }
            continue;
        } else if (fabs(bestLine.getEval()) > opt_.getCutoffTreshold()) {
            current->st = Node::TRESHOLD_US;
            Out::output("[" + color_to_string(active)
                        + "] Bestline is above treshold (cut)\n", 2);
            if (bestLine.getEval() < 0) {
                current->st = Node::TRESHOLD_THEM;
                displayNodeHistory(current);
                Err::handle("A node has gone from draw to threshold, this is an error"
                            " until we decide on what to do, and if it's a bug"
                            " in the engine.");
            }
            continue;
        }

        /*If we are here, bestLine is draw, and we should continue to explore*/
        vector<Line> draw;
        vector<Line> cut;

        getLines(lines, draw, cut);

        current->st = Node::DRAW;

        /*
         *Add all the imbalanced line to the hashtable (either mat or "treshold")
         *(save some iterations in main loop : we could also push all the
         *unbalanced lines and see...)
         */
        proceedUnbalancedLines(pos, current, cut);



        /*
         *Then proceed the balanced lines according to the side the engine
         *play for.
         */
        if (playFor_ != active)
            Err::handle("Current side should be the engine plays for");

        /*
         * General Idea : try to force repetition by trying to find a playable
         * position in the hashtable. If not sort the playable moves
         * according to a user defined comparator
         */
        Node *next = NULL;
        /*A move*/
        string mv;
        /*A fen*/
        string fenpos;
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
            fenpos = pos.fen();
            pos.undoLastMove();
            //Jean Louis' idea to force finding positions in oracle
            next = oracleTable_->findPos(fenpos);
            if (next) {
                /*FIXME "safeaddparent"*/
                next->addParent(current);
                break;
            }
        }
        /*FIXME: what if positive eval ?*/
        /*Maybe just continue, but then we should register that mate or
         * treshold is winning or losing*/
        if (!next) {
            std::sort(draw.begin(), draw.end(),
                         [&pos](const Line &lhs, const Line &rhs)
                         {
                             return pos.compareLines(lhs, rhs);
                         });
            l = draw[0];
            mv = l.firstMove();
            pos.tryAndApplyMove(mv);
            fenpos = pos.fen();
            pos.undoLastMove();

            //no next position in the table, push the node to stack
            next = new Node(current);
            next->pos = fenpos;
            Out::output("[" + color_to_string(active)
                    + "] Pushed first line (" + mv + ") : " + fenpos + "\n", 2);
            toProceed_.push_front(next);
        }
        /*Whatever the move is, add it to our move list*/
        MoveNode move(mv, next);
        current->legal_moves.push_back(move);
        Out::output("-----------------------\n", 1);

    }

    /*
     *TODO we still need to do some cleaning in the table :
     *    - if all moves after a draw are mate or tresh, then this line is not
     *    a draw and should be updated.
     *    - what to do with treshold/mate node ? Matfinder has to close
     *    these lines
     */
    //Display info at the end of computation
    Out::output("[End] Finder is done. Starting board was : \n");
    pos.set(initFen);
    Out::output(pos.pretty() + "\n");


    Out::output("Hashtable size = "
            + std::to_string(oracleTable_->size()) + ") : \n");
    Out::output(oracleTable_->to_string() + "\n", 2);
    Out::output("(size = " + std::to_string(oracleTable_->size()) + ") : \n", 2);

    return 0;
}




void OracleFinder::getLines(const vector<Line> all, vector<Line> &balanced,
                            vector<Line> &unbalanced)
{
    for (Line l : all) {
        /*TODO check if break would work*/
        if (l.empty())
            continue;
        int limit = opt_.getCutoffTreshold();
        if (l.isMat())
            unbalanced.push_back(l);
        else if (fabs(l.getEval()) <= limit)
            balanced.push_back(l);
        else
            unbalanced.push_back(l);
    }
}

/*FIXME re-inline this in worker*/
void OracleFinder::proceedAgainstNode(Position &pos, Node *againstNode)
{
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
        Node *next = new Node(againstNode);
        next->pos = fen;
        /*FIXME safeinsert*/
        toProceed_.push_front(next);
        MoveNode move(uciMv, next);
        againstNode->legal_moves.push_back(move);
    }
    Out::output("\n", 2);
}

/*TODO refactor this to "addlines to hashtable"*/
void OracleFinder::proceedUnbalancedLines(Position &pos, const Node *cur,
                                          vector<Line> &unbalanced)
{
    for (Line l : unbalanced) {
        Node::Status s = Node::TRESHOLD_THEM;
        if (l.isMat())
            s = Node::MATE_THEM;
        /*FIXME there is likely a memory leak here, if position already in table*/
        Node *toAdd = new Node(cur);
        vector<MoveNode> moves;
        string next = l.firstMove();
        toAdd->legal_moves = moves;
        if (!pos.tryAndApplyMove(next))
            Err::handle("Illegal move pushed ! (In an unbalanced line)");
        toAdd->pos = pos.fen();
        uint64_t hash = pos.hash();
        pos.undoLastMove();
        toAdd->st = s;
        pair<uint64_t, Node *> p(hash, toAdd);
        oracleTable_->insert(p);
    }
}

bool OracleFinder::cutNode(const Position &, const Node *)
{
    /*TODO evaluate if we should process this node or not, according to
     * the chessboard state.*/
    return false;
}

void OracleFinder::displayNodeHistory(const Node *start)
{
    const Node *cur = start;
    /*
     * I guess 30 positions are enough to display, since we display this message
     * as soon as we detect an inversion in evaluation.
     */
    int limit = 30;
    int i = 0;
    Out::output("Displaying node history for " + start->pos
                + " (reverse order)\n");
    /*TODO think about what to do if multiple parent*/
    while (cur && i < limit) {
        Out::output(cur->to_string() + "\n");
        cur = cur->getParents().back();
        i++;
    }
}
