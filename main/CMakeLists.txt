idf_component_register(SRCS 

    main.cpp

    config.h

    settings.h
    settings.cpp

    wifi.h
    wifi.cpp

    logs.h
    logs.cpp

    http_server.h
    http_server.cpp

    mdns_manager.h
    mdns.cpp

INCLUDE_DIRS 
"../."
"../etl/include"
)

target_add_binary_data(${COMPONENT_LIB} "${CMAKE_CURRENT_SOURCE_DIR}/logs.html" TEXT)