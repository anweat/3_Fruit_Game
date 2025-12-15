cmake_minimum_required(VERSION 3.16)

# 查找Qt Test模块和Sql模块
find_package(Qt6 REQUIRED COMPONENTS Test Sql)

# 核心源文件 (测试需要链接这些实现)
set(CORE_SOURCES_FOR_TEST
    ../src/core/FruitGenerator.cpp
    ../src/core/MatchDetector.cpp
    ../src/core/FallProcessor.cpp
    ../src/core/ScoreCalculator.cpp
    ../src/core/SpecialFruitGenerator.cpp
    ../src/core/SpecialEffectProcessor.cpp
    ../src/core/SwapHandler.cpp
    ../src/core/AnimationRecorder.cpp
    ../src/core/GameCycleProcessor.cpp
    ../src/core/GameEngine.cpp
    ../src/props/PropManager.cpp
    ../src/props/Hammer.cpp
    ../src/props/MagicWand.cpp
    ../src/props/Clamp.cpp
    ../src/data/Database.cpp
    ../src/achievement/AchievementManager.cpp
    ../src/achievement/detectors/AchievementDetectorManager.cpp
    ../src/achievement/detectors/DetectorFactory.cpp
    ../src/achievement/detectors/BeginnerAchievementDetector.cpp
    ../src/achievement/detectors/ChallengeAchievementDetector.cpp
    ../src/achievement/detectors/ComboAchievementDetector.cpp
    ../src/achievement/detectors/MilestoneAchievementDetector.cpp
    ../src/achievement/detectors/MultiMatchAchievementDetector.cpp
    ../src/achievement/detectors/PropAchievementDetector.cpp
    ../src/achievement/detectors/ScoreAchievementDetector.cpp
    ../src/achievement/detectors/SpecialAchievementDetector.cpp
)

# 测试源文件
set(TEST_SOURCES
    test_match_detector.cpp
    test_fruit_generator.cpp
    test_fall_processor.cpp
    test_score_calculator.cpp
    test_special_fruit_generator.cpp
    test_special_effect_processor.cpp
    test_game_engine.cpp
)

# 为每个测试文件创建可执行文件
foreach(TEST_SOURCE ${TEST_SOURCES})
    get_filename_component(TEST_NAME ${TEST_SOURCE} NAME_WE)
    
    add_executable(${TEST_NAME}
        ${TEST_SOURCE}
        ${CORE_SOURCES_FOR_TEST}
    )
    
    # 添加包含目录
    target_include_directories(${TEST_NAME} PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/src/core
        ${CMAKE_SOURCE_DIR}/src/props
        ${CMAKE_SOURCE_DIR}/src/data
        ${CMAKE_SOURCE_DIR}/src/achievement
        ${CMAKE_SOURCE_DIR}/src/achievement/detectors
    )
    
    target_link_libraries(${TEST_NAME}
        Qt6::Test
        Qt6::Core
        Qt6::Sql
    )
    
    add_test(NAME ${TEST_NAME} COMMAND ${TEST_NAME})
endforeach()

# 输出测试可执行文件到bin/tests目录
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/tests)
