
#include "gpio.h"

void GPIOPin::setControl(Control c) const {
    setControl(port(), pin(), c);
}

void GPIOPin::irqInit() const
{
    /*
     * Use the External IRQ controller to make this pin an IRQ input.
     *
     * All pins with the same number share an EXTI controller. If you've
     * already used an external input on PA5, for example, PB5-PG5 are
     * not simultaneously usable for IRQ inputs.
     */

    unsigned portID = portIndex();
    unsigned pinID = pin();
    
    /*
     * Set EXTICRx. We have one nybble per EXTI controller, four nybbles
     * per word, and four words. This nybble is a multiplexor input
     * which selects a port ID for that EXTI controller to listen on.
     */

    volatile uint32_t *cr = &AFIO.EXTICR[pinID >> 2];
    unsigned crShift = (pinID & 3) << 2;
    *cr = (*cr & ~(0xF << crShift)) | (portID << crShift);
}
