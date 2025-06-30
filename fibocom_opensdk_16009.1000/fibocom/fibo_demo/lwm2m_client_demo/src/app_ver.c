#include "osi_api.h"
#include "osi_log.h"
#include "fibo_opencpu.h"

void set_app_ver(char *ver)
{
     fibo_app_version_set(ver);
}
