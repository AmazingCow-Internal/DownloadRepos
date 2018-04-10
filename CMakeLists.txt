# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

set(PROJECT "DownloadRepos")
project(${PROJECT})

# Include sub-projects.
add_subdirectory("libs/acow_algo")
add_subdirectory("libs/acow_c_goodies")
add_subdirectory("libs/acow_cpp_goodies")
add_subdirectory("libs/acow_posix_goodies")
add_subdirectory("libs/acow_ConioEx")
add_subdirectory("libs/acow_URLDownloadFile")
add_subdirectory("libs/CMD")
add_subdirectory("libs/CoreAssert")
add_subdirectory("libs/CoreDir")
add_subdirectory("libs/CoreFile")
add_subdirectory("libs/CoreFS")
add_subdirectory("libs/CoreLog")
add_subdirectory("libs/CoreString")
add_subdirectory("libs/libtermcolor")



set(SOURCES 	
	"src/main.cpp"
)
add_executable(${PROJECT} ${SOURCES})
	

##------------------------------------------------------------------------------
## Dependencies.
target_link_libraries(${PROJECT} LINK_PUBLIC acow_algo)
target_link_libraries(${PROJECT} LINK_PUBLIC acow_c_goodies)
target_link_libraries(${PROJECT} LINK_PUBLIC acow_cpp_goodies)
target_link_libraries(${PROJECT} LINK_PUBLIC acow_posix_goodies)
target_link_libraries(${PROJECT} LINK_PUBLIC acow_ConioEx)
target_link_libraries(${PROJECT} LINK_PUBLIC acow_URLDownloadFile)
target_link_libraries(${PROJECT} LINK_PUBLIC CMD)
target_link_libraries(${PROJECT} LINK_PUBLIC CoreAssert)
target_link_libraries(${PROJECT} LINK_PUBLIC CoreDir)
target_link_libraries(${PROJECT} LINK_PUBLIC CoreFile)
target_link_libraries(${PROJECT} LINK_PUBLIC CoreFS)
target_link_libraries(${PROJECT} LINK_PUBLIC CoreLog)
target_link_libraries(${PROJECT} LINK_PUBLIC CoreString)
target_link_libraries(${PROJECT} LINK_PUBLIC libtermcolor)


install(TARGETS ${PROJECT} RUNTIME DESTINATION C:/Users/n2omatt/bin)