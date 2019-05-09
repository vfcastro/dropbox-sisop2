#ifndef __SERIALIZE__
#define __SERIALIZE__

#include "support.h"

void* serialize_pacote(Pacote *pkg);
Pacote unserialize_pacote(void *buffer);

#endif