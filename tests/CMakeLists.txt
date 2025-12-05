cmake_minimum_required(VERSION 3.16)

# 查找Qt Test模块
find_package(Qt6 REQUIRED COMPONENTS Test)

# 测试源文件
set(TEST_SOURCES
    test_match_detector.cpp
    test_fruit_generator.cpp
    test_score_calculator.cpp
)

# 为每个测试文件创建可执行文件
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
    )
    
    target_link_libraries(${TEST_NAME}
        Qt6::Test
        Qt6::Core
    )
    
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# 输出测试可执行文件到bin/tests目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests)
