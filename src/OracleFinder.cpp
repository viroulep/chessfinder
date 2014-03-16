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
#include "OracleFinder.h"
#include "MatfinderOptions.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"
#include "Hashing.h"
#include "CompareMove.h"



OracleFinder::OracleFinder() : Finder()
{
    //engine_side_ = cb_->getActiveSide();
    //engine_play_for_ = MatfinderOptions::getPlayFor();
    string inputFilename = MatfinderOptions::getInputFile();
    if (inputFilename.size() > 0) {
        Utils::output("Loading table from " + inputFilename + ".\n", 2);
        ifstream inputFile(inputFilename, ios::binary);
        if (!inputFile.good())
            Utils::handleError("Unable to load table from file "
                    + inputFilename);
        oracleTable_ = HashTable::fromPolyglot(inputFile);
    } else {
        Utils::output("Creating new empty table.\n", 2);
        oracleTable_ = new HashTable();
    }
}

OracleFinder::~OracleFinder()
{
    string outputFilename = MatfinderOptions::getOutputFile();
    if (outputFilename.size() > 0) {
        Utils::output("Saving table to " + outputFilename + ".\n", 2);
        ofstream outputFile(outputFilename, ios::binary);
        if (!outputFile.good())
            Utils::output("Unable to save table to file "
                    + outputFilename + "\n");
        else
            oracleTable_->toPolyglot(outputFile);
    }
    delete oracleTable_;
}

/* Return 1 if the difference is negative, otherwise 0.  */
int timeval_subtract(struct timeval *result, struct timeval *t2, struct timeval *t1)
{
    long int diff = (t2->tv_usec + 1000000 * t2->tv_sec) - (t1->tv_usec + 1000000 * t1->tv_sec);
    result->tv_sec = diff / 1000000;
    result->tv_usec = diff % 1000000;

    return (diff<0);
}

void timeval_print(struct timeval *tv)
{
    char buffer[30];
    time_t curtime;

    printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}

int OracleFinder::runFinderOnCurrentPosition()
{
    int maxMoves = 254;
    engine_play_for_ = cb_->getActiveSide();

    Utils::output("Updating MultiPV to " + to_string(maxMoves) + "\n", 2);
    sendOptionToEngine("MultiPV", to_string(maxMoves));
    sendToEngine("isready");
    waitReadyok();

    Utils::output("Starting board is :\n" + cb_->to_string() + "\n");
/*
 *    Utils::output("Doing some basic evaluation on submitted position...\n");
 *
 *    sendCurrentPositionToEngine();
 *    lines_.assign(maxMoves, Line::emptyLine);
 *    sendToEngine("go movetime "
 *            + to_string(MatfinderOptions::getPlayforMovetime()));
 *    waitBestmove();
 *    Utils::output("Evaluation is :\n");
 *    Utils::output(getPrettyLines());
 *    lines_.clear();
 */


    Node *init = new Node();
    Node *rootNode_ = init;
    init->pos = cb_->exportToFEN();
    //depth-first
    toProceed_.push_front(rootNode_);

    //Main loop
    while (toProceed_.size() != 0) {
        Node *current = toProceed_.front();
        toProceed_.pop_front();
        /*Check we are not computing an already existing position*/
        if (oracleTable_->findPos(current->pos)) {
            Utils::output("Position already in table.\n", 1);
            /*TODO think of this*/
            /*delete current;*/
            continue;
        }
        if (cutNode(current)) {
            Utils::output("Node cut by user-defined function", 1);
            continue;
        }

        Line bestLine;
        /*Set the chessboard to current pos*/
        cb_->reInitFromFEN(current->pos);
        Side active = cb_->getActiveSide();

        Utils::output("[" + Board::to_string(active)
                + "] Proceed size : " + to_string(toProceed_.size()) + "\n");

        /*Register current pos in the table*/
        uint64_t curHash = HashTable::hashBoard(cb_);
        pair<uint64_t, Node *> p(curHash, current);
        oracleTable_->insert(p);

        /*Clear cut*/
        if (!cb_->sufficientMaterial()) {
            current->st = Node::DRAW;
            Utils::output("[" + Board::to_string(active)
                    + "] Insuficient material.\n", 2);
            //Proceed to next node...
            continue;
        }


        Utils::output(cb_->to_string(), 2);

        sendCurrentPositionToEngine();
        /*Initialize vector with empty lines*/
        lines_.assign(maxMoves, Line());

        if (engine_play_for_ != active) {
            current->st = Node::AGAINST;
            sendToEngine("go depth 1");
            waitBestmove();
            pushAllLines(current);
            continue;
        }

        /**********************************************/
        /*Here we are on a node with "playfor" to play*/
        /**********************************************/

        /*Thinking according to the side the engine play for*/
        int moveTime = MatfinderOptions::getPlayforMovetime();

        sendToEngine("go movetime " + to_string(moveTime));

        Utils::output("[" + Board::to_string(active)
                + "] Thinking... (" + to_string(moveTime) + ")\n", 1);

        //Wait for engine to finish thinking
        waitBestmove();
        Utils::output(getPrettyLines(), 2);

        bestLine = lines_[0];
        if (bestLine.empty()) {
            //STALEMATE
            current->st = Node::STALEMATE;
            Utils::output("[" + Board::to_string(active)
                    + "] Bestline is stalemate (cut)\n", 2);
            //Proceed to next node...
            continue;
        } else if (bestLine.isMat()) {
            //TODO: register if winning or losing ?
            current->st = Node::MATE;
            Utils::output("[" + Board::to_string(active)
                    + "] Bestline is mate (cut)\n", 2);
            continue;
        } else if (fabs(bestLine.getEval()) > MatfinderOptions::getCpTreshold()) {
            //TODO: register if winning or losing ?
            current->st = Node::TRESHOLD;
            Utils::output("[" + Board::to_string(active)
                    + "] Bestline is above treshold (cut)\n", 2);
            continue;
        }

        /*If we are here, bestLine is draw, and we should continue to explore*/
        /*TODO refactor this with two param ref to vector "draw" and "cut"*/
        SortedLines all = getLines();
        current->st = Node::DRAW;

        /*
         *Add all the imbalanced line to the hashtable (either mat or "treshold")
         *(save some iterations in main loop : we could also push all the
         *unbalanced lines and see...)
         */
        proceedUnbalancedLines(all[1]);

        vector<Line *> balancedLines = all[0];

        //Then proceed the balanced lines according to the side the engine
        //play for.
        if (engine_play_for_ != active)
            Utils::handleError("Current side should be the engine plays for");

        /*
         * General Idea : try to force repetition by trying to find a playable
         * position in the hashtable. If not sort the playable moves
         * according to a user defined comparator
         */
        Node *next = NULL;
        Line *l = NULL;
        Board::UCIMove mv;
        SimplePos sp;
        /*Try to find a position in the table*/
        for (int i = balancedLines.size() - 1; i >= 0; --i) {
            l = balancedLines[i];
            /*l is not null (or bug)*/
            mv = l->firstMove();
            cb_->uciApplyMove(mv);
            /*This is the next pos*/
            sp = cb_->exportToFEN();
            cb_->undoMove();
            //Jean Louis' idea to force finding positions in oracle
            next = oracleTable_->findPos(sp);
            if (next)
                break;
        }
        /*FIXME: what if positive eval ?*/
        /*Maybe just continue, but then we should register that mate or
         * treshold is winning or losing*/
        if (!next) {
            auto compareFn = std::bind(&Chessboard::compareLines, cb_,
                    std::placeholders::_1, std::placeholders::_2);
            std::sort(balancedLines.begin(), balancedLines.end(),
                    compareFn);
            l = balancedLines[0];
            mv = l->firstMove();
            cb_->uciApplyMove(mv);
            sp = cb_->exportToFEN();
            cb_->undoMove();

            //no next position in the table, push the node to stack
            next = new Node();
            next->pos = sp;
            Utils::output("[" + Board::to_string(active)
                    + "] Pushed first line (" + mv + ") : " + sp + "\n", 2);
            toProceed_.push_front(next);
        }
        /*Whatever the move is, add it to our move list*/
        MoveNode move(mv, next);
        current->legal_moves.push_back(move);
        Utils::output("-----------------------\n", 1);

    }

    /*
     *TODO we still need to do some cleaning in the table :
     *    - if all moves after a draw are mate or tresh, then this line is not
     *    a draw and should be updated.
     *    - what to do with treshold/mate node ? Matfinder has to close
     *    these lines
     */

    //Display info at the end of computation
    Utils::output("[End] Finder is done. Starting board was : \n");
    cb_->reInitFromFEN(init->pos);
    Utils::output(cb_->to_string() + "\n");


    Utils::output("Hashtable size = "
            + std::to_string(oracleTable_->size()) + ") : \n");
    Utils::output(oracleTable_->to_string() + "\n", 2);
    Utils::output("(size = " + std::to_string(oracleTable_->size()) + ") : \n", 2);


    return 0;
}




SortedLines OracleFinder::getLines()
{
    /*TODO refactor this to take references instead of pointers*/
    vector<Line *> balanced;
    vector<Line *> unbalanced;
    SortedLines retVal;
    for (int i = 0; i < (int) lines_.size(); ++i) {
        Line l = lines_[i];
        if (l.empty())
            continue;
        int limit = MatfinderOptions::getCpTreshold();
        if (l.isMat())
            unbalanced.push_back(&(lines_[i]));
        else if (fabs(l.getEval()) <= limit)
            balanced.push_back(&(lines_[i]));
        else
            unbalanced.push_back(&(lines_[i]));
    }
    retVal[0] = balanced;
    retVal[1] = unbalanced;
    return retVal;
}

Board::LegalMoves OracleFinder::getAllMoves()
{
    LegalMoves list;
    int maxMoves = 254;

    Utils::output("Getting legal moves\n");
    Utils::output("Updating MultiPV to " + to_string(maxMoves) + "\n", 2);
    sendOptionToEngine("MultiPV", to_string(maxMoves));
    sendToEngine("isready");
    waitReadyok();

    sendCurrentPositionToEngine();
    lines_.assign(maxMoves, Line());
    sendToEngine("go depth 1");
    waitBestmove();
    Utils::output("Evaluation is :\n");
    Utils::output(getPrettyLines());
    lines_.clear();
    return list;
}

/*TODO refactor this to "addlines to hashtable"*/
void OracleFinder::proceedUnbalancedLines(vector<Line *> unbalanced)
{
    //Do not check access, assume Line * is legal addr
    for (int i = 0; i < (int) unbalanced.size(); ++i) {
        Line *l = unbalanced[i];
        if (!l)
            Utils::handleError("An unbalanced line is null ("
                    + to_string(i) + ")");
        Node::Status s = Node::TRESHOLD;
        if (l->isMat())
            s = Node::MATE;
        Node *toAdd = new Node();
        vector<MoveNode> moves;
        UCIMove next = l->firstMove();
        toAdd->legal_moves = moves;
        cb_->uciApplyMove(next);
        toAdd->pos = cb_->exportToFEN();
        uint64_t hash = HashTable::hashBoard(cb_);
        cb_->undoMove();
        toAdd->st = s;
        pair<uint64_t, Node *> p(hash, toAdd);
        oracleTable_->insert(p);
    }
}

void OracleFinder::pushAllLines(Node *currentNode)
{
    /*
     * This function is to be called on "opposite" node, since we need
     * to push all node for the side we "play for".
     * eg: if we are building an oracle for white, we need to push all
     * possible white positions when computing a black node.
     */
    Utils::output("Push all lines : ", 2);
    for (Line l : lines_) {
        /*Stop at the first empty line*/
        if (l.empty())
            break;
        Utils::output("+", 2);
        Board::UCIMove mv = l.firstMove();
        cb_->uciApplyMove(mv);
        SimplePos sp = cb_->exportToFEN();
        cb_->undoMove();
        Node *next = new Node();
        next->pos = sp;
        toProceed_.push_front(next);
        MoveNode move(mv, next);
        currentNode->legal_moves.push_back(move);
    }
    Utils::output("\n", 2);
}

bool OracleFinder::cutNode(Node *)
{
    /*TODO evaluate if we should process this node or not, according to
     * the chessboard state.*/
    return false;
}
