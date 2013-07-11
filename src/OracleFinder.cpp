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
#include <unistd.h>
#include "Finder.h"
#include "OracleFinder.h"
#include "Options.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"
#include "Hashing.h"

string to_string(Status s)
{
    switch (s) {
        case MATE:
            return "mate";
        case STALEMATE:
            return "stalemate";
        case DRAW:
            return "draw";
        case TRESHOLD:
            return "treshold";
    }
}

string to_string(Node &n)
{
    string retVal;
    //TODO: display moves ?
    retVal += "(" + to_string(n.st) + "," + n.pos + ")";
    return retVal;
}

string to_string(HashTable &ht)
{
    string retVal;
    for (HashTable::iterator it = ht.begin(), itEnd = ht.end();
            it != itEnd; ++it) {
        Node n = *(it->second);
        retVal += "#" + to_string(it->first) + ":" + to_string(n) + "\n";
    }
    if (ht.size() == 0)
        retVal += "<empty>";
    return retVal;
}

void clearAndFree(HashTable &ht)
{
    for (HashTable::iterator it = ht.begin(), itEnd = ht.end();
            it != itEnd; ++it) {
        delete(it->second);
    }
    ht.clear();
}

Node *find(SimplePos sp, HashTable &table)
{
    uint64_t hash = Hashing::hashFEN(sp);
    auto range = table.equal_range(hash);
    Node *found = NULL;
    queue<string> infos;
    stringstream ss(sp);
    string tmpInfo;
    while (getline(ss, tmpInfo, ' '))
        infos.push(tmpInfo);
    string simpleFen = infos.front();
    infos.pop();
    simpleFen += " " + infos.front();
    Utils::output("Looking for " + simpleFen + "\n", 3);
    for (auto it = range.first, itEnd = range.second;
            it != itEnd && !found ; ++it) {
        Utils::output("against " + it->second->pos + "\n", 3);
        if (simpleFen == it->second->pos.substr(0, simpleFen.size()))
            found = it->second;
    }
    return found;
}

OracleFinder::OracleFinder() : Finder()
{
    //engine_side_ = cb_->getActiveSide();
    //engine_play_for_ = Options::getPlayFor();
}

OracleFinder::~OracleFinder()
{
    clearAndFree(oracleTable_);
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
    Utils::output("Doing some basic evaluation on submitted position...\n");

    sendCurrentPositionToEngine();
    lines_.assign(maxMoves, Line::emptyLine);
    sendToEngine("go movetime "
            + to_string(Options::getPlayforMovetime()));
    waitBestmove();
    Utils::output("Evaluation is :\n");
    Utils::output(getPrettyLines());
    lines_.clear();

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
        if (find(current->pos, oracleTable_))
            continue;

        Line bestLine;
        //Set the chessboard to current pos
        cb_->reInitFromFEN(current->pos);
        Side active = cb_->getActiveSide();

        //Register current pos in the table
        //(we shouldn't be visiting the same position twice)
        uint64_t curHash = Hashing::hashBoard(cb_);
        pair<uint64_t, Node *> p(curHash, current);
        oracleTable_.insert(p);

        Utils::output("[" + Board::to_string(active)
                + "] Proceed size : " + to_string(toProceed_.size()) + "\n");

        sendCurrentPositionToEngine();

        Utils::output(cb_->to_string(), 2);

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
            current->st = STALEMATE;
            Utils::output("[" + Board::to_string(active)
                    + "] Bestline is stalemate (cut)\n", 2);
            //Proceed to next node...
            continue;
        } else if (bestLine.isMat()) {
            //TODO: register if winning or losing ?
            current->st = MATE;
            Utils::output("[" + Board::to_string(active)
                    + "] Bestline is mate (cut)\n", 2);
            continue;
        } else if (fabs(bestLine.getEval()) > Options::getCpTreshold()) {
            //TODO: register if winning or losing ?
            current->st = TRESHOLD;
            Utils::output("[" + Board::to_string(active)
                    + "] Bestline is above treshold (cut)\n", 2);
            continue;
        }
        //If we are here, bestLine is draw, and we should continue to explore
        SortedLines all = getLines();
        current->st = DRAW;

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
            //Here balancedLines shouldn't be empty, if so it's a bug
            //(Go reverse to have the bestLine at last)
            for (int i = balancedLines.size() - 1; i >= 0; --i) {
                l = balancedLines[i];
                //l is not null (or bug)
                mv = l->firstMove();
                cb_->uciApplyMove(mv);
                //This is the next pose
                sp = cb_->exportToFEN();
                cb_->undoMove();
                //Jean Louis' idea to force finding positions in oracle
#if 1
                next = find(sp, oracleTable_);
                if (next)
                    break;
#endif
            }
            //TODO: what if positive eval ?
            if (!next) {
                //no next position in the table, push the node to stack
                next = new Node();
                next->pos = sp;
                Utils::output("[" + Board::to_string(active)
                        + "] Pushed first line : " + sp + "\n", 2);
#if 0
                //can also make opposite positions prior on 'ours'
                //depth first
                toProceed_.push_front(next);
#else
                //Breads first
                toProceed_.push_back(next);
#endif
            } else
                Utils::output("[" + Board::to_string(active)
                        + "] Found a line in table !\n", 2);
            //Whatever the move goes, add it to our move list
            MoveNode move(mv, next);
            current->legal_moves.push_back(move);

        } else {
            //get all draw lines and push them

            for (int i = 0; i < balancedLines.size(); ++i) {
                Line *l = balancedLines[i];
                Board::UCIMove mv = l->firstMove();
                cb_->uciApplyMove(mv);
                SimplePos sp = cb_->exportToFEN();
                cb_->undoMove();
                Node *next = find(sp, oracleTable_);
                if (!next) {
                    //Pos is not in the table, push the node to stack
                    Utils::output("[" + Board::to_string(active)
                            + "] Pushed a draw (" + to_string(l->getEval())
                            + ") line : " + sp + "\n", 2);
                    next = new Node();
                    next->pos = sp;
#if 1
                    //can also make our positions prior on 'opposite'
                    //depth first
                    toProceed_.push_front(next);
#else
                    //Breads first
                    toProceed_.push_back(next);
#endif
                } else
                    Utils::output("[" + Board::to_string(active)
                            + "] Found a line in table !\n", 2);
                MoveNode move(mv, next);
                current->legal_moves.push_back(move);
            }
        }

    }

    //Display info at the end of computation
    Utils::output("[End] Finder is done. Starting board was : \n");
    Utils::output(cb_->to_string() + "\n");


    Utils::output("Hashtable is (size = " 
            + std::to_string(oracleTable_.size()) + ") : \n");
    Utils::output(to_string(oracleTable_) + "\n");


    return 0;
}


SortedLines OracleFinder::getLines()
{
    vector<Line *> balanced;
    vector<Line *> unbalanced;
    SortedLines retVal;
    for (int i = 0; i < lines_.size(); ++i) {
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

void OracleFinder::proceedUnbalancedLines(vector<Line *> unbalanced)
{
    //Do not check access, assume Line * is legal addr
    for (int i = 0; i < unbalanced.size(); ++i) {
        Line *l = unbalanced[i];
        if (!l)
            Utils::handleError("An unbalanced line is null ("
                    + to_string(i) + ")");
        Status s = TRESHOLD;
        if (l->isMat())
            s = MATE;
        Node *toAdd = new Node();
        vector<MoveNode> moves;
        UCIMove next = l->firstMove();
        toAdd->legal_moves = moves;
        cb_->uciApplyMove(next);
        toAdd->pos = cb_->exportToFEN();
        uint64_t hash = Hashing::hashBoard(cb_);
        cb_->undoMove();
        toAdd->st = s;
        pair<uint64_t, Node *> p(hash, toAdd);
        oracleTable_.insert(p);
    }
}
