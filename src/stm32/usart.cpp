
#include "usart.h"
#include "sys.h"
#include "gpio.h"
#include "dma.h"

void Usart::init(GPIOPin rx, GPIOPin tx, int rate, bool dma, StopBits bits)
{
    if (uart == &USART1) {
        RCC.APB2ENR |= (1 << 14);

        if (dma) {
            dmaRxChan = Dma::initChannel(&DMA1, 4, dmaRXCallback, this);  // DMA1, channel 5
            dmaTxChan = Dma::initChannel(&DMA1, 3, dmaTXCallback, this);  // DMA1, channel 4
        }
    }
    else if (uart == &USART2) {
        RCC.APB1ENR |= (1 << 17);

        if (dma) {
            dmaRxChan = Dma::initChannel(&DMA1, 5, dmaRXCallback, this);  // DMA1, channel 6
            dmaTxChan = Dma::initChannel(&DMA1, 6, dmaTXCallback, this);  // DMA1, channel 7
        }
    }
    else if (uart == &USART3) {
        RCC.APB1ENR |= (1 << 18);

        if (dma) {
            dmaRxChan = Dma::initChannel(&DMA1, 2, dmaRXCallback, this);  // DMA1, channel 3
            dmaTxChan = Dma::initChannel(&DMA1, 1, dmaTXCallback, this);  // DMA1, channel 2
        }
    }
    else if (uart == &UART4) {
        RCC.APB1ENR |= (1 << 19);

        if (dma) {
            dmaRxChan = Dma::initChannel(&DMA2, 2, dmaRXCallback, this);  // DMA2, channel 3
            dmaTxChan = Dma::initChannel(&DMA2, 4, dmaTXCallback, this);  // DMA2, channel 5
        }
    }
    else if (uart == &UART5) {
        RCC.APB1ENR |= (1 << 20);

        if (dma) {
            dmaRxChan = Dma::initChannel(&DMA1, 1, dmaRXCallback, this);  // DMA1, channel 2
            dmaTxChan = Dma::initChannel(&DMA1, 2, dmaTXCallback, this);  // DMA1, channel 3
        }
    }

#if 0 //BOARD >= BOARD_TC_MASTER_REV3
    AFIO.MAPR |= (1 << 2);     // remap UART1 to PB6-7
#endif

    rx.setControl(GPIOPin::IN_FLOAT);
    tx.setControl(GPIOPin::OUT_ALT_50MHZ);

    // NOTE - these divisors must reflect the startup values configured in setup.cpp
    const unsigned APB2RATE = (Sys::CPU_HZ / 1);
    const unsigned APB1RATE = (Sys::CPU_HZ / 2);

    if (uart == &USART1) {
        uart->BRR = APB2RATE / rate;
    } else {
        uart->BRR = APB1RATE / rate;
    }

    uart->SR = 0;

    uart->CR1 = (1 << 13) |     // UE - enable
                (0 << 12) |     // M - word length, 8 bits
                (0 << 8) |      // PEIE - interrupt on parity error
                (1 << 5) |      // RXNEIE - interrupt on ORE (overrun) or RXNE (rx register not empty)
                (1 << 3) |      // TE - transmitter enable
                (1 << 2);       // RE - receiver enable
    uart->CR2 = (1 << 14) |     // LINEN - LIN mode enable
                (bits << 12);   // STOP - bits

    if (dma) {
        uart->CR3 = (1 << 7) |  // DMAT - dma TX
                    (1 << 6) |  // DMAR - dma RX
                    (1 << 0);   // EIE - error interrupt enable

        // point DMA channels at data register
        dmaRxChan->CPAR = (uint32_t)&uart->DR;
        dmaTxChan->CPAR = (uint32_t)&uart->DR;
    }

    (void)uart->SR;  // SR reset step 1.
    (void)uart->DR;  // SR reset step 2.
}

void Usart::deinit()
{
    // XXX: disable dma as well, if enabled
    uart->CR1 = 0;
    uart->CR2 = 0;
    uart->CR3 = 0;

    if (uart == &USART1) {
        RCC.APB2ENR &= ~(1 << 14);
    }
    else if (uart == &USART2) {
        RCC.APB1ENR &= ~(1 << 17);
    }
    else if (uart == &USART3) {
        RCC.APB1ENR &= ~(1 << 18);
    }
    else if (uart == &UART4) {
        RCC.APB1ENR &= ~(1 << 19);
    }
    else if (uart == &UART5) {
        RCC.APB1ENR &= ~(1 << 20);
    }
}

/*
 * Return the status register to indicate what kind of event we responded to.
 * If we received a byte and the caller provided a buf, give it to them.
 */
uint16_t Usart::isr(uint8_t &byte)
{
    uint16_t sr = uart->SR;
    uint16_t dr = uart->DR;  // always read DR to reset SR

    if (sr & (SR_PARITY_ERR | SR_FRAMING_ERR | SR_NOISE_ERR | SR_OVERRUN)) {
        // do something helpful?
#if 0
        __asm__("bkpt #0\n");
#endif
    }

    // RXNE: data available
    if (sr & SR_RXED) {
        byte = dr;
    }

    return sr;
}

void Usart::dmaTXCallback(void *p, uint8_t flags)
{
    /*
     * static handler for DMA TX events.
     */

    (void)flags;

    Usart *u = static_cast<Usart*>(p);
    u->dmaTxChan->CCR = 0;

    if (u->txCompletionCB) {
        u->txCompletionCB();
    }
}

void Usart::dmaRXCallback(void *p, uint8_t flags)
{
    /*
     * static handler for DMA RX events.
     */

    (void)flags;

    Usart *u = static_cast<Usart*>(p);
    u->dmaRxChan->CCR = 0;

    if (u->rxCompletionCB) {
        u->rxCompletionCB();
    }
}

/*
 * Just synchronous for now, to serve as a stupid data dump stream.
 */
void Usart::write(const uint8_t *buf, int size)
{
    while (size--) {
        put(*buf++);
    }
}

void Usart::write(const char *buf)
{
    while (*buf) {
        put(*buf++);
    }
}

void Usart::writeHex(uint32_t value, unsigned numDigits)
{
    static const char lut[] = "0123456789abcdef";

    // Left-justify
    value <<= (8 - numDigits) << 2;

    while (numDigits--) {
        put(lut[value >> 28]);
        value <<= 4;
    }
}

void Usart::writeHexBytes(const void *data, int size)
{
    const uint8_t *bytes = reinterpret_cast<const uint8_t *>(data);
    while (size > 0) {
        writeHex(*bytes, 2);
        bytes++;
        size--;
    }
}

void Usart::writeDma(const uint8_t *buf, unsigned len)
{
    uart->SR &= ~SR_TC;  // TC: clear transmission complete

    dmaTxChan->CNDTR = len;
    dmaTxChan->CMAR = (uint32_t)buf;
    dmaTxChan->CCR =    (1 << 7) |  // MINC - memory pointer increment
                        (1 << 4) |  // DIR - direction, 1 == read from memory
                        (1 << 3) |  // TEIE - transfer error ISR enable
                        (1 << 1);   // TCIE - transfer complete ISR enable

    // enable the TX channel
    dmaTxChan->CCR |= 0x1;
}

void Usart::readDma(const uint8_t *buf, unsigned len)
{
    dmaRxChan->CNDTR = len;
    dmaRxChan->CMAR = (uint32_t)buf;
    dmaRxChan->CCR = (1 << 7) |  // MINC - memory pointer increment
                     (0 << 4) |  // DIR - direction, 0 == read from peripheral
                     (1 << 3) |  // TEIE - transfer error ISR enable
                     (0 << 2) |  // HTIE - half complete ISR enable
                     (1 << 1);   // TCIE - transfer complete ISR enable

    // enable the RX channel
    dmaRxChan->CCR |= 0x1;
}

/*
 * Just synchronous for now, to serve as a stupid data dump stream.
 */
void Usart::read(uint8_t *buf, int size)
{
    while (size--) {
        while (!(uart->SR & SR_RXED)); // wait for data register to be not-not empty
        *buf++ = uart->DR;
    }
}

void Usart::put(char c)
{
    while (!(uart->SR & SR_TXED)); // wait for empty data register
    uart->DR = c;
}

char Usart::get()
{
    while (!(uart->SR & SR_RXED)); // wait for data register to be not-not empty
    return uart->DR;
}
