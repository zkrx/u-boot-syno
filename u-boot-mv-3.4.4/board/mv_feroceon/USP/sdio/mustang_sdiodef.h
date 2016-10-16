

#define MUSTANG_SDHC_BASE  	0xf1080000	// from 00-4c	
#define INTEGRATOR_GPIO_BASE 	0x1b000000
//#define SC_BASE

#define P(x) (MUSTANG_SDHC_BASE + (2 * x))

#define GPIO_REG_READ32(addr) (*(volatile unsigned int*)(INTEGRATOR_GPIO_BASE+addr))
#define GPIO_REG_WRITE32(addr, val) (*(volatile unsigned int*)(INTEGRATOR_GPIO_BASE+addr)=val)

#define SDIO_REG_WRITE16(addr,val) (*(volatile unsigned short*)(P(addr)) = val)
#define SDIO_REG_READ16(addr) (*(volatile unsigned short*)(P(addr)))

/* SDIO register offset */
#define SDIO_SYS_ADDR_LOW			0x000
#define SDIO_SYS_ADDR_HI			0x002
#define SDIO_BLK_SIZE				0x004
#define SDIO_BLK_COUNT				0x006
#define SDIO_ARG_LOW				0x008
#define SDIO_ARG_HI				0x00A
#define SDIO_XFER_MODE				0x00C
#define SDIO_CMD				0x00E
#define SDIO_RSP0				0x010
#define SDIO_RSP1				0x012
#define SDIO_RSP2				0x014
#define SDIO_RSP3				0x016
#define SDIO_RSP4				0x018
#define SDIO_RSP5				0x01A
#define SDIO_RSP6				0x01C
#define SDIO_RSP7				0x01E
#define SDIO_BUF_DATA_PORT			0x020
#define SDIO_RSVED				0x022
#define SDIO_PRESENT_STATE0			0x024
#define CARD_BUSY				((unsigned short)(0x1 << 1))
#define SDIO_PRESENT_STATE1			0x026
#define SDIO_HOST_CTRL				0x028
#define SDIO_BLK_GAP_CTRL			0x02A
#define SDIO_CLK_CTRL				0x02C
#define SDIO_SW_RESET				0x02E
#define SDIO_NOR_INTR_STATUS			0x030
#define SDIO_ERR_INTR_STATUS			0x032
#define SDIO_NOR_STATUS_EN			0x034
#define SDIO_ERR_STATUS_EN			0x036
#define SDIO_NOR_INTR_EN			0x038
#define SDIO_ERR_INTR_EN			0x03A
#define SDIO_AUTOCMD12_ERR_STATUS		0x03C
#define SDIO_CURR_BLK_SIZE			0x03E
#define SDIO_CURR_BLK_COUNT			0x040
#define SDIO_AUTOCMD12_ARG_LOW			0x042
#define SDIO_AUTOCMD12_ARG_HI			0x044
#define SDIO_AUTOCMD12_INDEX			0x046
#define SDIO_AUTO_RSP0				0x048
#define SDIO_AUTO_RSP1				0x04A
#define SDIO_AUTO_RSP2				0x04C

/************** SDIO_HOST_CTRL  ****/
#define SDIO_HOST_CTRL_PUSH_PULL_EN 1
#define SDIO_HOST_CTRL_CARD_TYPE_MEM_ONLY 0
#define SDIO_HOST_CTRL_CARD_TYPE_IO_ONLY (1<<1)
#define SDIO_HOST_CTRL_CARD_TYPE_IO_MEM_COMBO (2<<1)
#define SDIO_HOST_CTRL_CARD_TYPE_IO_MMC (3<<1)
#define SDIO_HOST_CTRL_BIG_ENDAN (1<<3)
#define SDIO_HOST_CTRL_LSB_FIRST (1<<4)
#define SDIO_HOST_CTRL_ID_MODE_LOW_FREQ (1<<5)
#define SDIO_HOST_CTRL_HALF_SPEED (1<<6)
#define SDIO_HOST_CTRL_DATA_WIDTH_4_BITS (1<<9)
#define SDIO_HOST_CTRL_HI_SPEED_EN (1<<10)
#define SDIO_HOST_CTRL_TMOUT_SHIFT 11
#define SDIO_HOST_CTRL_TMOUT_EN (1<<15)

#define SDIO_HOST_CTRL_DFAULT_OPEN_DRAIN (0xF<<SDIO_HOST_CTRL_TMOUT_SHIFT)
#define SDIO_HOST_CTRL_DFAULT_PUSH_PULL ((0xF<<SDIO_HOST_CTRL_TMOUT_SHIFT)|SDIO_HOST_CTRL_PUSH_PULL_EN)




/* nor status mask */
#define SDIO_NOR_UNEXP_RSP			((unsigned short)0x1 << 14)
#define SDIO_NOR_AUTOCMD12_DONE		((unsigned short)0x1 << 13)
#define SDIO_NOR_SUSPENSE_ON		((unsigned short)0x1 << 12)
#define SDIO_NOR_LMB_FF_8W_AVAIL	((unsigned short)0x1 << 11)
#define SDIO_NOR_LMB_FF_8W_FILLED	((unsigned short)0x1 << 10)
#define SDIO_NOR_READ_WAIT_ON		((unsigned short)0x1 << 9)
#define SDIO_NOR_CARD_INT			((unsigned short)0x1 << 8)
#define SDIO_NOR_READ_READY			((unsigned short)0x1 << 5)
#define SDIO_NOR_WRITE_READY		((unsigned short)0x1 << 4)
#define SDIO_NOR_DMA_INI			((unsigned short)0x1 << 3)
#define SDIO_NOR_BLK_GAP_EVT		((unsigned short)0x1 << 2)
#define SDIO_NOR_XFER_DONE			((unsigned short)0x1 << 1)
#define SDIO_NOR_CMD_DONE			((unsigned short)0x1 << 0)

/* err status mask */
#define SDIO_ERR_CRC_STATUS			((unsigned short)0x1 << 14)
#define SDIO_ERR_CRC_STARTBIT		((unsigned short)0x1 << 13)
#define SDIO_ERR_CRC_ENDBIT			((unsigned short)0x1 << 12)
#define SDIO_ERR_RESP_TBIT			((unsigned short)0x1 << 11)
#define SDIO_ERR_SIZE				((unsigned short)0x1 << 10)
#define SDIO_ERR_CMD_STARTBIT		((unsigned short)0x1 << 9)
#define SDIO_ERR_AUTOCMD12			((unsigned short)0x1 << 8)
#define SDIO_ERR_DATA_ENDBIT		((unsigned short)0x1 << 6)
#define SDIO_ERR_DATA_CRC			((unsigned short)0x1 << 5)
#define SDIO_ERR_DATA_TIMEOUT		((unsigned short)0x1 << 4)
#define SDIO_ERR_CMD_INDEX			((unsigned short)0x1 << 3)
#define SDIO_ERR_CMD_ENDBIT			((unsigned short)0x1 << 2)
#define SDIO_ERR_CMD_CRC			((unsigned short)0x1 << 1)
#define SDIO_ERR_CMD_TIMEOUT		((unsigned short)0x1 << 0)


#define SDIO_ERR_INTR_MASK 0xFFFF


#define SDIO_CMD_RSP_NONE 				((unsigned short)0x0)
#define SDIO_CMD_RSP_136 				((unsigned short)0x1)
#define SDIO_CMD_RSP_48 				((unsigned short)0x2)
#define SDIO_CMD_RSP_48BUSY				((unsigned short)0x3)


#define MMC_BLOCK_SIZE                  512
#define MMC_CMD_RESET                   0
#define MMC_CMD_SEND_OP_COND            1
#define MMC_CMD_ALL_SEND_CID            2
#define MMC_CMD_SET_RCA                 3
#define MMC_CMD_SELECT_CARD		7
#define MMC_CMD_SEND_CSD                9
#define MMC_CMD_SEND_CID                10
#define MMC_CMD_SEND_STATUS             13
#define MMC_CMD_SET_BLOCKLEN            16
#define MMC_CMD_READ_BLOCK              17
#define MMC_CMD_RD_BLK_MULTI            18
#define MMC_CMD_WRITE_BLOCK             24

#define MMC_MAX_BLOCK_SIZE              512

//*****************************************************************************
// GPIO definition for SD host controller
// GPIO1 => EXTINT1 (IRQ24) a can be used as SD_CARD_IN
// GPIO6 => can be used as SD_WP
// GPIO7 => can be used as SD_ON

#define SDHC_GPIO_CARD_IN		0x00000002
#define SDHC_GPIO_WP			0x00000040
#define SDHC_GPIO_POWER			0x00000080

#define MUSTANG_GPIO_DIRECTION_OFFSET  		8
#define MUSTANG_GPIO_CLEAR_OFFSET 			4
#define MUSTANG_GPIO_SET_OFFSET 			0
#define MUSTANG_GPIO_DATAIN 			 	0

//SC Flag bit 0 for Card Detection
// if bit 0 of 0x11000030 is zero,
// Card Insertion interrupt will be detected
// if bit 0 of 0x11000030 is 1 
// card Removal interrupt will be detected
#define MUSTANG_SCFLAG_STATUS_OFFSET	0x0
#define MUSTANG_SCFLAG_SET_OFFSET		0x0
#define MUSTANG_SCFLAG_CLEAR_OFFSET		0x4

#define MUSTANG_SD_CARDDET_TOGGLE_BIT	0x1

typedef struct mmc_cid
{
	/* FIXME: BYTE_ORDER */
	uchar year:4,
	month:4;
	uchar sn[3];
	uchar fwrev:4,
	hwrev:4;
	uchar name[6];
	uchar id[3];
} mmc_cid_t;

typedef struct mmc_csd
{
	uchar   ecc:2,
		file_format:2,
		tmp_write_protect:1,
		perm_write_protect:1,
		copy:1,
		file_format_grp:1;
	uint64_t content_prot_app:1,
		rsvd3:4,
		write_bl_partial:1,
		write_bl_len:4,
		r2w_factor:3,
		default_ecc:2,
		wp_grp_enable:1,
		wp_grp_size:5,
		erase_grp_mult:5,
		erase_grp_size:5,
		c_size_mult1:3,
		vdd_w_curr_max:3,
		vdd_w_curr_min:3,
		vdd_r_curr_max:3,
		vdd_r_curr_min:3,
		c_size:12,
		rsvd2:2,
		dsr_imp:1,
		read_blk_misalign:1,
		write_blk_misalign:1,
		read_bl_partial:1;
	ushort  read_bl_len:4,
		ccc:12;
	uchar   tran_speed;
	uchar   nsac;
	uchar   taac;
	uchar   rsvd1:2,
		spec_vers:4,
		csd_structure:2;
} mmc_csd_t;                

