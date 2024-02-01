/* Copyright (C) 2018 RDA Technologies Limited and/or its affiliates("RDA").
 * All rights reserved.
 *
 * This software is supplied "AS IS" without any warranties.
 * RDA assumes no responsibility or liability for the use of the software,
 * conveys no license or title under any patent, copyright, or mask work
 * right to the product. RDA reserves the right to make changes in the
 * software without notification.  RDA also make no representation or
 * warranty that such application will be suitable for the specified use
 * without further testing or modification.
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "CmSys.h"
#include "CmFs.h"
#include "CmUtils.h"

#include "ohos_init.h"
#include "log.h"
#include "adapter.h"
#include "securec.h"
#include "bundle_info.h"
#include "bundle_manager.h"
#include "want.h"
////////app start//////////////////////////////////////////////////////////////////////////////////////

extern void OHOS_SystemInit();
static bool ohos_started = false;
int ohos_start(int argc, char** argv)
{
    if (ohos_started) {
        CmTraceLog(LOG_LEVEL_INFO, "ohos started!");
        return 0;
    }
    CmTraceLog(LOG_LEVEL_INFO, "OHOS_SystemInit start");
    ohos_started = true;
    OHOS_SystemInit();
    CmTraceLog(LOG_LEVEL_INFO, "OHOS_SystemInit done");
    return 0;
}

#define CREATE_FILE(n)                 \
    do {                               \
        int fd = vfs_open(n, O_CREAT); \
        vfs_close(fd);                 \
    } while (0)

int appimg_enter(void* param)
{
    CmTraceLog(LOG_LEVEL_INFO, "application image start");

    // vfs_rmdir_recursive("/data");
    // vfs_rmdir_recursive("/dev");
    vfs_mkdir("/data", 0);
    vfs_mkdir("/dev", 0);
    vfs_mkdir("/data/storage", 0);
    CREATE_FILE("/data/storage/hks_keystore");
    CREATE_FILE("/data/persist_parameters");
    CREATE_FILE("/data/tmp_persist_parameters");

    CmUSerialAddCommand("ohos_start", ohos_start, "Start OpenHarmony");
    return 0;
}

void appimg_exit(void)
{
    CmTraceLog(LOG_LEVEL_INFO, "application image exit");
}
