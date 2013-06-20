#include <iostream>
#include <sstream>
#include "UCIReceiver.h"
#include "Stream.h"

using namespace std;

UCIReceiver::UCIReceiver(MatFinder *finder) : matFinder_(finder)
{
    input_ = new InputStream(matFinder_->getEngineOutRead());
}

UCIReceiver::~UCIReceiver()
{
    //delete input_;
}

void *UCIReceiver::run()
{
    string str;
    //Should get each engine message, then parse it
    //then eventually update and notify matFinder
    while (true) {
        getline((*input_), str);
        parseMessage(str);
    }
}

void UCIReceiver::readyok(istringstream &is)
{
    cout << "Engine is ready.\n";
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
            //TODO (assert depth is already consumed)
        } else if (token == "time") {
            is >> curThinktime;
            matFinder_->updateThinktime(curThinktime);
        } else if (token == "nodes") {
            //TODO
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
            //TODO
        } else if (token == "currmovenumber") {
            //TODO
        } else if (token == "hashfull") {
            //TODO
        } else if (token == "nps") {
            is >> curNps;
            matFinder_->updateNps(curNps);
        } else if (token == "tbhits") {
            //TODO
        } else if (token == "cpuload") {
            //TODO
        } else if (token == "string") {
            //TODO
        } else if (token == "refutation") {
            //TODO
        } else if (token == "currline") {
            //TODO
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
    istringstream is(msg);
    is >> skipws >> token;
    if (token == "id") {
        ;//not implemented
    } else if (token == "uciok") {
        ;//drop
    } else if (token == "bestmove") bestmove(is);//TODO
    else if (token == "readyok") readyok(is);
    else if (token == "info") info(is);//TODO
    else if (token == "option") option(is);
    else {
        cerr << "Unrecognise command from engine :\n";
        cerr << "\"" << msg << "\"\n";
    }
}

