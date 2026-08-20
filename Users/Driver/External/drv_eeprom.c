#include "drv_eeprom.h"
#include <stdint.h>

/*==================================================================================================
*                                PRIVATE FUNCTIONS DECLARATIONS
==================================================================================================*/

/**
 * @brief  Assert the EEPROM chip select signal.
 *
 * This function selects the EEPROM device by driving the
 * chip select (CS) pin to the active level.
 */
static inline void drv_eeprom_select(void);

/**
 * @brief  Deassert the EEPROM chip select signal.
 *
 * This function deselects the EEPROM device by driving the
 * chip select (CS) pin to the inactive level.
 */
static inline void drv_eeprom_deselect(void);

/**
 * @brief          Enable write EEPROM
 * @details        This function enable write data to EEPROM by sending WREN command
 * @param[in]      None
 * @return         None
 */
static void drv_eeprom_write_enable(void);

/**
 * @brief          Disable write EEPROM
 * @details        This function disable write data to EEPROM by sending WRDI command
 * @param[in]      None
 * @return         None
 */
static void drv_eeprom_write_disable(void);

/**
 * @brief          Read Status Register EEPROM
 * @details        This function Read Status Register of EEPROM
 * @param[in]      None
 * @return         status register value
 */
static uint8_t drv_eeprom_read_status_reg(void);

/**
 * @brief          Write Status Register EEPROM
 * @details        This function Write Status Register of EEPROM
 * @param[in]      None
 * @return         None
 */
static bool drv_eeprom_write_status_reg(uint8_t u8StatusReg);

/**
 * @brief          Check EEPROM busy flag
 * @details        This function read Status register and check WIP bit
 * @param[in]      None
 * @return         true if EEPROM is busy, false if EEPROM ready
 */
static bool drv_eeprom_is_busy(void);

/*==================================================================================================
*                                   PUBLIC FUNCTIONS DEFINITIONS
==================================================================================================*/

void drv_eeprom_init(void)
{
    drv_spi_init();
    uint8_t u8StatusReg = (EE_SR_SRWD_DIS << EE_SR_SRWD_POS) | (EE_SR_BP_NONE << EE_SR_BP_POS);
    drv_eeprom_write_status_reg(u8StatusReg);
}

bool drv_eeprom_read_data(uint16_t u16Address, uint8_t *pData, uint16_t u16Size)
{
    if ((pData == NULL) || (u16Size == 0) || (u16Address + u16Size > EE_TOTAL_SIZE))
    {
        return false;
    }

    if (drv_eeprom_is_busy())
    {
        sys_delay_ms(EE_WAIT_TIMEOUT);
        if (drv_eeprom_is_busy())
        {
            return false;
        }
    }

    uint8_t au8TxBuf[] = {EE_CMD_READ, (uint8_t)(u16Address >> 8), (uint8_t)(u16Address & 0xFF)};

    drv_eeprom_select();
    drv_spi_send(au8TxBuf, sizeof(au8TxBuf));
    drv_spi_receive(pData, u16Size);
    drv_eeprom_deselect();

    return true;
}

bool drv_eeprom_write_data(uint16_t u16Address, const uint8_t *pData, uint16_t u16Size)
{
    if ((pData == NULL) || (u16Size == 0) || (u16Address + u16Size > EE_TOTAL_SIZE))
    {
        return false;
    }

    if (drv_eeprom_is_busy())
    {
        sys_delay_ms(EE_WAIT_TIMEOUT);
        if (drv_eeprom_is_busy())
        {
            return false;
        }
    }

    uint16_t u16Writed = 0; // Number of bytes successfully written

    while (u16Writed < u16Size)
    {
        uint16_t u16WritableThisPage = EE_PAGE_SIZE - (u16Address % EE_PAGE_SIZE);                                               // Calculate remaining writable bytes in current page
        uint16_t u16WriteThisPage = ((u16Size - u16Writed) > u16WritableThisPage) ? u16WritableThisPage : (u16Size - u16Writed); // Determine how many bytes to write in this page cycle

        // Prepare transmit buffer: Command + Address + Data
        uint8_t au8TxBuf[EE_CMD_SIZE + EE_ADDR_SIZE + EE_PAGE_SIZE];
        au8TxBuf[0] = EE_CMD_WRITE;
        au8TxBuf[1] = (uint8_t)((u16Address & 0xFF00) >> 8);
        au8TxBuf[2] = (uint8_t)(u16Address & 0x00FF);
        for (uint16_t i = 0; i < u16WriteThisPage; i++) // Copy chunk of data into transmit buffer
        {
            au8TxBuf[EE_CMD_SIZE + EE_ADDR_SIZE + i] = pData[u16Writed + i];
        }

        drv_eeprom_write_enable(); // Enable write operation on EEPROM
        drv_eeprom_select();
        drv_spi_send(au8TxBuf, EE_CMD_SIZE + EE_ADDR_SIZE + u16WriteThisPage);
        drv_eeprom_deselect();

        // Wait until EEPROM completes internal write cycle
        if (drv_eeprom_is_busy())
        {
            sys_delay_ms(EE_WAIT_TIMEOUT);
            if (drv_eeprom_is_busy())
            {
                return false;
            }
        }

        // Update counters and next address
        u16Writed += u16WriteThisPage;
        u16Address += u16WriteThisPage;
    }

    return true;
}

/*==================================================================================================
*                                   PRIVATE FUNCTIONS DEFINITIONS
==================================================================================================*/

static inline void drv_eeprom_select(void)
{
    drv_gpio_write(SPI_CS_EE_PIN, 0);
}

static inline void drv_eeprom_deselect(void)
{
    drv_gpio_write(SPI_CS_EE_PIN, 1);
}

static void drv_eeprom_write_enable(void)
{
    uint8_t u8Cmd = EE_CMD_WREN;
    drv_eeprom_select();
    drv_spi_send(&u8Cmd, 1);
    drv_eeprom_deselect();
}

static void drv_eeprom_write_disable(void)
{
    uint8_t u8Cmd = EE_CMD_WRDI;
    drv_eeprom_select();
    drv_spi_send(&u8Cmd, 1);
    drv_eeprom_deselect();
}

static uint8_t drv_eeprom_read_status_reg(void)
{
    uint8_t u8StatusReg;

    uint8_t u8Cmd = EE_CMD_RDSR;
    drv_eeprom_select();
    drv_spi_send(&u8Cmd, 1);
    drv_spi_receive(&u8StatusReg, 1);
    drv_eeprom_deselect();

    return u8StatusReg;
}

static bool drv_eeprom_write_status_reg(uint8_t u8StatusReg)
{
    if (drv_eeprom_is_busy())
    {
        sys_delay_ms(EE_WAIT_TIMEOUT);
        if (drv_eeprom_is_busy())
        {
            return false;
        }
    }

    uint8_t u8TxBuf[] = {EE_CMD_WRSR, u8StatusReg};
    drv_eeprom_write_enable(); // Enable write operation on EEPROM
    drv_eeprom_select();
    drv_spi_send(u8TxBuf, sizeof(u8TxBuf));
    drv_eeprom_deselect();
    drv_eeprom_write_disable(); // Disable write operation on EEPROM

    return true;
}

static bool drv_eeprom_is_busy(void)
{
    uint8_t u8StatusReg = drv_eeprom_read_status_reg();

    if ((u8StatusReg & EE_SR_WIP_MASK) >> EE_SR_WIP_POS == EE_SR_WIP_BUSY)
    {
        return true;
    }

    return false;
}