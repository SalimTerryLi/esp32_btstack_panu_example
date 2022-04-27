idf_component_register(
        SRCS "bnep-tether.c" "main.c" "http_client.c"
        PRIV_REQUIRES esp_netif btstack esp-tapif esp_http_client esp-tls
)
