#ifndef EVENT_H
#define EVENT_H

#include <string>

class Event
{
public:
    Event(std::string, void*, void*, void*);
    ~Event();

    std::string Name;
    void *Source;
    void *NewState;
    void *OldState;
};

#endif // EVENT_H

