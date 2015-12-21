#ifndef OBSERVER_H
#define OBSERVER_H

#include "sf_event.h"

namespace sf
{

class Observer
{
public:
    virtual void Update ( Event *event ) = 0;
};

}
#endif // OBSERVER_H
