#ifndef __MATFINDEROPTIONS_H__
#define __MATFINDEROPTIONS_H__

#include <string>
#include <list>
#include "Line.h"

#define DEFAULT_MAX_LINES 8
#define DEFAULT_PLAY_FOR WHITE
#define DEFAULT_VERBOSE_LEVEL 0
#define DEFAULT_HASHMAP_SIZE 4096
#define DEFAULT_PLAYFOR_MOVETIME_ 2500
#define DEFAULT_PLAYAGAINST_MOVETIME_ 1000
#define DEFAULT_STARTPOS "startpos"
#define DEFAULT_ENGINE "stockfish"
#define DEFAULT_PATH "/usr/local/bin"

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

    static side_t getPlayFor();
    static void setPlayFor(side_t playFor);

    static int getMaxLines();
    static void setMaxLines(int maxLines);

    static int getHashmapSize();
    static void setHashmapSize(int size);

    //Pretty-print the options
    static string getPretty();

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

    static side_t PLAY_FOR_;

    // 0 = minimal output
    // 1 = not-that-huge output
    // [...]
    // 5 = display too many things
    static int VERBOSE_LEVEL_;

    // in Mo
    static int HASHMAP_SIZE_;
};


#endif
