#include "Utils.h"
#include <iostream>

using namespace std;

void Utils::handleError(string caller, int rc)
{
    if (rc) {
        cout << "Error on : " << caller << ", rc=" << rc << endl;
        exit(EXIT_FAILURE);
    }
}
