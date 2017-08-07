#include "dsm.h"
#include "inputs.h"
#include "cameracontrol.h"
#include "flightmanager.h"
#include "tasks.h"

Dsm Dsm::instance;

void Dsm::init()
{
    for (unsigned i = 0; i < arraysize(channels); ++i) {
        channels[i] = DsmLowVal;
    }

}

void Dsm::onLoiterButtonEvt(Button *b, Button::Event evt)
{
    UNUSED(b);

    // OPEN SOLO MOD: Toggle RC CH7 high and low with the pause button
    // Hold (1.5 sec) sets CH7 low.  Long Hold (3 sec) sets CH7 high.
    if ((evt == Button::Release) && ((b)->wasHeldLong())) {
        channels[DsmCh7] = DsmHighVal;
        Haptic::startPattern(Haptic::SingleLong);
        Ui::instance.pendEvent(Event::CH7high);    
    } else if ((evt == Button::Release) && ((b)->wasHeld())) {
        channels[DsmCh7] = DsmLowVal;
        Haptic::startPattern(Haptic::SingleMedium);
        Ui::instance.pendEvent(Event::CH7low);
    }
}

void Dsm::onBButtonEvt(Button *b, Button::Event evt)
{
    UNUSED(b);

    // OPEN SOLO MOD: Toggle RC CH8 high and low with the A button
    // Hold (1.5 sec) sets CH8 low.  Long Hold (3 sec) sets CH8 high.
    if ((evt == Button::Release) && ((b)->wasHeldLong())) {
        channels[DsmCh8] = DsmHighVal;
        Haptic::startPattern(Haptic::SingleLong);
        Ui::instance.pendEvent(Event::CH8high);    
    } else if ((evt == Button::Release) && ((b)->wasHeld())) {
        channels[DsmCh8] = DsmLowVal;
        Haptic::startPattern(Haptic::SingleMedium);
        Ui::instance.pendEvent(Event::CH8low);
    }

}

void Dsm::producePacket(HostProtocol::Packet &pkt)
{
    pkt.delimitSlip();
    pkt.appendSlip(HostProtocol::DsmChannels);

    // first 4 channels are stick axes
    for (unsigned i = 0; i < 4; ++i) {
        const StickAxis & stick = Inputs::stick(Io::StickID(i));
        channels[i] = stick.angularPPMValue();
#if 0
        // (not used currently) Send partial vehicle control (assisted landing with broken stick for advanced users)
        if (stick.hasValidInput()) {
            channels[i] = stick.angularPPMValue();
        } else {
            channels[i] = stick.angularPPMDefault(); // send default value if stick value is invalid
        }
#endif
    }

    // TODO: vehicle does not have explicit knowledge when receiving this information that the sticks are invalid, app and Artoo do
    const StickAxis & stickGimbalY = Inputs::stick(Io::StickGimbalY);
    if (stickGimbalY.hasInvalidInput()) {
        channels[DsmCh6] = CameraControl::instance.targetPositionDefault();
        // channels[DsmCh8] = stickGimbalY.scaledAngularDefault();
    } else {
        channels[DsmCh6] = CameraControl::instance.targetPosition();
        // channels[DsmCh8] = stickGimbalY.scaledAngularValue();
    }

    pkt.appendSlip(channels, sizeof(channels));

    pkt.delimitSlip();
}
