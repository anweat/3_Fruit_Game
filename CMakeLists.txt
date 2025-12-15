cmake_minimum_required(VERSION 3.16)

project(FruitCrush VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(WIN32)
    set(PLATFORM_NAME "Windows")
elseif(APPLE)
    set(PLATFORM_NAME "macOS")
elseif(UNIX)
    set(PLATFORM_NAME "Linux")
endif()

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets OpenGL OpenGLWidgets Sql)
find_package(OpenGL REQUIRED)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/props
    ${CMAKE_SOURCE_DIR}/src/mode
    ${CMAKE_SOURCE_DIR}/src/achievement
    ${CMAKE_SOURCE_DIR}/src/achievement/detectors
    ${CMAKE_SOURCE_DIR}/src/data
    ${CMAKE_SOURCE_DIR}/src/utils
    ${CMAKE_SOURCE_DIR}/ui
    ${CMAKE_SOURCE_DIR}/ui/views
    ${CMAKE_SOURCE_DIR}/ui/views/animation
    ${CMAKE_SOURCE_DIR}/ui/input
)

set(CORE_SOURCES
    src/core/GameEngine.cpp
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

set(PROPS_SOURCES
    src/props/PropManager.cpp
)

set(PROPS_HEADERS
    src/props/PropManager.h
)

set(MODE_SOURCES
    src/mode/CompetitionMode.cpp
)

set(MODE_HEADERS
    src/mode/CompetitionMode.h
)

set(ACHIEVEMENT_SOURCES
    src/achievement/AchievementManager.cpp
    src/achievement/detectors/BeginnerAchievementDetector.cpp
    src/achievement/detectors/ComboAchievementDetector.cpp
    src/achievement/detectors/MultiMatchAchievementDetector.cpp
    src/achievement/detectors/SpecialAchievementDetector.cpp
    src/achievement/detectors/ScoreAchievementDetector.cpp
    src/achievement/detectors/PropAchievementDetector.cpp
    src/achievement/detectors/ChallengeAchievementDetector.cpp
    src/achievement/detectors/MilestoneAchievementDetector.cpp
    src/achievement/detectors/AchievementDetectorManager.cpp
    src/achievement/detectors/DetectorFactory.cpp
)

set(ACHIEVEMENT_HEADERS
    src/achievement/AchievementManager.h
    src/achievement/AchievementDef.h
    src/achievement/detectors/IAchievementDetector.h
    src/achievement/detectors/BeginnerAchievementDetector.h
    src/achievement/detectors/ComboAchievementDetector.h
    src/achievement/detectors/MultiMatchAchievementDetector.h
    src/achievement/detectors/SpecialAchievementDetector.h
    src/achievement/detectors/ScoreAchievementDetector.h
    src/achievement/detectors/PropAchievementDetector.h
    src/achievement/detectors/ChallengeAchievementDetector.h
    src/achievement/detectors/MilestoneAchievementDetector.h
    src/achievement/detectors/AchievementDetectorManager.h
    src/achievement/detectors/DetectorFactory.h
)

set(DATA_SOURCES
    src/data/RankManager.cpp
    src/data/Database.cpp
)

set(DATA_HEADERS
    src/data/RankManager.h
    src/data/Database.h
)

set(UTILS_SOURCES
)

set(UTILS_HEADERS
)

set(UI_SOURCES
    ui/MainWindow.cpp
    ui/LoginWidget.cpp
    ui/AchievementDialog.cpp
    ui/SettingsDialog.cpp
    ui/CompetitionModeDialog.cpp
    ui/LeaderboardDialog.cpp
    ui/views/GameView.cpp
    ui/views/RankView.cpp
    ui/views/AchievementView.cpp
    ui/views/AchievementNotificationWidget.cpp
)

set(UI_HEADERS
    ui/MainWindow.h
    ui/LoginWidget.h
    ui/AchievementDialog.h
    ui/SettingsDialog.h
    ui/CompetitionModeDialog.h
    ui/LeaderboardDialog.h
    ui/StyleLoader.h
    ui/views/GameView.h
    ui/views/RankView.h
    ui/views/AchievementView.h
    ui/views/AchievementNotificationWidget.h
)


set(UI_FORMS
    ui/MainWindow.ui
)

set(ANIMATION_VIEW_SOURCES
    ui/views/animation/AnimationController.cpp
    ui/views/animation/SnapshotManager.cpp
    ui/views/animation/IAnimationRenderer.cpp
    ui/views/animation/SwapAnimationRenderer.cpp
    ui/views/animation/EliminationAnimationRenderer.cpp
    ui/views/animation/FallAnimationRenderer.cpp
    ui/views/animation/ShuffleAnimationRenderer.cpp
    ui/views/ScoreFloatOverlay.cpp
)

set(ANIMATION_VIEW_HEADERS
    ui/views/animation/AnimationController.h
    ui/views/animation/SnapshotManager.h
    ui/views/animation/IAnimationRenderer.h
    ui/views/animation/SwapAnimationRenderer.h
    ui/views/animation/EliminationAnimationRenderer.h
    ui/views/animation/FallAnimationRenderer.h
    ui/views/animation/ShuffleAnimationRenderer.h
    ui/views/ScoreFloatOverlay.h
)

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
    resources/styles/main.qss
    resources/styles/buttons.qss
    resources/styles/login.qss
    resources/styles/game.qss
    resources/styles/achievement.qss
    resources/styles/scrollbar.qss
)

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
)

target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Sql
    OpenGL::GL
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} opengl32)
elseif(APPLE)
    find_library(COCOA_LIBRARY Cocoa)
    find_library(IOKIT_LIBRARY IOKit)
    target_link_libraries(${PROJECT_NAME} ${COCOA_LIBRARY} ${IOKIT_LIBRARY})
endif()

foreach(RESOURCE_FILE ${RESOURCE_FILES})
    file(RELATIVE_PATH REL_PATH ${CMAKE_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/${RESOURCE_FILE})
    set(DST_FILE ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${REL_PATH})
    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/${RESOURCE_FILE}
        ${DST_FILE}
    )
endforeach()

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE ON
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.fruitcrush.game"
        MACOSX_BUNDLE_BUNDLE_NAME "FruitCrush"
        MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
        MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION}
    )
endif()

if(UNIX AND NOT APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INSTALL_RPATH "$ORIGIN/../lib"
        BUILD_WITH_INSTALL_RPATH TRUE
    )
endif()

option(BUILD_TESTS "Build tests" ON)

if(BUILD_TESTS)
    enable_testing()
endif()

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
    BUNDLE DESTINATION .
)

install(DIRECTORY resources/ DESTINATION bin/resources)
