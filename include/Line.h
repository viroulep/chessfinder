#ifndef __LINE_H__
#define __LINE_H__

#include <string>
#include <list>

using namespace std;

typedef enum side_e {
    WHITE,
    BLACK,
    UNDEFINED
} side_t;
static const char* SideNames[3] = { "white", "black", "undefined" };

//Class representing a particular line for a position
//TODO: make the line inherit from list<string>
class Line {
public:
    Line();
    Line(float ev, int depth, list<string> mv, bool isMat = false);
    //Return more readable format
    string getPretty(side_t engineSide);

    void update(Line &line);
    bool isMat();
    float getEval();
    bool empty();
    string firstMove();
    bool operator<(const Line &rhs);

    static Line emptyLine;
private:
    //Line Evaluation
    //Might be a mat
    float eval_;
    bool isMat_ = false;
    int depth_;
    list<string> moves_;
};

#endif
