cmake_minimum_required(VERSION 3.10)

project(tloggerd VERSION 0.0)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(project_conf.h.in project_conf.h)

set(complie-opt-general
  -O2
  -Wall
  -Wextra
  -pedantic
)

#######################################################################################
#######################################################################################
add_executable(tloggerd
  apps/tloggerd.c
  src/buffered_writer.c
  src/log_system.c
  src/si7021.c
  src/tmp102.c
  src/w1_ds18b20.c
)


target_include_directories(tloggerd PUBLIC "${PROJECT_BINARY_DIR}" inc)
target_compile_options(tloggerd PUBLIC ${compile-opt-general})

#######################################################################################
#######################################################################################
add_executable(w1read
  apps/w1_ds18b20_read.c
  src/w1_ds18b20.c
)

target_include_directories(w1read PUBLIC "${PROJECT_BINARY_DIR}" inc)
target_compile_options(w1read PUBLIC ${compile-opt-general})

#######################################################################################
#######################################################################################
add_executable(si7021read
  apps/si7021_read.c
  src/si7021.c
)

target_include_directories(si7021read PUBLIC "${PROJECT_BINARY_DIR}" inc)
target_compile_options(si7021read PUBLIC ${compile-opt-general})

#######################################################################################
#######################################################################################
add_executable(tmp102read
  apps/tmp102_read.c
  src/tmp102.c
)

target_include_directories(tmp102read PUBLIC "${PROJECT_BINARY_DIR}" inc)
target_compile_options(tmp102read PUBLIC ${compile-opt-general})

