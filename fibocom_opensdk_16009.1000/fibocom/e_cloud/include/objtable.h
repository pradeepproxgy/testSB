#ifndef __OBJ_TABLE_H__
#define __OBJ_TABLE_H__

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

bool ecloud_add_mult_objinsts(lwm2m_context_t *contextP, uint16_t objid, uint32_t instCount, uint8_t *bitmap);
bool ecloud_lwm2m_isResDiscover(uint16_t objId, uint8_t ref);
void ecloud_lwm2m_resetResDiscoverState(uint16_t objId, uint8_t ref);

#endif
