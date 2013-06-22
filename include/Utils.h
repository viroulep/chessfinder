#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <list>
#include <vector>
#include "Line.h"
#include <sys/time.h>

class Utils {
public:
    //implicitly inlined
    static void handleError(int rc);
    static void handleError(string msg);
    static void handleError(string caller, int rc);

    static void output(string msg, int level = 0);

    static string helpMessage();

    static side_t getSideFromFen(string fen);
    static string listToString(list<string> &theList);
    static int parseMovelist(list<string> &theList, string moves);
    static int parseSide(side_t *side, string sidestr);
    static void getTimeout(struct timespec *ts, int seconds);
};
#endif
