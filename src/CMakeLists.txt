PROJECT(estar_go)

SET(ESTAR_GO_SRCS
        Config.cpp
        Gtp.cpp
        GoBoard.cpp
        utils/Logger.cpp
        utils/INIReader.cpp
        MCTSPlayer.cpp)

ADD_LIBRARY(estar_go_lib SHARED ${ESTAR_GO_SRCS})
ADD_LIBRARY(estar_go_static STATIC ${ESTAR_GO_SRCS})
SET_TARGET_PROPERTIES(estar_go_static PROPERTIES OUTPUT_NAME "estar_go_lib")
SET_TARGET_PROPERTIES(estar_go_lib PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(estar_go_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)