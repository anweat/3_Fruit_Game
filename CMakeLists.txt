cmake_minimum_required(VERSION 3.16)

project(FruitCrush VERSION 1.0.0 LANGUAGES CXX)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Qt设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# 查找Qt包
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets Sql)

# 输出路径设置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 包含目录
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/special
    ${CMAKE_SOURCE_DIR}/src/props
    ${CMAKE_SOURCE_DIR}/src/score
    ${CMAKE_SOURCE_DIR}/src/mode
    ${CMAKE_SOURCE_DIR}/src/achievement
    ${CMAKE_SOURCE_DIR}/src/data
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/animation
)

# 源文件
set(CORE_SOURCES
    src/core/GameEngine.cpp
    src/core/MapManager.cpp
    src/core/MatchDetector.cpp
    src/core/FruitGenerator.cpp
    src/core/FallProcessor.cpp
    src/core/ScoreCalculator.cpp
)

set(CORE_HEADERS
    src/core/FruitTypes.h
    src/core/GameEngine.h
    src/core/MapManager.h
    src/core/MatchDetector.h
    src/core/FruitGenerator.h
    src/core/FallProcessor.h
    src/core/ScoreCalculator.h
)

set(SPECIAL_SOURCES
    src/special/SpecialFruit.cpp
    src/special/LineBomb.cpp
    src/special/DiamondBomb.cpp
    src/special/RainbowFruit.cpp
)

set(SPECIAL_HEADERS
    src/special/SpecialFruit.h
    src/special/LineBomb.h
    src/special/DiamondBomb.h
    src/special/RainbowFruit.h
)

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

set(SCORE_SOURCES
    src/score/ScoreManager.cpp
    src/score/ComboTracker.cpp
    src/score/PointSystem.cpp
)

set(SCORE_HEADERS
    src/score/ScoreManager.h
    src/score/ComboTracker.h
    src/score/PointSystem.h
)

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
    ui/views/MainMenuView.cpp
    ui/views/GameView.cpp
    ui/views/RankView.cpp
    ui/views/AchievementView.cpp
    ui/widgets/FruitWidget.cpp
    ui/widgets/PropButton.cpp
    ui/widgets/ScoreBoard.cpp
)

set(UI_HEADERS
    ui/MainWindow.h
    ui/views/MainMenuView.h
    ui/views/GameView.h
    ui/views/RankView.h
    ui/views/AchievementView.h
    ui/widgets/FruitWidget.h
    ui/widgets/PropButton.h
    ui/widgets/ScoreBoard.h
)

set(UI_FORMS
    ui/MainWindow.ui
)

set(ANIMATION_SOURCES
    animation/AnimationManager.cpp
    animation/GLRenderer.cpp
    animation/ParticleSystem.cpp
    animation/effects/SwapEffect.cpp
    animation/effects/EliminateEffect.cpp
    animation/effects/FallEffect.cpp
    animation/effects/BombEffect.cpp
)

set(ANIMATION_HEADERS
    animation/AnimationManager.h
    animation/GLRenderer.h
    animation/ParticleSystem.h
    animation/effects/SwapEffect.h
    animation/effects/EliminateEffect.h
    animation/effects/FallEffect.h
    animation/effects/BombEffect.h
)

# 资源文件（如果有的话）
# set(RESOURCES resources/resources.qrc)

# 主可执行文件
add_executable(${PROJECT_NAME}
    main.cpp
    ${CORE_SOURCES} ${CORE_HEADERS}
    ${SPECIAL_SOURCES} ${SPECIAL_HEADERS}
    ${PROPS_SOURCES} ${PROPS_HEADERS}
    ${SCORE_SOURCES} ${SCORE_HEADERS}
    ${MODE_SOURCES} ${MODE_HEADERS}
    ${ACHIEVEMENT_SOURCES} ${ACHIEVEMENT_HEADERS}
    ${DATA_SOURCES} ${DATA_HEADERS}
    ${UTILS_SOURCES} ${UTILS_HEADERS}
    ${UI_SOURCES} ${UI_HEADERS}
    ${UI_FORMS}
    ${ANIMATION_SOURCES} ${ANIMATION_HEADERS}
    # ${RESOURCES}
)

# 链接Qt库
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Sql
    opengl32
)

# Windows平台设置
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE ON
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
)
