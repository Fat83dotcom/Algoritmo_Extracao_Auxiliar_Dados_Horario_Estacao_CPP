cmake_minimum_required(VERSION 3.5)

#-- Caminho para o arquivo FindPQXX.cmake
list(APPEND CMAKE_MODULE_PATH "/home/fernando/Área\ de\ Trabalho/EstudosC++/Algoritmo_Extracao_Auxiliar_Dados_Horario_Estacao_CPP/Algoritmo_Extracao")

project(AlgoritmoExtracaoCPP LANGUAGES CXX)

#-- Seta flags para o compilador
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++20 -lpqxx -lpq -lfmt -lcurl")

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(AlgoritmoExtracaoCPP main.cpp)

#-- Parteresponsavel pela lib pqxx
find_package(PostgreSQL REQUIRED)
include_directories(${PostgreSQL_INCLUDE_DIRS})
link_directories(${PostgreSQL_LIBRARY_DIRS})

find_package(PQXX REQUIRED)
target_include_directories(AlgoritmoExtracaoCPP PUBLIC ${PQXX_INCLUDE_DIRS} )
target_link_libraries(AlgoritmoExtracaoCPP PUBLIC ${PQXX_LIBRARIES} curl)

#configuração para compilar curl.h Ubuntu 22.04
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
link_directories( ${CURL_LIBRARIES} )
# target_link_libraries(AlgoritmoExtracaoCPP curl)


set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)