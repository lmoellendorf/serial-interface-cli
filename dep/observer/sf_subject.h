#ifndef SUBJECT_H
#define SUBJECT_H

#include <forward_list>

class Observer;
class Event;

class Subject
{

protected:
    static std::forward_list<Observer*> observers;
public:
    Subject();
    ~Subject();

    typedef bool (*Filter) ( Observer *observer, Event *event );
    //TODO: overload Attach() to enable the Oberserver to pass a Event "id"
    // it is interested in
    static void Attach ( Observer *observer );
    static void Detach ( Observer *observer );
    static void Notify ( Event *event, Filter filter);
};

#endif // SUBJECT_H
