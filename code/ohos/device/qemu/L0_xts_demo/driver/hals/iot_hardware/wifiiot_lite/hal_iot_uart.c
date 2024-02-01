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
#include "iot_uart.h"
#include "app_uart.h"
#include "app_io.h"
#include "los_sem.h"

#define UART_TIMEOUT     1000

static void app_uart0_callback(app_uart_evt_t *p_evt);
static void app_uart1_callback(app_uart_evt_t *p_evt);

static app_uart_params_t uart_param[APP_UART_ID_MAX] = {

    /* UART 0 */
    {
        .id = APP_UART_ID_0,
        .pin_cfg = {
            .tx = {
                .type = APP_IO_TYPE_NORMAL,
                .pin  = APP_IO_PIN_10,
                .mux  = APP_IO_MUX_2,
                .pull = APP_IO_PULLUP,
            },
            .rx = {
                .type = APP_IO_TYPE_NORMAL,
                .pin  = APP_IO_PIN_11,
                .mux  = APP_IO_MUX_2,
                .pull = APP_IO_PULLUP,
            }
        },
        .use_mode = {
            .type = APP_UART_TYPE_DMA,
            .tx_dma_channel = DMA_Channel0,
            .rx_dma_channel = DMA_Channel1,
        }
    },

    /* UART 1 */
    {
        .id = APP_UART_ID_1,
        .pin_cfg = {
            .tx = {
                .type = APP_IO_TYPE_NORMAL,
                .pin  = APP_IO_PIN_7,
                .mux  = APP_IO_MUX_3,
                .pull = APP_IO_PULLUP,
            },
            .rx = {
                .type = APP_IO_TYPE_NORMAL,
                .pin  = APP_IO_PIN_6,
                .mux  = APP_IO_MUX_3,
                .pull = APP_IO_PULLUP,
            }
        },
        .use_mode = {
            .type = APP_UART_TYPE_INTERRUPT, /* UART1 only supports interrupt mode */
        }
    },
};

static uint32_t uart_rx_sem[APP_UART_ID_MAX];
static uint32_t uart_tx_mutex[APP_UART_ID_MAX];
static uint32_t uart_rx_mutex[APP_UART_ID_MAX];
static uint32_t g_rx_num[APP_UART_ID_MAX];

static const app_uart_evt_handler_t *evt_handler[APP_UART_ID_MAX] = {
    app_uart0_callback,
    app_uart1_callback
};

static void app_uart0_callback(app_uart_evt_t *p_evt)
{
    if (p_evt->type == APP_UART_EVT_RX_DATA) {
        g_rx_num[APP_UART_ID_0] = p_evt->data.size;
        LOS_SemPost(uart_rx_sem[APP_UART_ID_0]);
    } else if (p_evt->type == APP_UART_EVT_ERROR) {
        LOS_SemPost(uart_rx_sem[APP_UART_ID_0]);
    }
}

static void app_uart1_callback(app_uart_evt_t *p_evt)
{
    if (p_evt->type == APP_UART_EVT_RX_DATA) {
        g_rx_num[APP_UART_ID_1] = p_evt->data.size;
        LOS_SemPost(uart_rx_sem[APP_UART_ID_1]);
    } else if (p_evt->type == APP_UART_EVT_ERROR) {
        LOS_SemPost(uart_rx_sem[APP_UART_ID_1]);
    }
}

struct app_uart_params_t* uart_cfg(unsigned int id, const IotUartAttribute *param)
{
    app_uart_params_t *params = &uart_param[id];
    params->init.baud_rate = param->baudRate;
    params->init.rx_timeout_mode = UART_RECEIVER_TIMEOUT_ENABLE;

    switch (param->dataBits) {
        case IOT_UART_DATA_BIT_5:
            params->init.data_bits  = UART_DATABITS_5;
            break;
        case IOT_UART_DATA_BIT_6:
            params->init.data_bits  = UART_DATABITS_6;
            break;
        case IOT_UART_DATA_BIT_7:
            params->init.data_bits  = UART_DATABITS_7;
            break;
        case IOT_UART_DATA_BIT_8:
            params->init.data_bits  = UART_DATABITS_8;
            break;
        default:
            break;
    }

    if (param->stopBits == IOT_UART_STOP_BIT_1) {
        params->init.stop_bits = UART_STOPBITS_1;
    } else if  (param->stopBits == IOT_UART_STOP_BIT_2) {
        params->init.stop_bits = UART_STOPBITS_2;
    }

    if (param->parity == IOT_UART_PARITY_NONE) {
        params->init.parity = UART_PARITY_NONE;
    } else if (param->parity == IOT_UART_PARITY_ODD) {
        params->init.parity = UART_PARITY_ODD;
    } else if (param->parity == IOT_UART_PARITY_EVEN) {
        params->init.parity = UART_PARITY_EVEN;
    }
    return params;
}

unsigned int IoTUartInit(unsigned int id, const IotUartAttribute *param)
{
    app_uart_tx_buf_t uart_buffer;
    app_uart_params_t *params;
    int ret = 0;
    uint32_t uwRet = 0;

    uart_buffer.tx_buf       = NULL;
    uart_buffer.tx_buf_size  = 0;

    if (param == NULL) {
        return IOT_FAILURE;
    }

    if (id >= APP_UART_ID_MAX) {
        return IOT_FAILURE;
    }
    params = uart_cfg(id, param);

    ret = app_uart_init(params, evt_handler[id], &uart_buffer);
    if (ret != 0) {
        return IOT_FAILURE;
    }

    uwRet = LOS_BinarySemCreate(0, &uart_rx_sem[id]);
    if (uwRet != LOS_OK) {
        return IOT_FAILURE;
    }

    uwRet = LOS_MuxCreate(&uart_tx_mutex[id]);
    if (uwRet != LOS_OK) {
        LOS_SemDelete(&uart_rx_sem[id]);
        return IOT_FAILURE;
    }

    uwRet = LOS_MuxCreate(&uart_rx_mutex[id]);
    if (uwRet != LOS_OK) {
        LOS_SemDelete(uart_rx_sem[id]);
        LOS_SemDelete(uart_tx_mutex[id]);
        return IOT_FAILURE;
    }

    return IOT_SUCCESS;
}

int IoTUartRead(unsigned int id, unsigned char *data, unsigned int dataLen)
{
    int ret = 0;
    uint32_t uwRet = 0;

    LOS_MuxPend(uart_rx_mutex[id], LOS_WAIT_FOREVER);

    g_rx_num[id] = 0;
    LOS_SemPend(uart_rx_sem[id], 0);
    ret = app_uart_receive_async(id, data, dataLen);
    if (ret != 0) {
        LOS_MuxPost(uart_rx_mutex[id]);
        return IOT_FAILURE;
    }

    uwRet = LOS_SemPend(uart_rx_sem[id], LOS_WAIT_FOREVER);
    if (uwRet != LOS_OK)  {
        LOS_MuxPost(uart_rx_mutex[id]);
        return IOT_FAILURE;
    }

    LOS_MuxPost(uart_rx_mutex[id]);

    return g_rx_num[id];
}

int IoTUartWrite(unsigned int id, const unsigned char *data, unsigned int dataLen)
{
    int ret = 0;

    LOS_MuxPend(uart_tx_mutex[id], LOS_WAIT_FOREVER);
    ret = app_uart_transmit_sync(id, data, dataLen, UART_TIMEOUT);
    if (ret != 0) {
        LOS_MuxPost(uart_tx_mutex[id]);
        return IOT_FAILURE;
    }
    LOS_MuxPost(uart_tx_mutex[id]);

    return IOT_SUCCESS;
}

unsigned int IoTUartDeinit(unsigned int id)
{
    app_uart_deinit(id);
    LOS_SemDelete(uart_rx_sem[id]);
    LOS_SemDelete(uart_tx_mutex[id]);
    LOS_SemDelete(uart_rx_mutex[id]);

    return IOT_SUCCESS;
}

unsigned int IoTUartSetFlowCtrl(unsigned int id, IotFlowCtrl flowCtrl)
{
    int ret = 0;
    app_uart_tx_buf_t uart_buffer;
    uart_buffer.tx_buf       = NULL;
    uart_buffer.tx_buf_size  = 0;

    if (id >= APP_UART_ID_MAX) {
        return IOT_FAILURE;
    }

    app_uart_params_t *params = &uart_param[id];
    switch (flowCtrl) {
        case IOT_FLOW_CTRL_NONE:
            params->init.hw_flow_ctrl = UART_HWCONTROL_NONE;
            break;
        case IOT_FLOW_CTRL_RTS_CTS:
            params->init.hw_flow_ctrl = UART_HWCONTROL_RTS_CTS;
            break;
        case IOT_FLOW_CTRL_RTS_ONLY:
        case IOT_FLOW_CTRL_CTS_ONLY:
            return IOT_FAILURE;  /* not support */
            break;
        default:
            break;
    }

    app_uart_deinit(params->id);
    ret = app_uart_init(params, evt_handler[id], &uart_buffer);
    if (ret != 0) {
        return IOT_FAILURE;
    }

    return IOT_SUCCESS;
}
