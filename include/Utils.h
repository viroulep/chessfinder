#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>
#include <vector>
#include "Line.h"
#include <sys/time.h>

class Utils {
public:
    //implicitly inlined
    static void handleErrorAndExit(string msg);
    static void handleError(std::string caller, int rc);
    static side_t getSideFromFen(std::string fen);
    static void getTimeout(struct timespec *ts, int seconds);
};
#endif
