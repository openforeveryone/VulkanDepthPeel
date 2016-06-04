cmake_minimum_required (VERSION 2.6)
project (VulkanDemos)


add_executable(vulkanDepthPeel main.cpp Simulation.cpp btQuickprof.cpp)

target_compile_features(vulkanDepthPeel PRIVATE cxx_range_for)
target_link_libraries(vulkanDepthPeel vulkan xcb xcb-icccm m)
