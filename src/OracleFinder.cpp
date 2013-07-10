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

    //Main loop
    while (true) {
        Line bestLine;
        Side active = cb_->getActiveSide();

        Utils::output("[" + Board::to_string(active)
                + "] Depth " + to_string(addedMoves_) + "\n");

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

        SortedLines all = getLines();
        //Add all the imbalanced line to the hashtable (either mat or "treshold")
        proceedUnbalancedLines(all[1]);

        vector<Line *> balancedLines = all[0];

        if (engine_play_for_ == active) {
            //Get all draw lines (if empty = stalemate or mate)
            //check if one in hashtable, if not push the first next pos to proceed

        } else {
            //get all draw lines and push them

        }

        Utils::output(getPrettyLines(), 2);
        break;
    }

    //Display info at the end of computation
    Utils::output("[End] Finder is done. Starting board was : \n");
    Utils::output(cb_->to_string() + "\n");


    Utils::output("Hashtable is : \n");
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
        if (fabs(l.getEval()) <= limit)
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
        toAdd->pos = cb_->getSimplePos();
        uint64_t hash = Hashing::hashBoard(cb_);
        cb_->undoMove();
        toAdd->st = s;
        pair<uint64_t, Node *> p(hash, toAdd);
        oracleTable_.insert(p);
    }
}
