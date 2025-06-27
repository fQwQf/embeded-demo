#ifndef S5_H
#define S5_H

#include "i2c.h"

// MS523 CMD
enum MS523_CMD
{
	PCD_IDLE       = 0x00,
	PCD_AUTHENT    = 0x0E,
	PCD_RECEIVE    = 0x08,
	PCD_TRANSMIT   = 0x04,
	PCD_TRANSCEIVE = 0x0C,
	PCD_RESETPHASE = 0x0F,
	PCD_CALCCRC    = 0x03,
};

// Mifare_One(S50) card CMD
enum PICC_CMD
{
	PICC_REQIDL    = 0x26,
	PICC_REQALL    = 0x52,
	PICC_ANTICOLL1 = 0x93,
	PICC_ANTICOLL2 = 0x95,
	PICC_AUTHENT1A = 0x60,
	PICC_AUTHENT1B = 0x61,
	PICC_READ      = 0x30,
	PICC_WRITE     = 0xA0,
	PICC_DECREMENT = 0xC0,
	PICC_INCREMENT = 0xC1,
	PICC_RESTORE   = 0xC2,
	PICC_TRANSFER  = 0xB0,
	PICC_HALT      = 0x50,
};

// MS523 register
// PAGE 0
#define RFU00              0x00
#define CommandReg         0x01
#define ComIEnReg          0x02
#define DivlEnReg          0x03
#define ComIrqReg          0x04
#define DivIrqReg          0x05
#define ErrorReg           0x06
#define Status1Reg         0x07
#define Status2Reg         0x08
#define FIFODataReg        0x09
#define FIFOLevelReg       0x0A
#define WaterLevelReg      0x0B
#define ControlReg         0x0C
#define BitFramingReg      0x0D
#define CollReg            0x0E
#define RFU0F              0x0F

// PAGE 1     
#define RFU10              0x10
#define ModeReg            0x11
#define TxModeReg          0x12
#define RxModeReg          0x13
#define TxControlReg       0x14
#define TxAutoReg          0x15
#define TxSelReg           0x16
#define RxSelReg           0x17
#define RxThresholdReg     0x18
#define DemodReg           0x19
#define RFU1A              0x1A
#define RFU1B              0x1B
#define MifareReg          0x1C
#define RFU1D              0x1D
#define RFU1E              0x1E
#define SerialSpeedReg     0x1F

// PAGE 2    
#define RFU20              0x20
#define CRCResultRegM      0x21
#define CRCResultRegL      0x22
#define RFU23              0x23
#define ModWidthReg        0x24
#define RFU25              0x25
#define RFCfgReg           0x26
#define GsNReg             0x27
#define CWGsCfgReg         0x28
#define ModGsCfgReg        0x29
#define TModeReg           0x2A
#define TPrescalerReg      0x2B
#define TReloadRegH        0x2C
#define TReloadRegL        0x2D
#define TCounterValueRegH  0x2E
#define TCounterValueRegL  0x2F

// PAGE 3
#define RFU30              0x30
#define TestSel1Reg        0x31
#define TestSel2Reg        0x32
#define TestPinEnReg       0x33
#define TestPinValueReg    0x34
#define TestBusReg         0x35
#define AutoTestReg        0x36
#define VersionReg         0x37
#define AnalogTestReg      0x38
#define TestDAC1Reg        0x39
#define TestDAC2Reg        0x3A
#define TestADCReg         0x3B
#define RFU3C              0x3C
#define RFU3D              0x3D
#define RFU3E              0x3E
#define RFU3F              0x3F

// ERROR CODE
#define MI_OK              0
#define MI_NOTAGERR        1
#define MI_ERR             2

/* NFC从机信息 */
extern i2c_slave_info s5_nfc_info;

/* NFC函数声明 */
i2c_slave_info s5_nfc_init(void);
char s5_nfc_request(i2c_slave_info info, unsigned char RequestType, unsigned char *pCardType);
char s5_nfc_anticoll(i2c_slave_info info, unsigned char *pCardID);
char s5_nfc_select(i2c_slave_info info, unsigned char *pCardID);
char s5_nfc_auth(i2c_slave_info info, unsigned char AuthMode, unsigned char addr, unsigned char *pKey, unsigned char *pCardID);
char s5_nfc_read(i2c_slave_info info, unsigned char addr, unsigned char *pData);
char s5_nfc_write(i2c_slave_info info, unsigned char addr, unsigned char *pData);
char s5_nfc_halt(i2c_slave_info info);

#endif /* S5_H */
