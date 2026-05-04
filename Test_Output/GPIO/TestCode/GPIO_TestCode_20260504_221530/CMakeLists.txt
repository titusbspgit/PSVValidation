cmake_minimum_required(VERSION 3.13)
project(test_gpio_negedge_intr_en C)

add_executable(test_gpio_negedge_intr_en
    src/test_gpio_negedge_intr_en.c
)

# Compile-time selection via -DUSE_GPIO0 or -DUSE_GPIO1
if(DEFINED USE_GPIO0)
  target_compile_definitions(test_gpio_negedge_intr_en PRIVATE USE_GPIO0)
endif()
if(DEFINED USE_GPIO1)
  target_compile_definitions(test_gpio_negedge_intr_en PRIVATE USE_GPIO1)
endif()

target_compile_options(test_gpio_negedge_intr_en PRIVATE -O2 -Wall -Wextra)
