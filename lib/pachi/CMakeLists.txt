PROJECT(my_pachi)

SET(PACHI_SRCS
        board.c
        board.h
        board_play.h
        board_undo.c
        board_undo.h
        debug.h
        engine.c
        engine.h
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
        playout.c
        playout.h
        random.c
        random.h
        stone.c
        stone.h
        timeinfo.c
        timeinfo.h
        util.h
        )

set(CMAKE_C_FLAGS "-fPIC")
set(CMAKE_C_FLAGS_RELEASE "-O2")
set(CMAKE_LD_FLAGS "-Wl,-Bsymbolic")
ADD_LIBRARY(my_pachi SHARED ${PACHI_SRCS})
ADD_LIBRARY(my_pachi_static STATIC ${PACHI_SRCS})
SET_TARGET_PROPERTIES(my_pachi_static PROPERTIES OUTPUT_NAME "my_pachi")
SET_TARGET_PROPERTIES(my_pachi PROPERTIES CLEAN_DIRECT_OUTPUT 1)
SET_TARGET_PROPERTIES(my_pachi_static PROPERTIES CLEAN_DIRECT_OUTPUT 1)