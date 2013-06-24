#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <list>
#include <vector>
#include <sys/time.h>
//FIXME: this include should not exist
#include "Board.h"

class Utils {
public:
    //implicitly inlined
    static void handleError(int rc);
    static void handleError(const string &msg);
    static void handleError(const string &caller, int rc);
    static void handleError(const string &msg, int rc,
            const string &fileName, int lineNumber);

    static void output(const string &msg, int level = 0);

    static string helpMessage();
    static string listToString(list<string> &theList);
    static void getTimeout(struct timespec *ts, int seconds);

    //FIXME: deprecated, main should start a chesboard,
    //then take the side
    static Board::Side getSideFromFen(string fen);
    //FIXME: should belong to board ?
    static int parseMovelist(list<string> &theList, string moves);
};
#endif
