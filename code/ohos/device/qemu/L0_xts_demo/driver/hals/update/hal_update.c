/*
 * Copyright (c) 2021-2022 Talkweb Co., Ltd.
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

#include "fcntl.h"
#include "securec.h"
#include "sys/stat.h"
#include "utils_file.h"

#include "hal_hota_board.h"

int HotaHalInit(void) { return 0; }

int HotaHalDeInit(void) { return -1; }

int HotaHalGetUpdateIndex(unsigned int *index) { return 0; }

int HotaHalWrite(int partition, unsigned char *buffer, unsigned int offset, unsigned int bufLen) { return 0; }

int HotaHalRead(int partition, unsigned int offset, unsigned int bufLen, unsigned char *buffer) { return -1; }

int HotaHalSetBootSettings(void) { return 0; }

int HotaHalRestart(void) { return 0; }

int HotaHalRollback(void) { return 0; }

const ComponentTableInfo *HotaHalGetPartitionInfo(void) { return 0; }

unsigned char *HotaHalGetPubKey(unsigned int *length) { return 0; }

int HotaHalGetUpdateAbility(void) { return 0; }

int HotaHalGetOtaPkgPath(char *path, int len) { return 0; }

int HotaHalIsDeviceCanReboot(void) { return 0; }

int HotaHalIsDevelopMode(void) { return 0; }

int HotaHalGetMetaData(UpdateMetaData *metaData) { return 0; }

int HotaHalSetMetaData(UpdateMetaData *metaData) { return 0; }

int HotaHalRebootAndCleanUserData(void) { return 0; }

int HotaHalRebootAndCleanCache(void) { return 0; }

int HotaHalCheckVersionValid(const char *currentVersion, const char *pkgVersion, unsigned int pkgVersionLength) { return 0; }
