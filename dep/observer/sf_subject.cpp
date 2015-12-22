/**
 * @code
 *  ___ _____ _   ___ _  _____ ___  ___  ___ ___
 * / __|_   _/_\ / __| |/ / __/ _ \| _ \/ __| __|
 * \__ \ | |/ _ \ (__| ' <| _| (_) |   / (__| _|
 * |___/ |_/_/ \_\___|_|\_\_| \___/|_|_\\___|___|
 * embedded.connectivity.solutions.==============
 * @endcode
 *
 * @file
 * @copyright  STACKFORCE GmbH, Heitersheim, Germany, http://www.stackforce.de
 * @author     STACKFORCE
 * @brief      Generic implementation of the Oberserver pattern.
 *
 */

#include <forward_list>

#include "sf_observer.h"
#include "sf_subject.h"
#include "sf_event.h"

namespace sf
{

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

}