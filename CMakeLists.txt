cmake_minimum_required(VERSION 3.5)

project(transport_catalogue LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(transport_catalog main.cpp
    geo.h


    transport_catalogue.h transport_catalogue.cpp
    domain.h domain.cpp
    geo.cpp
    json.h json.cpp
    svg.h svg.cpp
    json_reader.h json_reader.cpp

    map_renderer.h map_renderer.cpp
    json_builder.h json_builder.cpp
    graph.h
    ranges.h
    router.h
    transport_router.h transport_router.cpp)

include(GNUInstallDirs)
install(TARGETS transport_catalog
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
