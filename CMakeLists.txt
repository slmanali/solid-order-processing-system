cmake_minimum_required(VERSION 3.20)

project(
    OrderFlow
    VERSION 1.0.0
    DESCRIPTION "A C++20 order-processing application demonstrating SOLID principles"
    LANGUAGES CXX
)

option(ORDERFLOW_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)

add_library(orderflow
    src/application/checkout_service.cpp
    src/application/order_query_service.cpp
    src/domain/money.cpp
    src/domain/order.cpp
    src/infrastructure/in_memory_order_repository.cpp
    src/infrastructure/stream_order_notifier.cpp
    src/strategies/discounts.cpp
    src/strategies/payments.cpp
    src/strategies/shipping.cpp
)

target_include_directories(orderflow PUBLIC ${PROJECT_SOURCE_DIR}/include)
target_compile_features(orderflow PUBLIC cxx_std_20)
set_target_properties(orderflow PROPERTIES CXX_EXTENSIONS OFF)

function(orderflow_enable_warnings target)
    if(MSVC)
        target_compile_options(${target} PRIVATE /W4 /permissive-)
        if(ORDERFLOW_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE /WX)
        endif()
    else()
        target_compile_options(
            ${target}
            PRIVATE -Wall -Wextra -Wpedantic -Wconversion -Wshadow
        )
        if(ORDERFLOW_WARNINGS_AS_ERRORS)
            target_compile_options(${target} PRIVATE -Werror)
        endif()
    endif()
endfunction()

orderflow_enable_warnings(orderflow)

add_executable(orderflow_cli app/main.cpp)
target_link_libraries(orderflow_cli PRIVATE orderflow)
set_target_properties(orderflow_cli PROPERTIES OUTPUT_NAME orderflow)
orderflow_enable_warnings(orderflow_cli)

include(CTest)
if(BUILD_TESTING)
    add_executable(orderflow_tests tests/test_main.cpp)
    target_link_libraries(orderflow_tests PRIVATE orderflow)
    orderflow_enable_warnings(orderflow_tests)
    add_test(NAME orderflow_tests COMMAND orderflow_tests)
endif()

include(GNUInstallDirs)
install(TARGETS orderflow orderflow_cli
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
