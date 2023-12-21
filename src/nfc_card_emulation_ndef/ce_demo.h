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

/**
  * CCLEN : Indicates the size of this CC File <BR>
  * T4T_VNo : Indicates the Mapping Version <BR>
  * MLe high : Max R-APDU size <BR>
  * MLc high : Max C-APDU size <BR>
  * NDEF FCI T: Indicates the NDEF-File_Ctrl_TLV <BR>
  * NDEF FCI L: The length of the V-field <BR>
  * NDEF FCI V1: NDEF File Identifier <BR>
  * NDEF FCI V2: NDEF File size <BR>
  * NDEF FCI V3: NDEF Read AC <BR>
  * NDEF FCI V4: NDEF Write AC <BR>
  */
static const uint8_t ccfile[] = {
                            0x00, 0x0F,                                       /* CCLEN      */
                            0x20,                                             /* T4T_VNo    */
                            0x00, 0x7F,                                       /* MLe        */
                            0x00, 0x7F,                                       /* MLc        */
                            0x04,                                             /* T          */
                            0x06,                                             /* L          */
                            (FID_NDEF & 0xFF00) >> 8, (FID_NDEF & 0x00FF),    /* V1         */
                            (NDEF_SIZE & 0xFF00) >> 8, (NDEF_SIZE & 0x00FF),  /* V2         */
                            0x00,                                             /* V3         */
                            0x00                                              /* V4         */
};

static const uint8_t ndef_uri[] = { 0x00, 0x15,                      /* NDEF length                */
                                    0xD1,                            /* NDEF Header                */
                                    0x01,                            /* NDEF type length           */
                                    0x11,                            /* NDEF payload length        */
                                    0x55,                            /* NDEF Type                  */
                                    0x01,                            /* NDEF URI abreviation field */
                                    0x73, 0x74, 0x2E, 0x63, 0x6F,    /* NDEF URI string            */
                                    0x6D, 0x2F, 0x73, 0x74, 0x32,
                                    0x35, 0x2D, 0x64, 0x65, 0x6D, 0x6F };

static const uint32_t pdwFileSize[] = {sizeof(ccfile), NDEF_SIZE};  /*!< Current emulated files size     */

int handle_t4t_select(const command_t *cmd);
int handle_t4t_read(const command_t *cmd);
int handle_t4t_update(const command_t *cmd);
void card_emulation_init(void);

#endif // CE_DEMO_H