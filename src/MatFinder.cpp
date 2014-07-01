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
#include <unistd.h>
#include "Finder.h"
#include "MatFinder.h"
#include "Utils.h"
#include "Output.h"
#include "UCICommunicator.h"


using namespace std;
using namespace Board;

MatFinder::MatFinder(vector<int> &comm) : Finder(comm)
{
    commId_ = comm.front();
}

MatFinder::~MatFinder()
{
}


int MatFinder::runFinderOnPosition(const Position &p, const list<string> &moves)
{
    Position pos;
    pos.set(p.fen());
    for (string mv : moves) {
        pos.tryAndApplyMove(mv);
    }
    string startingFen = pos.fen();
    pos.clear();
    pos.set(startingFen);

    Color sideToMove = pos.side_to_move();
    /*playFor should be the weaker side*/
    playFor_ = sideToMove;
    Options opt = Options::getInstance();

    Out::output("Starting board is :\n" + pos.pretty() + "\n");
    Out::output("Doing some basic evaluation on submitted position...\n");
    pool_.sendOption(commId_, "MultiPV", "8");

    sendPositionToEngine(pos, commId_);
    string init = "go movetime " + to_string(opt.getPlayforMovetime());
    pool_.sendAndWaitBestmove(commId_, init);

    const vector<Line> &lines = pool_.getResultLines(commId_);

    Out::output("Evaluation is :\n");
    Out::output(Utils::getPrettyLines(pos, lines));
    if (!lines[0].empty()) {
        if ((lines[0].getEval() < 0 && sideToMove == WHITE)
                || (lines[0].getEval() > 0 && sideToMove == BLACK))
            playFor_ = BLACK;
        else
            playFor_ = WHITE;
        playFor_ = (playFor_ == WHITE)? BLACK : WHITE;
    }
    Out::output("Engine will play for : " + color_to_string(playFor_) + "\n");



    //Main loop
    while (true) {
        Color active = pos.side_to_move();
        Line bestLine;

        Out::output("[" + color_to_string(active) + "] Depth "
                    + to_string(addedMoves_) + "\n");

        sendPositionToEngine(pos, commId_);

        Out::output(pos.pretty(), 2);

        //Thinking according to the side the engine play for
        int moveTime = (active == playFor_ || !addedMoves_) ?
                        Options::getInstance().getPlayforMovetime() :
                        Options::getInstance().getPlayagainstMovetime();

        //Compute optimal multipv
        int pv = computeMultiPV(lines);


        //Scaling moveTime
        //According to number of lines
        moveTime = (int)(moveTime * ((float)
                    ((float)pv/(float)Options::getInstance().getMaxLines())
                    ));
        if (moveTime <= 600)
            moveTime = 600;
        //Acccording to depth
        moveTime += 10 * addedMoves_;

        //Increase movetime with depth
        pool_.sendAndWaitBestmove(commId_,
                                  "go movetime " + to_string(moveTime));

        Out::output("[" + color_to_string(active) + "] Thinking... ("
                    + to_string(moveTime) + ")\n", 1);

        Out::output(Utils::getPrettyLines(pos, lines), 2);
        bestLine = getBestLine(pos, lines);
        if (bestLine.empty() || bestLine.isMat() ||
                fabs(bestLine.getEval()) > Options::getInstance().getMateThreshold()) {
            /*Handle the case where we should backtrack*/
            if (addedMoves_ > 0) {
                Out::output("\tBacktracking " + pos.getLastMove()
                            + " (addedMove#" + to_string(addedMoves_) + ")\n");

                /*
                 * We did a "mistake" : a line previously unbalanced is now a
                 * draw, we should better the backtracking by, for example,
                 * increasing attacking time
                 */
                if (bestLine.empty())
                    Out::output("\n\n\n\n DEFENDER SURVIVED \n\n\n\n ", 1);

                //Remove opposite side previous move
                addedMoves_--;
                pos.undoLastMove();

                if (active == playFor_) {
                    /*
                     *Remove our previous move if we had one, since the
                     *mat is "recorded" by engine
                     *(not the case if starting side is not the side
                     *the engine play for)
                     */
                    if (addedMoves_ > 0) {
                        addedMoves_--;
                        pos.undoLastMove();
                    }
                }
                continue;
            } else {
                /*This is the end (hold your breath and count to ten)*/
                break;
            }
        }

        /*If we are here, we just need to handle the next move*/
        Out::output("[" + color_to_string(active)
                + "] Chosen line : \n", 1);
        Out::output("\t" + Utils::getPrettyLine(pos, bestLine) + "\n", 1);

        string next = bestLine.firstMove();
        Out::output("\tNext move is " + next + "\n", 3);
        addedMoves_++;
        if (!pos.tryAndApplyMove(next))
            Err::handle("Invalid Move !");
    }

    //Display info at the end of computation
    Out::output("[End] Finder is done. Starting board was : \n");
    Out::output(pos.pretty() + "\n");

    if (playFor_ == sideToMove)
        Out::output("All lines should now be draw or mat :\n");
    else
        Out::output("Best line should be mat or draw.\n");
    Out::output(Utils::getPrettyLines(pos, lines));
    Out::output("[End] Full best line is : \n");
    Out::output("[End] " + Utils::getPrettyLine(pos, lines[0]) + "\n");
    Out::output("[End] " + Utils::listToString(lines[0].getMoves()) + "\n");
    return 0;
}


int MatFinder::computeMultiPV(const vector<Line> &lines)
{
    int diffLimit = 800;
    int multiPV = Options::getInstance().getMaxLines();
    int lastEvalValue = 0;
    bool lastEvalMat = false;
    bool allMat = true;
    int nonEmptyLines = 0;
    for (int i = 0; i < (int) lines.size(); ++i)
        if (!lines[i].empty())
            nonEmptyLines++;
    Out::output("Non empty : " + to_string(nonEmptyLines) + "\n", 3);
    for (int i = 0; i < (int) lines.size(); ++i) {
        if (i > 0) {
            if (lastEvalMat
                || fabs(lines[i].getEval() - lastEvalValue) > diffLimit) {
                Out::output("Eval/lastEval : "
                        + to_string(lines[i].getEval())
                        + "/" + to_string(lastEvalValue), 3);
                multiPV = i;
                allMat &= lastEvalMat;
                break;
            }
        }
        allMat &= lines[i].isMat();
        lastEvalValue = lines[i].getEval();
        lastEvalMat = lines[i].isMat();
    }
    if (allMat)
        multiPV = Options::getInstance().getMaxLines();

    if (multiPV != nonEmptyLines) {
        Out::output("Updating MultiPV to " + to_string(multiPV) + "\n", 2);
        pool_.sendOption(commId_, "MultiPV", to_string(multiPV));
        if (!pool_.isReady(commId_))
            Err::handle("Unable to update MultiPV");
    }
    return multiPV;
}

/**
 * This function determine the "best" line to follow
 */
const Line &MatFinder::getBestLine(const Position &pos,
                                   const vector<Line> &lines)
{
    Color active = pos.side_to_move();
    for (int i = 0; i < (int) lines.size(); ++i) {
        int limit = Options::getInstance().getCutoffThreshold();
        //FIXME: find a clearer way to define "balance"
        //eval is in centipawn, 100 ~ a pawn
        if (lines[i].isMat()) {
            //Best line is closed !
            return lines[i];
        } else {
            if (fabs(lines[i].getEval()) > limit) {
                //We should only considered lost positions according to
                //the side the engine play for
                if ((lines[i].getEval() < 0 && active == playFor_) ||
                        (lines[i].getEval() > 0 && active != playFor_))
                    return lines[i];
            } else {
                //If the line is a draw we don't want it
                return emptyLine_;
            }
        }
    }
    //if all are draw, return the same line
    return emptyLine_;

}

