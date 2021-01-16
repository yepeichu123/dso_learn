
add_executable(testPixelSelector testPixelSelector.cpp
                                 Setting.cpp
                                 Frame.cpp
                                 PixelSelector.cpp)
target_link_libraries(testPixelSelector ${OpenCV_LIBS})