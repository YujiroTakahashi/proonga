find_path(DLFCN_H_PATH dlfcn.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(INTTYPES_H_PATH inttypes.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(MEMORY_H_PATH memory.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(STDINT_H_PATH stdint.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(STDLIB_H_PATH stdlib.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(STRINGS_H_PATH strings.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(STRING_H_PATH string.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(SYS_STAT_H_PATH sys/stat.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(SYS_TYPES_H_PATH sys/types.h PATHS /usr /usr/local PATH_SUFFIXES include)
find_path(UNISTD_H_PATH unistd.h PATHS /usr /usr/local PATH_SUFFIXES include)

if (DLFCN_H_PATH)
    set(HAVE_DLFCN_H 1)
endif (DLFCN_H_PATH)

if (INTTYPES_H_PATH)
    set(HAVE_INTTYPES_H 1)
endif (INTTYPES_H_PATH)

if (MEMORY_H_PATH)
    set(HAVE_MEMORY_H 1)
endif (MEMORY_H_PATH)

if (STDINT_H_PATH)
    set(HAVE_STDINT_H 1)
endif (STDINT_H_PATH)

if (STDLIB_H_PATH)
    set(HAVE_STDLIB_H 1)
endif (STDLIB_H_PATH)

if (STRINGS_H_PATH)
    set(HAVE_STRINGS_H 1)
endif (STRINGS_H_PATH)

if (STRING_H_PATH)
    set(HAVE_STRING_H 1)
endif (STRING_H_PATH)

if (SYS_STAT_H_PATH)
    set(HAVE_SYS_STAT_H 1)
endif (SYS_STAT_H_PATH)

if (SYS_TYPES_H_PATH)
    set(HAVE_SYS_TYPES_H 1)
endif (SYS_TYPES_H_PATH)

if (UNISTD_H_PATH)
    set(HAVE_UNISTD_H 1)
endif (UNISTD_H_PATH)


set(PACKAGE_BUGREPORT "")
set(PACKAGE_NAME "")
set(PACKAGE_STRING "")
set(PACKAGE_TARNAME "")
set(PACKAGE_URL "")
set(PACKAGE_VERSION "")

if (DLFCN_H_PATH AND
    INTTYPES_H_PATH AND
    MEMORY_H_PATH AND
    STDINT_H_PATH AND
    STDLIB_H_PATH AND
    STRINGS_H_PATH AND
    STRING_H_PATH AND
    SYS_STAT_H_PATH AND
    SYS_TYPES_H_PATH AND
    UNISTD_H_PATH)
    set(STDC_HEADERS 1)
endif (DLFCN_H_PATH AND
    INTTYPES_H_PATH AND
    MEMORY_H_PATH AND
    STDINT_H_PATH AND
    STDLIB_H_PATH AND
    STRINGS_H_PATH AND
    STRING_H_PATH AND
    SYS_STAT_H_PATH AND
    SYS_TYPES_H_PATH AND
    UNISTD_H_PATH)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/config/config.h.in
    ${CMAKE_BINARY_DIR}/config.h)