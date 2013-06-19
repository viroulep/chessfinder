#include <iostream>
#include "UCIReceiver.h"
#include "Stream.h"

using namespace std;

UCIReceiver::UCIReceiver(MatFinder *finder) : matFinder_(finder)
{
    input_.open(matFinder_->getEngineOutRead());
}

void *UCIReceiver::run()
{
    string str;
    //Should get each engine message, then parse it
    //then eventually update and notify matFinder
    while (true) {
        getline(input_, str);
        cout << "I just received :" << endl;
        cout << "\"" << str << "\"" << endl;
    }
    cout << "Exiting receiver\n";
}

void UCIReceiver::parseMessage(string msg)
{
    ;
}
