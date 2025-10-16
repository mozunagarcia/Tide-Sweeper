cmake_minimum_required(VERSION 3.16)
project(TideSweeper)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------------------------------
# Platform-specific SDL2 setup
# -------------------------------------------------------
if (WIN32)
    # --- Windows manual paths ---
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

else()
    # --- macOS/Linux automatic discovery ---
    find_package(SDL2 REQUIRED)
    find_package(SDL2_image REQUIRED)
    include_directories(${SDL2_INCLUDE_DIRS} ${SDL2_IMAGE_INCLUDE_DIRS})
    set(EXTRA_LIBS ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
endif()

# -------------------------------------------------------
# Build the executable
# -------------------------------------------------------
add_executable(TideSweeper src/main.cpp)
target_link_libraries(TideSweeper ${EXTRA_LIBS})

# -------------------------------------------------------
# Copy assets to build directory (so textures always load)
# -------------------------------------------------------
set(ASSETS_SOURCE_DIR "${CMAKE_SOURCE_DIR}/assets")
set(ASSETS_DEST_DIR "${CMAKE_CURRENT_BINARY_DIR}/assets")

add_custom_command(
    TARGET TideSweeper POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E make_directory "${ASSETS_DEST_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${ASSETS_SOURCE_DIR}" "${ASSETS_DEST_DIR}"
    COMMENT "Copying assets folder to build directory..."
)

# -------------------------------------------------------
# Copy DLLs on Windows (optional convenience)
# -------------------------------------------------------
if (WIN32)
    add_custom_command(TARGET TideSweeper POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_DIR}/bin/SDL2.dll"
            "${SDL2_IMAGE_DIR}/bin/SDL2_image.dll"
            "$<TARGET_FILE_DIR:TideSweeper>"
        COMMENT "Copying SDL2 DLLs..."
    )
endif()
