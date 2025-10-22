cmake_minimum_required(VERSION 3.16)
project(TideSweeper LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (WIN32)
    # --- Windows ---
    set(SDL2_DIR "C:/Users/ozuna/OneDrive/Documents/Libs/SDL2-2.32.6/x86_64-w64-mingw32")
    set(SDL2_IMAGE_DIR "C:/Users/ozuna/OneDrive/Documents/Libs/SDL2_image-2.8.2/x86_64-w64-mingw32")

    include_directories(
        ${SDL2_DIR}/include/SDL2
        ${SDL2_IMAGE_DIR}/include/SDL2
    )
    link_directories(
        ${SDL2_DIR}/lib
        ${SDL2_IMAGE_DIR}/lib
    )

    set(EXTRA_LIBS mingw32 SDL2main SDL2 SDL2_image)

elseif (APPLE)
    # --- macOS ---
    # Locate SDL2 and SDL2_image via Homebrew (works for both Intel and Apple Silicon)
    set(SDL2_PREFIX "/opt/homebrew/opt/sdl2")
    set(SDL2_IMAGE_PREFIX "/opt/homebrew/opt/sdl2_image")

    # Fall back to /usr/local for Intel Macs
    if(NOT EXISTS "${SDL2_PREFIX}")
        set(SDL2_PREFIX "/usr/local/opt/sdl2")
    endif()
    if(NOT EXISTS "${SDL2_IMAGE_PREFIX}")
        set(SDL2_IMAGE_PREFIX "/usr/local/opt/sdl2_image")
    endif()

    include_directories(
        ${SDL2_PREFIX}/include/SDL2
        ${SDL2_IMAGE_PREFIX}/include/SDL2
    )
    link_directories(
        ${SDL2_PREFIX}/lib
        ${SDL2_IMAGE_PREFIX}/lib
    )

    set(EXTRA_LIBS SDL2 SDL2_image)

else()
    # --- Linux / others ---
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
    set(EXTRA_LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif()

add_executable(TideSweeper src/main.cpp)
target_link_libraries(TideSweeper ${EXTRA_LIBS})

# Copy assets automatically
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")
add_custom_command(TARGET TideSweeper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASSETS_DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_SOURCE_DIR}" "${ASSETS_DEST_DIR}"
)
