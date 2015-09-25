#ifndef OBSERVER_H
#define OBSERVER_H

#include "sf_event.h"

class Observer
{
public:
    virtual void Update ( Event *event ) = 0;
};

#endif // OBSERVER_H
