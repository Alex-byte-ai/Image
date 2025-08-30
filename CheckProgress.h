#pragma once

#include <functional>

class CheckProgress
{
private:
    std::function<void()> action;
    unsigned time, interval;
public:
    CheckProgress( const std::function<void()> &action, unsigned interval );
    unsigned check();
};
