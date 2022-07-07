cmake_minimum_required(VERSION 3.16)
project(pachi C)

set(CMAKE_C_STANDARD 99)
SET(PACHI_SRCS
        board.c
        board.h
        board_play.h
        board_undo.c
        board_undo.h
        debug.h
        fbook.c
        fbook.h
        fixp.h
        move.c
        move.h
        mq.h
        ownermap.c
        ownermap.h
        pattern.c
        pattern.h
        pattern3.c
        pattern3.h
        patternprob.c
        patternprob.h
        patternsp.c
        patternsp.h
        random.c
        random.h
        stone.c
        stone.h
        timeinfo.c
        timeinfo.h
        util.h
        )
add_library(pachi SHARED ${PACHI_SRCS})
LINK_DIRECTORIES( ${PROJECT_SOURCE_DIR}/lib)