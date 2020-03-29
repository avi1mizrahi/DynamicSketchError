# for debug
set(CMAKE_VERBOSE_MAKEFILE ON)

add_library(massdal STATIC
            prng.c cgt.c lossycount.c massdal.c frequent.c ccfc.c countmin.c)

target_link_libraries(massdal m)

add_executable(hotitems hotitems.c)

target_link_libraries(hotitems massdal)
