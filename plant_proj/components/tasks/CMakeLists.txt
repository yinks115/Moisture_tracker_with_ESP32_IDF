idf_component_register(SRCS "moisture_tracker.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES wifi_handler moisture_sensor)