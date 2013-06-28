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
#include "UCIReceiver.h"
#include "Stream.h"
#include "Utils.h"

using namespace std;

UCIReceiver::UCIReceiver(MatFinder *finder) : matFinder_(finder)
{
    input_ = new InputStream(matFinder_->getEngineOutRead());
}

UCIReceiver::~UCIReceiver()
{
    delete input_;
}

void *UCIReceiver::run()
{
    //Should get each engine message, then parse it
    //then eventually update and notify matFinder
    while (true) {
        getline((*input_), strBuf_);
        parseMessage(strBuf_);
    }
}

void UCIReceiver::readyok(istringstream &is)
{
    Utils::output("Engine is ready.\n", 1);
    matFinder_->signalReadyok();
}

void UCIReceiver::info(istringstream &is)
{
    string token;
    bool readLine = false;
    bool eval = false;
    int curDepth = 0;
    int curNps = 0;
    int curThinktime = 0;
    float curEval = 0;
    bool curIsMat = false;
    string curMv;
    list<string> curMoves;
    int curLineId = 0;
    while (is >> token) {
        if (token == "depth") {
            is >> curDepth;
        } else if (token == "seldepth") {
            //Drop
        } else if (token == "time") {
            is >> curThinktime;
            matFinder_->updateThinktime(curThinktime);
        } else if (token == "nodes") {
            //Drop
        } else if (token == "pv") {
            readLine = true;
            //NOTE: should be the last token of infoline
            while (is >> token)
                curMoves.push_back(token);
        } else if (token == "multipv") {
            is >> curLineId;
        } else if (token == "score") {
            eval = true;
            is >> token;
            if (token == "mate") {
                curIsMat = true;
                is >> curEval;
            } else if (token == "cp") {
                is >> curEval;
            } else {
                cerr << "********* No score ***********" << endl;
            }
        } else if (token == "currmove") {
            //Drop
        } else if (token == "currmovenumber") {
            //Drop
        } else if (token == "hashfull") {
            //NOTE: not supported by stockfish
            is >> token;
            cerr << "********** Hashfull : " << token << " *******\n";
        } else if (token == "nps") {
            is >> curNps;
            matFinder_->updateNps(curNps);
        } else if (token == "tbhits") {
            //TODO
        } else if (token == "cpuload") {
            //TODO
        } else if (token == "string") {
            //Drop
        } else if (token == "refutation") {
            //Drop
        } else if (token == "currline") {
            //Drop
        }
    }
    if (readLine) {
        if (!eval || curMoves.empty() || !curLineId) {
            cerr << "**** Line without eval or move... ****" << endl;
            return;
        }
        Line curLine(curEval, curDepth, curMoves, curIsMat);
        //Update the line in matFinder
        matFinder_->updateLine(curLineId - 1/*array in matFinder*/, curLine);
    }
}

void UCIReceiver::option(istringstream &is)
{
    //Just drop these, since we dont need them
}

void UCIReceiver::bestmove(istringstream &is)
{
    string bm;
    //consume bestmove
    is >> bm;
    matFinder_->signalBestmove(bm);
}

void UCIReceiver::parseMessage(string msg)
{
    string token;
    //cerr << "\tcmd: " << msg << "\n";
    istringstream is(msg);
    is >> skipws >> token;
    if (token == "id") {
        ;//not implemented
    } else if (token == "uciok") {
        ;//drop
    } else if (token == "bestmove") bestmove(is);
    else if (token == "readyok") readyok(is);
    else if (token == "info") info(is);
    else if (token == "option") option(is);
    else {
        cerr << "Unrecognise command from engine :\n";
        cerr << "\"" << msg << "\"\n";
    }
}

