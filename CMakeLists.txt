cmake_minimum_required(VERSION 3.7)
project(vulkan-triangle VERSION 0.1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE Debug)

include(CTest)
enable_testing()

find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(Vulkan REQUIRED)

include_directories(
    include/
)

function(add_shader TARGET SHADER)

	# Find glslc shader compiler.
	# On Android, the NDK includes the binary, so no external dependency.
	if(ANDROID)
		file(GLOB glslc-folders ${ANDROID_NDK}/shader-tools/*)
		find_program(GLSLC glslc HINTS ${glslc-folders})
	else()
		find_program(GLSLC glslc)
	endif()

	# All shaders for a sample are found here.
	set(current-shader-path ${CMAKE_CURRENT_SOURCE_DIR}/${SHADER})

	# For Android, write SPIR-V files to app/assets which is then packaged into the APK.
	# Otherwise, output in the binary directory.
	if(ANDROID)
		set(current-output-path ${CMAKE_CURRENT_SOURCE_DIR}/app/assets/shaders/${SHADER}.spv)
	else(ANDROID)
		set(current-output-path ${CMAKE_BINARY_DIR}/${SHADER}.spv)
	endif(ANDROID)

	# Add a custom command to compile GLSL to SPIR-V.
	get_filename_component(current-output-dir ${current-output-path} DIRECTORY)
	file(MAKE_DIRECTORY ${current-output-dir})
	add_custom_command(
		OUTPUT ${current-output-path}
		COMMAND ${GLSLC} -o ${current-output-path} ${current-shader-path}
		DEPENDS ${current-shader-path}
		IMPLICIT_DEPENDS CXX ${current-shader-path}
		VERBATIM)

	# Make sure our native build depends on this output.
	set_source_files_properties(${current-output-path} PROPERTIES GENERATED TRUE)
	target_sources(${TARGET} PRIVATE ${current-output-path})
endfunction(add_shader)

add_executable(vulkan-triangle 
    src/triangle.cpp
    src/main.cpp
)

add_shader(vulkan-triangle shaders/triangle.frag)
add_shader(vulkan-triangle shaders/triangle.vert)

target_link_libraries( vulkan-triangle
    glfw
    glm
    Vulkan::Vulkan
)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
