#ifndef SUBJECT_H
#define SUBJECT_H

#include <forward_list>

class Observer;
class Event;

class Subject
{
    std::forward_list<Observer*> observers;
public:
    Subject();
    ~Subject();

    virtual void Attach ( Observer* );
    virtual void Detach ( Observer* );
    virtual void Notify ( Event );
};

#endif // SUBJECT_H
