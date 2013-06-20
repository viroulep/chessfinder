#ifndef __UTILS_H__
#define __UTILS_H__

#include <string>

class Utils {
public:
    //implicitly inlined
    static void handleError(std::string caller, int rc);
};
#endif
