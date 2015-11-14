cmake_minimum_required(VERSION 2.8.4)
project(tsppddl)

# CPLEX ROOT DIR HINT
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(CPLEX_ROOT_DIR "/opt/ibm" CACHE FILEPATH "IBM Cplex install dir")
endif()
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(CPLEX_ROOT_DIR "/opt/ibm/ILOG/CPLEX_Studio126" CACHE FILEPATH "IBM Cplex install dir")
endif()

# BOOST ROOT DIR HINT (Linux only)
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(BOOST_ROOT "~/boost-trunk/boost")
endif()

# COMPILER: COMMON FLAGS
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -DIL_STD -m64 -Wall -Werror")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DDEBUG=true -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -DNDEBUG -DDEBUG=false -O3 -flto")

# COMPILER: PLATFORM-DEPENDENT FLAGS
if(CMAKE_SYSTEM_NAME STREQUAL Darwin)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -save-temps=obj")
endif()

# LOAD MODULES: cplex, boost
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
find_package(Cplex)
find_package(Boost)

set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

# INCLUDE DIRECTORIES
include_directories("src")
include_directories(SYSTEM ${CPLEX_INCLUDE_DIRS})
include_directories(SYSTEM ${Boost_INCLUDE_DIR})

# BUILD THE EXECUTABLE INSIDE ./build
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/build")

set(SOURCE_FILES
    src/heuristics/inserter.h
    src/heuristics/insertion_scorer.h
    src/heuristics/path_scorer.h
    src/heuristics/request_scorer.h
    src/heuristics/one_phase_heuristic.h
    src/heuristics/two_phase_heuristic.h
    src/heuristics/k_opt_heuristic.cpp
    src/heuristics/k_opt_heuristic.h
    src/network/arc.h
    src/network/graph_info.h
    src/network/graph_info.cpp
    src/network/graph_writer.cpp
    src/network/graph_writer.h
    src/network/node.h
    src/network/path.cpp
    src/network/path.h
    src/network/tsp_graph.cpp
    src/network/tsp_graph.h
    src/parser/params/bc_params.h
    src/parser/params/constructive_heuristics_params.h
    src/parser/params/k_opt_params.h
    src/parser/params/subgradient_params.h
    src/parser/params/tabu_search_params.h
    src/parser/parser.cpp
    src/parser/parser.h
    src/parser/program_params.h
    src/program/program.cpp
    src/program/program.h
    src/program/program_data.cpp
    src/program/program_data.h
    src/solver/bc/callbacks/callbacks_helper.h
    src/solver/bc/callbacks/cuts_callback.cpp
    src/solver/bc/callbacks/cuts_callback.h
    src/solver/bc/callbacks/cuts_lazy_constraint.cpp
    src/solver/bc/callbacks/cuts_lazy_constraint.h
    src/solver/bc/callbacks/feasibility_cuts_separator.cpp
    src/solver/bc/callbacks/feasibility_cuts_separator.h
    src/solver/bc/callbacks/print_relaxation_graph_callback.cpp
    src/solver/bc/callbacks/print_relaxation_graph_callback.h
    src/solver/bc/callbacks/vi_separator_capacity.cpp
    src/solver/bc/callbacks/vi_separator_capacity.h
    src/solver/bc/callbacks/vi_separator_fork.cpp
    src/solver/bc/callbacks/vi_separator_fork.h
    src/solver/bc/callbacks/vi_separator_generalised_order.cpp
    src/solver/bc/callbacks/vi_separator_generalised_order.h
    src/solver/bc/callbacks/vi_separator_subtour_elimination.cpp
    src/solver/bc/callbacks/vi_separator_subtour_elimination.h
    src/solver/bc/bc_solver.cpp
    src/solver/bc/bc_solver.h
    src/solver/heuristics/heuristic_solver.cpp
    src/solver/heuristics/heuristic_solver.h
    src/solver/metaheuristics/tabu/kopt3_solver.cpp
    src/solver/metaheuristics/tabu/kopt3_solver.h
    src/solver/metaheuristics/tabu/tabu_solver.cpp
    src/solver/metaheuristics/tabu/tabu_solver.h
src/main.cpp)

add_executable(tsppddl ${SOURCE_FILES})
target_link_libraries(tsppddl ${CPLEX_LIBRARIES})