#ifndef USBTO1WIRE_H
#define USBTO1WIRE_H

#include <stdint.h>

extern uint32_t onewirespeed;

void USBto1Wire_setup();
void USBto1Wire_loop();

#endif
