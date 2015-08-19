#include <forward_list>

#include "sf_subject.h"
#include "sf_observer.h"
#include "sf_event.h"


Subject::Subject() :  observers() { }

Subject::~Subject()
{

}

void Subject::Attach ( Observer *observer)
{
  observers.push_front ( observer );
}

void Subject::Detach ( Observer *observer )
{
  observers.remove ( observer );
}

void Subject::Notify ( Event event )
{
  for ( auto it = observers.begin(); it != observers.end(); ++it )
    {
      ( ( Observer* ) *it )->Update ( event );

    }
}
