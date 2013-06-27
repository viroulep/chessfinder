#ifndef __MATFINDEROPTIONS_H__
#define __MATFINDEROPTIONS_H__

#include <string>
#include <list>
#include "Board.h"

#define DEFAULT_MAX_LINES 8
#define DEFAULT_PLAY_FOR WHITE
#define DEFAULT_VERBOSE_LEVEL 0
#define DEFAULT_HASHMAP_SIZE 4096
#define DEFAULT_PLAYFOR_MOVETIME_ 2500
#define DEFAULT_PLAYAGAINST_MOVETIME_ 1000
#define DEFAULT_STARTPOS "startpos"
#define DEFAULT_ENGINE "stockfish"
#define DEFAULT_PATH "/usr/local/bin"
#define DEFAULT_TRESHOLD 100
#define TIMEOUT_READY 5
#define MOVES_DISPLAYED 8

using namespace std;

class MatFinderOptions {
public:

    static string getEngine();
    static void setEngine(string engine);

    static string getPath();
    static void setPath(string path);

    static string getStartingPos();
    static void setStartingPos(string startingPos);

    static list<string> &getUserMoves();
    static void setUserMoves(list<string> &theList);

    static int getPlayagainstMovetime();
    static void setPlayagainstMovetime(int movetime);

    static int getPlayforMovetime();
    static void setPlayforMovetime(int movetime);

    static int getVerboseLevel();
    static void setVerboseLevel(int verboseLevel);

    static Board::Side getPlayFor();
    static void setPlayFor(Board::Side playFor);

    static int getMaxLines();
    static void setMaxLines(int maxLines);

    static int getHashmapSize();
    static void setHashmapSize(int size);

    static int getCpTreshold();
    static void setCpTreshold(int treshold);

    //Pretty-print the options
    static string getPretty();

    //How long we wait for the engine to be ready (s)
    const static int isreadyTimeout = TIMEOUT_READY;

    //Number of moves displayed when printing lines
    const static int movesDisplayed = MOVES_DISPLAYED;

private:
    //Private (static class)
    MatFinderOptions();
    static string START_POS_;
    static string ENGINE_;
    static string PATH_;

    static list<string> USER_MOVES_;

    // in ms
    static int PLAYFOR_MOVETIME_;
    static int PLAYAGAINST_MOVETIME_;

    static int MAX_LINES_;

    //in centipawn
    static int CP_TRESHOLD_;

    static Board::Side PLAY_FOR_;

    // 0 = minimal output
    // 1 = not-that-huge output
    // [...]
    // 5 = display too many things
    static int VERBOSE_LEVEL_;

    // in Mo
    static int HASHMAP_SIZE_;
};


#endif
