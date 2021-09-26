//
// Created by salimterryli on 8/25/21.
//
#pragma once

#ifndef BNEP_ETHERNET_BT_COD_H
#define BNEP_ETHERNET_BT_COD_H

// https://www.ampedrftech.com/guides/cod_definition.pdf

#define BT_COD_GIAC 0x9E8B33
#define BT_COD_LIAC 0x9E8B00
#define BT_COD_GEN(major_serv, major_dev, minor_dev) ((major_serv) | (major_dev) | (minor_dev))

#define BT_COD_MAJOR_SERV_LIMITED_DISCOVERABLE  (1 << 12)   // Limited Discoverable Mode
#define BT_COD_MAJOR_SERV_POSITIONING           (1 << 15)   // Location identification
#define BT_COD_MAJOR_SERV_NETWORKING            (1 << 16)   // LAN, Ad hoc, ...
#define BT_COD_MAJOR_SERV_RENDERING             (1 << 17)   // Printing, Speaker, ...
#define BT_COD_MAJOR_SERV_CAPTURING             (1 << 18)   // Scanner, Microphone, ...
#define BT_COD_MAJOR_SERV_OBJECT_TRANSFER       (1 << 19)   // v-Inbox, v-Folder, ...
#define BT_COD_MAJOR_SERV_AUDIO                 (1 << 20)   // Speaker, Microphone, Headset service, ...
#define BT_COD_MAJOR_SERV_TELEPHONY             (1 << 21)   // Cordless telephony, Modem, Headset service, ...
#define BT_COD_MAJOR_SERV_INFORMATION           (1 << 22)   // WEB-server, WAP-server, ...

#define BT_COD_MAJOR_DEV_MISCELLANEOUS  (0b00000 << 8)  // Miscellaneous
#define BT_COD_MAJOR_DEV_COMPUTER       (0b00001 << 8)  // desktop,notebook, PDA, organizers, ....
#define BT_COD_MAJOR_DEV_PHONE          (0b00010 << 8)  // cellular, cordless, payphone, modem, ...
#define BT_COD_MAJOR_DEV_LAN_NAP        (0b00011 << 8)  // LAN /Network Access point
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO    (0b00100 << 8)  // headset,speaker,stereo, video display, vcr.....
#define BT_COD_MAJOR_DEV_PERIPHERAL     (0b00101 << 8)  // mouse, joystick, keyboards, .....
#define BT_COD_MAJOR_DEV_IMAGING        (0b00110 << 8)  // printing, scanner, camera, display, ...
#define BT_COD_MAJOR_DEV_WEARABLE       (0b00111 << 8)  // Wearable
#define BT_COD_MAJOR_DEV_TOY            (0b01000 << 8)  // Toy
#define BT_COD_MAJOR_DEV_HEALTH         (0b01001 << 8)  // Health
#define BT_COD_MAJOR_DEV_UNCATEGORIZED  (0b11111 << 8)  // Uncategorized, specific device code not specified

// BT_COD_MAJOR_DEV_COMPUTER
#define BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_UNCATEGORIZED   (0b000000 << 2)   // Uncategorized, code for device not assigned
#define BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_DESKTOP         (0b000001 << 2)   // Desktop workstation
#define BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_SERVER          (0b000010 << 2)   // Server-class computer
#define BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_LAPTOP          (0b000011 << 2)   // Laptop
#define BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_HANDHELD        (0b000100 << 2)   // Handheld PC/PDA (clam shell)
#define BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_PALM_SIZE       (0b000101 << 2)   // Palm sized PC/PDA
#define BT_COD_MAJOR_DEV_COMPUTER_MINOR_DEV_WEARABLE        (0b000110 << 2)   // Wearable computer (Watch sized)

// BT_COD_MAJOR_DEV_PHONE
#define BT_COD_MAJOR_DEV_PHONE_MINOR_DEV_UNCATEGORIZED  (0b000000 << 2) // Uncategorized, code for device not assigned
#define BT_COD_MAJOR_DEV_PHONE_MINOR_DEV_CELLULAR       (0b000001 << 2) // Cellular
#define BT_COD_MAJOR_DEV_PHONE_MINOR_DEV_CORDLESS       (0b000010 << 2) // Cordless
#define BT_COD_MAJOR_DEV_PHONE_MINOR_DEV_SMART_PHONE    (0b000011 << 2) // Smart phone
#define BT_COD_MAJOR_DEV_PHONE_MINOR_DEV_MODEM          (0b000100 << 2) // Wired modem or voice gateway
#define BT_COD_MAJOR_DEV_PHONE_MINOR_DEV_COMMON_ISDN    (0b000101 << 2) // Common ISDN Access

// BT_COD_MAJOR_DEV_LAN_NAP
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_FULLY_AVAILABLE  (0b000 << 5) // Fully available
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_1_17_PERCENT     (0b001 << 5) // 1 - 17% utilized
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_17_33_PERCENT    (0b010 << 5) // 17 - 33% utilized
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_33_50_PERCENT    (0b011 << 5) // 33 - 50% utilized
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_50_67_PERCENT    (0b100 << 5) // 50 - 67% utilized
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_67_83_PERCENT    (0b101 << 5) // 67 - 83% utilized
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_83_99_PERCENT    (0b110 << 5) // 83 - 99% utilized
#define BT_COD_MAJOR_DEV_LAN_NAP_MINOR_DEV_NOT_AVAILABLE    (0b111 << 5) // No service available

// BT_COD_MAJOR_DEV_AUDIO_VIDEO
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_UNCATEGORIZED        (0b000000 << 2) // Uncategorized, code not assigned
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_WEARABLE_HEADSET     (0b000001 << 2) // Wearable Headset Device
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_HANDS_FREE           (0b000010 << 2) // Hands-free Device
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_MICROPHONE           (0b000100 << 2) // Microphone
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_LOUDSPEAKER          (0b000101 << 2) // Loudspeaker
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_HEADPHONE            (0b000110 << 2) // Headphones
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_PORTABLE_AUDIO       (0b000111 << 2) // Portable Audio
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_CAR_AUDIO            (0b001000 << 2) // Car audio
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_SET_TOP_BOX          (0b001001 << 2) // Set-top box
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_HIFI_AUDIO           (0b001010 << 2) // HiFi Audio Device
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_VCR                  (0b001011 << 2) // VCR
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_VIDEO_CAMERA         (0b001100 << 2) // Video Camera
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_CAMCORDER            (0b001101 << 2) // Camcorder
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_VIDEO_MONITOR        (0b001110 << 2) // Video Monitor
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_VIDEO_DISPLAY        (0b001111 << 2) // Video Display and Loudspeaker
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_VIDEO_CONFERENCING   (0b010000 << 2) // Video Conferencing
#define BT_COD_MAJOR_DEV_AUDIO_VIDEO_MINOR_DEV_GAMING_TOY           (0b010010 << 2) // Gaming/Toy

// BT_COD_MAJOR_DEV_PERIPHERAL
#define BT_COD_MAJOR_DEV_PERIPHERAL_GEN(dev, sub_dev) ((dev) | (sub_dev))
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_DEV_NOT_KB_NOT_POINTER    (0b00 << 6) // Not Keyboard / Not Pointing Device
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_DEV_KEYBOARD              (0b01 << 6) // Keyboard
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_DEV_POINTING              (0b10 << 6) // Pointing device
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_DEV_KB_AND_POINTER        (0b11 << 6) // Combo keyboard/pointing device
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_SUBDEV_UNCATEGORIZED      (0b0000 << 2)   // Uncategorized device
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_SUBDEV_JOYSTICK           (0b0001 << 2)   // Joystick
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_SUBDEV_GAMEPAD            (0b0010 << 2)   // Gamepad
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_SUBDEV_REMOTE_CONTROL     (0b0011 << 2)   // Remote control
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_SUBDEV_SENSING            (0b0100 << 2)   // Sensing device
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_SUBDEV_DIGITIZER_TABLET   (0b0101 << 2)   // Digitizer tablet
#define BT_COD_MAJOR_DEV_PERIPHERAL_MINOR_SUBDEV_CARD_READER        (0b0110 << 2)   // Card Reader (e.g. SIM Card Reader)

// BT_COD_MAJOR_DEV_IMAGING
#define BT_COD_MAJOR_DEV_IMAGING_MINOR_DEV_DISPLAY  (0b000100 << 2) // Display
#define BT_COD_MAJOR_DEV_IMAGING_MINOR_DEV_CAMERA   (0b001000 << 2) // Camera
#define BT_COD_MAJOR_DEV_IMAGING_MINOR_DEV_SCANNER  (0b010000 << 2) // Scanner
#define BT_COD_MAJOR_DEV_IMAGING_MINOR_DEV_PRINTER  (0b100000 << 2) // Printer

// BT_COD_MAJOR_DEV_WEARABLE
#define BT_COD_MAJOR_DEV_WEARABLE_MINOR_DEV_WRIST_WATCH (0b000001 << 2)
#define BT_COD_MAJOR_DEV_WEARABLE_MINOR_DEV_PAGER       (0b000010 << 2)
#define BT_COD_MAJOR_DEV_WEARABLE_MINOR_DEV_JACKET      (0b000011 << 2)
#define BT_COD_MAJOR_DEV_WEARABLE_MINOR_DEV_HELMET      (0b000100 << 2)
#define BT_COD_MAJOR_DEV_WEARABLE_MINOR_DEV_GLASSES     (0b000101 << 2)

// BT_COD_MAJOR_DEV_TOY
#define BT_COD_MAJOR_DEV_TOY_MINOR_DEV_ROBOT        (0b000000 << 2) // Robot
#define BT_COD_MAJOR_DEV_TOY_MINOR_DEV_VEHICLE      (0b000000 << 2) // Vehicle
#define BT_COD_MAJOR_DEV_TOY_MINOR_DEV_DOLL         (0b000000 << 2) // Doll / Action Figure
#define BT_COD_MAJOR_DEV_TOY_MINOR_DEV_CONTROLLER   (0b000000 << 2) // Controller
#define BT_COD_MAJOR_DEV_TOY_MINOR_DEV_GAME         (0b000000 << 2) // Game

// BT_COD_MAJOR_DEV_HEALTH
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_UNDEFINED             (0b000000 << 2)
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_BLOOD_PRES            (0b000001 << 2)
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_THERMOMETER           (0b000010 << 2)
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_WEIGHING_SCALE        (0b000011 << 2)
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_GLUCOSE_METER         (0b000100 << 2)
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_PULSE_OXIMETER        (0b000101 << 2)
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_HEART_RATE            (0b000110 << 2)
#define BT_COD_MAJOR_DEV_HEALTH_MINOR_DEV_HEALTH_DATA_DISPLAY   (0b000111 << 2)

#endif //BNEP_ETHERNET_BT_COD_H
