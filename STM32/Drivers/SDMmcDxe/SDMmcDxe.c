/** @file
 * 
 *  Copyright (c) 2024, Phan Ba Gia Bao <phanbagiabao2001@gmail.com>
 *  Copyright (c) 2024, Alexandre TORGUE <alexandre.torgue@st.com>
 * 
**/

#include "SDMmcDxe.h"
#include <Library/DevicePathLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/CacheMaintenanceLib.h>
#include <Protocol/Cpu.h>
#include <stdint.h>

/* SDMMC REGISTERS OFFSET */
#define SDMMC_POWER		0x00	/* SDMMC power control             */
#define SDMMC_CLKCR		0x04	/* SDMMC clock control             */
#define SDMMC_ARG		0x08	/* SDMMC argument                  */
#define SDMMC_CMD		0x0C	/* SDMMC command                   */
#define SDMMC_RESP1		0x14	/* SDMMC response 1                */
#define SDMMC_RESP2		0x18	/* SDMMC response 2                */
#define SDMMC_RESP3		0x1C	/* SDMMC response 3                */
#define SDMMC_RESP4		0x20	/* SDMMC response 4                */
#define SDMMC_DTIMER		0x24	/* SDMMC data timer                */
#define SDMMC_DLEN		0x28	/* SDMMC data length               */
#define SDMMC_DCTRL		0x2C	/* SDMMC data control              */
#define SDMMC_DCOUNT		0x30	/* SDMMC data counter              */
#define SDMMC_STA		0x34	/* SDMMC status                    */
#define SDMMC_ICR		0x38	/* SDMMC interrupt clear           */
#define SDMMC_MASK		0x3C	/* SDMMC mask                      */
#define SDMMC_IDMACTRL		0x50	/* SDMMC DMA control               */
#define SDMMC_IDMABASE0		0x58	/* SDMMC DMA buffer 0 base address */

# define ULL(_x)	(_x##ULL)
# define   U(_x)	(_x)
#define MMC_CMD(_x)			U(_x)
#define MMC_ACMD(_x)			U(_x)
#define BIT(nr)			((1) << (nr))
#define GENMASK_32(h, l) 	(((~(UINT32)0) << (l)) & (~(UINT32)0 >> (32 - 1 - (h))))
#define GENMASK				    GENMASK_32

/* SDMMC_POWER register */
#define SDMMC_POWER_PWRCTRL_MASK	GENMASK(1, 0)
#define SDMMC_POWER_PWRCTRL_OFF		0
#define SDMMC_POWER_PWRCTRL_CYCLE	2
#define SDMMC_POWER_PWRCTRL_ON		3
#define SDMMC_POWER_VSWITCH		BIT(2)
#define SDMMC_POWER_VSWITCHEN		BIT(3)
#define SDMMC_POWER_DIRPOL		BIT(4)

/* SDMMC_CLKCR register */
#define SDMMC_CLKCR_CLKDIV		GENMASK(9, 0)
#define SDMMC_CLKCR_CLKDIV_MAX		SDMMC_CLKCR_CLKDIV
#define SDMMC_CLKCR_PWRSAV		BIT(12)
#define SDMMC_CLKCR_WIDBUS_4		BIT(14)
#define SDMMC_CLKCR_WIDBUS_8		BIT(15)
#define SDMMC_CLKCR_NEGEDGE		BIT(16)
#define SDMMC_CLKCR_HWFC_EN		BIT(17)
#define SDMMC_CLKCR_DDR			BIT(18)
#define SDMMC_CLKCR_BUSSPEED		BIT(19)
#define SDMMC_CLKCR_SELCLKRX_MASK	GENMASK(21, 20)
#define SDMMC_CLKCR_SELCLKRX_CK		0
#define SDMMC_CLKCR_SELCLKRX_CKIN	BIT(20)
#define SDMMC_CLKCR_SELCLKRX_FBCK	BIT(21)

/* SDMMC_CMD register */
#define SDMMC_CMD_CMDINDEX		GENMASK(5, 0)
#define SDMMC_CMD_CMDTRANS		BIT(6)
#define SDMMC_CMD_CMDSTOP		BIT(7)
#define SDMMC_CMD_WAITRESP		GENMASK(9, 8)
#define SDMMC_CMD_WAITRESP_0		BIT(8)
#define SDMMC_CMD_WAITRESP_1		BIT(9)
#define SDMMC_CMD_WAITINT		BIT(10)
#define SDMMC_CMD_WAITPEND		BIT(11)
#define SDMMC_CMD_CPSMEN		BIT(12)
#define SDMMC_CMD_DTHOLD		BIT(13)
#define SDMMC_CMD_BOOTMODE		BIT(14)
#define SDMMC_CMD_BOOTEN		BIT(15)
#define SDMMC_CMD_CMDSUSPEND		BIT(16)

/* SDMMC_DCTRL register */
#define SDMMC_DCTRL_DTEN		BIT(0)
#define SDMMC_DCTRL_DTDIR		BIT(1)
#define SDMMC_DCTRL_DTMODE		GENMASK(3, 2)
#define SDMMC_DCTRL_DBLOCKSIZE		GENMASK(7, 4)
#define SDMMC_DCTRL_DBLOCKSIZE_SHIFT	4
#define SDMMC_DCTRL_RWSTART		BIT(8)
#define SDMMC_DCTRL_RWSTOP		BIT(9)
#define SDMMC_DCTRL_RWMOD		BIT(10)
#define SDMMC_DCTRL_SDMMCEN		BIT(11)
#define SDMMC_DCTRL_BOOTACKEN		BIT(12)
#define SDMMC_DCTRL_FIFORST		BIT(13)

/* SDMMC_STA register */
#define SDMMC_STA_CCRCFAIL		BIT(0)
#define SDMMC_STA_DCRCFAIL		BIT(1)
#define SDMMC_STA_CTIMEOUT		BIT(2)
#define SDMMC_STA_DTIMEOUT		BIT(3)
#define SDMMC_STA_TXUNDERR		BIT(4)
#define SDMMC_STA_RXOVERR		BIT(5)
#define SDMMC_STA_CMDREND		BIT(6)
#define SDMMC_STA_CMDSENT		BIT(7)
#define SDMMC_STA_DATAEND		BIT(8)
#define SDMMC_STA_DHOLD			BIT(9)
#define SDMMC_STA_DBCKEND		BIT(10)
#define SDMMC_STA_DABORT		BIT(11)
#define SDMMC_STA_DPSMACT		BIT(12)
#define SDMMC_STA_CPSMACT		BIT(13)
#define SDMMC_STA_TXFIFOHE		BIT(14)
#define SDMMC_STA_RXFIFOHF		BIT(15)
#define SDMMC_STA_TXFIFOF		BIT(16)
#define SDMMC_STA_RXFIFOF		BIT(17)
#define SDMMC_STA_TXFIFOE		BIT(18)
#define SDMMC_STA_RXFIFOE		BIT(19)
#define SDMMC_STA_BUSYD0		BIT(20)
#define SDMMC_STA_BUSYD0END		BIT(21)
#define SDMMC_STA_SDMMCIT		BIT(22)
#define SDMMC_STA_ACKFAIL		BIT(23)
#define SDMMC_STA_ACKTIMEOUT		BIT(24)
#define SDMMC_STA_VSWEND		BIT(25)
#define SDMMC_STA_CKSTOP		BIT(26)
#define SDMMC_STA_IDMATE		BIT(27)
#define SDMMC_STA_IDMABTC		BIT(28)

#define SDMMC_STATIC_FLAGS			(SDMMC_STA_CCRCFAIL | \
						 SDMMC_STA_DCRCFAIL | \
						 SDMMC_STA_CTIMEOUT | \
						 SDMMC_STA_DTIMEOUT | \
						 SDMMC_STA_TXUNDERR | \
						 SDMMC_STA_RXOVERR  | \
						 SDMMC_STA_CMDREND  | \
						 SDMMC_STA_CMDSENT  | \
						 SDMMC_STA_DATAEND  | \
						 SDMMC_STA_DBCKEND  | \
						 SDMMC_STA_IDMATE   | \
						 SDMMC_STA_IDMABTC)

/* SDMMC_ICR register */
#define SDMMC_ICR_CCRCFAILC		BIT(0)
#define SDMMC_ICR_DCRCFAILC		BIT(1)
#define SDMMC_ICR_CTIMEOUTC		BIT(2)
#define SDMMC_ICR_DTIMEOUTC		BIT(3)
#define SDMMC_ICR_TXUNDERRC		BIT(4)
#define SDMMC_ICR_RXOVERRC		BIT(5)
#define SDMMC_ICR_CMDRENDC		BIT(6)
#define SDMMC_ICR_CMDSENTC		BIT(7)
#define SDMMC_ICR_DATAENDC		BIT(8)
#define SDMMC_ICR_DHOLDC		BIT(9)
#define SDMMC_ICR_DBCKENDC		BIT(10)
#define SDMMC_ICR_DABORTC		BIT(11)
#define SDMMC_ICR_BUSYD0ENDC		BIT(21)
#define SDMMC_ICR_SDMMCITC		BIT(22)
#define SDMMC_ICR_ACKFAILC		BIT(23)
#define SDMMC_ICR_ACKTIMEOUTC		BIT(24)
#define SDMMC_ICR_VSWENDC		BIT(25)
#define SDMMC_ICR_CKSTOPC		BIT(26)
#define SDMMC_ICR_IDMATEC		BIT(27)
#define SDMMC_ICR_IDMABTCC		BIT(28)
#define SDMMC_ICR_STATIC_FLAGS		((GENMASK(28, 21)) | (GENMASK(11, 0)))

/* SDMMC_MASK register */
#define SDMMC_MASK_CCRCFAILIE		BIT(0)
#define SDMMC_MASK_DCRCFAILIE		BIT(1)
#define SDMMC_MASK_CTIMEOUTIE		BIT(2)
#define SDMMC_MASK_DTIMEOUTIE		BIT(3)
#define SDMMC_MASK_TXUNDERRIE		BIT(4)
#define SDMMC_MASK_RXOVERRIE		BIT(5)
#define SDMMC_MASK_CMDRENDIE		BIT(6)
#define SDMMC_MASK_CMDSENTIE		BIT(7)
#define SDMMC_MASK_DATAENDIE		BIT(8)
#define SDMMC_MASK_DHOLDIE		BIT(9)
#define SDMMC_MASK_DBCKENDIE		BIT(10)
#define SDMMC_MASK_DABORTIE		BIT(11)
#define SDMMC_MASK_TXFIFOHEIE		BIT(14)
#define SDMMC_MASK_RXFIFOHFIE		BIT(15)
#define SDMMC_MASK_RXFIFOFIE		BIT(17)
#define SDMMC_MASK_TXFIFOEIE		BIT(18)
#define SDMMC_MASK_BUSYD0ENDIE		BIT(21)
#define SDMMC_MASK_SDMMCITIE		BIT(22)
#define SDMMC_MASK_ACKFAILIE		BIT(23)
#define SDMMC_MASK_ACKTIMEOUTIE		BIT(24)
#define SDMMC_MASK_VSWENDIE		BIT(25)
#define SDMMC_MASK_CKSTOPIE		BIT(26)
#define SDMMC_MASK_IDMABTCIE		BIT(28)

/* SDMMC_IDMACTRL register */
#define SDMMC_IDMACTRL_IDMAEN		BIT(0)

#define SDMMC_CMD_TIMEOUT		0xFFFFFFFF
#define SDMMC_BUSYD0END_TIMEOUT_US	2000000

#define MMCI0_BLOCKLEN 512
#define MMCI0_POW2_BLOCKLEN     9
#define MMCI0_TIMEOUT           1000

#define SYS_MCI_CARDIN          BIT0
#define SYS_MCI_WPROT           BIT1

#define MMC_DATA_READ		1
#define MMC_DATA_WRITE		2
#define MMC_RSP_PRESENT (1 << 0)
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define ARCH_DMA_MINALIGN   32

#define ROUNDDOWN(x, y) ((x) - ((x) % (y)))
#define ROUNDUP(x, y) ((((x) + ((y) - 1)) / (y)) * (y))

#define OCR_POWERUP			BIT(31)
#define OCR_HCS				BIT(30)
#define OCR_BYTE_MODE			(U(0) << 29)
#define OCR_SECTOR_MODE			(U(2) << 29)
#define OCR_ACCESS_MODE_MASK		(U(3) << 29)
#define OCR_3_5_3_6			BIT(23)
#define OCR_3_4_3_5			BIT(22)
#define OCR_3_3_3_4			BIT(21)
#define OCR_3_2_3_3			BIT(20)
#define OCR_3_1_3_2			BIT(19)
#define OCR_3_0_3_1			BIT(18)
#define OCR_2_9_3_0			BIT(17)
#define OCR_2_8_2_9			BIT(16)
#define OCR_2_7_2_8			BIT(15)
#define OCR_VDD_MIN_2V7			GENMASK(23, 15)
#define OCR_VDD_MIN_2V0			GENMASK(14, 8)
#define OCR_VDD_MIN_1V7			BIT(7)


/* Values in EXT CSD register */
#define MMC_BUS_WIDTH_1			U(0)
#define MMC_BUS_WIDTH_4			U(1)
#define MMC_BUS_WIDTH_8			U(2)
#define MMC_BUS_WIDTH_DDR_4		U(5)
#define MMC_BUS_WIDTH_DDR_8		U(6)
#define MMC_BOOT_MODE_BACKWARD		(U(0) << 3)
#define MMC_BOOT_MODE_HS_TIMING		(U(1) << 3)
#define MMC_BOOT_MODE_DDR		(U(2) << 3)



EFI_STATUS
MciSendCommand (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_CMD                    MmcCmd,
  IN UINT32                     Argument
  );

EFI_STATUS
MciReceiveResponse (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_RESPONSE_TYPE          Type,
  IN UINT32*                    Response
  );

EFI_CPU_ARCH_PROTOCOL  *mCpu;

STATIC BOOLEAN next_cmd_is_acmd;
STATIC MMC_CMD MmcCmdSave;
STATIC UINT32 ArgumentSave;
//STATIC UINT32 resp_data[4];
STATIC BOOLEAN ResendCmd = FALSE;

EFI_STATUS
DumpStatus(){
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_POWER : %x ", MmioRead32(MCI_SYSCTL + SDMMC_POWER)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_CLKCR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_CLKCR)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_ARGR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_ARG)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_CMDR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_CMD)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_RESP1R : %x ", MmioRead32(MCI_SYSCTL + SDMMC_RESP1)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_RESP2R : %x ", MmioRead32(MCI_SYSCTL + SDMMC_RESP2)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_RESP3R : %x ", MmioRead32(MCI_SYSCTL + SDMMC_RESP3)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_RESP4R : %x ", MmioRead32(MCI_SYSCTL + SDMMC_RESP4)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_DTIMER : %x ", MmioRead32(MCI_SYSCTL + SDMMC_DTIMER)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_DLENR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_DLEN)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_DCTRLR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_DCTRL)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_DCNTR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_DCOUNT)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_STAR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_STA)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_ICR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_ICR)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_MASKR : %x ", MmioRead32(MCI_SYSCTL + SDMMC_MASK)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_IDMACTRL : %x ", MmioRead32(MCI_SYSCTL + SDMMC_IDMACTRL)));
  DEBUG((DEBUG_INFO, "\nValue of SDMMC_IDMABASE : %x ", MmioRead32(MCI_SYSCTL + SDMMC_IDMABASE0)));
  DEBUG((DEBUG_INFO, "\nValue of MCI_SELECT_REG : %x \n", MmioRead32(MCI_SELECT_REG)));
  return EFI_SUCCESS;
}

BOOLEAN
MciIsReadOnly (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  DEBUG((DEBUG_INFO, "MciIsReadOnly\n"));

  return FALSE;
}

BOOLEAN
MciIsCardPresent (
  IN EFI_MMC_HOST_PROTOCOL     *This
  )
{
  return TRUE;
}

EFI_STATUS
MciStopTransfer(
  IN EFI_MMC_HOST_PROTOCOL     *This
  )

{
  EFI_STATUS              Status;
  UINT32                  Response[4];
  // Command 12 - Stop transmission (ends read or write)
  // Normally only needed for streaming transfers or after error.
  Status = MciSendCommand (This, MMC_CMD12, 0);
  if (!EFI_ERROR (Status)) {
    MciReceiveResponse (This, MMC_RESPONSE_TYPE_R1b, Response);
  }
	return Status;
}


EFI_STATUS
MciSendCommand (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_CMD                    MmcCmd,
  IN UINT32                     Argument
  )
{
  UINT32  Flags_data = 0;
  UINT32 resp_type;
  UINT32  Flag_cmd;
  UINT32  Status;
  UINT32  retry;
  UINTN err = 0;
  UINT32  Cmd;



  switch((MmcCmd)){
  case MMC_CMD6:
      if((ResendCmd == FALSE) && (Argument != 2)){
      MmcCmdSave = MmcCmd;
      ArgumentSave = Argument;
      ResendCmd = TRUE;

      return EFI_SUCCESS;
    }
    break;
  case MMC_ACMD51:
	case MMC_CMD17:
	case MMC_CMD18:
    if((ResendCmd == FALSE)){
      MmcCmdSave = MmcCmd;
      ArgumentSave = Argument;
      ResendCmd = TRUE;

      return EFI_SUCCESS;
    }
    break;
  default:
		break;
	}

  Flag_cmd = SDMMC_STA_CTIMEOUT;

  // Start cmd:
  if ((MmioRead32(MCI_SYSCTL + SDMMC_CMD) & SDMMC_CMD_CPSMEN) != 0){
    MmioWrite32(MCI_SYSCTL + SDMMC_CMD, 0);
  }

  // Create Command
  Cmd = MMC_GET_INDX(MmcCmd) | SDMMC_CMD_CPSMEN;
  //create response type
  resp_type = MmcCmd & 0xffff0000;

  if (MMC_GET_INDX(MmcCmd) == 0) {
    Flag_cmd |= SDMMC_STA_CMDSENT;
  }

  if (MmcCmd & MMC_CMD_WAIT_RESPONSE) {
		if (MmcCmd & MMC_CMD_LONG_RESPONSE) {
		  Flag_cmd |= SDMMC_STA_CMDREND;
      Cmd |= SDMMC_CMD_WAITRESP_0 | SDMMC_CMD_WAITRESP_1;
    } else if (MmcCmd & MMC_CMD_NO_CRC_RESPONSE) {
		  Flag_cmd |= SDMMC_STA_CMDREND;
      Cmd |= SDMMC_CMD_WAITRESP_1;
    } else {
      Flag_cmd |= SDMMC_STA_CMDREND | SDMMC_STA_CCRCFAIL;
      Cmd |= SDMMC_CMD_WAITRESP_0;
    }
  }

  switch( (MmcCmd)){
  case MMC_CMD1:
    Argument |= OCR_POWERUP;
    break;
  case MMC_CMD6:
   if(!next_cmd_is_acmd) {
      Cmd |= SDMMC_CMD_CMDTRANS;
      Flags_data |= SDMMC_STA_DCRCFAIL |
                  SDMMC_STA_DTIMEOUT |
                  SDMMC_STA_RXOVERR |
                  SDMMC_STA_DATAEND |
                  SDMMC_STA_DBCKEND |
                  SDMMC_STA_IDMATE;
   }
    break;
  case MMC_CMD8:
    // NOT SUPPORTED
    break;
  case MMC_CMD12:
    Cmd |= SDMMC_CMD_CMDSTOP;
		break;
	case MMC_CMD17:
	case MMC_CMD18:
    Cmd |= SDMMC_CMD_CMDTRANS;
    // IDMA mode by default
      Flags_data |= SDMMC_STA_DCRCFAIL |
                SDMMC_STA_DTIMEOUT |
                SDMMC_STA_RXOVERR |
                SDMMC_STA_DATAEND |
                SDMMC_STA_IDMATE;
    break;
  case MMC_ACMD41:
    Argument |= OCR_3_2_3_3 | OCR_3_3_3_4;
    break;
  case MMC_ACMD51:
    Cmd |= SDMMC_CMD_CMDTRANS;
      Flags_data |= SDMMC_STA_DCRCFAIL |
                SDMMC_STA_DTIMEOUT |
                SDMMC_STA_RXOVERR |
                SDMMC_STA_DATAEND |
                SDMMC_STA_DBCKEND |
                SDMMC_STA_IDMATE;
    break;
	default:
		break;
	}

  next_cmd_is_acmd = ((MmcCmd) == MMC_CMD55);

  /* Clear Status register static flags*/
  MmioWrite32(MCI_SYSCTL + SDMMC_ICR, SDMMC_STATIC_FLAGS);

  if (((Cmd & SDMMC_CMD_CMDTRANS) == 0U) && !next_cmd_is_acmd) {
  	MmioWrite32(MCI_SYSCTL + SDMMC_DCTRL, 0U);
  }
	/* Set the SDMMC Data TimeOut value */
  // if ((resp_type & MMC_RSP_BUSY) != 0U)
	// 	MmioWrite32(MCI_SYSCTL + SDMMC_DTIMER, UINT32_MAX);

	/* Set SDMMC argument value */
  MmioWrite32(MCI_SYSCTL + SDMMC_ARG, Argument);
	/* Set SDMMC command parameters */
  MmioWrite32(MCI_SYSCTL + SDMMC_CMD, Cmd);

	Status = MmioRead32(MCI_SYSCTL + SDMMC_STA);

  retry = 10;
	while (((Status & Flag_cmd) == 0U) && (retry!= 0) ) {
    MicroSecondDelay(1000);
		Status = MmioRead32(MCI_SYSCTL + SDMMC_STA);
    retry --;
	}

  if(retry==0){
    DEBUG ((DEBUG_ERROR, "timeout 10ms (cmd = %u,status = %x)\n", MMC_GET_INDX(MmcCmd), Status));
    err = -EFI_TIMEOUT;
    goto err_exit;
  }
  if((Status & (SDMMC_STA_CTIMEOUT | SDMMC_STA_CCRCFAIL)) != 0) {
    if((Status & SDMMC_STA_CTIMEOUT) != 0) {
      err = -EFI_TIMEOUT;
      if(!((MmcCmd == MMC_CMD1) ||
           (MmcCmd == MMC_CMD13) ||
           (MmcCmd == MMC_CMD8) )){
          DEBUG ((DEBUG_ERROR, "%s: CTIMEOUT (cmd = %u,status = %x)\n", __func__, MMC_GET_INDX(MmcCmd), Status));
      }
    } else {
        err = -EFI_CRC_ERROR;
        DEBUG ((DEBUG_ERROR, "%s: CRCFAIL (cmd = %u,status = %x)\n", __func__, MMC_GET_INDX(MmcCmd), Status));
    }
    goto err_exit;
  }

	if (Flags_data == 0U) {
		MmioWrite32(MCI_SYSCTL + SDMMC_ICR, SDMMC_STATIC_FLAGS);
		return 0;
	}

	Status = MmioRead32(MCI_SYSCTL + SDMMC_STA);

  retry = 2000;
	while (((Status & Flags_data) == 0U) && (retry!= 0) ) {
    MicroSecondDelay(1000);
		Status = MmioRead32(MCI_SYSCTL + SDMMC_STA);
    retry --;
	}
  if(retry==0){
    err = -EFI_TIMEOUT;
    DEBUG ((DEBUG_ERROR, "timeout 2s (cmd = %u,status = %x)\n", MMC_GET_INDX(MmcCmd), Status));
    goto err_exit;
  }

  if((Status & (SDMMC_STA_DTIMEOUT | SDMMC_STA_DCRCFAIL | SDMMC_STA_TXUNDERR | SDMMC_STA_RXOVERR | SDMMC_STA_IDMATE) ) != 0){
    DEBUG ((DEBUG_ERROR, "Error flag (cmd %u,status = %x)\n", MMC_GET_INDX(MmcCmd), Status));
    err = -EFI_INVALID_PARAMETER;
  }

err_exit:
  MmioWrite32(MCI_SYSCTL + SDMMC_ICR, SDMMC_STATIC_FLAGS);
  MmioWrite32(MCI_SYSCTL + SDMMC_CMD, MmioRead32(MCI_SYSCTL + SDMMC_CMD) & ~SDMMC_CMD_CMDTRANS);

//  DEBUG((DEBUG_INFO, "MMCIsendcommand err = %d\n", err));
	if ((err != 0) && ((Status & SDMMC_STA_DPSMACT) != 0U)) {
		err = MciStopTransfer(NULL);

		if (err != 0) {
			return err;
		}
	}
  return err;
}


EFI_STATUS
MciPrepareDataPath (
  IN EFI_LBA                    Lba,
  IN UINTN                      Length,
  IN UINT32*                    Buffer
  )
{
  //DEBUG((DEBUG_INFO, "MciPrepareDataPath\n"));
  UINT32 arg_size;
  UINT32 data_ctrl = (1<<1);
  EFI_STATUS Status;
  // EFI_MMC_HOST_PROTOCOL *MmcHost;
  Status = EFI_SUCCESS;

  if(Length > 512) {
    arg_size = 512;
  } else {
    arg_size = (UINT32) (Length);
  }

  WriteBackInvalidateDataCacheRange (Buffer, Length);

	/* Prepare CMD 16*/
  MmioWrite32(MCI_SYSCTL + SDMMC_DTIMER, 0);

  MmioWrite32(MCI_SYSCTL + SDMMC_DLEN, 0);

  MmioWrite32(MCI_SYSCTL + SDMMC_DCTRL, 0);

  Status = MciSendCommand (NULL, MMC_CMD16, arg_size);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR,"Error send CMD16 in prepare data path: %d and Error = %r\n", Status));
  }

	/* Prepare data command */
  MmioWrite32(MCI_SYSCTL + SDMMC_DTIMER, UINT32_MAX);
  MmioWrite32(MCI_SYSCTL + SDMMC_DLEN, Length);
  MmioWrite32(MCI_SYSCTL + SDMMC_IDMACTRL, (1<<0));

  MmioWrite32(MCI_SYSCTL + SDMMC_IDMABASE0, (UINT32)(UINTN)Buffer);

	data_ctrl |= __builtin_ctz(arg_size) << 4;

	MmioWrite32(MCI_SYSCTL + SDMMC_DCTRL,
          (MmioRead32(MCI_SYSCTL + SDMMC_DCTRL) & ~((1<<0)|(1<<1)|(1<<2)|(1<<3)|(1<<4)|(1<<5)|(1<<6)|(1<<7))) | data_ctrl);

  return Status;
}

EFI_STATUS
MciReceiveResponse (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_RESPONSE_TYPE          Type,
  IN UINT32*                    Response
  )
{
  if (Response == NULL) {
    return EFI_INVALID_PARAMETER;
  }

 if (   (Type == MMC_RESPONSE_TYPE_R1)
      || (Type == MMC_RESPONSE_TYPE_R1b)
      || (Type == MMC_RESPONSE_TYPE_R3)
      || (Type == MMC_RESPONSE_TYPE_R6)
      || (Type == MMC_RESPONSE_TYPE_R7))
  {
    Response[3] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP4);
    Response[2] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP3);
    Response[1] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP2);
    Response[0] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP1);
  } else if (Type == MMC_RESPONSE_TYPE_R2) {
    Response[3] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP1);
    Response[2] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP2);
    Response[1] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP3);
    Response[0] = MmioRead32 (MCI_SYSCTL + SDMMC_RESP4);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
MciReadBlockData (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN EFI_LBA                    Lba,
  IN UINTN                      Length,
  IN UINT32*                    Buffer
  )
{

  UINT32                 Response[4];

  EFI_STATUS RetVal;
  RetVal = EFI_SUCCESS;

  RetVal = MciPrepareDataPath (Lba, Length, Buffer);

  if (EFI_ERROR (RetVal)) {
     DEBUG((DEBUG_INFO, "Error MciPrepareDataPath \n"));
  }

  if (MmcCmdSave == MMC_ACMD51) {
    RetVal = MciSendCommand (NULL, MMC_CMD55, 0xaaaa0000);
    if (EFI_ERROR (RetVal)) {
      DEBUG ((DEBUG_ERROR, "%a (MMC_CMD55): Error and Status = %r\n", __func__, RetVal));
      return RetVal;
    }

    RetVal = MciReceiveResponse (NULL, MMC_RESPONSE_TYPE_R1, Response);
    if (EFI_ERROR (RetVal)) {
      DEBUG ((DEBUG_ERROR, "%a (MMC_CMD55): Error and Status = %r\n", __func__, RetVal));
      return RetVal;
    }
  }

  UINTN i =0;
  for (i=0;i<Length/4;i++){
    Buffer[i] = 0;
  }

  WriteBackInvalidateDataCacheRange (Buffer, Length);

  RetVal = MciSendCommand(NULL, MmcCmdSave, ArgumentSave);
  if(EFI_ERROR(RetVal)){
    DEBUG ((DEBUG_ERROR,"Error sending in ReadBlockData and Error = %r\n", RetVal));
  }

  ResendCmd = FALSE;

  WriteBackInvalidateDataCacheRange (Buffer, Length);

  return RetVal;
}

EFI_STATUS
MciWriteBlockData (
  IN EFI_MMC_HOST_PROTOCOL     *This,
  IN EFI_LBA                   Lba,
  IN UINTN                     Length,
  IN UINT32*                   Buffer
  )
{
  DEBUG((DEBUG_INFO, "MciWriteBlockData\n"));

  return EFI_SUCCESS;
}

EFI_STATUS
MciNotifyState (
  IN  EFI_MMC_HOST_PROTOCOL     *This,
  IN MMC_STATE                  State
  )
{
  return EFI_SUCCESS;
}

EFI_GUID mPL180MciDevicePathGuid = EFI_CALLER_ID_GUID;

EFI_STATUS
MciBuildDevicePath (
  IN EFI_MMC_HOST_PROTOCOL      *This,
  IN EFI_DEVICE_PATH_PROTOCOL   **DevicePath
  )
{
  DEBUG((DEBUG_INFO, "MciBuildDevicePath\n"));

  EFI_DEVICE_PATH_PROTOCOL    *NewDevicePathNode;

  NewDevicePathNode = CreateDeviceNode (HARDWARE_DEVICE_PATH, HW_VENDOR_DP, sizeof (VENDOR_DEVICE_PATH));
  CopyGuid (& ((VENDOR_DEVICE_PATH*)NewDevicePathNode)->Guid, &mPL180MciDevicePathGuid);

  *DevicePath = NewDevicePathNode;
  return EFI_SUCCESS;
}

EFI_STATUS
MciSetIos(
  IN  EFI_MMC_HOST_PROTOCOL     *This,
  IN  UINT32                    BusClockFreq,
  IN  UINT32                    BusWidth,
  IN  UINT32                    TimingMode
)
{
  DEBUG((DEBUG_INFO, "MciSetIos\n"));
  DEBUG((DEBUG_INFO, "BusClockFreq = %d\n", BusClockFreq));
  DEBUG((DEBUG_INFO, "BusWidth = %d\n", BusWidth));
  DEBUG((DEBUG_INFO, "TimingMode = %d\n", TimingMode));

   UINT32 bus_cfg = 0;
 //  UINT32 clock_div;

  switch (BusWidth) {
	case 0:
		break;
	case 4:
		bus_cfg |= SDMMC_CLKCR_WIDBUS_4;
		break;
	case 8:
		bus_cfg |= SDMMC_CLKCR_WIDBUS_8;
		break;
	default:
		DEBUG((DEBUG_ERROR, "MciSetIos Bus width not valid\n"));
		break;
	}

 
  // CLK 25MHz
  //MmioWrite32(MCI_SYSCTL + SDMMC_CLKCR, 0x34004);

  // CLK 50 MHz
  MmioWrite32(MCI_SYSCTL + SDMMC_CLKCR, 0x34002);

  return EFI_SUCCESS;
}

EFI_MMC_HOST_PROTOCOL gMciHost = {
  MMC_HOST_PROTOCOL_REVISION,
  MciIsCardPresent,
  MciIsReadOnly,
  MciBuildDevicePath,
  MciNotifyState,
  MciSendCommand,
  MciReceiveResponse,
  MciReadBlockData,
  MciWriteBlockData,
  MciSetIos
};


EFI_STATUS
MciDxeInitialize (
  IN EFI_HANDLE         ImageHandle,
  IN EFI_SYSTEM_TABLE   *SystemTable
  )
{
  UINT32 RCC_SDMMC1 = 0x54200000 + 0x830;

  EFI_STATUS    Status;
  EFI_HANDLE    Handle;

  MmioWrite32(RCC_SDMMC1, (1<<0)|(1<<1)|(1<<2));
  MicroSecondDelay(1000);

  MmioWrite32(RCC_SDMMC1, (1<<1)|(1<<2));
  MicroSecondDelay(1000);

  MmioWrite32(MCI_SYSCTL + SDMMC_POWER, (1<<1));
  MicroSecondDelay(2*1000); //POWER_CYCLE_DELAY

  MmioWrite32(MCI_SYSCTL + SDMMC_POWER, 0);
  MicroSecondDelay(2*1000); //POWER_OFF_DELAY

  MmioWrite32(MCI_SYSCTL + SDMMC_CLKCR, (1<<17) | (0xFA<<0));
  MmioWrite32(MCI_SYSCTL + SDMMC_POWER, (1<<0) | (1<<1));
	MicroSecondDelay(1000);

  Handle = NULL;

  //Publish Component Name, BlockIO protocol interfaces
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEmbeddedMmcHostProtocolGuid,
                  &gMciHost,
                  NULL
                  );
  ASSERT_EFI_ERROR (Status);

  Status = gBS->LocateProtocol (
                  &gEfiCpuArchProtocolGuid,
                  NULL,
                  (VOID **)&mCpu);
  ASSERT_EFI_ERROR (Status);

  return Status;
}