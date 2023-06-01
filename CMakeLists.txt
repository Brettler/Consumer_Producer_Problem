cmake_minimum_required(VERSION 3.25)
project(Task3 C)

set(CMAKE_C_STANDARD 11)

add_executable(Task3 main.c ProcessConfig.c Dispatcher.c Producer.c Queue.c FreeResource.c manager.c CoEditor.c Producer.h ProcessConfig.h Queue.h Dispatcher.h CoEditor.h manager.h Structs.h FreeResource.h initThreads.c initThreads.h initStructsObjects.c initStructsObjects.h)
