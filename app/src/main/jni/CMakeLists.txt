cmake_minimum_required (VERSION 2.6)
project (VulkanDemos)

set(VULKAN_SDK_PATH /home/matt/Downloads/VulkanSDK/1.0.13.0/x86_64/)
include_directories(${VULKAN_SDK_PATH}/include)
link_directories(${VULKAN_SDK_PATH}/lib)

add_executable(vulkanDepthPeel main.cpp Simulation.cpp btQuickprof.cpp)

target_compile_features(vulkanDepthPeel PRIVATE cxx_range_for)
target_link_libraries(vulkanDepthPeel vulkan xcb xcb-icccm m)
