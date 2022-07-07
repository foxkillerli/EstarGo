PROJECT(estar_go)

SET(ESTAR_GO_SRCS
        Config.cpp
        Gtp.cpp
        GoBoard.cpp
        MCTSPlayer.cpp
        MCTSTree.cpp)
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${PROJECT_SOURCE_DIR}/external/pachi)
link_directories(${PROJECT_SOURCE_DIR}/external/pachi)