/*
 * Copyright (c) 2022 Talkweb Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <sys/mount.h>
#include "littlefs.h"
#include "los_config.h"
#include "hdf_log.h"
#include "hdf_device_desc.h"
#ifdef LOSCFG_DRIVERS_HDF_CONFIG_MACRO
#include "hcs_macro.h"
#include "hdf_config_macro.h"
#else
#include "device_resource_if.h"
#endif
#include <sys/stat.h>
#include <dirent.h>

#define LITTLEFS_PHYS_ADDR 0x800000

#define READ_SIZE      64
#define PROG_SIZE      64
#define BLOCK_SIZE     4096
#define BLOCK_COUNT    2048
#define CACHE_SIZE     64
#define LOOKAHEAD_SIZE 64
#define BLOCK_CYCLES   16
#define ERASE_FLASH_BULK 0

struct fs_cfg {
    char *mount_point;
    struct lfs_config lfs_cfg;
};

static struct fs_cfg fs[LOSCFG_LFS_MAX_MOUNT_SIZE] = {0};
#ifdef LOSCFG_DRIVERS_HDF_CONFIG_MACRO
#define DISPLAY_MISC_FS_LITTLEFS_CONFIG HCS_NODE(HCS_NODE(HCS_NODE(HCS_ROOT, misc), fs_config), littlefs_config)
static uint32_t FsGetResource(struct fs_cfg *fs)
{
    int32_t num = HCS_ARRAYS_SIZE(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, mount_points));
    if (num < 0 || num > LOSCFG_LFS_MAX_MOUNT_SIZE) {
        HDF_LOGE("%s: invalid mount_points num %d", __func__, num);
        return HDF_FAILURE;
    }
    char * mount_points[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, mount_points));
    uint32_t partitions[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, partitions));
    uint32_t block_size[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, block_size));
    uint32_t block_count[] = HCS_ARRAYS(HCS_NODE(DISPLAY_MISC_FS_LITTLEFS_CONFIG, block_count));
    for (int32_t i = 0; i < num; i++) {
        fs[i].mount_point = mount_points[i];
        fs[i].lfs_cfg.context = partitions[i];
        fs[i].lfs_cfg.block_size = block_size[i];
        fs[i].lfs_cfg.block_count = block_count[i];

        HDF_LOGI("%s: fs[%d] mount_point=%s, partition=%u, block_size=%u, block_count=%u",
                 __func__, i, fs[i].mount_point, (uint32_t)fs[i].lfs_cfg.context, 
                 fs[i].lfs_cfg.block_size, fs[i].lfs_cfg.block_count);
    }
    return HDF_SUCCESS;
}
#else
static uint32_t FsGetResource(struct fs_cfg *fs, const struct DeviceResourceNode *resourceNode)
{
    struct DeviceResourceIface *resource = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (resource == NULL) {
        HDF_LOGE("Invalid DeviceResourceIface");
        return HDF_FAILURE;
    }
    int32_t num = resource->GetElemNum(resourceNode, "mount_points");
    if (num < 0 || num > LOSCFG_LFS_MAX_MOUNT_SIZE) {
        HDF_LOGE("%s: invalid mount_points num %d", __func__, num);
        return HDF_FAILURE;
    }
    for (int32_t i = 0; i < num; i++) {
        if (resource->GetStringArrayElem(resourceNode, "mount_points", 
            i, &fs[i].mount_point, NULL) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get mount_points", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "partitions", 
            i, (uint32_t *)&fs[i].lfs_cfg.context, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get partitions", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "block_size", 
            i, &fs[i].lfs_cfg.block_size, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get block_size", __func__);
            return HDF_FAILURE;
        }
        if (resource->GetUint32ArrayElem(resourceNode, "block_count",
            i, &fs[i].lfs_cfg.block_count, 0) != HDF_SUCCESS) {
            HDF_LOGE("%s: failed to get block_count", __func__);
            return HDF_FAILURE;
        }
        HDF_LOGI("%s: fs[%d] mount_point=%s, partition=%u, block_size=%u, block_count=%u",
                 __func__, i,fs[i].mount_point, (uint32_t)fs[i].lfs_cfg.context, 
                 fs[i].lfs_cfg.block_size, fs[i].lfs_cfg.block_count);
    }
    return HDF_SUCCESS;
}
#endif
static int32_t FsDriverCheck(struct HdfDeviceObject *object)
{
    HDF_LOGI("Fs Driver Init\n");
    if (object == NULL) {
        return HDF_FAILURE;
    }
#ifdef LOSCFG_DRIVERS_HDF_CONFIG_MACRO
    if (FsGetResource(fs) != HDF_SUCCESS) {
        HDF_LOGE("%s: FsGetResource failed", __func__);
        return HDF_FAILURE;
    }
#else
    if (object->property) {
        if (FsGetResource(fs, object->property) != HDF_SUCCESS) {
            HDF_LOGE("%s: FsGetResource failed", __func__);
            return HDF_FAILURE;
        }
    }
#endif

    if (W25x_InitSpiFlash(0, 0) != 0) {
        HDF_LOGI("InitSpiFlash failed\n");
        return HDF_FAILURE;
    }

#if (ERASE_FLASH_BULK == 1)
    W25x_BulkErase();
#endif
    return HDF_SUCCESS;
}

static int32_t FsDriverInit(struct HdfDeviceObject *object)
{
    if (HDF_SUCCESS != FsDriverCheck(object))
        return HDF_FAILURE;

    DIR *dir = NULL;
    for (int i = 0; i < sizeof(fs) / sizeof(fs[0]); i++) {
        if (fs[i].mount_point == NULL)
            continue;

        fs[i].lfs_cfg.read = LittlefsRead;
        fs[i].lfs_cfg.prog = LittlefsProg;
        fs[i].lfs_cfg.erase = LittlefsErase;
        fs[i].lfs_cfg.sync = LittlefsSync;
        fs[i].lfs_cfg.read_size = READ_SIZE;
        fs[i].lfs_cfg.prog_size = PROG_SIZE;
        fs[i].lfs_cfg.cache_size = CACHE_SIZE;
        fs[i].lfs_cfg.lookahead_size = LOOKAHEAD_SIZE;
        fs[i].lfs_cfg.block_cycles = BLOCK_CYCLES;

        int ret = mount(NULL, fs[i].mount_point, "littlefs", 0, &fs[i].lfs_cfg);
        HDF_LOGI("%s: mount fs on '%s' %s\n", __func__, fs[i].mount_point, (ret == 0) ? "succeed" : "failed");
        if ((dir = opendir(fs[i].mount_point)) == NULL) {
            HDF_LOGI("first time create file %s\n", fs[i].mount_point);
            ret = mkdir(fs[i].mount_point, S_IRUSR | S_IWUSR);
            if (ret != LOS_OK) {
                HDF_LOGE("Mkdir failed %d\n", ret);
                return;
            } else {
                HDF_LOGI("mkdir success %d\n", ret);
            }
        } else {
            HDF_LOGI("open dir success!\n");
            closedir(dir);
        }
    }
    return HDF_SUCCESS;
}

static int32_t FsDriverBind(struct HdfDeviceObject *device)
{
    (void)device;
    return HDF_SUCCESS;
}

static void FsDriverRelease(struct HdfDeviceObject *device)
{
    (void)device;
    W25x_DeInitSpiFlash();
}

static struct HdfDriverEntry g_fsDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_FS_LITTLEFS",
    .Bind = FsDriverBind,
    .Init = FsDriverInit,
    .Release = FsDriverRelease,
};

HDF_INIT(g_fsDriverEntry);

