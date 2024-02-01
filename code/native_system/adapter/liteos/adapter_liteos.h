#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include "string.h"
#include "osi_api.h"
#include "osi_log.h"
#include "cmsis_os2.h"
#include "los_compiler.h"

// arch相关
uint32_t ArchIntLock(void);
void ArchIntRestore(uint32_t intSave);

// los相关mutex
LITE_OS_SEC_TEXT_INIT UINT32 LOS_MuxCreate(UINT32* muxHandle);
UINT32 LOS_MuxPend(UINT32 muxHandle, UINT32 timeout);
UINT32 LOS_MuxPost(UINT32 muxHandle);
UINT32 LOS_MuxDelete(UINT32* muxHandle);
UINT64 LOS_TickCountGet();
// thread相关
void* osThreadGetArgument(void);
