#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/kernel.h>
#include <linux/input.h>
#include <linux/interrupt.h>
#include <linux/i2c.h>
#include <linux/timer.h>
#include <linux/gpio.h>

#include <linux/sysfs.h>
#include <linux/init.h>
#include <linux/mutex.h>
#include <mach/gpio.h>
#include <linux/spinlock.h>
#include <linux/delay.h>
#include <linux/slab.h>

#include <linux/syscalls.h>
#include <linux/file.h>
#include <linux/fs.h>
#include <linux/fcntl.h>
#include <linux/string.h>
#include <asm/unistd.h>
#include <linux/cdev.h>
#include <asm/uaccess.h>

#define MSG21XX_VENDOR_API	1

//#define TP_DIFF

/* Added by zengguang to add head file. 2013.5.5 start*/
#ifdef CONFIG_GET_HARDWARE_INFO
#include <mach/hardware_info.h>
static char tmp_str[50];
#endif
/*  Added by zengguang to add head file. 2013.5.5 end*/
#define DEBUG
#ifdef DEBUG
#define TP_DEBUG(format, args...)   pr_info("TP_:%s( )_%d_: " format, \
	__FUNCTION__ , __LINE__, ## args);
#define DBG() pr_info("[%s]:%d => \n",__FUNCTION__,__LINE__)
#else
#define TP_DEBUG(format, args...);
#define DBG()
#endif
#define __FIRMWARE_UPDATE__

#ifdef  __FIRMWARE_UPDATE__

#define __CHIP_MSG_2133A__

#ifndef __CHIP_MSG_2133A__
	#define 	__CHIP_MSG_2133__
#endif

#endif

#ifndef CTP_PROXIMITY_FUN
#define CTP_PROXIMITY_FUN	1 //Mstar Mark.Li Add  2013-01-10; 
#endif 

#define u8 unsigned char
#define U8 unsigned char
#define u32 unsigned int
#define U32 unsigned int
#define s32 signed int
#define u16 ushort
#define U16 ushort
#define REPORT_PACKET_LENGTH  8
#define MSG21XX_INT_GPIO 1
#define MSG21XX_RESET_GPIO 0     //高电平时候正常工作
#define FLAG_GPIO    122
#define ST_KEY_SEARCH 	217
#define MSG21XX_RETRY_COUNT 3
#define MS_TS_MSG20XX_X_MIN   0
#define MS_TS_MSG20XX_Y_MIN   0
//#define MS_TS_MSG20XX_X_MAX   (240)
//#define MS_TS_MSG20XX_Y_MAX   (320)
#define MS_TS_MSG21XX_X_MAX   (480)
#define MS_TS_MSG21XX_Y_MAX   (854)
#define FT_I2C_VTG_MIN_UV	1800000
#define FT_I2C_VTG_MAX_UV	1800000
#define CFG_MAX_TOUCH_POINTS 	2	//add by zg

static int msg21xx_irq;
static struct i2c_client *msg21xx_i2c_client;

#ifdef __FIRMWARE_UPDATE__

#define __AUTO_UPDATE__

#define FW_ADDR_MSG20XX_TP   	 (0x4C>>1) //device address of msg20xx    7Bit I2C Addr == 0x26;
#define FW_ADDR_MSG20XX      	 (0xC4>>1)  
#define FW_UPDATE_ADDR_MSG20XX   (0x92>>1)
// static  char *fw_version;
#define DWIIC_MODE_ISP    0
#define DWIIC_MODE_DBBUS  1

#ifdef __CHIP_MSG_2133A__
static u8 g_dwiic_info_data[1024]={0};
static u8 temp[33][1024];   //
static int FwDataCnt;
static u32 crc32_table[256];
typedef enum
{
	EMEM_MAIN = 0,
	EMEM_INFO,
	EMEM_ALL,
}EMEM_TYPE_t;
#ifdef __AUTO_UPDATE__
extern u8 FwData_LB[33][1024];
extern u8 FwData_OFilm[33][1024];
#endif
#else
static u8 temp[94][1024];   //
static int FwDataCnt;
#endif
#endif

#ifdef __FIRMWARE_UPDATE__

/***********************************************************************************/

/**********************************************************************************/
static void HalTscrCReadI2CSeq(u8 addr, u8* read_data, u8 size)
{
   //according to your platform.
   	int rc;
 
	struct i2c_msg msgs[] = {
		{
			.addr = addr,
			.flags = I2C_M_RD,
			.len = size,
			.buf = read_data,
		},
	};
	
	rc = i2c_transfer(msg21xx_i2c_client->adapter, msgs, 1);
	if (rc < 0) {
		printk("HalTscrCReadI2CSeq error %d\n", rc);
		//printk("I2C addr= %d,size = %d,\n", addr,size);
	}

   
}

static void HalTscrCDevWriteI2CSeq(u8 addr, u8* data, u16 size)
{
    //according to your platform.

   	int rc;
 
	struct i2c_msg msgs[] = {
		{
			.addr = addr,
			.flags = 0,
			.len = size,
			.buf = data,
		},
	};
	rc = i2c_transfer(msg21xx_i2c_client->adapter, msgs, 1);
	if(rc < 0){
		printk("HalTscrCDevWriteI2CSeq error %d\n", rc);
		//printk("I2C addr= %d,size = %d,\n", addr,size);
	}

}
static void dbbusDWIICEnterSerialDebugMode(void)
{
    U8 data[5];

    // Enter the Serial Debug Mode
    data[0] = 0x53;
    data[1] = 0x45;
    data[2] = 0x52;
    data[3] = 0x44;
    data[4] = 0x42;
	mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, data, 5);
}

static void dbbusDWIICStopMCU(void)
{
    U8 data[1];

    // Stop the MCU
    data[0] = 0x37;
	mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, data, 1);
}

static void dbbusDWIICIICUseBus(void)
{
    U8 data[1];

    // IIC Use Bus
    data[0] = 0x35;
	mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, data, 1);
}

static void dbbusDWIICIICReshape(void)
{
    U8 data[1];

    // IIC Re-shape
    data[0] = 0x71;
	mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, data, 1);
}

#if 0
static void dbbusDWIICIICNotUseBus(void)
{
    U8 data[1];

    // IIC Not Use Bus
    data[0] = 0x34;
	mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, data, 1);
}

static void dbbusDWIICNotStopMCU(void)
{
    U8 data[1];

    // Not Stop the MCU
    data[0] = 0x36;
	mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, data, 1);
}

static void dbbusDWIICExitSerialDebugMode(void)
{
    U8 data[1];

    // Exit the Serial Debug Mode
    data[0] = 0x45;
	mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, data, 1);

    // Delay some interval to guard the next transaction
    //udelay ( 200 );        // delay about 0.2ms
}
#endif

#ifdef __CHIP_MSG_2133A__

static void drvDB_WriteReg(u8 bBank, u8 bAddr, u16 bData)
{
    u8 bWriteData[5];
    bWriteData[0]=0x10;
    bWriteData[1] = bBank;
    bWriteData[2] = bAddr;
    bWriteData[4] = bData>>8;
    bWriteData[3] = bData&0xFF;
    mdelay(15);	
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, bWriteData, 5);
}




static void drvDB_WriteReg8Bit(u8 bBank, u8 bAddr, u8 bData)
{
	u8 bWriteData[5];
	bWriteData[0] = 0x10;
	bWriteData[1] = bBank;
	bWriteData[2] = bAddr;
	bWriteData[3] = bData;
	mdelay(15);
	HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, bWriteData, 4);
}

static u16 drvDB_ReadReg(u8 bBank,u8 bAddr)
{
    u16 val=0;
    u8 bWriteData[3]={0x10,bBank,bAddr};
    u8 bReadData[2]={0x00,0x00};
    mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_ADDR_MSG20XX, bWriteData, 3);
    
    mdelay(15);
    HalTscrCReadI2CSeq(FW_ADDR_MSG20XX, &bReadData[0], 2);
	
    val=(bReadData[1]<<8)|(bReadData[0]);

    return val;
}



static u32 Reflect(u32 ref, char ch)//unsigned int Reflect(unsigned int ref, char ch)
{
	u32 value=0;
	u32 i=0;

	for(i = 1; i < (ch + 1); i++)
	{
		if(ref & 1)
			value |= 1 << (ch - i);
		ref >>= 1;
	}
	return value;
}
static void Init_CRC32_Table(void)
{
	u32 magicnumber = 0x04c11db7;
	u32 i=0,j;

	for(i = 0; i <= 0xFF; i++)
	{
		crc32_table[i]=Reflect(i, 8) << 24;
		for (j = 0; j < 8; j++)
		{
			crc32_table[i] = (crc32_table[i] << 1) ^ (crc32_table[i] & (0x80000000L) ? magicnumber : 0);
		}
		crc32_table[i] = Reflect(crc32_table[i], 32);				
	}
}

static u32 Get_CRC(u32 text,u32 prevCRC)
{
	u32  ulCRC = prevCRC;	
    	{ulCRC = (ulCRC >> 8) ^ crc32_table[(ulCRC & 0xFF) ^ text];}
	return ulCRC ;
}

static void _HalTscrHWReset(void)
{
	//gpio_direction_output(MSG21XX_RESET_GPIO, 1);
	gpio_set_value(MSG21XX_RESET_GPIO, 1);	
	gpio_direction_output(MSG21XX_RESET_GPIO, 1);
	gpio_set_value(MSG21XX_RESET_GPIO, 0);
	gpio_direction_output(MSG21XX_RESET_GPIO, 0);
    mdelay(20);  /* Note that the RST must be in LOW 10ms at least */
	gpio_direction_output(MSG21XX_RESET_GPIO, 1);
	gpio_set_value(MSG21XX_RESET_GPIO, 1);
    /* Enable the interrupt service thread/routine for INT after 50ms */
    mdelay(350);
}


static int drvTP_erase_emem_A ( EMEM_TYPE_t emem_type )
{
    // stop mcu
    drvDB_WriteReg ( 0x0F, 0xE6, 0x0001 );

    //disable watch dog
    drvDB_WriteReg8Bit ( 0x3C, 0x60, 0x55 );
    drvDB_WriteReg8Bit ( 0x3C, 0x61, 0xAA );

    // set PROGRAM password
    drvDB_WriteReg8Bit ( 0x16, 0x1A, 0xBA );
    drvDB_WriteReg8Bit ( 0x16, 0x1B, 0xAB );

    //proto.MstarWriteReg(F1.loopDevice, 0x1618, 0x80);
    drvDB_WriteReg8Bit ( 0x16, 0x18, 0x80 );

    if ( emem_type == EMEM_ALL )
    {
        drvDB_WriteReg8Bit ( 0x16, 0x08, 0x10 ); //mark
    }

    drvDB_WriteReg8Bit ( 0x16, 0x18, 0x40 );
    mdelay ( 10 );

    drvDB_WriteReg8Bit ( 0x16, 0x18, 0x80 );

    // erase trigger
    if ( emem_type == EMEM_MAIN )
    {
        drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x04 ); //erase main
    }
    else
    {
        drvDB_WriteReg8Bit ( 0x16, 0x0E, 0x08 ); //erase all block
    }

    return ( 1 );
}

#if 1
static int drvTP_read_info_dwiic_A ( void )
{
    u8  dwiic_tx_data[5];
    //u8  dwiic_rx_data[4];
    u16 reg_data=0;
    //unsigned char dbbus_rx_data[2] = {0};

    mdelay ( 300 );


    // Stop Watchdog
    drvDB_WriteReg8Bit ( 0x3C, 0x60, 0x55 );
    drvDB_WriteReg8Bit ( 0x3C, 0x61, 0xAA );

    drvDB_WriteReg ( 0x3C, 0xE4, 0xA4AB );

	drvDB_WriteReg ( 0x1E, 0x04, 0x7d60 );

    // TP SW reset
    drvDB_WriteReg ( 0x1E, 0x04, 0x829F );
	mdelay (1);
	
    dwiic_tx_data[0] = 0x10;
    dwiic_tx_data[1] = 0x0F;
    dwiic_tx_data[2] = 0xE6;
    dwiic_tx_data[3] = 0x00;
    mdelay(15);	
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dwiic_tx_data, 4 );	
  
        // stop mcu
  //  drvDB_WriteReg ( 0x1E, 0xE6, 0x0001 );
	
    mdelay ( 100 );
	TP_DEBUG ( "read infor 1\n");
	
    do{
        reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
    }
    while ( reg_data != 0x5B58 );
	TP_DEBUG ( "read infor +++2\n");
    dwiic_tx_data[0] = 0x72;

   // dwiic_tx_data[3] = 0x04;
  //  dwiic_tx_data[4] = 0x00;	
    dwiic_tx_data[3] = 0x00;
    dwiic_tx_data[4] = 0x80;	

    for(reg_data=0;reg_data<8;reg_data++)
    {
    	dwiic_tx_data[1] = 0x80+(((reg_data*128)&0xff00)>>8);
    	dwiic_tx_data[2] = (reg_data*128)&0x00ff;
    	HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX_TP , dwiic_tx_data, 5 );
	
    	mdelay (50 );

    // recive info data
    	HalTscrCReadI2CSeq ( FW_ADDR_MSG20XX_TP, &g_dwiic_info_data[reg_data*128], 128);
    }
    return ( 1 );
}
#endif
#else

static void drvISP_EntryIspMode(void)
{
    U8 bWriteData[5] =
    {
        0x4D, 0x53, 0x54, 0x41, 0x52
    };
mdelay(15);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 5);

    //udelay ( 1000 );        // delay about 1ms
}

static U8 drvISP_Read(U8 n, U8* pDataToRead)    //First it needs send 0x11 to notify we want to get flash data back.
{
    U8 Read_cmd = 0x11;
    unsigned char dbbus_rx_data[2] = {0};
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &Read_cmd, 1);
    if (n == 1)
    {
        HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG20XX, &dbbus_rx_data[0], 2);
        *pDataToRead = dbbus_rx_data[0];
    }
    else
        HalTscrCReadI2CSeq(FW_UPDATE_ADDR_MSG20XX, pDataToRead, n);

    return 0;
}

static void drvISP_WriteEnable(void)
{
    U8 bWriteData[2] =
    {
        0x10, 0x06
    };
    U8 bWriteData1 = 0x12;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);
}


static void drvISP_ExitIspMode(void)
{
    U8 bWriteData = 0x24;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData, 1);
}

static U8 drvISP_ReadStatus(void)
{
    U8 bReadData = 0;
    U8 bWriteData[2] =
    {
        0x10, 0x05
    };
    U8 bWriteData1 = 0x12;

    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 2);
    drvISP_Read(1, &bReadData);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);
    return bReadData;
}


static void drvISP_BlockErase(U32 addr)
{
    U8 bWriteData[5] = { 0x00, 0x00, 0x00, 0x00, 0x00 };
    U8 bWriteData1 = 0x12;

    drvISP_WriteEnable();

    //Enable write status register
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x50;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);

    //Write Status
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x01;
    bWriteData[2] = 0x00;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 3);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);

    //Write disable
    bWriteData[0] = 0x10;
    bWriteData[1] = 0x04;
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 2);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);

    while ((drvISP_ReadStatus() & 0x01) == 0x01);

    drvISP_WriteEnable();

    bWriteData[0] = 0x10;
    bWriteData[1] = 0xC7;//0xD8;        //Block Erase
    //bWriteData[2] = ((addr >> 16) & 0xFF) ;
    //bWriteData[3] = ((addr >> 8) & 0xFF) ;
    //bWriteData[4] = (addr & 0xFF) ;
	HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 2);
    //HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData, 5);
    HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);

    while ((drvISP_ReadStatus() & 0x01) == 0x01);
}

static void drvISP_Program(U16 k, U8* pDataToWrite)
{
    U16 i = 0;
    U16 j = 0;
    //U16 n = 0;
    U8 TX_data[133];
    U8 bWriteData1 = 0x12;
    U32 addr = k * 1024;

    for (j = 0; j < 8; j++)   //128*8 cycle
    {
        TX_data[0] = 0x10;
        TX_data[1] = 0x02;// Page Program CMD
        TX_data[2] = (addr + 128 * j) >> 16;
        TX_data[3] = (addr + 128 * j) >> 8;
        TX_data[4] = (addr + 128 * j);
        for (i = 0; i < 128; i++)
        {
            TX_data[5 + i] = pDataToWrite[j * 128 + i];
        }
        while ((drvISP_ReadStatus() & 0x01) == 0x01);     //wait until not in write operation
        drvISP_WriteEnable();
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, TX_data, 133);   //write 133 byte per cycle
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);
    }
}


static void drvISP_Verify(U16 k, U8* pDataToVerify)
{
    U16 i = 0, j = 0;
    U8 bWriteData[5] =
    {
        0x10, 0x03, 0, 0, 0
    };
    U8 RX_data[256];
    U8 bWriteData1 = 0x12;
    U32 addr = k * 1024;
    U8 index=0;
    for (j = 0; j < 8; j++)   //128*8 cycle
    {
        bWriteData[2] = (U8)((addr + j * 128) >> 16);
        bWriteData[3] = (U8)((addr + j * 128) >> 8);
        bWriteData[4] = (U8)(addr + j * 128);
        while ((drvISP_ReadStatus() & 0x01) == 0x01);     //wait until not in write operation
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, bWriteData, 5);    //write read flash addr
        drvISP_Read(128, RX_data);
        HalTscrCDevWriteI2CSeq(FW_UPDATE_ADDR_MSG20XX, &bWriteData1, 1);    //cmd end
        for (i = 0; i < 128; i++)   //log out if verify error
        {
        if((RX_data[i]!=0)&&index<10)
		{
        TP_DEBUG("j=%d,RX_data[%d]=0x%x\n",j,i,RX_data[i]);
        index++;
		}
            if (RX_data[i] != pDataToVerify[128 * j + i])
            {
                TP_DEBUG("k=%d,j=%d,i=%d===============Update Firmware Error================",k,j,i);
            }
        }
    }
}


static void _HalTscrHWReset(void)
{
	gpio_direction_output(MSG21XX_RESET_GPIO, 1);
	gpio_set_value(MSG21XX_RESET_GPIO, 1);	
	gpio_set_value(MSG21XX_RESET_GPIO, 0);
    mdelay(10);  /* Note that the RST must be in LOW 10ms at least */
	gpio_set_value(MSG21XX_RESET_GPIO, 1);
    /* Enable the interrupt service thread/routine for INT after 50ms */
    mdelay(50);
}
#endif

#ifdef __FIRMWARE_UPDATE__
#ifdef __AUTO_UPDATE__
static int firmware_auto_update_c33(EMEM_TYPE_t emem_type)
{
    u8  life_counter[2];
    u32 i, j;
    u32 crc_main, crc_main_tp;
    u32 crc_info, crc_info_tp;
  
    int update_pass = 1;
    u16 reg_data = 0;

    crc_main = 0xffffffff;
    crc_info = 0xffffffff;
	
	//msg21xx_i2c_client->timing = 100;
    drvTP_read_info_dwiic_A();
	
    if ( g_dwiic_info_data[0] == 'M' && g_dwiic_info_data[1] == 'S' && g_dwiic_info_data[2] == 'T' && g_dwiic_info_data[3] == 'A' && g_dwiic_info_data[4] == 'R' && g_dwiic_info_data[5] == 'T' && g_dwiic_info_data[6] == 'P' && g_dwiic_info_data[7] == 'C' )
    {
        // updata FW Version
        //drvTP_info_updata_C33 ( 8, &temp[32][8], 5 );

		g_dwiic_info_data[8]=temp[32][8];
		g_dwiic_info_data[9]=temp[32][9];
		g_dwiic_info_data[10]=temp[32][10];
		g_dwiic_info_data[11]=temp[32][11];
        // updata life counter
        life_counter[1] = (( ( (g_dwiic_info_data[13] << 8 ) | g_dwiic_info_data[12]) + 1 ) >> 8 ) & 0xFF;
        life_counter[0] = ( ( (g_dwiic_info_data[13] << 8 ) | g_dwiic_info_data[12]) + 1 ) & 0xFF;
		g_dwiic_info_data[12]=life_counter[0];
		g_dwiic_info_data[13]=life_counter[1];
        //drvTP_info_updata_C33 ( 10, &life_counter[0], 3 );
        drvDB_WriteReg ( 0x3C, 0xE4, 0x78C5 );
		drvDB_WriteReg ( 0x1E, 0x04, 0x7d60 );
        // TP SW reset
        drvDB_WriteReg ( 0x1E, 0x04, 0x829F );

        mdelay ( 100 );
        //polling 0x3CE4 is 0x2F43
        do
        {
            reg_data = drvDB_ReadReg (0x3C, 0xE4);
        }
        while ( reg_data != 0x2F43 );

        // transmit lk info data
        for(i=0;i<8;i++)
        {
        HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX_TP , &g_dwiic_info_data[i*128], 128 );
        }
        //polling 0x3CE4 is 0xD0BC
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0xD0BC );

    }

    //erase main
    drvTP_erase_emem_A ( EMEM_MAIN );
    mdelay ( 1000 );
    //ResetSlave();
    _HalTscrHWReset();
	//	mdelay ( 300 );
    //drvDB_EnterDBBUS();
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    mdelay ( 300 );
    /////////////////////////
    // Program
    /////////////////////////

    //polling 0x3CE4 is 0x1C70
    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0x1C70 );
    }

    switch ( emem_type )
    {
        case EMEM_ALL:
            drvDB_WriteReg ( 0x3C, 0xE4, 0xE38F );  // for all-blocks
            break;
        case EMEM_MAIN:
            drvDB_WriteReg ( 0x3C, 0xE4, 0x7731 );  // for main block
            break;
        case EMEM_INFO:
            drvDB_WriteReg ( 0x3C, 0xE4, 0x7731 );  // for info block

            drvDB_WriteReg8Bit ( 0x0F, 0xE6, 0x01 );

            drvDB_WriteReg8Bit ( 0x3C, 0xE4, 0xC5 ); //
            drvDB_WriteReg8Bit ( 0x3C, 0xE5, 0x78 ); //

            drvDB_WriteReg8Bit ( 0x1E, 0x04, 0x9F );
            drvDB_WriteReg8Bit ( 0x1E, 0x05, 0x82 );

            drvDB_WriteReg8Bit ( 0x0F, 0xE6, 0x00 );
            mdelay ( 100 );
            break;
    }
	mdelay ( 100 );
    // polling 0x3CE4 is 0x2F43
    do
    {
        reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
    }
    while ( reg_data != 0x2F43 );

    // calculate CRC 32
    Init_CRC32_Table ();

    for ( i = 0; i < 33; i++ ) // total  33 KB : 2 byte per R/W
    {
        if ( emem_type == EMEM_INFO )
			i = 32;

        if ( i < 32 )   //emem_main
        {
            if ( i == 31 )
            {
                temp[i][1014] = 0x5A; //Fmr_Loader[1014]=0x5A;
                temp[i][1015] = 0xA5; //Fmr_Loader[1015]=0xA5;

                for ( j = 0; j < 1016; j++ )
                {
                    //crc_main=Get_CRC(Fmr_Loader[j],crc_main,&crc_tab[0]);
                    crc_main = Get_CRC ( temp[i][j], crc_main);
                }
            }
            else
            {
                for ( j = 0; j < 1024; j++ )
                {
                    //crc_main=Get_CRC(Fmr_Loader[j],crc_main,&crc_tab[0]);
                    crc_main = Get_CRC ( temp[i][j], crc_main);
                }
            }
        }
        else  //emem_info
        {
            for ( j = 0; j < 1024; j++ )
            {
                //crc_info=Get_CRC(Fmr_Loader[j],crc_info,&crc_tab[0]);
                crc_info = Get_CRC ( g_dwiic_info_data[j], crc_info);
            }
            if ( emem_type == EMEM_MAIN ) break;
        }

        //drvDWIIC_MasterTransmit( DWIIC_MODE_DWIIC_ID, 1024, Fmr_Loader );
        for(j=0;j<8;j++)
        {
        	HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX_TP, &temp[i][j*128], 128 );
        }
		mdelay ( 100 );
        // polling 0x3CE4 is 0xD0BC
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }
        while ( reg_data != 0xD0BC );

        drvDB_WriteReg ( 0x3C, 0xE4, 0x2F43 );
    }

    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        // write file done and check crc
        drvDB_WriteReg ( 0x3C, 0xE4, 0x1380 );
    }
    mdelay ( 10 ); //MCR_CLBK_DEBUG_DELAY ( 10, MCU_LOOP_DELAY_COUNT_MS );

    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        // polling 0x3CE4 is 0x9432
        do
        {
            reg_data = drvDB_ReadReg ( 0x3C, 0xE4 );
        }while ( reg_data != 0x9432 );
    }

    crc_main = crc_main ^ 0xffffffff;
    crc_info = crc_info ^ 0xffffffff;

    if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        // CRC Main from TP
        crc_main_tp = drvDB_ReadReg ( 0x3C, 0x80 );
        crc_main_tp = ( crc_main_tp << 16 ) | drvDB_ReadReg ( 0x3C, 0x82 );

        // CRC Info from TP
        crc_info_tp = drvDB_ReadReg ( 0x3C, 0xA0 );
        crc_info_tp = ( crc_info_tp << 16 ) | drvDB_ReadReg ( 0x3C, 0xA2 );
    }
    TP_DEBUG ( "crc_main=0x%x, crc_info=0x%x, crc_main_tp=0x%x, crc_info_tp=0x%x\n",
               crc_main, crc_info, crc_main_tp, crc_info_tp );

    //drvDB_ExitDBBUS();

    update_pass = 1;
	if ( ( emem_type == EMEM_ALL ) || ( emem_type == EMEM_MAIN ) )
    {
        if ( crc_main_tp != crc_main )
            update_pass = 0;

        if ( crc_info_tp != crc_info )
            update_pass = 0;
    }

    if ( !update_pass )
    {
        printk ( "update FAILED\n" );
		_HalTscrHWReset();
        FwDataCnt = 0;
    	enable_irq(msg21xx_irq);
        return ( 0 );
    }

    printk ( "update OK\n" );
	_HalTscrHWReset();
    FwDataCnt = 0;
    enable_irq(msg21xx_irq);
	
    return ( 1 );
}

static int firmware_auto_update(void)
{
   u8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};

	disable_irq(msg21xx_irq);

    _HalTscrHWReset();

    // Erase TP Flash first
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    mdelay ( 300 );

    // Disable the Watchdog
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 4 );
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 4 );
    // Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 4 );
    /////////////////////////
    // Difference between C2 and C3
    /////////////////////////
	// c2:2133 c32:2133a(2) c33:2138
    //check id
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0xCC;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 3 );
    HalTscrCReadI2CSeq ( FW_ADDR_MSG20XX, &dbbus_rx_data[0], 2 );
    if ( dbbus_rx_data[0] == 2 )
    {
        // check version
        dbbus_tx_data[0] = 0x10;
        dbbus_tx_data[1] = 0x3C;
        dbbus_tx_data[2] = 0xEA;
        HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 3 );
        HalTscrCReadI2CSeq ( FW_ADDR_MSG20XX, &dbbus_rx_data[0], 2 );
        TP_DEBUG ( "dbbus_rx version[0]=0x%x", dbbus_rx_data[0] );

        if ( dbbus_rx_data[0] == 3 ){
            return firmware_auto_update_c33(EMEM_MAIN );
		}
    }
    enable_irq(msg21xx_irq);
    return 0;
}
#endif

#endif
#endif

#if 0
/*add for read ic type*************************************************************/
#define	CTP_ID_MSG21XX		1
#define	CTP_ID_MSG21XXA		2

#define FW_ADDR_MSG20XX_TP   	 (0x4C>>1) //device address of msg20xx    7Bit I2C Addr == 0x26;
#define FW_ADDR_MSG20XX      	 (0xC4>>1)  
// #define FW_ADDR_MSG21XX   (0xC4>>1)
// #define FW_ADDR_MSG21XX_TP   (0x4C>>1)
static int Check_Device(void)
{
	int ret;
    u8 dbbus_tx_data[4];
    unsigned char dbbus_rx_data[2] = {0};
    u8 curr_ic_type;
	
	_HalTscrHWReset();
    disable_irq(msg21xx_irq);
    mdelay ( 100 );
    
    dbbusDWIICEnterSerialDebugMode();
    dbbusDWIICStopMCU();
    dbbusDWIICIICUseBus();
    dbbusDWIICIICReshape();
    mdelay ( 100 );

    // Disable the Watchdog
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x60;
    dbbus_tx_data[3] = 0x55;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 4 );
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x3C;
    dbbus_tx_data[2] = 0x61;
    dbbus_tx_data[3] = 0xAA;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 4 );
    // Stop MCU
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x0F;
    dbbus_tx_data[2] = 0xE6;
    dbbus_tx_data[3] = 0x01;
    HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 4 );
    /////////////////////////
    // Difference between C2 and C3
    /////////////////////////
    // c2:2133 c32:2133a(2) c33:2138
    //check id
    dbbus_tx_data[0] = 0x10;
    dbbus_tx_data[1] = 0x1E;
    dbbus_tx_data[2] = 0xCC;
    //HalTscrCDevWriteI2CSeq ( FW_ADDR_MSG20XX, dbbus_tx_data, 3 );
    //HalTscrCReadI2CSeq ( FW_ADDR_MSG21XX, &dbbus_rx_data[0], 2 );
     
    msg21xx_i2c_client->addr = FW_ADDR_MSG20XX;
    ret = i2c_master_send(msg21xx_i2c_client, dbbus_tx_data, 3);    
    if(ret <= 0)
    {   
        msg21xx_i2c_client->addr = FW_ADDR_MSG20XX_TP;
		TP_DEBUG("Device error %d,addr = %d\n", ret,FW_ADDR_MSG20XX);
		return -1;
	}
	
    ret = i2c_master_recv(msg21xx_i2c_client, dbbus_rx_data, 2);
    msg21xx_i2c_client->addr = FW_ADDR_MSG20XX_TP;
    if(ret <= 0)
    {
		TP_DEBUG("Device error %d,addr = %d\n", ret,FW_ADDR_MSG20XX);
		return -1;
	}
	#ifdef __FIRMWARE_UPDATE__
    if ( dbbus_rx_data[0] == 2 )
    {
    	curr_ic_type = CTP_ID_MSG21XXA;
    }
    else
    {
    	curr_ic_type = CTP_ID_MSG21XX;
    }
	TP_DEBUG("CURR_IC_TYPE = %d \n",curr_ic_type);
	#endif
    dbbusDWIICIICNotUseBus();
    dbbusDWIICNotStopMCU();
    dbbusDWIICExitSerialDebugMode();
    enable_irq(msg21xx_irq);
   
    return 1;
}
#endif
