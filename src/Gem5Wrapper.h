#ifndef __GEM5_WRAPPER_H
#define __GEM5_WRAPPER_H

#include <string>

using namespace std;

namespace ramulator
{

class Request;
class MemoryBase;

class Gem5Wrapper 
{
private:
    MemoryBase *mem;
public:
    double tCK;
    Gem5Wrapper(const string& config_file, int cacheline);
    ~Gem5Wrapper();
    void tick();
    bool send(Request req);
    void finish(void);
};

} /*namespace ramulator*/

#endif /*__GEM5_WRAPPER_H*/
