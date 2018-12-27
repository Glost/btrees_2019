add_library(btrees_c
        btree_c.h
        btree_c.cpp)

find_package(btrees_lib REQUIRED PATHS "../../../lib/btrees_lib")
target_link_libraries(btrees_c "${BTREES_LIB}")
target_include_directories(btrees_c PUBLIC "${BTREES_LIB_INCLUDE_DIR}")
