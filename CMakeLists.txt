idf_component_register(SRCS "main.c" "camera.c" "IODs.c"
                       REQUIRES spi_flash esp_driver_uart
                       INCLUDE_DIRS "")
