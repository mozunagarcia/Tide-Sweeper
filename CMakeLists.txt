cmake_minimum_required(VERSION 3.16)
project(TideSweeper LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(${CMAKE_SOURCE_DIR}/include)

# WINDOWS (MinGW)
if (WIN32)

    set(SDL2_DIR "C:/Users/ozuna/OneDrive/Documents/Libs/SDL2-2.32.6/x86_64-w64-mingw32")
    set(SDL2_IMAGE_DIR "C:/Users/ozuna/OneDrive/Documents/Libs/SDL2_image-2.8.2/x86_64-w64-mingw32")
    set(SDL2_TTF_DIR "C:/Users/ozuna/OneDrive/Documents/Libs/SDL2_ttf-2.24.0/x86_64-w64-mingw32")
    set(SDL2_MIXER_DIR "C:/Users/ozuna/OneDrive/Documents/Libs/SDL2_mixer-2.8.1/x86_64-w64-mingw32")

    include_directories(
        ${SDL2_DIR}/include/SDL2
        ${SDL2_IMAGE_DIR}/include/SDL2
        ${SDL2_TTF_DIR}/include/SDL2
        ${SDL2_MIXER_DIR}/include/SDL2     # ← FIXED HERE
    )

    link_directories(
        ${SDL2_DIR}/lib
        ${SDL2_IMAGE_DIR}/lib
        ${SDL2_TTF_DIR}/lib
        ${SDL2_MIXER_DIR}/lib
    )

    set(EXTRA_LIBS
        mingw32
        SDL2main
        SDL2
        SDL2_image
        SDL2_ttf
        SDL2_mixer
    )


# macOS (Homebrew)
elseif (APPLE)

    set(SDL2_PREFIX "/opt/homebrew/opt/sdl2")
    set(SDL2_IMAGE_PREFIX "/opt/homebrew/opt/sdl2_image")
    set(SDL2_TTF_PREFIX "/opt/homebrew/opt/sdl2_ttf")
    set(SDL2_MIXER_PREFIX "/opt/homebrew/opt/sdl2_mixer")

    include_directories(
        ${SDL2_PREFIX}/include/SDL2
        ${SDL2_IMAGE_PREFIX}/include/SDL2
        ${SDL2_TTF_PREFIX}/include/SDL2
        ${SDL2_MIXER_PREFIX}/include/SDL2
    )

    link_directories(
        ${SDL2_PREFIX}/lib
        ${SDL2_IMAGE_PREFIX}/lib
        ${SDL2_TTF_PREFIX}/lib
        ${SDL2_MIXER_PREFIX}/lib
    )

    set(EXTRA_LIBS
        SDL2
        SDL2_image
        SDL2_ttf
        SDL2_mixer
    )

# Linux
else()
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    find_package(SDL2_ttf REQUIRED)
    find_package(SDL2_mixer REQUIRED)

    include_directories(
        ${SDL2_INCLUDE_DIRS}
        ${SDL2_IMAGE_INCLUDE_DIRS}
        ${SDL2_TTF_INCLUDE_DIRS}
        ${SDL2_MIXER_INCLUDE_DIRS}
    )

    set(EXTRA_LIBS
        ${SDL2_LIBRARIES}
        ${SDL2_IMAGE_LIBRARIES}
        ${SDL2_TTF_LIBRARIES}
        ${SDL2_MIXER_LIBRARIES}
    )
endif()

# Build
add_executable(TideSweeper 
    src/main.cpp
    src/ScoreDisplay.cpp
    src/Menu.cpp 
    src/GameManager.cpp
    src/Submarine.cpp
    src/Level.cpp
    src/Litter.cpp
    src/Enemies.cpp
    src/Scoreboard.cpp
    src/Messages.cpp
    src/GameOverScreen.cpp
    src/StoryManager.cpp

)

target_link_libraries(TideSweeper ${EXTRA_LIBS})

# Copy assets
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/Assets")
set(ASSETS_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/Assets")
add_custom_command(TARGET TideSweeper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASSETS_DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_SOURCE_DIR}" "${ASSETS_DEST_DIR}"
)
