# Project Ancorage: LEGO Control Plus Wireless Protocol v3.0 client

# Build status
[![Actions Status](https://github.com/shtras/ancorage/workflows/CMake/badge.svg)](https://github.com/shtras/ancorage/actions)

# Description
This project allows you to control Lego Control Plus models from your Windows PC that has a bluetooth adapter.  
The UI is next to not existent and is just a debugging frontend for XInput controller.  
VC++ Redistributable needs to be installed. You can get it from here: https://support.microsoft.com/en-us/help/2977003/the-latest-supported-visual-c-downloads

# Disclamer
Heavy WiP. Will not work. Don't try this at home. Under construction.

# Roadmap
- [x] Basic motor support
- [x] Servo motor support
- [x] Stepper motor support
- [x] Keyboard control
- [x] XInput control (XBox compatible gamepad)
- [ ] DirectInput control (Generic gamepad)
- [ ] Profile support for controller bindings
- [x] Multi hub support
- [x] Configurable JSON profile
- [ ] Multiple motors in profile (bindings for pairs of motors working in a synchronized way)
- [ ] Receiving feedback messages from hubs (servo/stepper position, etc.)
- [ ] Synchronized motor mode (motors working in pairs)
- [ ] UI rework from scratch
- [ ] UI profile editing
- [ ] UI hub selection

# Instructions
Be aware that at this stage of the development this project is in no way ready for public use and is full of black magic and dirty hacks.  
Use the following instruction at your own risk.

## Profile preparation
1. Pair your Technic Hub with your PC
1. Find it in the device manager under "Bluetooth"
1. In the "Device inctance path" section copy the value, convert to lower case and get the part that comes after the last backslash
1. This is your hub id for the profile.
1. Take the sample_profile.json and put it in your working directory as profile.json
1. Paste the id from the item above in the desired hub section
1. This sample profile contains pre-defined profiles for 42114 as "Truck" and 42099 as "Buggy". Modify as desired.

## Running
1. In the "Device" menu section find your hub name and select "Connect" (multiple hubs can be connected at the same time)
1. Click outside the main window and then inside, so that the focus would be on the window and not any of the widgets
1. Use the keys you defined in the profile to control your model

# Credits
This project is inspired by https://github.com/imurvai/brickcontroller2  
Probably the only BLE API example on the internet https://social.msdn.microsoft.com/Forums/en-US/bad452cb-4fc2-4a86-9b60-070b43577cc9/is-there-a-simple-example-desktop-programming-c-for-bluetooth-low-energy-devices?forum=wdk  
LEGO Wireless protocol description https://lego.github.io/lego-ble-wireless-protocol-docs/index.html
