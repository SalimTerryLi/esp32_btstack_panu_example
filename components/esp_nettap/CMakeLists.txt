set(srcs "src/esp_nettap.c" "src/esp_netif_lwip_tap.c" "src/esp_netif_lwip_defaults.c" "src/esp_netif_defaults.c")
set(include "include")

idf_component_register(SRCS ${srcs}
                    INCLUDE_DIRS ${include}
                    REQUIRES esp_netif)
