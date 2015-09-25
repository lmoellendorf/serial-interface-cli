#ifndef EVENT_H
#define EVENT_H

#include <string>

class Event
{
public:
    Event ( int identifier, void *source, void *content, size_t content_size );
    ~Event();

public:
    virtual int GetIdentifier ( );
    virtual void* GetSource ( );
    virtual size_t GetDetails ( void **content );

private:
    int identifier;
    void *source;
    void *content;
    size_t content_size;
};

#endif // EVENT_H
