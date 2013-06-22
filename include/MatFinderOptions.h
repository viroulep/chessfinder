#ifndef __MATFINDEROPTIONS_H__
#define __MATFINDEROPTIONS_H__

#include <string>

#define DEFAULT_MAX_LINES 8
#define DEFAULT_DEBUG_LEVEL 0
#define DEFAULT_HASHMAP_SIZE 4096
#define DEFAULT_STARTPOS "startpos"
#define DEFAULT_ENGINE "stockfish"
#define DEFAULT_PATH "/usr/local/bin"

class MatFinderOptions {
public:

    static string getEngine();
    static void setEngine(string engine);

    static string getPath();
    static void setPath(string path);

    static string getStartingPos();
    static void setStartingPos(string startingPos);

    static int getDebugLevel();
    static void setDebugLevel(int debugLevel);

    static int getMaxLines();
    static void setMaxLines(int maxLines);

private:
    static string START_POS_ = DEFAULT_STARTPOS;
    static string ENGINE_ = DEFAULT_ENGINE;
    static string PATH_ = DEFAULT_PATH;
    static int MAX_LINES_ = DEFAULT_MAX_LINES;
    static int DEBUG_LEVEL_ = DEFAULT_DEBUG_LEVEL;
};


#endif
