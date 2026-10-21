cmake_minimum_required(VERSION 3.31)
project(my_phlex_example VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL "14.1")
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "15")
      add_compile_options(-Wno-stringop-overflow)
    endif()
    if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "16")
      add_compile_options(-Wno-array-bounds)
    endif()
  endif()
endif()

find_package(Boost REQUIRED json)
find_package(TBB REQUIRED)
find_package(spdlog REQUIRED)
find_package(phlex REQUIRED)
find_package(ROOT REQUIRED COMPONENTS Core RIO)
find_package(HepMC3 REQUIRED)
# libHepMC3rootIO carries the ROOT dictionaries FORM needs to write
# HepMC3::GenEventData; ROOT loads it through its rootmap when asked for them.
if(NOT TARGET HepMC3::rootIO)
  message(FATAL_ERROR "HepMC3 at ${HepMC3_DIR} was built without rootIO (HEPMC3_ENABLE_ROOTIO)")
endif()

add_subdirectory(migration)

add_compile_options(-Wall -Wpedantic -Werror)

# Proxies for framework-agnostic experiment libraries
# N.B. The specified library type should be SHARED.
add_library(my_add SHARED my_add.cpp)
add_library(my_geometry SHARED my_geometry.cpp)
add_library(mcparticle SHARED mcparticle.cpp)
add_library(mctruth SHARED mctruth.cpp)
target_include_directories(mctruth PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(mctruth PUBLIC mcparticle ROOT::Core)
# ROOT dictionary so FORM can write MCTruth (and the MCParticle types it holds)
root_generate_dictionary(G__mctruth mctruth.hpp mcparticle.hpp MODULE mctruth LINKDEF mctruth_LinkDef.h)

# Register providers for data products
add_library(numbers_source MODULE numbers_source.cpp)
target_link_libraries(numbers_source PRIVATE phlex::module)

add_library(geometry_source MODULE geometry_source.cpp)
target_link_libraries(geometry_source PRIVATE phlex::module my_geometry)

# Register experiment libraries with Phlex
add_library(add_and_verify MODULE add_and_verify.cpp)
target_link_libraries(add_and_verify PRIVATE phlex::module my_add my_geometry)

# Enable asserts for add_and_verify
# N.B. This should not be done for production code, but it is useful for testing and development.
target_compile_options(add_and_verify PRIVATE -UNDEBUG)

# Unfold example: vector<int> on parent layer -> int on child layer
add_library(vector_source MODULE vector_source.cpp)
target_link_libraries(vector_source PRIVATE phlex::module)

add_library(split_square_verify MODULE split_square_verify.cpp)
target_link_libraries(split_square_verify PRIVATE phlex::module)

add_library(simple_generator MODULE simple_generator.cpp)
target_link_libraries(simple_generator PRIVATE phlex::module my_geometry mctruth mcparticle)

add_library(simple_generator_hepmc MODULE simple_generator_hepmc.cpp)
target_link_libraries(simple_generator_hepmc PRIVATE phlex::source HepMC3::HepMC3)

add_library(convert_hepmcevent SHARED convert_hepmcevent.cpp)
target_link_libraries(convert_hepmcevent PUBLIC HepMC3::HepMC3)

add_library(convert_hepmcevent_to_data MODULE convert_to_data_module.cpp)
target_link_libraries(convert_hepmcevent_to_data PRIVATE phlex::module convert_hepmcevent)

add_library(convert_hepmcevent_from_data MODULE convert_from_data_module.cpp)
target_link_libraries(convert_hepmcevent_from_data PRIVATE phlex::module convert_hepmcevent)

# Enable asserts for split_square_verify
target_compile_options(split_square_verify PRIVATE -UNDEBUG)
