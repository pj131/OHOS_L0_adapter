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
#include "iot_gpio.h"
#include "app_io.h"
#include "app_gpiote.h"
#include "stdbool.h"


/* ID 0~31    Normal GPIO */
/* ID 32~39   AON    GPIO */
/* ID 40~45   MSIO   GPIO */
typedef enum {
    NORMAL_GPIO0  = 0,
    NORMAL_GPIO1  = 1,
    NORMAL_GPIO2  = 2,
    NORMAL_GPIO3  = 3,
    NORMAL_GPIO4  = 4,
    NORMAL_GPIO5  = 5,
    NORMAL_GPIO6  = 6,
    NORMAL_GPIO7  = 7,
    NORMAL_GPIO8  = 8,
    NORMAL_GPIO9  = 9,
    NORMAL_GPIO10 = 10,
    NORMAL_GPIO11 = 11,
    NORMAL_GPIO12 = 12,
    NORMAL_GPIO13 = 13,
    NORMAL_GPIO14 = 14,
    NORMAL_GPIO15 = 15,
    NORMAL_GPIO16 = 16,
    NORMAL_GPIO17 = 17,
    NORMAL_GPIO18 = 18,
    NORMAL_GPIO19 = 19,
    NORMAL_GPIO20 = 20,
    NORMAL_GPIO21 = 21,
    NORMAL_GPIO22 = 22,
    NORMAL_GPIO23 = 23,
    NORMAL_GPIO24 = 24,
    NORMAL_GPIO25 = 25,
    NORMAL_GPIO26 = 26,
    NORMAL_GPIO27 = 27,
    NORMAL_GPIO28 = 28,
    NORMAL_GPIO29 = 29,
    NORMAL_GPIO30 = 30,
    NORMAL_GPIO31 = 31,
    NORMAL_GPIO_MAX = NORMAL_GPIO31,

    AON_GPIO0 = 32,
    AON_GPIO1 = 33,
    AON_GPIO2 = 34,
    AON_GPIO3 = 35,
    AON_GPIO4 = 36,
    AON_GPIO5 = 37,
    AON_GPIO6 = 38,
    AON_GPIO7 = 39,
    AON_GPIO_MAX = AON_GPIO7,

    MSIO_GPIO0 = 40,
    MSIO_GPIO1 = 41,
    MSIO_GPIO2 = 42,
    MSIO_GPIO3 = 43,
    MSIO_GPIO4 = 44,
    MSIO_GPIO_MAX = MSIO_GPIO4
} IotGpioID;

typedef struct {
    app_io_mode_t mode;
    bool initialized;
    GpioIsrCallbackFunc func;
    char *arg;
} isr_cfg_info_t;

#define GR551X_AON_GPIO_NUM          (8)
#define GR551X_AON_GPIO_START_IDX    (AON_GPIO_MAX - GR551X_AON_GPIO_NUM + 1)
#define GR551X_MSIO_GPIO_NUM         (5)
#define GR551X_MSIO_GPIO_START_IDX   (MSIO_GPIO_MAX - GR551X_MSIO_GPIO_NUM + 1)
#define GR551X_GPIO_IDX_MAX          (45)
#define GR551X_INT_GPIO_IDX_MAX      (40)  /* MSIO pin does not support interrupts */

static uint8_t g_gpio_dir[GR551X_GPIO_IDX_MAX];
static uint8_t g_gpio_out[GR551X_GPIO_IDX_MAX];
static isr_cfg_info_t isr_cfg_info[GR551X_INT_GPIO_IDX_MAX];

static inline app_io_type_t pin_map(uint32_t id, uint32_t *pin)
{
    if (id <= NORMAL_GPIO_MAX) {
        *pin = (1UL << id);
        return APP_IO_TYPE_NORMAL;
    } else if (id <= AON_GPIO_MAX) {
        *pin = (1UL << (id - GR551X_AON_GPIO_START_IDX));
        return APP_IO_TYPE_AON;
    } else if (id <= MSIO_GPIO_MAX) {
        *pin = (1UL << (id - GR551X_MSIO_GPIO_START_IDX));
        return APP_IO_TYPE_MSIO;
    }
}

static int get_pin_index(uint32_t pin)
{
    int index = 0;
    while ((pin & 1) != 1) {
        index++;
        pin = pin >> 1;
    }
    return index;
}

static void app_io_callback(app_gpiote_evt_t *p_evt)
{
    uint32_t index = 0;

    if (p_evt->type == APP_IO_TYPE_NORMAL) {
        index = get_pin_index(p_evt->pin);
    } else if (p_evt->type == APP_IO_TYPE_AON) {
        index = get_pin_index(p_evt->pin) + GR551X_AON_GPIO_START_IDX;
    }

    if (isr_cfg_info[index].func != NULL) {
        isr_cfg_info[index].func(isr_cfg_info[index].arg);
    }
}


unsigned int IoTGpioInit(unsigned int id)
{
    if (id > GR551X_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }
    return IOT_SUCCESS;
}

unsigned int IoTGpioSetDir(unsigned int id, IotGpioDir dir)
{
    app_io_init_t io_init;
    uint32_t pin = 0;
    app_io_type_t io_type;

    if (id > GR551X_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    io_type = pin_map(id, &pin);
    io_init.pin  = pin;
    io_init.pull = APP_IO_PULLUP;
    io_init.mux  = APP_IO_MUX_7;

    if (dir == IOT_GPIO_DIR_IN) {
        io_init.mode = APP_IO_MODE_INPUT;
    } else if (dir == IOT_GPIO_DIR_OUT) {
        io_init.mode = APP_IO_MODE_OUT_PUT;
    }

    g_gpio_dir[id] = dir;
    app_io_init(io_type, &io_init);

    return IOT_SUCCESS;
}

unsigned int IoTGpioGetDir(unsigned int id, IotGpioDir *dir)
{
    if (id > GR551X_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    *dir = g_gpio_dir[id];
    return IOT_SUCCESS;
}

unsigned int IoTGpioSetOutputVal(unsigned int id, IotGpioValue val)
{
    app_io_init_t io_init;
    uint32_t pin = 0;
    app_io_type_t io_type;

    if (id > GR551X_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    io_type = pin_map(id, &pin);

    if (val == IOT_GPIO_VALUE0) {
        app_io_write_pin(io_type, pin, APP_IO_PIN_RESET);
    } else if (val == IOT_GPIO_VALUE1) {
        app_io_write_pin(io_type, pin, APP_IO_PIN_SET);
    }
    g_gpio_out[id] = val;

    return IOT_SUCCESS;
}

unsigned int IoTGpioGetOutputVal(unsigned int id, IotGpioValue *val)
{
    if (id > GR551X_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    *val = g_gpio_out[id];

    return IOT_SUCCESS;
}

unsigned int IoTGpioGetInputVal(unsigned int id, IotGpioValue *val)
{
    app_io_init_t io_init;
    uint32_t pin = 0;
    app_io_type_t io_type;

    if (id > GR551X_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    io_type = pin_map(id, &pin);
    *val = app_io_read_pin(io_type, pin);

    return IOT_SUCCESS;
}

unsigned int IoTGpioRegisterIsrFunc(unsigned int id, IotGpioIntType intType, IotGpioIntPolarity intPolarity,
                                    GpioIsrCallbackFunc func, char *arg)
{
    app_io_init_t io_init;
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_gpiote_param_t gpiote_param;

    if (id > GR551X_INT_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    io_type = pin_map(id, &pin);
    gpiote_param.type = io_type;
    gpiote_param.pin  = pin;
    gpiote_param.pull = APP_IO_PULLUP;
    gpiote_param.handle_mode = APP_IO_ENABLE_WAKEUP;
    if (intType == IOT_INT_TYPE_LEVEL) {
        if (intPolarity == IOT_GPIO_EDGE_FALL_LEVEL_LOW) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_LOW;
        } else if (intPolarity == IOT_GPIO_EDGE_RISE_LEVEL_HIGH) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_HIGH;
        }
    } else if (intType == IOT_INT_TYPE_EDGE) {
        if (intPolarity == IOT_GPIO_EDGE_FALL_LEVEL_LOW) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_FALLING;
        } else if (intPolarity == IOT_GPIO_EDGE_RISE_LEVEL_HIGH) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_RISING;
        }
    }

    isr_cfg_info[id].func = func;
    isr_cfg_info[id].arg  = arg;

    gpiote_param.mode = isr_cfg_info[id].mode;
    gpiote_param.io_evt_cb = app_io_callback;

    if (isr_cfg_info[id].initialized == false) {
        isr_cfg_info[id].initialized = true;
        app_gpiote_init(&gpiote_param, 1);
    } else {
        app_gpiote_config(&gpiote_param);
    }
    return IOT_SUCCESS;
}

unsigned int IoTGpioUnregisterIsrFunc(unsigned int id)
{
    app_io_init_t io_init;
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_gpiote_param_t gpiote_param;

    if (id > GR551X_INT_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    isr_cfg_info[id].func = NULL;
    isr_cfg_info[id].arg  = NULL;

    io_type = pin_map(id, &pin);
    gpiote_param.type = io_type;
    gpiote_param.pin  = pin;
    gpiote_param.mode = isr_cfg_info[id].mode;
    gpiote_param.pull = APP_IO_PULLUP;
    gpiote_param.handle_mode = APP_IO_ENABLE_WAKEUP;
    gpiote_param.io_evt_cb = NULL;

    if (isr_cfg_info[id].initialized == false) {
        isr_cfg_info[id].initialized = true;
        app_gpiote_init(&gpiote_param, 1);
    } else {
        app_gpiote_config(&gpiote_param);
    }

    return IOT_SUCCESS;
}

unsigned int IoTGpioSetIsrMask(unsigned int id, unsigned char mask)
{
    if (id > GR551X_INT_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    return IOT_SUCCESS;
}

unsigned int IoTGpioSetIsrMode(unsigned int id, IotGpioIntType intType, IotGpioIntPolarity intPolarity)
{
    if (id > GR551X_INT_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    app_io_init_t io_init;
    uint32_t pin = 0;
    app_io_type_t io_type;
    app_gpiote_param_t gpiote_param;

    io_type = pin_map(id, &pin);
    gpiote_param.type = io_type;
    gpiote_param.pin  = pin;
    gpiote_param.pull = APP_IO_PULLUP;
    gpiote_param.handle_mode = APP_IO_ENABLE_WAKEUP;
    gpiote_param.io_evt_cb = app_io_callback;

    if (intType == IOT_INT_TYPE_LEVEL) {
        if (intPolarity == IOT_GPIO_EDGE_FALL_LEVEL_LOW) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_LOW;
        } else if (intPolarity == IOT_GPIO_EDGE_RISE_LEVEL_HIGH) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_HIGH;
        }
    } else if (intType == IOT_INT_TYPE_EDGE) {
        if (intPolarity == IOT_GPIO_EDGE_FALL_LEVEL_LOW) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_FALLING;
        } else if (intPolarity == IOT_GPIO_EDGE_RISE_LEVEL_HIGH) {
            isr_cfg_info[id].mode = APP_IO_MODE_IT_RISING;
        }
    }

    gpiote_param.mode = isr_cfg_info[id].mode;

    if (isr_cfg_info[id].initialized == false) {
        isr_cfg_info[id].initialized = true;
        app_gpiote_init(&gpiote_param, 1);
    } else {
        app_gpiote_config(&gpiote_param);
    }

    return IOT_SUCCESS;
}

unsigned int IoTGpioDeinit(unsigned int id)
{
    app_io_init_t io_init;
    uint32_t pin = 0;
    app_io_type_t io_type;

    if (id > GR551X_GPIO_IDX_MAX) {
        return IOT_FAILURE;
    }

    io_type = pin_map(id, &pin);
    app_io_deinit(io_type, pin);
    g_gpio_dir[id] = 0;
    g_gpio_out[id] = 0;

    return IOT_SUCCESS;
}
