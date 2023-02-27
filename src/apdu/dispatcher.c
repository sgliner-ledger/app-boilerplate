/*****************************************************************************
 *   Ledger App Boilerplate.
 *   (c) 2020 Ledger SAS.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *****************************************************************************/

#include <stdint.h>
#include <stdbool.h>

#include "dispatcher.h"
#include "../constants.h"
#include "../globals.h"
#include "../types.h"
#include "../io.h"
#include "../sw.h"
#include "../common/buffer.h"
#include "../handler/get_version.h"
#include "../handler/get_app_name.h"
#include "../handler/get_public_key.h"
#include "../handler/sign_tx.h"

#include "nbgl_use_case.h"
#include "nbgl_layout.h"

// #include "menu.h"

extern uint8_t touch_debug[NB_TOUCH_DEBUG*TOUCH_DEBUG_LEN];
extern uint16_t point_idx;
extern uint8_t last_touch_state;
extern uint16_t G_ticks;

extern uint8_t step;
extern bool drawRect;

int apdu_dispatcher(const command_t *cmd) {
    if (cmd->cla != CLA) {
        return io_send_sw(SW_CLA_NOT_SUPPORTED);
    }

    buffer_t buf = {0};
    uint8_t sensi_buffer[224];

    switch (cmd->ins) {
        case GET_VERSION:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            return handler_get_version();
        case GET_APP_NAME:
            if (cmd->p1 != 0 || cmd->p2 != 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            return handler_get_app_name();
        case GET_PUBLIC_KEY:
            if (cmd->p1 > 1 || cmd->p2 > 0) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_get_public_key(&buf, (bool) cmd->p1);
        case SIGN_TX:
            if ((cmd->p1 == P1_START && cmd->p2 != P2_MORE) ||  //
                cmd->p1 > P1_MAX ||                             //
                (cmd->p2 != P2_LAST && cmd->p2 != P2_MORE)) {
                return io_send_sw(SW_WRONG_P1P2);
            }

            if (!cmd->data) {
                return io_send_sw(SW_WRONG_DATA_LENGTH);
            }

            buf.ptr = cmd->data;
            buf.size = cmd->lc;
            buf.offset = 0;

            return handler_sign_tx(&buf, cmd->p1, (bool) (cmd->p2 & P2_MORE));
        case TOUCH_DEBUG_GET:
            buf.ptr = touch_debug;
            buf.size = sizeof(touch_debug);
            buf.offset = 0;
            int ret = io_send_response(&buf, SW_OK);
            memset(touch_debug, 0xFF, sizeof(touch_debug));
            point_idx = 0;
            return ret;
        case TOUCH_DEBUG_CLEAR:
            memset(touch_debug, 0xFF, sizeof(touch_debug));
            point_idx = 0;
            G_ticks = 0;
            return io_send_sw(SW_OK);
        case TOUCH_DEBUG_READ_SENSI:
            io_seproxyhal_touch_read_sensi(sensi_buffer);
            buf.ptr = sensi_buffer;
            buf.size = sizeof(sensi_buffer);
            buf.offset = 0;
            return io_send_response(&buf, SW_OK);
        case INFO:
            drawRect = false;
            uint8_t step = cmd->p1;
            nbgl_test_info(step);
            return io_send_sw(SW_OK);
        case DRAW_RECT:
            if (!drawRect) {
                return io_send_sw(SW_WRONG_UX_STEP);
            }
            else {
                if ((cmd->p1 != 0x00) || (cmd->p2 != 0x00)) {
                    return io_send_sw(SW_WRONG_P1P2);
                }
                if (cmd->lc != 0x08) {
                    return io_send_sw(SW_WRONG_DATA_LENGTH);
                }
                nbgl_fullScreenClear(WHITE, false);
                nbgl_area_t rect_area = {
                    .x0 = ((cmd->data[0])<<8) | cmd->data[1],
                    .y0 = ((cmd->data[2])<<8) | cmd->data[3],
                    .width = ((cmd->data[4])<<8) | cmd->data[5],
                    .height = ((cmd->data[6])<<8) | cmd->data[7],
                    // .bpp = NBGL_BPP_1,
                    .backgroundColor = BLACK,
                };
                nbgl_frontDrawRect(&rect_area);
                rect_area.x0 = 0,
                rect_area.y0 = 0,
                rect_area.width = SCREEN_WIDTH,
                rect_area.height = SCREEN_HEIGHT,
                rect_area.backgroundColor = WHITE,
                nbgl_frontRefreshArea(&rect_area, FULL_COLOR_REFRESH);
                return io_send_sw(SW_OK);
            }

        case QUIT_APP:
            app_exit();
            return io_send_sw(SW_OK);
        default:
            return io_send_sw(SW_INS_NOT_SUPPORTED);
    }
}
