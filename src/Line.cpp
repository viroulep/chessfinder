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

string Line::getPrettyEval(bool invertEval)
{

    ostringstream out;

    float displayEval = (invertEval)?-eval_:eval_;
    if (isMat_)
        out << "#";
    else
        displayEval /= 100;
    char eval[10];
    //Tricky: avoid invalid write of '0' in output stream
    sprintf(eval, "%.2f", displayEval);
    out << eval;
    return out.str();
}

//TODO: getPretty(bool invertEval) ?! (so that Line doesn't depend on board)
string Line::getPretty(bool invertEval)
{
    ostringstream out;

    float displayEval = (invertEval)?-eval_:eval_;
    if (isMat_)
        out << "#";
    else
        displayEval /= 100;
    char eval[10];
    //Tricky: avoid invalid write of '0' in output stream
    sprintf(eval, "%.2f", displayEval);
    out << eval;
    out << ", d: ";
    out << depth_;
    out << ", line : ";
    list<string> tmpList = moves_;
    int i = 0;
    while (!tmpList.empty() && i < MatFinderOptions::movesDisplayed) {
        out << tmpList.front();
        out << " ";
        tmpList.pop_front();
        i++;
    }
    if (i < tmpList.size())
        out << "[...moves...]";
    out << "\n";

    return out.str();
}

const list<string> &Line::getMoves()
{
    return moves_;
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
