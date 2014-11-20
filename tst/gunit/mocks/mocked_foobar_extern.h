#include <gmock/gmock.h>

extern "C"{
#include "stzedn_typedefs.h"
}

class Extern
{
public:
   virtual ~Extern() {}
   virtual uint8_t foo_extern_getNumber(uint8_t c_seed) = 0;
};


class MockExtern : public Extern
{
public:
   MOCK_METHOD1(foo_extern_getNumber ,  uint8_t(uint8_t c_seed));
};


/*==============================================================================
                              Helper Function Prototypes
==============================================================================*/
void setMockExternPointer(Extern *externParam);
