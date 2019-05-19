add_executable(btrees_coef_exps
        main.cpp
)

find_package(btrees_lib REQUIRED PATHS "../../../lib/btrees_lib")
target_link_libraries(btrees_coef_exps "${BTREES_LIB}")
target_include_directories(btrees_coef_exps PUBLIC "${BTREES_LIB_INCLUDE_DIR}")
