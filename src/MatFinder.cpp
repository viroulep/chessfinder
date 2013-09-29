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
#include "Options.h"
#include "Stream.h"
#include "UCIReceiver.h"
#include "Utils.h"



MatFinder::MatFinder() : Finder()
{
}

MatFinder::~MatFinder()
{
}


int MatFinder::runFinderOnCurrentPosition()
{
    Board::Side sideToMove = cb_->getActiveSide();

    Utils::output("Starting board is :\n" + cb_->to_string() + "\n");
    Utils::output("Doing some basic evaluation on submitted position...\n");

    sendCurrentPositionToEngine();
    lines_.assign(Options::getMaxLines(), Line::emptyLine);
    sendToEngine("go movetime "
            + to_string(Options::getPlayforMovetime()));
    waitBestmove();
    Utils::output("Evaluation is :\n");
    Utils::output(getPrettyLines());
    //TODO: check
    if (!lines_[0].empty()) {
        Side winning;
        if ((lines_[0].getEval() < 0 && sideToMove == Side::WHITE)
                || (lines_[0].getEval() > 0 && sideToMove == Side::BLACK))
            winning = Side::BLACK;
        else
            winning = Side::WHITE;
        engine_play_for_ = (winning == Side::WHITE)?
            Side::BLACK:Side::WHITE;

    }
    Utils::output("Engine will play for : "
            + Board::to_string(engine_play_for_) + "\n");


    //Main loop
    while (true) {
        Line bestLine;
        Side active = cb_->getActiveSide();

        Utils::output("[" + Board::to_string(active)
                + "] Depth " + to_string(addedMoves_) + "\n");

        sendCurrentPositionToEngine();

        Utils::output(cb_->to_string(), 2);

        //Thinking according to the side the engine play for
        int moveTime = (active == engine_play_for_ || !addedMoves_) ?
            Options::getPlayforMovetime() :
            Options::getPlayagainstMovetime();

        //Compute optimal multipv
        int pv = updateMultiPV();


        //Scaling moveTime
        //According to number of lines
        moveTime = (int)(moveTime * ((float)
                    ((float)pv/(float)Options::getMaxLines())
                    ));
        if (moveTime <= 600)
            moveTime = 600;
        //Acccording to depth
        moveTime += 10 * addedMoves_;

        //Initialize vector with empty lines
        lines_.assign(Options::getMaxLines(), Line::emptyLine);

        //Increase movetime with depth
        sendToEngine("go movetime " + to_string(moveTime));

        Utils::output("[" + Board::to_string(active)
                + "] Thinking... (" + to_string(moveTime) + ")\n", 1);

        //Wait for engine to finish thinking
        waitBestmove();

        Utils::output(getPrettyLines(), 2);
        bestLine = getBestLine();
        if (bestLine.empty() || bestLine.isMat()) {
            //Handle the case where we should backtrack
            if (addedMoves_ > 0) {
                Utils::output("\tBacktracking " + cb_->getUciMoves().back()
                    + " (addedMove#" + to_string(addedMoves_)
                    + ")\n");
                //Remove opposite side previous move
                addedMoves_--;
                cb_->undoMove();

                if (active == engine_play_for_) {
                    //Remove our previous move if we had one, since the
                    //mat is "recorded" by engine
                    //(not the case if starting side is not the side 
                    //the engine play for)
                    if (addedMoves_ > 0) {
                        addedMoves_--;
                        cb_->undoMove();
                    }
                }
                continue;
            } else {
                //This is the end (hold your breath and count to ten)
                break;
            }
        }

        //If we are here, we just need to handle the next move
        Utils::output("[" + Board::to_string(active)
                + "] Chosen line : \n", 1);
        Utils::output("\t" + getPrettyLine(bestLine,
                    Options::movesDisplayed) + "\n", 1);

        string next = bestLine.firstMove();
        Utils::output("\tNext move is " + next + "\n", 3);
        addedMoves_++;
        cb_->uciApplyMove(next);
    }

    //Display info at the end of computation
    Utils::output("[End] Finder is done. Starting board was : \n");
    Utils::output(cb_->to_string() + "\n");

    if (engine_play_for_ == sideToMove)
        Utils::output("All lines should now be draw or mat :\n");
    else
        Utils::output("Best line should be mat or draw.\n");
    Utils::output(getPrettyLines());
    Utils::output("[End] Full best line is : \n");
    Utils::output("[End] " + getPrettyLine(lines_[0]) + "\n");
    Utils::output("[End] " + Utils::listToString(lines_[0].getMoves()) + "\n");

    return 0;
}


int MatFinder::updateMultiPV()
{
    int diffLimit = 800;
    int multiPV = Options::getMaxLines();
    int lastEvalValue = 0;
    bool lastEvalMat = false;
    bool allMat = true;
    int nonEmptyLines = 0;
    for (int i = 0; i < lines_.size(); ++i)
        if (!lines_[i].empty())
            nonEmptyLines++;
    Utils::output("Non empty : " + to_string(nonEmptyLines) + "\n", 3);
    for (int i = 0; i < lines_.size(); ++i) {
        if (i > 0) {
            if (lastEvalMat 
                    || fabs(lines_[i].getEval() - lastEvalValue) > diffLimit) {
                Utils::output("Eval/lastEval : "
                        + to_string(lines_[i].getEval())
                        + "/" + to_string(lastEvalValue), 3);
                multiPV = i;
                allMat &= lastEvalMat;
                break;
            }
        }
        allMat &= lines_[i].isMat();
        lastEvalValue = lines_[i].getEval();
        lastEvalMat = lines_[i].isMat();
    }
    if (allMat)
        multiPV = Options::getMaxLines();

    if (multiPV != nonEmptyLines) {
        Utils::output("Updating MultiPV to " + to_string(multiPV) + "\n", 2);
        sendOptionToEngine("MultiPV", to_string(multiPV));
        sendToEngine("isready");
        waitReadyok();
    }
    return multiPV;
}

/**
 * This function determine the "best" line to follow
 */
Line &MatFinder::getBestLine()
{
    Side active = cb_->getActiveSide();
    for (int i = 0; i < lines_.size(); ++i) {
        int limit = Options::getCpTreshold();
        //FIXME: find a clearer way to define "balance"
        //eval is in centipawn, 100 ~ a pawn
        if (lines_[i].isMat()) {
            //Best line is closed !
            return lines_[i];
        } else {
            if (fabs(lines_[i].getEval()) > limit) {
                //We should only considered lost positions according to
                //the side the engine play for
                if ((lines_[i].getEval() < 0 && active == engine_play_for_) ||
                        (lines_[i].getEval() > 0 && active != engine_play_for_))
                    return lines_[i];
            } else {
                //If the line is a draw we don't want it
                return Line::emptyLine;
            }
        }
    }
    //if all are draw, return the same line
    return Line::emptyLine;

}

