cmake_minimum_required(VERSION 3.5)

project(transport_catalogue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(transport_catalog main.cpp
    geo.h
    input_reader.h input_reader.cpp
    stat_reader.h stat_reader.cpp
    transport_catalogue.h transport_catalogue.cpp)

include(GNUInstallDirs)
install(TARGETS transport_catalog
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
