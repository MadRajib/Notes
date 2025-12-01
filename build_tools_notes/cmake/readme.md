Base CMakeList.txt
```cmake
# set minimum cmake version
cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

# project name and language
project(recipe-01 LANGUAGES C)

add_executable(hello-world hello-world.c)
```

To configure cmake Create a build dir
```bash
mkdir -p build
cd build
cmake ..
```
or 
```bash
mkdir -p build/out
cd build/out
cmake ../../
```
If everything went well, the configuration for the project has been generated in the build directory.

To compile/build
```bash
cmake --build .
```

Note:
> we need to configure/compile from build or out folder.
> and point to the dir where cmakelist is present

we cn shorten configure step to:
```bash
cmake -H. -Bbuild
```
__-H.__ we instruct cmake to search for the root CMAKELists.txt file in the current directory.  
__-Bbuild__ tells Cmake to generate all of its files in a directory called build.

To building/compiling by indication the configured directory.
```bash
cmake --build build/out/
```

To build a specific target :
```bash
cmake --build . --target <target-name>
```
target can be:  
* all: is the default tager and will build all other targets in the project
* clean: to remove all generated files.
* depend: to generate the dependencies if any for the src files.
* rebuild_Cache: will once again invoke Cmake to rebuild the CmakeCache.txt.
* edit_cache: edit the cache entires directly

For more complex projects:
* test: will run the test suite with the help of CTest.
* install: runs installation rules 
* package: invokes CPack to generate a redistributable package for the project.

CMake is a build system generator and a single CMakeLists.txt can be se to configure projects for different toolstacks on diff platforms.

## How to do it
1. configure the project:
   ```bash
   mkdir -p build
   cd build
   cmake -G Ninja ..
   ```
2. buld the project:
   ```bash
   cmake --build .
   ```
    The output of the compilation step and the contents of the build directory will however be different, as every generator has its own specific set of files:

    * __build.ninja__ and __rules.ninja__: contains all the build statements and build rules for Ninja.
    * CmakeCache.txt: Cmake always generates its own cache in this file, regardless of the generator.
    * CmakeFiles: tmp files
    * cmake_install.cmake: Cmake script handling install rules and which is used at install time.

## Building and linking static and shared libraries

Say 2 files are to be added a libs to another project say Parser.c
```bash
API.c && API.h
```
1.  Create a new Target, name of the lib will be the name of the target.
    ```cmake
    # for statically linking
    add_library(api
        STATIC
            API.h
            API.c
        )
    # for shared way
    add_library(api
        SHARED
            API.h
            API.c
        )

    ```
2. create the executable target
    ```cmake
    add_executable(hello-world parser.c)
    ```
3. Link the libraries to hello-world
   ```cmake
   target_link_libraries(hello-world api)
   ```
4. configuration and build will remain the same.

### Points
* Actual name of th generated library will be formed as lib+suffix. eg libapi.a (for static) and linapi.so (for shared)
* target_link_libraries guarantees that the hello-world executable properly depends on the api lib.

2nd arguments values for add_library func:
* __STATIC__: to create static libraries, archives of object files for use when linking other targets.
* __SHARED__: to create shared libraries, can be loaded at runtime. 
* __OBJECT__: to compile the sources in the list given to add_library to object files, but then neither archiving them into a static lib nor linking them into a shared object. Use full when one needs to create both static and shared in one go.
* __MODULE__: DSO(dynamic shared objects). They are not linked to any target within the project, by may be loaded dynamically later on. Used when building a runtime plugin.
  
Cmake also generates libs which produces no output in the build system but are extremely helpful in organizing dependencies and build requirement betn targets.
* __IMPORTED__: located outside the project, immutable
* __INTERFACE__: same as __IMPORTED__ but mutable and has no location.
* __ALIAS__: defines an alias for a pre-exiting library target within the project. Not possible to choose an alias for an __IMPORTED__ library.

we can change Target properties using __*set_target_properties*__ 
```cmake
add_library(message-static
    STATIC
        API.h
        API.c
)

set_target_properties(message-static
    PROPERTIES
        OUTPUT_NAME "Message"
)
```

## Controlling compilation with conditionals
1. Introduce a local variable __USE_LIBRARY__
   ```cmake
   # OFF is the default value of USE_LIBRARY
   set(USE_LIBRARY OFF)
   
   # print value to user
   message(STATUS "Compile sources into a library? ${USE_LIBRARY}")
   ``` 

2. Set __BUILD_SHARED_LIBS__ global var, to OFF.
    > calling add_library and omitting the second argument will build a static lib.
    ```cmake
    set(BUILD_SHARED_LIBS OFF)
    ```
3. Append srcs to a new ___sources__ variable.
   ```cmake
   list(APPEND _sources API.c API.h)
   ```
4. Based on the value of USE_LIB, API sources will be packaged into a lib.
    ```cmake
    if(USE_LIBRARY)
        
        add_library(message ${_sources})

        add_executable(hello-world hello-world.cpp)

        target_link_libraries(hello-world message)
    else()
        add_executable(hello-world hello-world.cpp ${_sources})
    endif()
    ```
    Here since __USE_LIBRARY__ is OFF the executable will be compiled from all sources.

* In CMAKE try ir false values can be expressed in a number of ways  
  1. True: 1, ON, YES, TRUE, Y, or a non-zero number
  2. False:0, OFF, NO, FALSE, N IGNORE, NOTFOUND, and empty string, or it ends in the suffice -NOTFOUND 
* __BUILD_SHARED_LIBS__ is set to false because it is looked up in first,
  if false or undefined, a static library will be generated.

### Options
* It is possible and convenient to provide an option to user to toggle the logical variable values.

  ```cmake
  # OFF is the default value
  option(USE_LIBRARY "Compile sources into a library" OFF)
  ```
* To toggle
  ```bash
  $ mkdir -p build
  $ cd build
  $ cmake -D USE_LIBRARY=ON ..

  -- ..
  -- Compile sources into a library? ON

  $ cmake --build . 
  ```
  __-D__ to set any type of variables if variable.

* syntax:
  ```cmake
  option(<option_variable> "help string" [initial value])
  # initial value: ON or OFF
  ```

### Dependencies on other variable
* We want to build shared and static lib only in __USE_LIBRARY__ is enabled.using __cmake_dependent_option()__
    ```cmake
    # included a module
    include(CMakeDependentOption)

    # second option depends on the value of the first
    cmake_dependent_option(
        MAKE_STATIC_LIBRARY "Compile sources into a static library" OFF
        "USE_LIBRARY" ON
    )

    # third option depends on the value of the first
    cmake_dependent_option(
        MAKE_SHARED_LIBRARY "Compile sources into a shared library" ON
        "USE_LIBRARY" ON
    )
    ```
    Here if __USE_LIBRARY__ is ON __MAKE_STATIC_LIBRARY__ is OFF and __MAKE_SHARED_LIBRARY__ is ON.

    ```bash
    $ cmake -DUSE_LIBRARY=OFF -DMAKE_SHARED_LIBRARY=ON ..
    # this will not built a library as USE_LIBRARY is still set to OFF    
    ```

    > Note: Type cmake --help-option \<module name> to get more info.

    
### Specifying the compiler
cmake stores compilers for each language in the __CMAKE\_\<LANG>\_COMPILER__ variable.
The user can set this variables in two ways:
1. By using the -D option in CLI
   ```bash
    $ cmake -D CMAKE_CXX_COMPILER=clang++ ..
   ```
2. By exporting env variables.
   ```bash
   # CXX for c++, CC for C compiler
   env CXX=clang++ cmake ..
   ```


* By using __--system-information__ flag will dump all the information about your system to the screen or a file.
  ```bash
  cmake --system-information information.txt
  ```
* Additional Variables:  
    * __CMAKE\_\<LANG>_COMPILER_LOADED__: set to true if the language was enabled for the project.
    * __CMAKE\_\<LANG>_COMPILER_ID__: unique to the compiler vendor.
    * __CMAKE\_COMPILER_IS_GNU\<LANG>__: set to TRUE, if the compiler for the language is part of the GNU Compiler Collection.
    * __CMAKE\_\<LANG>_COMPILER_VERSION__:version for the compiler for the given language.
    eg
    ```cmake
    # set minimum cmake version
    cmake_minimum_required(VERSION 3.5 FATAL_ERROR)

    # project name and language
    project(recipe-06 LANGUAGES C CXX)

    message(STATUS "Is the C++ compiler loaded? ${CMAKE_CXX_COMPILER_LOADED}")
        if(CMAKE_CXX_COMPILER_LOADED)
        message(STATUS "The C++ compiler ID is: ${CMAKE_CXX_COMPILER_ID}")
        message(STATUS "Is the C++ from GNU? ${CMAKE_COMPILER_IS_GNUCXX}")
        message(STATUS "The C++ compiler version is: ${CMAKE_CXX_COMPILER_VERSION}")
    endif()

    message(STATUS "Is the C compiler loaded? ${CMAKE_C_COMPILER_LOADED}")
    if(CMAKE_C_COMPILER_LOADED)
        message(STATUS "The C compiler ID is: ${CMAKE_C_COMPILER_ID}")
        message(STATUS "Is the C from GNU? ${CMAKE_COMPILER_IS_GNUCC}")
        message(STATUS "The C compiler version is: ${CMAKE_C_COMPILER_VERSION}")
    endif()
    ```
    
### Switching build type
  * __CMAKE_BUILD_TYPE__ governs the configuration to be used when generating the build system.
  * empty by default.  
    1. __Debug__: without optimization and with debug symbols.
    2. __Release__: with optimization and without debug symbols.
    3. __RelWithDebInfo__: with less aggressive optimization and with debug symbols.
    4. __MinSizeRel__: with optimizations that do not increase object code size.
   
1. To set default build type
   ```cmake
   # Variable is set as a CACHE variable, sot that i can be subsequently edited through the cache
   if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
   endif()

   message(STATUS "BUild type: ${CMAKE_BUILD_TYPE}")
   ```
2. Lets print corresponding compile flags set by CMake as a function of the build type:
   ```cmake
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

    message(STATUS "C flags, Debug configuration: ${CMAKE_C_FLAGS_DEBUG}")
    message(STATUS "C flags, Release configuration: ${CMAKE_C_FLAGS_RELEASE}")
    message(STATUS "C flags, Release configuration with Debug info: ${CMAKE_C_FLAGS_RELWITHDEBINFO}")
    message(STATUS "C flags, minimal Release configuration: ${CMAKE_C_FLAGS_MINSIZEREL}")

    message(STATUS "C++ flags, Debug configuration: ${CMAKE_CXX_FLAGS_DEBUG}")
    message(STATUS "C++ flags, Release configuration: ${CMAKE_CXX_FLAGS_RELEASE}")
    message(STATUS "C++ flags, Release configuration with Debug info: ${CMAKE_CXX_FLAGS_RELWITHDEBINFO}")
    message(STATUS "C++ flags, minimal Release configuration: ${CMAKE_CXX_FLAGS_MINSIZEREL}")
   ```
3. Lets switch the build type
   ```bash
   $ cmake -D CMAKE_BUILD_TYPE=Debug ..
   ```
* Better to purse the output by running cmake __--system-information__.
* For multiple-configuration generators such as Visual Studio and Xcode, they can generate multiple build types.
  ```bash
  $ cmake .. -G"Visual Studio 12 2017 Win64" -D CMAKE_CONFIGURATION_TYPES="Release;Debug"
  ```
    This will generate a build tree for both the Release and Debug configuration. To decide which one to build use __--config flag__:
    ```bash
    $ cmake --build . --config Release
    ```

### Controlling compiler flags
* We can directly modify the __CMAKE\_\<LANG>_FLAGS\_\<CONFIG>__ variable using the -D CLI switch.
* To print current set of compiler flags.
  ```cmake
  message("C++ compiler flags: ${CMAKE_CXX_FLAGS}")
  ```
* Prepare a list of flags for out targets.
  ```cmake
  list(APPEND flags "-fPIC" "-Wall")
  if(NOT WIN32)
    list(APPEND flags "-Wextra" "-Wpendantic")
  endif()
  ```
* after add_library we set compile options for the target:
  ```cmake
  target_compile_options(geometry
    PRIVATE
        ${flags}
  )
  ```
* Then add a target for executable
  ```cmake
  add_executable(compute-areas compute-areas.cpp)
  ```
* Lets set compiler options for the executable target, earlier we set for the library target.
  ```cmake
  target_compile_options(compute-areas
    PRIVATE
        "-fPIC"
  )
  ```
* Finally we link the executables with the lib

    ```cmake
    target_link_libraries(compute-areas geometry)
    ```

    > Notes:
    * Compiler options can be added with three levels of visibility:
    
      1. __PRIVATE__: applied to only given target, not to other targets consuming it. I.e in above libraries compiler options won't be inherited by the executable.
      2. __INTERFACE__:applied to targets consuming it.
      3. __PUBLIC__:applied to given target and all other targets consuming it. 
    * we can use __VERBOSE=1__ with --build option to see all these flags in action
      ```bash
      $ cmake --build . --VERBOSE=1
      ```
* Second option of controlling compiler flags involes no modifications to CMakeLists.txt. we can pass from the cli.
```bash
$ cmake -d CMAKE_CXX_FLAGS="-fno-exceptoons -fno-rtti"
```

There are various approaches for having compiler specific flags.  
1. Having if-endif on __CMAKE\_\<LANG>\_COMPILER_ID__ variable
   ```cmake
   if (CMAKE_CXX_COMPILER_ID MATCHES GNU)
    list(APPEND CMAKE_CXX_FLAGS "-fno-rtti" "-fno-exceptions")
    list(APPEND CMAKE_CXX_FLAGS_DEBUG "-Wsuggest-final-types" ..)
    list(APPEND CMAKE_CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
   endif()

   if (CMAKE_CXX_COMPILER_ID MATCHES Clang)
    list(APPEND CMAKE_CXX_FLAGS "-fno-rtti" "-fno-exceptions")
    list(APPEND CMAKE_CXX_FLAGS_DEBUG "-Wdocumentation")
    list(APPEND CMAKE_CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
   endif()
   ```
2. A more refined way without tampering the CMAKE\_\<LANG>\_FLAGS_\<CONFIG> is to define a project-specific lists of flags:
```cmake
    set(COMPILER_FLAGS)
    set(COMPILER_FLAGS_DEBUG)
    set(COMPILER_FLAGS_RELEASE)
    
    if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
        list(APPEND CXX_FLAGS "-fno-rtti" "-fno-exceptions")
        list(APPEND CXX_FLAGS_DEBUG "-Wsuggest-final-types" ..)
        list(APPEND CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
    endif()

    if(CMAKE_CXX_COMPILER_ID MATCHES Clang)
        list(APPEND CXX_FLAGS "-fno-rtti" "-fno-exceptions")
        list(APPEND CXX_FLAGS_DEBUG "-Wdocumentation")
        list(APPEND CXX_FLAGS_RELEASE "-O3" "-Wno-unused")
    endif()

    target_compile_option(compute-areas
        PRIVATE
        ${CXX_FLAGS}
        "$<$<CONFIG:Debug>:${CXX_FLAGS_DEBUG}>"
        "$<$<CONFIG:Release>:${CXX_FLAGS_RELEASE}>"
    )
```
### Setting up language standards
```cmake
set_target_properties(animals
    PROPERTIES
        CXX_STANDARD 14
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
        POSITION_INDEPENDENT_CODE 1
)

add_executable(animal-farm animal-farm.cpp)
set_target_properties(animal-farm
    PROPERTIES
        CXX_STANDARD 14
        CXX_EXTENSIONS OFF
        CXX_STANDARD_REQUIRED ON
)

target_link_libraries(animal-farm animals)
```

### Using foreach and while control flow constructs
```cmake
list(
    APPEND sources_with_lower_optimization
        geometry_circle.cpp
        geometry_rhombus.cpp
    )


message(STATUS "Setting source properties using IN LIST syntax:")
foreach(_source IN LISTS sources_with_lower_optimization)
    set_source_files_properties(${_source} PROPERTIES COMPILE_FLAGS -O2)
    message(STATUS "Appending -O2 flag for ${_source}")
endforeach()

# To make sure source properties were set, we loop again and print the COMPILE_FLAGS.
message(STATUS "Querying sources properties using plain syntax:")
foreach(_source IN LISTS sources_with_lower_optimization)
    get_source_files_properties(_flags ${_source} COMPILE_FLAGS)
    message(STATUS "Source ${_source} has the following extra COMPILE_FLAGS:${_flags}")
endforeach()

```

* __set_source_files_properties(file PROPERTIES property value)__  
* __get_source_files_properties(VAR file property)__ retrieves the value of the desired property for the given file and stores it in the Cmake VAR variable.
* In CMake lists are semicolon-separated groups of strings. A list can be created either by the list or the set commands. eg
  ```cmake
  set(var a b c d e)
  list(APPEND a b c d e)
  will create the list a;b;c;d;e.
  ```
* foreach() construct can be used in four different ways
    1. foreach(loop_var arg1 arg2 ...): explicit list of items are provided. Used to print compiler flags sets for the items in sources_with_lower_optimization. The list has to be explicitly expanded ${sources_with_lower_optimization}
    2. foreach(loop_var RANGE total) or foreach(loop_var RANGE start stop [step])
    3. foreach(loop_var IN LISTS [list1 [...]]). Arguments are interpreted as lists and their contents automatically expanded accordingly.
    4. foreach(loop_var IN ITEMS [item1 [...]]) The contents if the arguments are not expanded.

### Detecting the Environment
#### Discovering the OS
* To print a custom message depending on detected OS:
```cmake
if(CMAKE_SYSTEM_NAME  STREQUAL "Linux")
  message(STATUS "LINUX")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  message(STATUS "macOS")
else()
  ...
endif()
```
* Systems having __uname__ cmd, CMAKE_SYSTEM_NAME is set to the output of __uname -s__.

#### Dealing with platfrom-dependent source code
* cpp code eg:
```c++
#ifdef IS_WINDOWS
  return std::string("Windows");
#elif IS_LINUX
  return std::string("Linus");
#else
  return ...
#endif
```
* cmake to conditionally compile the source code based on the target os.
```cmake
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_compile_definitions(hello-world PUBLIC "IS_LINUX")
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_compile_definitions(hello-world PUBLIC "IS_WINDOWS")
endif()
```
* These definitions are defined at configure time by CMake in CmakeLists.txt by using __target_compile_definitions__ before being passed on to the preprocessor.
* __add_definitions(-DIS_LINUX)__ could be used instead of using __target_compiler_definitions__.
  * Disadvantages: it modifies compile definitions for the entire project, whereas the later has target specific scope.
  * Also target_compiler_definations__ provides qualifiers to restrict visibility of thee defininations (PRIVATE, PUBLIC or INTERFACE).

### Compiler dependent src code
```cmake
# pass the COMPILER_NAME macro to the preprocessor
target_compile_definitions(hello-world PUBLIC "COMPILER_NAME=\"${CMAKE_CXX_COMPILER_ID}\"")

# create a macro defination based on the compiler flavour
if(CMAKE_CXX_COMPILER_ID MATCHES Intel)
  target_compile_definitions(hello-world PUBLIC "IS_INTEL_CXX_COMPILER")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES GNU)
  target_compile_definitions(hello-world PUBLIC "IS_GNU_CXX_COMPILER")
endif()

```
* More compact way
```cmake
target_compile_definitions(hello-world PUBLIC "IS_${CMAKE_CXX_COMPILER_ID}_CXX_COMPILER")
```
> Note:  
> __CMAKE\_\<LANG>\_COMPILER_ID__ is not guranteed to be deined for all compilers or language.

### Discovering the host processor arch
* we will check for void pointer type. This is defined in the __CMAKE_SIZEOF_VOID_P__ CMAKE variable and will tell us whether the CPU is 32 or 64 bits.
```cmake
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  target_compile_definitions(hello-world PUBLIC "IS_64_BIT_ARCH")
  message(STATUS "Target is 64 bits")
else()
  target_compile_definitions(hello-world PUBLIC "IS_32_BIT_ARCH")
  message(STATUS "Target is 32 bits")
endif()
```
* To know the host processor architecture:
```cmake
if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i386")
  message(STATUS "i386 architecture detected")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
  message(STATUS "i686 architrecture detected")
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "x86_64")
  message(STATUS "x86_64 architecture detected")
else()
  message(STATUS "host processor arch is unknown!")
endif()

target_compile_definitions(arch-dependent
  PUBLIC "ARCHITECTURE=${CMAKE_HOST_SYSTEM_PROCESSOR}"
)
```
*__CMAKE_HOST_SYSTEM_PROCESSOR__ : name of cpu CMake is currently __runing on__.
*__CMAKE_SYSTEM_PROCESSOR__ : name of cput currently __building for__.
* An alternative to detecting host arch from CMAKE is to use sysmbols defined within C/C++ and use CMake try_run function to build an attempt to execute the src code.
```c
#if defined(__i386) || defined(__i386__) ||defined(M_IX86_)
  #error cmake_arch i386
#elif defined(__x86_64) || defined(_x86_64__) || defined(__amd64) || defined(_M_X64)
  #error cmake_arch i386
#endif
```
* Another way is to have different src file based on arc
```cmake
add_executable(arch-dependent "")

if(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i386")
  message(STATUS "i386 architecture detected")
  target_sources(arch-dependent
    PRIVATE
      arcg-dependent-i386.cpp
  )
elseif(CMAKE_HOST_SYSTEM_PROCESSOR MATCHES "i686")
  message(STATUS "i686 architecture detected")
  target_sources(arch-dependent
    PRIVATE
      arcg-dependent-i686.cpp
  )
...
endi()

```
### Dicovering the host instruction set
```cmake
add_executable(processor-info "")
target_sources(processor-info
  PRIVATE
    processor-info.cpp
)

target_include_directories(processor-info
  PRIVATE
    ${PROJECT_BINARY_DIR}
)
```
* we then go on to querry the host system info for a number of keys:
```cmake
foreach(key
  IN ITEMS
    NUMBER_OF_LOGICAL_CORES
    NUMBER_OF_PHYSICAL_CORES
    TOTAL_VIRTUAL_MEMORY
    AVAILABLE_VIRTUAL_MEMORY
    TOTAL_PHYSICAL_MEMORY
    AVAILABLE_PHYSICAL_MEMORY
    IS_64BIT
    HAS_FPU
    HAS_MMX
    HAS_MMX_PLUS
    HAS_SSE
    HAS_SSE2
    HAS_SSE_FP
    HAS_SSE_MMX
    HAS_AMD_3DNOW
    HAS_AMD_3DNOW_PLUS
    HAS_IA64
    OS_NAME
    OS_RELEASE
    OS_VERSION
    OS_PLATFORM
  )
  # querries system information of the host system
  cmake_host_system_information(RESULT _${key} QUERY ${key})
endforeach()
```
* Now we configure config.h
```cmake
# now use these variables to configure the placeholder in config.h.in to generate config,h
configure_file(config.h.in config.h @ONLY)
```
* In c++ code we will include config.h file will be generated from config.h.in:
```c++
#pragma once

#define NUMBER_OF_LOGICAL_CORES   @_NUMBER_OF_LOGICAL_CORES@
#define NUMBER_OF_PHYSICAL_CORES  @_NUMBER_OF_PHYSICAL_CORES@
#define TOTAL_VIRTUAL_MEMORY      @_TOTAL_VIRTUAL_MEMORY@
#define AVAILABLE_VIRTUAL_MEMORY  @_AVAILABLE_VIRTUAL_MEMORY@
#define TOTAL_PHYSICAL_MEMORY     @_TOTAL_PHYSICAL_MEMORY@
#define AVAILABLE_PHYSICAL_MEMORY @_AVAILABLE_PHYSICAL_MEMORY@
#define IS_64BIT                  @_IS_64BIT@
#define HAS_FPU                   @_HAS_FPU@
#define HAS_MMX                   @_HAS_MMX@
#define HAS_MMX_PLUS              @_HAS_MMX_PLUS@
#define HAS_SSE                   @_HAS_SSE@
#define HAS_SSE2                  @_HAS_SSE2@
#define HAS_SSE_FP                @_HAS_SSE_FP@
#define HAS_SSE_MMX               @_HAS_SSE_MMX@
#define HAS_AMD_3DNOW             @_HAS_AMD_3DNOW@
#define HAS_AMD_3DNOW_PLUS        @_HAS_AMD_3DNOW_PLUS@
#define HAS_IA64                  @_HAS_IA64@
#define OS_NAME                  "@_OS_NAME@"
#define OS_RELEASE               "@_OS_RELEASE@"
#define OS_VERSION               "@_OS_VERSION@"
#define OS_PLATFORM              "@_OS_PLATFORM@"
``` 
* Sometimes the host building the code may not be same as the host running the code. eg On computer clustoers where the login node architecture may dfiffer from the architecture found on compute nodes.  
One way to solve this is to submit the configuration and compilation as a computation step and delpoy it to the compute nodes.

### Finding libraries 
* we wish to use the Eigen library, we need to find its header files on the system.
  ```cmake
  find_package(Eigen3 3.3 REQUIRED CONFIG)
  ```
* we include the __CheckCXXCompilerFlag.cmake__ standard module file:  
  > check_cxx_compiler_flag, 2 args:  
  > 1. compiler flag to check
  > 2. storing the result, true or false.
  
   
```cmake
include(ChechCXXCompilerFlag)

# Checking if the -march=native compiler flag works for GNU compilers
check_cxx_compiler_flag("-march=native" _march_native_works)

# alternative -xHost compiler flag is also checked  for Intel compilers
check_cxx_compiler_flag("-xHost" _xhost_works)

#set an empyt varialble
set(_CXX_FLAGS)

# if previously we saw _march_native_works/_xhost_works we set_CXX_FLAGS to -xHost. If none of them worked we will leave it emoty and vectorization will be dissabled:
if(_march_native_works)
  message(STATUS "Using processor's vector instructions (-march=native compiler flag set)")
  set(_CXX_FLAGS "-march=native")
elseif(_xhost_works)
  message(STATUS "Using processor's vector instructions (-xHost compiler flag set)")
  set(_CXX_FLAGS "-xHost")
else()
  message(STATUS "No suitable compiler flag found for vectorization")
endif()
```
* We will defined with optimized and unoptimizited version for comparision:
```cmake
# Unoptimized
add_executable(linear-algebra-unoptiomized linear-algebra.cpp)

target_link_libraries(linear-algebra-unoptimized
  PRIVATE
    Eigen3:Eigen
)
# optimized
add_executable(linear-algebra-optiomized linear-algebra.cpp)

target_compile_options(linear-algebra
  PRIVATE
    ${_CXX_FLAGS}
)

target_link_libraries(linear-algebra-unoptimized
  PRIVATE
    Eigen3:Eigen
)
```

> __Note__: *__cmake --help-module-list__* to get list of existing modules

## Detecting External Libraries and Programs
* find_file: to find a full path to a named file
* fing_library to find a library
* find_package to find and load settings from an external project
* find_path to find a directory containing the named file
* find_program to find a program.

### Detecting python interpreter
1. using __find_package__ command to find the pyhton interpreter
```cmake
  find_package(PthonInterp REQUIRED) # enforcing the dependencies
```
2. Then we execute a py command and capture its output and return value:
```cmake
execute_process(
  COMMAND 
    ${PYTHON_EXECUTABLE} "-c" "print('HELLO, world!')"
  RESULT_VARIABLE _status
  OUTPUT_VARIABLE _hello_world
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
  )
# print the return value and the output the python code:
message(STATUS "RESULT_VARIABLE is: ${_status}")
message(STATUS "OUTPUT_VARIABLE is: ${_hello_world}")

#A handy alternative way for debugging is to use
include(CMakPrintHelpers)
cmake_print_variables(_status _hello_world)

# -- _status="0" ; _hello_world="Hello, world!"

```
* The relevant module __FindPythonInterp.cmake__ shipped with CMake provides the following variables:
  * PYTHONINTERP_FOUND : a bool 
  * PYTHON_EXECUTABLE : the path to the executable for the pyhton interpreter.
  * PYTHON_VERSION_STRING : the full version of the pyhton
  * PYTHON_VERSION_MAJOR/MINOR/PATCH

* It is possible to force CMake to look for specific version of a package:
  ```cmake
  find_package(PythonInterp 2.7)
  ```
* To instruct CMake to look into certain specific locations to find certain software :
```bash
$ cmake -D PYTHON_EXECUTABLE=/custom/location/python ..
```

## Detecting the python library
* For mixed-language Projects i.e pyhton in c or c in python etc, requires the following:
  * A working version of the python interpreter
  * The availibility of the Python Header fike Python.h
  * The python runtime library libpython
* Getting ready, python embedding into a C program:
```c
#include <Python.h>

int main(int argc, char *argv[]) {
  Py_SetProgramName(argv[0]);
  Py_Initialize();
  PyRun_SimpleString("from time import time.ctime\n"
                     "print 'Today is'.ctime(time())\n");
  Py_Finalize();
  return 0;
}
```
* CMakeLists.txt
```cmake
#find the Pyhton interpreter
find_package(PythonInterp REQUIRED)

# fint eh pyhton header and the lib, FindPythonLibs.cmake is called
find_package(PythonLibs ${PYTHON_VERSION_MAJOR}.${PYTHON_VERSION_MINOR} EXACT REQUIRED)
# or we could have the precise match
find_package(PythonLibs ${PYTHON_VERSION_STRING} EXACT REQUIRED)

add_executable(hello-embedded-python hello-embedded-python.c)

# inclued the directory for the header file included
target_include_directories(hello-embedded-python
  PRIVATE
    ${PYTHON_INCLUDE_DIRS}
)

# link the executable to the python library
target_link_libraries(hello-embedded-python
  PRIVATE
    ${PYTHON_LIBRARIES}
)
```
* Build
```bash
$ mkdir -p build
$ cd build
$ cmake ..
```
* It is possible to force CMake to look in specific directories by pasisng the PYTHON_LIBRARY and PYTHON_INCLUDE_DIR options via the _D option to the cli. This ensures that the desired version of the Pyhton will be picked up.
  * PYTHON_LIBRARY, the path to the Pyhton Library
  * PYTHON_INCLUDE_DIR, the path to where Pyhton.h is located
* New pyhton detection modules usage
  ```cmake
  find_package(Python COMPONENTS Interpreter Developement REQUIRED)
  ```

## Detecting Python Modules and packages
Steps for NUMPY:
1. define minimum CMake version, project name, language and c++ standard.
2. Finding the interpreter, header and libraries.
3. Get intialtion dir and version.
```cmake
execute_process(
  COMMAND
    ${PYTHON_EXECUTABLE} "-c" "import re, numpy; print(re.compile('/__init__.py.*').sub(''.numpy.__file__))"
  RESULT_VARIABLE _numpy_status
  OUTPUT_VARIABLE _numpy_location
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)
```
4. We save its location to a new var
```cmake
if(NOT _numpy_status)
  set(Numpy ${_numpy_location} CACHE STRING "LOCATION OF NUMPY")
endif()
```
5. Check the version of Numpy
```cmake
execute_process(
  COMMAND
    ${PYTHON_EXECUTABLE} "-c" "import numpy; print(numpy.__version__)"
  RESULT_VARIABLE _numpy_version
  ERROR_QUIET
  OUTPUT_STRIP_TRAILING_WHITESPACE
)   
```
6. Use FindPackageHandleStandardArgs package to set up the NumPy_FOUND variable and ouput status info:
```cmake
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(NumPy
  FOUND_VAR NumPy_FOUND
  REQUIRED_VARS NumPy
  VERSION_VAR  _numpy_version
)
```
7. compile the executable and link it to the python lib
```cmake
add_executable(pure-embedding "")

target_sources(pure-embedding
  PRIVATE
    embedding.cpp
)

# inclued the directory for the header file included
target_include_directories(pure-embedding
  PRIVATE
    ${PYTHON_INCLUDE_DIRS}
)

# link the executable to the python library
target_link_libraries(pure-embedding
  PRIVATE
    ${PYTHON_LIBRARIES}
)
```
8. Make sure to use use_numpy.py
```cmake

target_sources(pure-embedding
  PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/use_numpy.py
)
```
> NOTE: to better understanc cmake subdirectory
> https://cgold.readthedocs.io/en/latest/tutorials/cmake-sources/subdirectories.html


* we can print the properties of interface with the __cmake_print_properties__ command, offered by CMakePrintHelpers.cmake
```cmake
include(CMakePrintHelpers)
cmake_print_properties(
  TARGETS
    OpenMP::OpenMP_CXX # imported type
  PROPERTIES
    INTERFACE_COMPILE_OPTIONS
    INTERFACE_COMPILE_DIRECTORIES
    INTERFACE_COMPILE_LIBRARIES
)
``` 

### Detecting external libraries: using pkg-config
1. look for pkg-config, using its find-module shipped with CMake.
```cmake
# QUIET : will only print messages only if the required pkg-config is not found !
find_package(PkgConfig REQUIRED QUIET)
```

2. When found, search for any lib or program that ships a package configuration .pc file using pkg_search_moldue.
```cmake
pkg_search_module(
  ZeroMQ
  REQUIRED
    libzeromq libzmq lib0mq
  IMPORTED_TARGET
)

if(TARGET PkgConfig::ZeroMQ)
  message(STATUS "Found ZeroMQ")
endif()


add_Executable(hwserver hwserver.c)
target_link_libraries(hwserver PkgConfig::ZeroMQ)
```

### Detecing external libraries: writing a find-mdoule