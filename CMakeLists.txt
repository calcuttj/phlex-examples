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

# ROOT dictionary so FORM can write HepMC3::GenEvent. HEPMC3_ROOTIO makes the
# HepMC3 headers declare the custom GenEvent streamer in hepmc3_streamer.cpp;
# it must be defined wherever those headers are seen, hence PUBLIC.
add_library(hepmc3_dict SHARED hepmc3_streamer.cpp)
target_compile_definitions(hepmc3_dict PUBLIC HEPMC3_ROOTIO)
target_link_libraries(hepmc3_dict PUBLIC HepMC3::HepMC3 ROOT::Core)
# root_generate_dictionary only searches the target's own include directories
target_include_directories(hepmc3_dict PRIVATE ${HEPMC3_INCLUDE_DIR})
root_generate_dictionary(
  G__hepmc3
  HepMC3/GenEvent.h
  HepMC3/Data/GenEventData.h
  MODULE hepmc3_dict
  LINKDEF hepmc3_selection.xml
)

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

# Enable asserts for split_square_verify
target_compile_options(split_square_verify PRIVATE -UNDEBUG)
