# BTStack BNEP tether on ESP-IDF

This is a working example for bluetooth tethering on ESP32. With the UGLY workaround by copying private header out of existing IDF component.

## Howto

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
BTstack up and running at 78:E3:6D:CD:07:E2
I (859) bnep_tether: Start SDP BNEP query for remote PAN Network Access Point (NAP).
I (1583) bnep_tether: SDP BNEP Record complete
I (1583) bnep_tether: SDP Record: Nr: 0
I (1583) bnep_tether: SDP Attribute 0x0001: BNEP PAN protocol UUID: 1116
I (1589) bnep_tether: SDP Attribute: 0x0004
I (1594) bnep_tether: Summary: uuid 0x1116, l2cap_psm 0x000f, bnep_version 0x0100
I (1602) bnep_tether: SDP Attribute: 0x0100: Network service
I (1608) bnep_tether: SDP Attribute: 0x0101: Network service
I (1631) bnep_tether: SDP BNEP Record complete
I (1632) bnep_tether: General query done with status 0, bnep psm 000f.
I (1694) bnep_tether: BNEP connection open succeeded to A4:6B:B6:3F:DF:67 source UUID 0x1115 dest UUID: 0x1116, max frame size 1676
I (2698) bnep_tether: TAP interface Got IP Address
I (2699) bnep_tether: ~~~~~~~~~~~
I (2699) bnep_tether: IP: 10.110.209.156
I (2702) bnep_tether: Netmask: 255.255.255.0
I (2707) bnep_tether: Gateway: 10.110.209.1
I (2711) bnep_tether: ~~~~~~~~~~~

```

That's all.
