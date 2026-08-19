#include "drv_uart.h"
#include "usart.h"

static uint8_t au8Uart1RxBuf[UART1_RX_BUFFER_SIZE];
static volatile uint16_t u16Uart1RxSize;
static volatile bool bUart1RxFlag = false;

static uint8_t au8Uart2RxBuf[UART2_RX_BUFFER_SIZE];
static volatile uint16_t u16Uart2RxSize;
static volatile bool bUart2RxFlag = false;

static uint8_t au8Uart3RxBuf[UART3_RX_BUFFER_SIZE];
static volatile uint16_t u16Uart3RxCurrentPointer;
static volatile uint16_t u16Uart3RxPreviousPointer;
static volatile bool bUart3RxFlag = false;

/**
 * @brief  Flush Uart3 RX buffer.
 *
 * This function moves the previous DMA pointer to the current DMA pointer,
 * so old unread data is ignored.
 */
static void drv_uart3_flush_rx_buffer(void);

void drv_uart1_init(uint32_t u32Baud, uint32_t u32Parity)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = u32Baud;
    huart1.Init.WordLength = UART_WORDLENGTH_9B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = u32Parity;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, au8Uart1RxBuf, UART1_RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT | DMA_IT_TC);
}

void drv_uart1_deinit(void)
{
    HAL_UART_MspDeInit(&huart1);
}

void drv_uart1_send(const uint8_t *pData, uint16_t u16Size)
{
    HAL_UART_Transmit(&huart1, pData, u16Size, UART_POLLING_TIMEOUT);
}

bool drv_uart1_receive(uint8_t *pData, uint16_t *u16Size)
{
    if ((pData == NULL) || (u16Size == NULL)) return false;

    if (!bUart1RxFlag) return false;

    *u16Size = u16Uart1RxSize;
    memcpy(pData, au8Uart1RxBuf, *u16Size);

    bUart1RxFlag = false;
    return true;
}

void drv_uart2_init(uint32_t u32Baud, uint32_t u32Parity)
{
    huart2.Instance = USART2;
    huart2.Init.BaudRate = u32Baud;
    huart2.Init.WordLength = UART_WORDLENGTH_8B;
    huart2.Init.StopBits = UART_STOPBITS_1;
    huart2.Init.Parity = u32Parity;
    huart2.Init.Mode = UART_MODE_TX_RX;
    huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart2.Init.OverSampling = UART_OVERSAMPLING_16;
    huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart2) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, au8Uart2RxBuf, UART2_RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT | DMA_IT_TC);
}

void drv_uart2_deinit(void)
{
    HAL_UART_MspDeInit(&huart2);
}

void drv_uart2_send(const uint8_t *pData, uint16_t u16Size)
{
    HAL_UART_Transmit(&huart2, pData, u16Size, UART_POLLING_TIMEOUT);
}

bool drv_uart2_receive(uint8_t *pData, uint16_t *u16Size)
{
    if ((pData == NULL) || (u16Size == NULL)) return false;

    if (!bUart2RxFlag) return false;

    *u16Size = u16Uart2RxSize;
    memcpy(pData, au8Uart2RxBuf, *u16Size);

    bUart2RxFlag = false;
    return true;
}

void drv_uart3_init(uint32_t u32Baud, uint32_t u32Parity)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = u32Baud;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = u32Parity;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart3.Init.OverSampling = UART_OVERSAMPLING_16;
    huart3.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
    huart3.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
    if (HAL_UART_Init(&huart3) != HAL_OK)
    {
        Error_Handler();
    }

    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, au8Uart3RxBuf, UART3_RX_BUFFER_SIZE);
    __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT | DMA_IT_TC);
}

void drv_uart3_deinit(void)
{
    HAL_UART_MspDeInit(&huart3);
}

void drv_uart3_send(const uint8_t *pData, uint16_t u16Size)
{
    drv_uart3_flush_rx_buffer();
    HAL_UART_Transmit(&huart3, pData, u16Size, UART_POLLING_TIMEOUT);
}

bool drv_uart3_receive(uint8_t *pData, uint16_t *u16Size)
{
    static bool bFrameStarted = false;
    static uint32_t u32LastRxTime = 0;

    if ((pData == NULL) || (u16Size == NULL)) return false;

    *u16Size = 0;

    // Check whether new DMA data is available
    if (bUart3RxFlag && (u16Uart3RxCurrentPointer != u16Uart3RxPreviousPointer))
    {
        bUart3RxFlag = false;
        bFrameStarted = true;
        u32LastRxTime = sys_time_ms();
    }

    if ((bFrameStarted == true) && (sys_time_ms() - u32LastRxTime >= UART_RX_FRAME_TIMEOUT))
    {
        bFrameStarted = false;
        if (u16Uart3RxCurrentPointer > u16Uart3RxPreviousPointer)
        {
            *u16Size = u16Uart3RxCurrentPointer - u16Uart3RxPreviousPointer;
            memcpy(pData, &au8Uart3RxBuf[u16Uart3RxPreviousPointer], *u16Size);
        }
        else
        {
            memcpy(pData, &au8Uart3RxBuf[u16Uart3RxPreviousPointer], UART3_RX_BUFFER_SIZE - u16Uart3RxPreviousPointer);
            if (u16Uart3RxCurrentPointer > 0)
            {
                memcpy(&pData[UART3_RX_BUFFER_SIZE - u16Uart3RxPreviousPointer], &au8Uart3RxBuf[0], u16Uart3RxCurrentPointer);
            }
            *u16Size = u16Uart3RxCurrentPointer + UART3_RX_BUFFER_SIZE - u16Uart3RxPreviousPointer;
        }

        u16Uart3RxPreviousPointer = u16Uart3RxCurrentPointer;
        return true;
    }

    return false;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        bUart1RxFlag = true;
        u16Uart1RxSize = Size;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, au8Uart1RxBuf, UART1_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT | DMA_IT_TC);
    }

    if (huart->Instance == USART2)
    {
        bUart2RxFlag = true;
        u16Uart2RxSize = Size;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, au8Uart2RxBuf, UART2_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT | DMA_IT_TC);
    }

    if (huart->Instance == USART3)
    {
        bUart3RxFlag = true;
        u16Uart3RxCurrentPointer = Size;
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, au8Uart3RxBuf, UART3_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT | DMA_IT_TC);
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart1, au8Uart1RxBuf, UART1_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT | DMA_IT_TC);
    }

    if (huart->Instance == USART2)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart2, au8Uart2RxBuf, UART2_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart2.hdmarx, DMA_IT_HT | DMA_IT_TC);
    }

    if (huart->Instance == USART3)
    {
        HAL_UARTEx_ReceiveToIdle_DMA(&huart3, au8Uart3RxBuf, UART3_RX_BUFFER_SIZE);
        __HAL_DMA_DISABLE_IT(huart3.hdmarx, DMA_IT_HT | DMA_IT_TC);
    }
}

static void drv_uart3_flush_rx_buffer(void)
{
    u16Uart3RxPreviousPointer = u16Uart3RxCurrentPointer;
}