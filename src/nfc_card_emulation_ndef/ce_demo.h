#ifndef CE_DEMO_H
#define CE_DEMO_H

#include "buffer.h"
#include "io.h"
#include "sw.h"
#include "parser.h"

#define NDEF_SIZE           100      /*!< Max NDEF size emulated. Range: 0005h - 7FFFh    */
#define T4T_CLA_00          0x00      /*!< CLA value for type 4 command                    */
#define T4T_INS_SELECT      0xA4      /*!< INS value for select command                    */
#define T4T_INS_READ        0xB0      /*!< INS value for reabbinary command                */
#define T4T_INS_UPDATE      0xD6      /*!< INS value for update command                    */

#define FID_CC              0xE103    /*!< File ID number for CCFile                       */
#define FID_NDEF            0x0001    /*!< File ID number for NDEF file                    */
#define T3T_BLOCK_SIZE      0x10      /*!< Block size in Type 3 Tag                        */

#define SW_FILE_NOT_FOUND   0x6A82
#define SW_COMMAND_ABORTED	0x6f00

int handle_t4t_select(const command_t *cmd);
int handle_t4t_read(const command_t *cmd);
int handle_t4t_update(const command_t *cmd);
void card_emulation_init(void);

#endif // CE_DEMO_H