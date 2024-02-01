/*
 * Copyright (c) 2021 GOODIX.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "iot_errno.h"
#include "iot_i2c.h"
#include "app_i2c.h"
#include "los_sem.h"

/* I2C0 config params */
/* SCL : GPIO30 */
#define USER_I2C0_SCL_PIN          APP_IO_PIN_30
#define USER_I2C0_SCL_PIN_TYPE     APP_IO_TYPE_NORMAL
#define USER_I2C0_SCL_PIN_MUX      APP_IO_MUX_2
#define USER_I2C0_SCL_PIN_PULL     APP_IO_PULLUP

/* SDA  : GPIO26 */
#define USER_I2C0_SDA_PIN          APP_IO_PIN_26
#define USER_I2C0_SDA_PIN_TYPE     APP_IO_TYPE_NORMAL
#define USER_I2C0_SDA_PIN_MUX      APP_IO_MUX_2
#define USER_I2C0_SDA_PIN_PULL     APP_IO_PULLUP
#define USER_I2C0_SPEED            I2C_SPEED_400K
#define I2C0_IO_CONFIG  {{USER_I2C0_SCL_PIN_TYPE, USER_I2C0_SCL_PIN_MUX, USER_I2C0_SCL_PIN, USER_I2C0_SCL_PIN_PULL}, \
                         {USER_I2C0_SDA_PIN_TYPE, USER_I2C0_SDA_PIN_MUX, USER_I2C0_SDA_PIN, USER_I2C0_SDA_PIN_PULL}}
#define I2C0_MODE_CONFIG           {APP_I2C_TYPE_INTERRUPT, DMA_Channel0, DMA_Channel0}
#define I2C0_I2C_CONFIG            {USER_I2C0_SPEED, 0x00, I2C_ADDRESSINGMODE_7BIT, I2C_GENERALCALL_DISABLE}
#define I2C0_PARAM_CONFIG    {APP_I2C_ID_0, APP_I2C_ROLE_MASTER, I2C0_IO_CONFIG, I2C0_MODE_CONFIG, I2C0_I2C_CONFIG}

/* I2C1 config params */
/* SCL : GPIO8 */
#define USER_I2C1_SCL_PIN           APP_IO_PIN_8
#define USER_I2C1_SCL_PIN_TYPE      APP_IO_TYPE_NORMAL
#define USER_I2C1_SCL_PIN_MUX       APP_IO_MUX_1
#define USER_I2C1_SCL_PIN_PULL      APP_IO_PULLUP

/* SDA : GPIO9 */
#define USER_I2C1_SDA_PIN           APP_IO_PIN_9
#define USER_I2C1_SDA_PIN_TYPE      APP_IO_TYPE_NORMAL
#define USER_I2C1_SDA_PIN_MUX       APP_IO_MUX_1
#define USER_I2C1_SDA_PIN_PULL      APP_IO_PULLUP
#define USER_I2C1_SPEED             I2C_SPEED_400K

#define I2C1_IO_CONFIG  {{USER_I2C1_SCL_PIN_TYPE, USER_I2C1_SCL_PIN_MUX, USER_I2C1_SCL_PIN, USER_I2C1_SCL_PIN_PULL}, \
                         {USER_I2C1_SDA_PIN_TYPE, USER_I2C1_SDA_PIN_MUX, USER_I2C1_SDA_PIN, USER_I2C1_SDA_PIN_PULL}}
#define I2C1_MODE_CONFIG            {APP_I2C_TYPE_INTERRUPT, DMA_Channel0, DMA_Channel0}
#define I2C1_I2C_CONFIG             {USER_I2C1_SPEED, 0x00, I2C_ADDRESSINGMODE_7BIT, I2C_GENERALCALL_DISABLE}
#define I2C1_PARAM_CONFIG    {APP_I2C_ID_1, APP_I2C_ROLE_MASTER, I2C1_IO_CONFIG, I2C1_MODE_CONFIG, I2C1_I2C_CONFIG}
#define I2C_SYNC_TIMEOUT            20

#define I2C_BAUDRATE_100K           100
#define I2C_BAUDRATE_400K           400
#define I2C_BAUDRATE_1000K          1000
#define I2C_BAUDRATE_2000K          2000

static const app_i2c_params_t i2c_cfg_params[APP_I2C_ID_MAX] = {
    I2C0_PARAM_CONFIG,
    I2C1_PARAM_CONFIG
};
static uint32_t i2c_tx_mutex[APP_I2C_ID_MAX];
static uint32_t i2c_rx_mutex[APP_I2C_ID_MAX];

unsigned int IoTI2cWrite(unsigned int id, unsigned short deviceAddr, const unsigned char *data, unsigned int dataLen)
{
    int ret = 0;

    if (id > APP_I2C_ID_MAX) {
        return IOT_FAILURE;
    }

    LOS_MuxPend(i2c_tx_mutex[id], LOS_WAIT_FOREVER);
    ret = app_i2c_transmit_sync(id, deviceAddr, data, dataLen, I2C_SYNC_TIMEOUT);
    if (ret != 0) {
        printf("ret=%d\r\n", ret);
        LOS_MuxPost(i2c_tx_mutex[id]);
        return IOT_FAILURE;
    }
    LOS_MuxPost(i2c_tx_mutex[id]);

    return IOT_SUCCESS;
}

unsigned int IoTI2cRead(unsigned int id, unsigned short deviceAddr, unsigned char *data, unsigned int dataLen)
{
    int ret = 0;

    LOS_MuxPend(i2c_rx_mutex[id], LOS_WAIT_FOREVER);
    ret = app_i2c_receive_sync(id, deviceAddr, data, dataLen, I2C_SYNC_TIMEOUT);
    if (ret != 0) {
        LOS_MuxPost(i2c_rx_mutex[id]);
        return IOT_FAILURE;
    }
    LOS_MuxPost(i2c_rx_mutex[id]);

    return IOT_SUCCESS;
}

unsigned int IoTI2cInit(unsigned int id, unsigned int baudrate)
{
    int ret = 0;
    app_i2c_params_t i2c_params;
    uint32_t uwRet;

    if (id > APP_I2C_ID_MAX) {
        return IOT_FAILURE;
    }

    memcpy_s(&i2c_params, sizeof(app_i2c_params_t), &i2c_cfg_params[id], sizeof(app_i2c_params_t));
    if (baudrate == I2C_BAUDRATE_100K) {
        i2c_params.init.speed = I2C_SPEED_100K;
    } else if (baudrate == I2C_BAUDRATE_400K) {
        i2c_params.init.speed = I2C_SPEED_400K;
    }  else if (baudrate == I2C_BAUDRATE_1000K) {
        i2c_params.init.speed = I2C_SPEED_1000K;
    } else if (baudrate == I2C_BAUDRATE_2000K) {
        i2c_params.init.speed = I2C_SPEED_2000K;
    } else {
        return IOT_FAILURE;
    }

    ret = app_i2c_init(&i2c_params, NULL);
    if (ret != 0) {
        return IOT_FAILURE;
    }

    uwRet = LOS_MuxCreate(&i2c_tx_mutex[id]);
    if (uwRet != LOS_OK) {
        return IOT_FAILURE;
    }

    uwRet = LOS_MuxCreate(&i2c_rx_mutex[id]);
    if (uwRet != LOS_OK) {
        LOS_SemDelete(i2c_tx_mutex[id]);
        return IOT_FAILURE;
    }

    return IOT_SUCCESS;
}

unsigned int IoTI2cDeinit(unsigned int id)
{
    if (id > APP_I2C_ID_MAX) {
        return IOT_FAILURE;
    }

    app_i2c_deinit(id);
    LOS_SemDelete(i2c_tx_mutex[id]);
    LOS_SemDelete(i2c_rx_mutex[id]);

    return IOT_SUCCESS;
}

unsigned int IoTI2cSetBaudrate(unsigned int id, unsigned int baudrate)
{
    int ret = 0;
    app_i2c_params_t i2c_params;

    if (id > APP_I2C_ID_MAX) {
        return IOT_FAILURE;
    }

    memcpy_s(&i2c_params, sizeof(app_i2c_params_t), &i2c_cfg_params[id], sizeof(app_i2c_params_t));
    if (baudrate == I2C_BAUDRATE_100K) {
        i2c_params.init.speed = I2C_SPEED_100K;
    } else if (baudrate == I2C_BAUDRATE_400K) {
        i2c_params.init.speed = I2C_SPEED_400K;
    }  else if (baudrate == I2C_BAUDRATE_1000K) {
        i2c_params.init.speed = I2C_SPEED_1000K;
    } else if (baudrate == I2C_BAUDRATE_2000K) {
        i2c_params.init.speed = I2C_SPEED_2000K;
    } else {
        return IOT_FAILURE;
    }

    app_i2c_deinit(id);
    ret = app_i2c_init(&i2c_params, NULL);
    if (ret != 0) {
        return IOT_FAILURE;
    }

    return IOT_SUCCESS;
}
