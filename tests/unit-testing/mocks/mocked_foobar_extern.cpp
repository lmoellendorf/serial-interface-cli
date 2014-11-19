extern "C" {
#include <assert.h>
}
#include <gmock/gmock.h>

#include "mocked_foobar_extern.h"

Extern *g_extern;

extern "C"{
/*==============================================================================
                                Mock Callers
==============================================================================*/
uint8_t foo_extern_getNumber(uint8_t c_seed) {
   assert(g_extern != NULL);
   return g_extern->foo_extern_getNumber(c_seed);
}
} /* extern */

/*==============================================================================
                               Helper Functions
==============================================================================*/
void setMockExternPointer(Extern *externParam) {
   g_extern = externParam;
}
