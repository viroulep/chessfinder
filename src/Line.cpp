#include <cmath>
#include <iostream>
#include <sstream>
#include "Line.h"
#include "MatFinderOptions.h"

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
    while (!tmpList.empty() && i < MatFinderOptions::movesDisplayed) {
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

void Line::update(Line &line)
{
    eval_ = line.eval_;
    depth_ = line.depth_;
    moves_.clear();
    //clear and copy
    moves_.insert(moves_.end(), line.moves_.begin(), line.moves_.end());
    isMat_ = line.isMat_;
    //update(line.eval_, line.depth_, line.moves_, line.isMat_);
}

bool Line::isMat()
{
    return isMat_;
}

float Line::getEval()
{
    return eval_;
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
