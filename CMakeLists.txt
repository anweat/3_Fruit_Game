cmake_minimum_required(VERSION 3.16)

project(FruitCrush VERSION 1.0.0 LANGUAGES CXX)

# 生成compile_commands.json供IDE使用
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 平台检测
if(WIN32)
    message(STATUS "Building for Windows")
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    message(STATUS "Building for macOS")
    set(PLATFORM_NAME "macOS")
elseif(UNIX)
    message(STATUS "Building for Linux/Unix")
    set(PLATFORM_NAME "Linux")
endif()

# Qt设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets Sql)

# 查找OpenGL（跨平台）
find_package(OpenGL REQUIRED)

# 输出路径设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/props
    ${CMAKE_SOURCE_DIR}/src/mode
    ${CMAKE_SOURCE_DIR}/src/achievement
    ${CMAKE_SOURCE_DIR}/src/data
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/ui/views/animation
    ${CMAKE_SOURCE_DIR}/ui/input
)

# 源文件
set(CORE_SOURCES
    src/core/GameEngine.cpp
    src/core/MapManager.cpp
    src/core/MatchDetector.cpp
    src/core/FruitGenerator.cpp
    src/core/FallProcessor.cpp
    src/core/ScoreCalculator.cpp
    src/core/SpecialFruitGenerator.cpp
    src/core/SpecialEffectProcessor.cpp
    src/core/SwapHandler.cpp
    src/core/AnimationRecorder.cpp
    src/core/GameCycleProcessor.cpp
)

set(CORE_HEADERS
    src/core/FruitTypes.h
    src/core/GameEngine.h
    src/core/MapManager.h
    src/core/MatchDetector.h
    src/core/FruitGenerator.h
    src/core/FallProcessor.h
    src/core/ScoreCalculator.h
    src/core/SpecialFruitGenerator.h
    src/core/SpecialEffectProcessor.h
    src/core/SwapHandler.h
    src/core/AnimationRecorder.h
    src/core/GameCycleProcessor.h
)

# 已删除: src/special/ 目录 (功能已由 SpecialFruitGenerator + SpecialEffectProcessor 实现)

set(PROPS_SOURCES
    src/props/PropManager.cpp
    src/props/Hammer.cpp
    src/props/Clamp.cpp
    src/props/MagicWand.cpp
)

set(PROPS_HEADERS
    src/props/PropManager.h
    src/props/Hammer.h
    src/props/Clamp.h
    src/props/MagicWand.h
)

# 已删除: src/score/ 目录 (功能已由 ScoreCalculator 实现)

set(MODE_SOURCES
    src/mode/GameMode.cpp
    src/mode/CasualMode.cpp
    src/mode/CompetitionMode.cpp
)

set(MODE_HEADERS
    src/mode/GameMode.h
    src/mode/CasualMode.h
    src/mode/CompetitionMode.h
)

set(ACHIEVEMENT_SOURCES
    src/achievement/AchievementManager.cpp
)

set(ACHIEVEMENT_HEADERS
    src/achievement/AchievementManager.h
    src/achievement/AchievementDef.h
)

set(DATA_SOURCES
    src/data/SaveManager.cpp
    src/data/RankManager.cpp
    src/data/Database.cpp
)

set(DATA_HEADERS
    src/data/SaveManager.h
    src/data/RankManager.h
    src/data/Database.h
)

set(UTILS_SOURCES
    src/utils/Random.cpp
)

set(UTILS_HEADERS
    src/utils/Random.h
)

set(UI_SOURCES
    ui/MainWindow.cpp
    ui/views/GameView.cpp
    ui/views/RankView.cpp
    ui/views/AchievementView.cpp
)

set(UI_HEADERS
    ui/MainWindow.h
    ui/views/GameView.h
    ui/views/RankView.h
    ui/views/AchievementView.h
)

# 已删除: ui/views/MainMenuView 和 ui/widgets/ (使用 OpenGL 直接渲染)

set(UI_FORMS
    ui/MainWindow.ui
)

# GameView 动画系统（解耦后的新架构）
set(ANIMATION_VIEW_SOURCES
    ui/views/animation/AnimationController.cpp
    ui/views/animation/SnapshotManager.cpp
    ui/views/animation/IAnimationRenderer.cpp
    ui/views/animation/SwapAnimationRenderer.cpp
    ui/views/animation/EliminationAnimationRenderer.cpp
    ui/views/animation/FallAnimationRenderer.cpp
    ui/views/animation/ShuffleAnimationRenderer.cpp
)

set(ANIMATION_VIEW_HEADERS
    ui/views/animation/AnimationController.h
    ui/views/animation/SnapshotManager.h
    ui/views/animation/IAnimationRenderer.h
    ui/views/animation/SwapAnimationRenderer.h
    ui/views/animation/EliminationAnimationRenderer.h
    ui/views/animation/FallAnimationRenderer.h
    ui/views/animation/ShuffleAnimationRenderer.h
)

# 输入处理系统（鼠标交互解耦）
set(INPUT_SOURCES
    ui/input/InputHandler.cpp
    ui/input/NormalClickStrategy.cpp
    ui/input/PropClickStrategy.cpp
)

set(INPUT_HEADERS
    ui/input/InputHandler.h
    ui/input/IClickStrategy.h
    ui/input/NormalClickStrategy.h
    ui/input/PropClickStrategy.h
)

# 旧的动画系统（保留用于其他模块）

set(ANIMATION_SOURCES
    ui/animation/SwapEffect.cpp
    ui/animation/EliminateEffect.cpp
    ui/animation/FallEffect.cpp
    ui/animation/BombEffect.cpp
    ui/animation/AnimationRenderer.cpp
)

set(ANIMATION_HEADERS
    ui/animation/IAnimationEffect.h
    ui/animation/SwapEffect.h
    ui/animation/EliminateEffect.h
    ui/animation/FallEffect.h
    ui/animation/BombEffect.h
    ui/animation/AnimationRenderer.h
)

# 资源文件列表
set(RESOURCE_FILES
    resources/props/hammer.png
    resources/props/clamp.png
    resources/props/magic_wand.png
    resources/texture.png
    resources/textures/apple.png
    resources/textures/banana.png
    resources/textures/cherry.png
    resources/textures/grape.png
    resources/textures/lemon.png
    resources/textures/mango.png
    resources/textures/orange.png
    resources/textures/pear.png
    resources/textures/strawberry.png
    resources/textures/watermelon.png
    resources/textures/Candy.png
)

# 主可执行文件
add_executable(${PROJECT_NAME}
    main.cpp
    ${CORE_SOURCES} ${CORE_HEADERS}
    ${PROPS_SOURCES} ${PROPS_HEADERS}
    ${MODE_SOURCES} ${MODE_HEADERS}
    ${ACHIEVEMENT_SOURCES} ${ACHIEVEMENT_HEADERS}
    ${DATA_SOURCES} ${DATA_HEADERS}
    ${UTILS_SOURCES} ${UTILS_HEADERS}
    ${UI_SOURCES} ${UI_HEADERS}
    ${UI_FORMS}
    ${ANIMATION_VIEW_SOURCES} ${ANIMATION_VIEW_HEADERS}
    ${INPUT_SOURCES} ${INPUT_HEADERS}
    ${ANIMATION_SOURCES} ${ANIMATION_HEADERS}
    # ${RESOURCES}
)

# 链接Qt库和平台相关库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Sql
    OpenGL::GL
)

# 平台特定链接库
if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif(APPLE)
    # macOS使用框架
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
endif()

# 复制资源文件到build目录
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    # 获取资源文件的相对路径
    file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/${RESOURCE_FILE})
    # 设置目标路径
    set(DST_FILE ${CMAKE_BINARY_DIR}/${REL_PATH})
    # 添加自定义命令，在构建后复制文件
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/${RESOURCE_FILE}
        ${DST_FILE}
        COMMENT "Copying resource: ${RESOURCE_FILE}"
    )
endforeach()

# 同时复制到bin目录（可执行文件旁边）
foreach(RESOURCE_FILE ${RESOURCE_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/${RESOURCE_FILE})
    set(DST_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${REL_PATH})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/${RESOURCE_FILE}
        ${DST_FILE}
        COMMENT "Copying resource to bin: ${RESOURCE_FILE}"
    )
endforeach()

# Windows平台设置
if(WIN32)
    # 不设置WIN32_EXECUTABLE，使用控制台窗口以避免EntryPoint问题
    # set_target_properties(${PROJECT_NAME} PROPERTIES
    #     WIN32_EXECUTABLE ON
    # )
    # Windows下复制Qt DLL到输出目录（可选，用于发布）
    # add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    #     COMMAND ${CMAKE_COMMAND} -E copy_if_different
    #     $<TARGET_FILE:Qt6::Core>
    #     $<TARGET_FILE_DIR:${PROJECT_NAME}>
    # )
endif()

# macOS平台设置
if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.fruitcrush.game"
        MACOSX_BUNDLE_BUNDLE_NAME "FruitCrush"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()

# Linux平台设置
if(UNIX AND NOT APPLE)
    # 设置RPATH以便找到共享库
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

# 测试支持
option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()

# 安装规则
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    BUNDLE DESTINATION .
)

# 安装资源文件
install(DIRECTORY resources/ DESTINATION bin/resources)

# 打印构建信息
message(STATUS "")
message(STATUS "=== FruitCrush Build Configuration ===")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ Standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Qt6 Version: ${Qt6_VERSION}")
message(STATUS "Install prefix: ${CMAKE_INSTALL_PREFIX}")
message(STATUS "======================================")
message(STATUS "")
