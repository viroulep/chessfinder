
#include <sstream>
#include "Utils.h"
#include "MatFinderOptions.h"



string MatFinderOptions::START_POS_ = DEFAULT_STARTPOS;
string MatFinderOptions::ENGINE_ = DEFAULT_ENGINE;
string MatFinderOptions::PATH_ = DEFAULT_PATH;
list<string> MatFinderOptions::USER_MOVES_;
int MatFinderOptions::PLAYFOR_MOVETIME_ = DEFAULT_PLAYFOR_MOVETIME_;
int MatFinderOptions::PLAYAGAINST_MOVETIME_ = DEFAULT_PLAYAGAINST_MOVETIME_;
int MatFinderOptions::MAX_LINES_ = DEFAULT_MAX_LINES;
side_t MatFinderOptions::PLAY_FOR_ = DEFAULT_PLAY_FOR;
int MatFinderOptions::VERBOSE_LEVEL_ = DEFAULT_VERBOSE_LEVEL;
int MatFinderOptions::HASHMAP_SIZE_ = DEFAULT_HASHMAP_SIZE;



string MatFinderOptions::getEngine() { return ENGINE_; }
void MatFinderOptions::setEngine(string engine) { ENGINE_ = engine; }

string MatFinderOptions::getPath() { return PATH_; }
void MatFinderOptions::setPath(string path) { PATH_ = path; }

string MatFinderOptions::getStartingPos() { return START_POS_; }
void MatFinderOptions::setStartingPos(string startingPos)
{
    START_POS_ = startingPos;
}

list<string> &MatFinderOptions::getUserMoves() { return USER_MOVES_; }
void MatFinderOptions::setUserMoves(list<string> &theList)
{
    USER_MOVES_.clear();
    //Delete moves from theList and insert into class member
    USER_MOVES_.splice(USER_MOVES_.end(), theList);
}

int MatFinderOptions::getPlayagainstMovetime()
{
    return PLAYAGAINST_MOVETIME_;
}
void MatFinderOptions::setPlayagainstMovetime(int movetime)
{
    PLAYAGAINST_MOVETIME_ = movetime;
}

int MatFinderOptions::getPlayforMovetime() { return PLAYFOR_MOVETIME_; }
void MatFinderOptions::setPlayforMovetime(int movetime)
{
    PLAYFOR_MOVETIME_ = movetime;
}

int MatFinderOptions::getVerboseLevel() { return VERBOSE_LEVEL_; }
void MatFinderOptions::setVerboseLevel(int verboseLevel)
{
    VERBOSE_LEVEL_ = verboseLevel;
}

side_t MatFinderOptions::getPlayFor() { return PLAY_FOR_; }
void MatFinderOptions::setPlayFor(side_t playFor) { PLAY_FOR_ = playFor; }

int MatFinderOptions::getMaxLines() { return MAX_LINES_; }
void MatFinderOptions::setMaxLines(int maxLines) { MAX_LINES_ = maxLines; }

int MatFinderOptions::getHashmapSize() { return HASHMAP_SIZE_; }
void MatFinderOptions::setHashmapSize(int size)
{
    HASHMAP_SIZE_ = size;
}

string MatFinderOptions::getPretty()
{
    ostringstream oss;
    oss << "Options :\n";
    oss << "\t" << "Startpos\t\t" << " = " << START_POS_ << endl;
    oss << "\t" << "Engine\t\t\t" << " = " << ENGINE_ << endl;
    oss << "\t" << "Path\t\t\t" << " = " << PATH_ << endl;
    oss << "\t" << "Usermoves\t\t" << " = "
        << Utils::listToString(USER_MOVES_) << endl;
    oss << "\t" << "Playfor\t\t\t" << " = " << SideNames[PLAY_FOR_] << endl;
    oss << "\t" << "Playfor Movetime\t" << " = " << PLAYFOR_MOVETIME_ << endl;
    oss << "\t" << "Playagainst Movetime\t" << " = "
        << PLAYAGAINST_MOVETIME_ << endl;
    oss << "\t" << "Max lines\t\t" << " = " << MAX_LINES_ << endl;
    oss << "\t" << "Verbose level\t\t" << " = " << VERBOSE_LEVEL_ << endl;
    oss << "\t" << "Hashmap size\t\t" << " = " << HASHMAP_SIZE_ << endl;

    return oss.str();
}
