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
#include "iot_pwm.h"
#include "app_pwm.h"

#define PWM_IO_CONFIG           {{ APP_IO_TYPE_MSIO, APP_IO_MUX_0, APP_IO_PIN_0, APP_IO_NOPULL, APP_PWM_PIN_ENABLE }, \
                                 { APP_IO_TYPE_MSIO, APP_IO_MUX_0, APP_IO_PIN_1, APP_IO_NOPULL, APP_PWM_PIN_ENABLE }, \
                                 { APP_IO_TYPE_MSIO, APP_IO_MUX_0, APP_IO_PIN_2, APP_IO_NOPULL, APP_PWM_PIN_ENABLE }}

#define PWM_ACTIVE_CAHN          APP_PWM_ACTIVE_CHANNEL_ALL
#define PWM_CONFIG              { PWM_MODE_FLICKER, PWM_ALIGNED_EDGE, 10, 500, 200,  \
                                 { 50, PWM_DRIVEPOLARITY_POSITIVE }, \
                                 { 50, PWM_DRIVEPOLARITY_POSITIVE }, \
                                 { 50, PWM_DRIVEPOLARITY_POSITIVE }}
#define PWM_PARAM_CONFIG        {0, PWM_IO_CONFIG, PWM_ACTIVE_CAHN, PWM_CONFIG }

unsigned int IoTPwmInit(unsigned int port)
{
    uint16_t ret = APP_DRV_SUCCESS;
    app_pwm_params_t pwm_params = PWM_PARAM_CONFIG;

    if (port > APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    pwm_params.id = port;
    ret = app_pwm_init(&pwm_params);
    if (ret != APP_DRV_SUCCESS) {
        return IOT_FAILURE;
    }
    return IOT_SUCCESS;
}

unsigned int IoTPwmDeinit(unsigned int port)
{
    int ret = 0;

    if (port > APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    app_pwm_stop(port);
    ret = app_pwm_deinit(port);
    if (ret != 0) {
        return IOT_FAILURE;
    }
    return IOT_SUCCESS;
}

unsigned int IoTPwmStart(unsigned int port, unsigned short duty, unsigned int freq)
{
    app_pwm_params_t pwm_params = PWM_PARAM_CONFIG;
    app_pwm_channel_init_t channel_cfg = {0};

    if (port > APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    app_pwm_update_freq(port, freq);
    channel_cfg.duty = duty;
    channel_cfg.drive_polarity = PWM_DRIVEPOLARITY_POSITIVE;
    app_pwm_config_channel(port, APP_PWM_ACTIVE_CHANNEL_ALL, &channel_cfg);
    app_pwm_start(port);

    return IOT_SUCCESS;
}

unsigned int IoTPwmStop(unsigned int port)
{
    if (port > APP_PWM_ID_MAX) {
        return IOT_FAILURE;
    }

    app_pwm_stop(port);
    return IOT_SUCCESS;
}
