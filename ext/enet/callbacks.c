/** 
 @file callbacks.c
 @brief ENet callback functions
*/
#define ENET_BUILDING_LIB 1
#include "enet/enet.h"

#ifndef _WIN32
enet_uint32
_time_get (void)
{
    struct timeval timeVal;

    gettimeofday(&timeVal, NULL);

    return timeVal.tv_sec * 1000 + timeVal.tv_usec / 1000 - timeBase;
}
#else
enet_uint32
_time_get (void)
{
    return (enet_uint32)timeGetTime();
}
#endif

static ENetCallbacks callbacks = { malloc, free, abort, _time_get };

int
enet_initialize_with_callbacks (ENetVersion version, const ENetCallbacks * inits)
{
   if (version < ENET_VERSION_CREATE (1, 3, 0))
     return -1;

   if (inits -> malloc != NULL || inits -> free != NULL)
   {
      if (inits -> malloc == NULL || inits -> free == NULL)
        return -1;

      callbacks.malloc = inits -> malloc;
      callbacks.free = inits -> free;
   }
      
   if (inits -> no_memory != NULL)
     callbacks.no_memory = inits -> no_memory;

   if (inits -> time_get != NULL)
     callbacks.time_get = inits -> time_get;

   return enet_initialize ();
}

ENetVersion
enet_linked_version (void)
{
    return ENET_VERSION;
}
           
void *
enet_malloc (size_t size)
{
   void * memory = callbacks.malloc (size);

   if (memory == NULL)
     callbacks.no_memory ();

   return memory;
}

void
enet_free (void * memory)
{
   callbacks.free (memory);
}

enet_uint32
enet_time_get (void)
{
    return callbacks.time_get();
}
