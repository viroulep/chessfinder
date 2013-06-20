#include "Line.h"
#include <cmath>
#include <iostream>
#include <sstream>

Line::Line() : eval_(0)
{
    moves_.clear();
}

Line::Line(float ev, int depth, list<string> mv, bool isMat) :
    eval_(ev), depth_(depth), isMat_(isMat)
{
    //*copy* the moves from mv to move_
    moves_.insert(moves_.end(), mv.begin(), mv.end());
}

string Line::getPretty(side_t engineSide)
{
    ostringstream oss;
    //TODO: implement a chess board to display pgn-style infos

    float displayEval = (engineSide == BLACK)?-eval_:eval_;
    if (isMat_)
        oss << "#";
    else
        displayEval /= 100;
    oss << displayEval;
    oss << ", d: ";
    oss << depth_;
    oss << ", line : ";
    list<string> tmpList = moves_;
    int i = 0;
    //TODO: #define max number of displayed move
    while (!tmpList.empty() && i < 8) {
        oss << tmpList.front();
        oss << " ";
        tmpList.pop_front();
        i++;
    }
    if (i < tmpList.size())
        oss << "[...moves...]";
    oss << "\n";

    return oss.str();
}

void Line::update(float newEval, int newDepth, list<string> newMoves,
        bool isMat/* = false*/)
{
    //TODO: is this really needed ?
    eval_ = newEval;
    depth_ = newDepth;
    moves_.clear();
    //clear and copy
    moves_.insert(moves_.end(), newMoves.begin(), newMoves.end());
    isMat_ = isMat;
}

void Line::update(Line &line)
{
    update(line.eval_, line.depth_, line.moves_, line.isMat_);
}

bool Line::isMat()
{
    return isMat_;
}

float Line::getEval()
{
    return eval_;
}

bool Line::addMove(string mv)
{
    //TODO : is this really needed ?
    if (mv.size() != 4)
        return false;
    //Maybe check that mv 0 and 2 \in a-h
    moves_.push_back(mv);
}

bool Line::empty()
{
    return moves_.empty();
}

string Line::firstMove()
{
    return moves_.front();
}

bool Line::operator<(const Line &rhs)
{
    return fabs(eval_) < fabs(rhs.eval_);
}

Line Line::emptyLine;
