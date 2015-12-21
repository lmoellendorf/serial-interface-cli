#include "sf_event.h"

namespace sf
{

Event::Event ( int identifier, void *source, void *content, size_t content_size ) //: identifier(), source(), content(), content_size() {} TODO
{
  this->identifier = identifier;
  this->source = source;
  this->content = content;
  this->content_size = content_size;
}

Event::~Event() {}

int Event::GetIdentifier()
{
  return this->identifier;
}

void* Event::GetSource()
{
  return this->source;
}

size_t Event::GetDetails ( void** content )
{
  *content = this->content;
  return content_size;
}

}