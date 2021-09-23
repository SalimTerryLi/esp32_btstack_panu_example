# BTStack BNEP tether on ESP-IDF

This is a working example for bluetooth tethering on ESP32. With the UGLY workaround by copying private header out of existing IDF component.

## Howto

**This repo already contains a btstack port provided by [mringwal](https://github.com/bluekitchen/btstack/tree/master/port/esp32)** as component. Compiled and tested on IDF V4.3 and not released V4.4

Modify `const char * remote_addr_string = "a4:6b:b6:3f:df:67";` in `bnep-tether.c` to your BT MAC.

Build with standard IDF way:

```
// first init IDF environment with alias `get_idf` or sth else.
idf.py build flash monitor
```

Pairing the device from your PC/Phone the first time and trust it. It is named `PANU xx:xx:xx:xx:xx:xx`.

Then reset ESP32 and let it run over again.

Then watch the output:

```
I (625) bnep_tether: SDP service record size: 169
I (625) BTDM_INIT: BT controller compile version [1e3e264]
I (625) system_api: Base MAC address is not set
I (635) system_api: read default base MAC address from EFUSE
I (645) phy_init: phy_version 4670,719f9f6,Feb 18 2021,17:07:07
BTstack up and running at 8C:AA:B5:B5:A8:2A
I (1255) bnep_tether: Start SDP BNEP query for remote PAN Network Access Point (NAP).
I (5985) bnep_tether: SDP BNEP Record complete
I (5985) bnep_tether: SDP Record: Nr: 0
I (5985) bnep_tether: SDP Attribute 0x0001: BNEP PAN protocol UUID: 1116
I (5985) bnep_tether: SDP Attribute: 0x0004
I (5995) bnep_tether: Summary: uuid 0x1116, l2cap_psm 0x000f, bnep_version 0x0100
I (5995) bnep_tether: SDP Attribute: 0x0100: Network service
I (6005) bnep_tether: SDP Attribute: 0x0101: Network service
I (6035) bnep_tether: SDP BNEP Record complete
I (6035) bnep_tether: General query done with status 0, bnep psm 000f.
I (6175) bnep_tether: BNEP connection open succeeded to A4:6B:B6:3F:DF:67 source UUID 0x1115 dest UUID: 0x1116, max frame size 1676
I (10185) bnep_tether: TAP interface Got IP Address
I (10185) bnep_tether: ~~~~~~~~~~~
I (10185) bnep_tether: IP: 10.110.209.240
I (10185) bnep_tether: Netmask: 255.255.255.0
I (10195) bnep_tether: Gateway: 10.110.209.1
I (10195) bnep_tether: ~~~~~~~~~~~
I (10725) esp-x509-crt-bundle: Certificate validated
I (13485) httpc: HTTPS Status = 200
I (13485) httpc: HTTP_EVENT_DISCONNECTED
```

That's all.
