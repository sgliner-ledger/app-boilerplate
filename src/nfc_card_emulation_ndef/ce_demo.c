#include "ce_demo.h"

enum States
{
    STATE_IDLE                      = 0,    /*!< Emulated Tag state idle                  */
    STATE_APP_SELECTED              = 1,    /*!< Emulated Tag state application selected  */
    STATE_CC_SELECTED               = 2,    /*!< Emulated Tag state CCFile selected       */
    STATE_FID_SELECTED              = 3,    /*!< Emulated Tag state FileID selected       */
};

static uint8_t nState;
static int32_t nSelectedIdx;    /*!< current file selected                     */
static int32_t nFiles;           /*!< Number of file emulated                   */
static uint8_t ndefFile[NDEF_SIZE];  /*!< Buffer to store NDEF File                 */

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

void card_emulation_init(void) {
	nState = STATE_IDLE;
	nSelectedIdx = -1;
	nFiles = 2;
	memcpy(ndefFile, (uint8_t *)ndef_uri, sizeof(ndef_uri));
}

/**
  *****************************************************************************
  * @brief  Compare 2 commands supplied in parameters
  *
  * @param[in]  cmd : pointer to the received command.
  * @param[in]  find : pointer to the avalaible command.
  * @param[in]  len : length of the available command.
  *
  * @retval True : Same command.
  * @retval False : Different command.
  *****************************************************************************
  */
static bool cmdCompare(uint8_t *cmd, uint8_t *find, uint16_t len)
{
  for(int i = 0; i < 20; i++)
  {
    if(!memcmp(&cmd[i],find, len))
    {
      return true;
    }
  }
  return false;
}

int handle_t4t_select(const command_t *cmd) {
	bool success = false;

	uint8_t aid[] = {0xD2, 0x76, 0x00, 0x00, 0x85, 0x01, 0x01};
	uint8_t fidCC[] = {FID_CC >> 8 , FID_CC & 0xFF};
	uint8_t fidNDEF[] = {FID_NDEF >> 8, FID_NDEF & 0xFF};
	uint8_t selectFileId[] = {0xA4, 0x00, 0x0C, 0x02, 0x00, 0x01 };

	if(cmdCompare(cmd->data, aid, sizeof(aid)))
	{ /* Select Appli */
		nState = STATE_APP_SELECTED;
		success = true;
	}
	else if((nState >= STATE_APP_SELECTED) && cmdCompare(cmd->data, fidCC, sizeof(fidCC)))
	{ /* Select CC */
		nState = STATE_CC_SELECTED;
		nSelectedIdx = 0;
		success = true;
	}
	else if((nState >= STATE_APP_SELECTED) && (cmdCompare(cmd->data,fidNDEF,sizeof(fidNDEF)) || cmdCompare(cmd->data,selectFileId,sizeof(selectFileId))))
	{ /* Select NDEF */
		nState = STATE_FID_SELECTED;
		nSelectedIdx = 1;
		success = true;
	}
	else
	{
		nState = STATE_IDLE;
	}
	return io_send_sw(success? SW_OK : SW_FILE_NOT_FOUND);
}

int handle_t4t_read(const command_t *cmd) {
	  /*
   * Cmd: CLA(1) | INS(1) | P1(1).. offset inside file high | P2(1).. offset inside file high | Le(1).. nBytes to read
   * Rsp: BytesRead | SW12
   */
  unsigned short offset = (cmd->p1 << 8) | cmd->p2;
  unsigned short toRead = cmd->lc;
  uint8_t * ppbMemory;
    
  /* Any file selected */
  if(nSelectedIdx < 0 || nSelectedIdx >= nFiles)
  {
      return io_send_sw(SW_FILE_NOT_FOUND);
  }

  /* offset + length exceed file size */
  if((unsigned long)(offset + toRead) > pdwFileSize[nSelectedIdx])
  {
      toRead = pdwFileSize[nSelectedIdx] - offset;
  }
  
  ppbMemory = (nSelectedIdx == 0 ? ccfile : ndefFile);
  return io_send_response_pointer(&ppbMemory[offset], toRead, SW_OK);
}

int handle_t4t_update(const command_t *cmd) {
	uint32_t offset = (cmd->p1 << 8) | cmd->p2;
	uint32_t length = cmd->lc;


	if(nSelectedIdx != 1)
	{
		return io_send_sw(SW_FILE_NOT_FOUND);
	}

	if((unsigned long)(offset + length) > pdwFileSize[nSelectedIdx])
	{
		return io_send_sw(SW_FILE_NOT_FOUND);
	}

	memcpy((ndefFile + offset), cmd->data, length);

	return io_send_sw(SW_OK);
}