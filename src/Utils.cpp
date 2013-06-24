#include <iostream>
#include <sstream>
#include "Utils.h"
#include "MatFinderOptions.h"
#include "Line.h"

using namespace std;

//TODO: exit program correctly on error

void Utils::handleError(int rc)
{
    handleError("unspecified caller", rc);
}

void Utils::handleError(const string &msg)
{
    cerr << msg << endl;
    exit(EXIT_FAILURE);
}

void Utils::handleError(const string &caller, int rc)
{
    if (rc) {
        handleError("Error on : " + caller + ", rc=" + to_string(rc));
    }
}

void Utils::handleError(const string &msg, int rc,
        const string &fileName, int lineNumber)
{
    if (rc)
        cerr << fileName << ":" << lineNumber << endl;
    handleError(msg, rc);
}


void Utils::output(const string &msg, int level/* = 0*/)
{
    if (level <= MatFinderOptions::getVerboseLevel())
        cout << msg;
}

string Utils::helpMessage()
{
    ostringstream oss;
    oss << "MatFinder, a program to help engines to find mat\n";
    oss << "\n";
    oss << "Options\n";
    oss << "\n";
    oss << "\t" << "--startpos=fenstring, -s fen_string\n";
    oss << "\t\t" << "Defines the initial board state, fen format.\n"
        << "\t\t" << "Example value : "
        << "\"rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1\"\n"
        << "\t\t" << "Default value : \"" << DEFAULT_STARTPOS << "\"\n";
    oss << "\n";
    oss << "\t" << "--moves=moves, -s moves\n";
    oss << "\t\t" << "Defines the main line after the starting position.\n"
        << "\t\t" << "Example value : "
        << "\"e2e4 c7d5\"\n"
        << "\t\t" << "Default value : \""
        << Utils::listToString(MatFinderOptions::getUserMoves()) << "\"\n";
    oss << "\n";
    oss << "\t" << "--engine=engine, -e engine\n";
    oss << "\t\t" << "Defines the uci engine command.\n"
        << "\t\t" << "Default value : \"" << DEFAULT_ENGINE << "\"\n";
    oss << "\n";
    oss << "\t" << "--playfor=side, -o side\n";
    oss << "\t\t" << "Defines the side the engine plays for.\n"
        << "\t\t" << "Default value : \""
        << Board::to_string(Board::Side::DEFAULT_PLAY_FOR) << "\"\n";
    oss << "\n";
    oss << "\t" << "--path=path, -p path\n";
    oss << "\t\t" << "Defines the engine binary's path.\n"
        << "\t\t" << "Default value : \"" << DEFAULT_PATH << "\"\n";
    oss << "\n";
    oss << "\t" << "--pf_movetime=msec, -f msec\n";
    oss << "\t\t" << "Playfor Movetime. Defines the engine thinking time,\n"
        << "\t\t\twhen playing on the side he \"plays for\".\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_PLAYFOR_MOVETIME_) << "\n";
    oss << "\n";
    oss << "\t" << "--pa_movetime=msec, -a msec\n";
    oss << "\t\t" << "Playagainst Movetime. Defines the engine thinking time,\n"
        << "\t\t\twhen he answers the side he \"plays for\".\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_PLAYAGAINST_MOVETIME_) << "\n";
    oss << "\n";
    oss << "\t" << "--lines=n, -l n\n";
    oss << "\t\t" << "Defines the number of lines to evaluate"
        << " (MultiPV uci's option)\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_MAX_LINES) << "\n";
    oss << "\n";
    oss << "\t" << "--verbose[=level], -v\n";
    oss << "\t\t" << "Defines the verbose level. Level is optional : "
        << "default arg value is 1\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_VERBOSE_LEVEL) << "\n";
    oss << "\n";
    oss << "\t" << "--hashmap_size=size, -t size\n";
    oss << "\t\t" << "Defines the engine Hashmap size in MBytes.\n"
        << "\t\t" << "Default value : "
        << to_string(DEFAULT_HASHMAP_SIZE) << "\n";
    /*
     *oss << "\t" << "Hashmap size\t\t" << " = " << HASHMAP_SIZE_ << endl;
     */
    oss << "\n";
    oss << "Examples\n";
    oss << "\n";
    oss << "\tStart the finder on gardnerfish, studying 1. c4 opening,\n"
        << "\twith detail informations about lines played :\n";
    oss << "\t$ ./matfinder --verbose --moves=\"c3c4\" --engine=\"gardnerfish\"\n";
    oss << "\n";
    oss << "Contact\n";
    oss << "\tPhilippe Virouleau <philippe.viroulea@imag.fr>\n";
    return oss.str();
}

string Utils::listToString(list<string> &theList)
{
    ostringstream oss;
    if (theList.empty())
        oss << "<none>";
    for (list<string>::iterator it = theList.begin(),
            itEnd = theList.end();
            it != itEnd; ++it) {
        if (it != theList.begin())
            oss << " ";
        oss << (*it);
    }
    return oss.str();
}

int Utils::parseMovelist(list<string> &theList, string moves)
{
    istringstream is(moves);
    string mv;
    //TODO: handle the case e7e8q
    while (is >> skipws >> mv) {
        if ( mv.size() != 4
                || mv[0] > 'h' || mv[0] < 'a'
                || mv[1] > '8' || mv[1] < '1'
                || mv[2] > 'h' || mv[2] < 'a'
                || mv[3] > '8' || mv[3] < '1')
            return 1;
        theList.push_back(mv);
    }
    return 0;
}

Board::Side Utils::getSideFromFen(string fen)
{
    vector<string> infos;
    stringstream ss(fen);
    string tmpInfo;
    while (getline(ss, tmpInfo, ' '))
        infos.push_back(tmpInfo);
    if (infos.size() > 1) {
        string side = infos[1];
        if (side == "w") {
            return Board::Side::WHITE;
        } else if (side == "b") {
            return Board::Side::BLACK;
        }
    }
    Utils::handleError("Unable to get side from fen string.");
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
