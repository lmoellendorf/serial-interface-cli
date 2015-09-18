#include <forward_list>

#include "sf_observer.h"
#include "sf_subject.h"
#include "sf_event.h"

std::forward_list<Observer*> Subject::observers;

Subject::Subject()
{

}

Subject::~Subject()
{

}

void Subject::Attach ( Observer *observer )
{
  Subject::observers.push_front ( observer );
}

void Subject::Detach ( Observer *observer )
{
  Subject::observers.remove ( observer );
}

void Subject::Notify ( Event *event, Filter filter )
{
  for ( auto it = observers.begin(); it != observers.end(); ++it )
    {
      if ( filter ( ( Observer* ) *it, event ) )
        {
          ( ( Observer* ) *it )->Update ( event );
        }
    }
}
