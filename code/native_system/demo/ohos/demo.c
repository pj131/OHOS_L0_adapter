#include <stdio.h>
#include "CmUtils.h"
#include "CmSys.h"

#include "ohos_init.h"
#include "log.h"

///////OHOS DEMO thread////////////////////////////////////////////////////////////////////////////////

static void DemoSdkTask(void* arg)
{
    (void)arg;
    HILOG_INFO(HILOG_MODULE_APP, "DemoSdkTask start , arg:%p", arg);
    for (int n = 0; n < 1000; n++) {
        HILOG_INFO(HILOG_MODULE_APP, "DemoSdkTask hello ohos ++++++++++ %d", n);
        osiThreadSleep(5000);
    }
    HILOG_INFO(HILOG_MODULE_APP, "DemoSdkTask end");
}

void DemoSdkMain(void)
{
    CmTraceLog(LOG_LEVEL_INFO, "ohos DemoSdkMain start");
    osThreadAttr_t attr = {0};
    attr.stack_size     = 1024;
    attr.priority       = osPriorityNormal;
    attr.name           = "DemoSdk";
    if (osThreadNew((osThreadFunc_t)DemoSdkTask, NULL, &attr) == NULL) {
        HILOG_ERROR(HILOG_MODULE_APP, "Failed to create DemoSdkTask\r\n");
    }
}

APP_FEATURE_INIT(DemoSdkMain);
