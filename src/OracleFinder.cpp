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
#include "Options.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"
#include "Hashing.h"
#include "CompareMove.h"



OracleFinder::OracleFinder() : Finder()
{
    //engine_side_ = cb_->getActiveSide();
    //engine_play_for_ = Options::getPlayFor();
    string inputFilename = Options::getInputFile();
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
    string outputFilename = Options::getOutputFile();
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
 *            + to_string(Options::getPlayforMovetime()));
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
        //FIXME: this cond is only useful when breads first searching,
        //should be a way to not add the node if in the stack.
        if (oracleTable_->findPos(current->pos)) {
            Utils::output("Position already in table.\n", 2);
            continue;
        }

        Line bestLine;
        //Set the chessboard to current pos
        cb_->reInitFromFEN(current->pos);
        Side active = cb_->getActiveSide();

        //Register current pos in the table
        //(we shouldn't be visiting the same position twice)
        uint64_t curHash = HashTable::hashBoard(cb_);
        pair<uint64_t, Node *> p(curHash, current);
        oracleTable_->insert(p);

        if (!cb_->sufficientMaterial()) {
            current->st = Node::DRAW;
            Utils::output("[" + Board::to_string(active)
                    + "] Insuficient material.\n", 2);
            //Proceed to next node...
            continue;
        }


        Utils::output("[" + Board::to_string(active)
                + "] Proceed size : " + to_string(toProceed_.size()) + "\n");

        Utils::output(cb_->to_string(), 2);

        sendCurrentPositionToEngine();
        //Thinking according to the side the engine play for
        int moveTime = Options::getPlayforMovetime();


        //Initialize vector with empty lines
        lines_.assign(maxMoves, Line::emptyLine);

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
        } else if (fabs(bestLine.getEval()) > Options::getCpTreshold()) {
            //TODO: register if winning or losing ?
            current->st = Node::TRESHOLD;
            Utils::output("[" + Board::to_string(active)
                    + "] Bestline is above treshold (cut)\n", 2);
            continue;
        }

        //If we are here, bestLine is draw, and we should continue to explore
        SortedLines all = getLines();
        current->st = Node::DRAW;

        //Add all the imbalanced line to the hashtable (either mat or "treshold")
        //(save some iterations in main loop : we could also push all the
        //unbalanced lines and see...)
        proceedUnbalancedLines(all[1]);

        vector<Line *> balancedLines = all[0];

        //Then proceed the balanced lines according to the side the engine
        //play for.
        if (engine_play_for_ == active) {
            //check if one in hashtable, if not push the first
            //balanced line to proceed
            Node *next = NULL;
            Line *l = NULL;
            Board::UCIMove mv;
            SimplePos sp;
            /*Try to find a position in the table*/
            for (int i = balancedLines.size() - 1; i >= 0; --i) {
                l = balancedLines[i];
                //l is not null (or bug)
                mv = l->firstMove();
                cb_->uciApplyMove(mv);
                //This is the next pos
                sp = cb_->exportToFEN();
                cb_->undoMove();
                //Jean Louis' idea to force finding positions in oracle
                next = oracleTable_->findPos(sp);
                if (next)
                    break;
            }
            //FIXME: what if positive eval ?
            if (!next) {
                //Sort the balanced line so that we take the shortest
                //Here balancedLines shouldn't be empty, if so it's a bug
                //(Go reverse to have the bestLine at last)
#if 0
                //Retrieve best line or shortest line
                std::sort(balancedLines.begin(), balancedLines.end(),
                        Line::compareLineLength);
                if (balancedLines[0]->getMoves().size() 
                        < l->getMoves().size()) {
                    //Find a shortest line if the "best" line
                    //is not one of the shortest
                    l = balancedLines[0];
                    //l is not null (or bug)
                    mv = l->firstMove();
                    cb_->uciApplyMove(mv);
                    //This is the next pose
                    //TODO: do not remove clock ?
                    sp = cb_->exportToFEN();
                    cb_->undoMove();
                }
#endif
                auto compareFn = std::bind(&Chessboard::compareTake, cb_,
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
                        + "] Pushed first line ("
                        + to_string(l->getMoves().size()) + " moves) : "
                        + sp + "\n", 2);
                toProceed_.push_front(next);
            }
                /*
                 *Utils::output("[" + Board::to_string(active)
                 *        + "] Found a line in table !\n", 2);
                 */
            //Whatever the move is, add it to our move list
            MoveNode move(mv, next);
            current->legal_moves.push_back(move);
        } else {
            //get all draw lines and push them

            Utils::output("Pushed lines : ", 2);
            for (int i = 0; i < (int) balancedLines.size(); ++i) {
                Line *l = balancedLines[i];
                Board::UCIMove mv = l->firstMove();
                cb_->uciApplyMove(mv);
                SimplePos sp = cb_->exportToFEN();
                cb_->undoMove();
                Node *next = oracleTable_->findPos(sp);
                if (!next) {
                    //Pos is not in the table, push the node to stack
                    Utils::output("+", 2);
                    next = new Node();
                    next->pos = sp;
                    toProceed_.push_front(next);
                } else
                    Utils::output("-", 2);
                MoveNode move(mv, next);
                current->legal_moves.push_back(move);
            }
            Utils::output("\n", 2);
        }

    }

    //Display info at the end of computation
    Utils::output("[End] Finder is done. Starting board was : \n");
    Utils::output(cb_->to_string() + "\n");


    Utils::output("Hashtable size = "
            + std::to_string(oracleTable_->size()) + ") : \n");
    Utils::output(oracleTable_->to_string() + "\n", 2);
    Utils::output("(size = " + std::to_string(oracleTable_->size()) + ") : \n", 2);


    return 0;
}




SortedLines OracleFinder::getLines()
{
    vector<Line *> balanced;
    vector<Line *> unbalanced;
    SortedLines retVal;
    for (int i = 0; i < (int) lines_.size(); ++i) {
        Line l = lines_[i];
        if (l.empty())
            continue;
        int limit = Options::getCpTreshold();
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
    lines_.assign(maxMoves, Line::emptyLine);
    sendToEngine("go depth 1");
    waitBestmove();
    Utils::output("Evaluation is :\n");
    Utils::output(getPrettyLines());
    lines_.clear();
    return list;
}

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
