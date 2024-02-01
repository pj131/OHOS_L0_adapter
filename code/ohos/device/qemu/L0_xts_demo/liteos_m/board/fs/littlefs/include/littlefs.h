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

#ifndef _LITTLEFS_H_
#define _LITTLEFS_H_

#include "lfs.h"

#ifdef __cplusplus
extern "C" {
#endif

int32_t LittlefsRead(const struct lfs_config *cfg, lfs_block_t block,
                        lfs_off_t off, void *buffer, lfs_size_t size);
int32_t LittlefsProg(const struct lfs_config *cfg, lfs_block_t block,
                        lfs_off_t off, const void *buffer, lfs_size_t size);
int32_t LittlefsErase(const struct lfs_config *cfg, lfs_block_t block);
int32_t LittlefsSync(const struct lfs_config *cfg);

#ifdef __cplusplus
}
#endif

#endif /* _LITTLEFS_H_ */

