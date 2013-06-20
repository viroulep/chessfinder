#ifndef __UCIRECEIVER_H__
#define __UCIRECEIVER_H__

#include "Thread.h"
#include "Stream.h"
#include "MatFinder.h"

/**
 * This class is responsible for parsing commands from engine
 * and updating the finder's state
 */
class UCIReceiver : public Runnable {
public:
    UCIReceiver(MatFinder *finder);
    ~UCIReceiver();
    void *run();
private:
    void bestmove(istringstream &is);
    void readyok(istringstream &is);
    void info(istringstream &is);
    void option(istringstream &is);
    InputStream *input_;
    MatFinder *matFinder_;
    void parseMessage(std::string msg);
};

#endif
