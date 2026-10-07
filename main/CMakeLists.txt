idf_component_register(SRCS "app_main.c" "cJSON.c" "display.c" "oled_display.c" "potreader.c"
                    "flashmem.c" "factoryreset.c" "apwebserver/server.c" "ota/ota.c" "rotator.c" 
                    "statistics/statistics.c" "device/device.c" 
            INCLUDE_DIRS "."
            EMBED_TXTFILES ${project_dir}/server_certs/ca_cert.pem)
target_compile_options(${COMPONENT_LIB} PRIVATE "-Wno-format")
