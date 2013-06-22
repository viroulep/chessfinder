#include "Utils.h"
#include "Line.h"
#include <iostream>
#include <sstream>

using namespace std;

void Utils::handleErrorAndExit(string msg)
{
    cerr << msg << endl;
    exit(EXIT_FAILURE);
}

void Utils::handleError(string caller, int rc)
{
    if (rc) {
        cerr << "Error on : " << caller << ", rc=" << rc << endl;
        exit(EXIT_FAILURE);
    }
}

side_t Utils::getSideFromFen(string fen)
{
    vector<string> infos;
    stringstream ss(fen);
    string tmpInfo;
    while (getline(ss, tmpInfo, ' '))
        infos.push_back(tmpInfo);
    if (infos.size() > 1) {
        string side = infos[1];
        if (side == "w") {
            return WHITE;
        } else if (side == "b") {
            return BLACK;
        }
    }
    return UNDEFINED;
}

void Utils::getTimeout(struct timespec *ts, int seconds)
{
    struct timeval tp;
    int rc = gettimeofday(&tp, NULL);
    Utils::handleError("gettimeofday()\n", rc);

    /* Convert from timeval to timespec */
    ts->tv_sec  = tp.tv_sec;
    ts->tv_nsec = tp.tv_usec * 1000;
    ts->tv_sec += seconds;
}
