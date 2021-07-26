set(srcs "src/esp_eth_phy_btstack_bnep.c")
set(include "include")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${include}
                    REQUIRES btstack)
