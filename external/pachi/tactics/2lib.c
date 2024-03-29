#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define QUICK_BOARD_CODE

#define DEBUG
#include "../board.h"
#include "../debug.h"
#include "../mq.h"
#include "2lib.h"
#include "ladder.h"
#include "selfatari.h"


/* Whether to avoid capturing/atariing doomed groups (this is big
 * performance hit and may reduce playouts balance; it does increase
 * the strength, but not quite proportionally to the performance). */
//#define NO_DOOMED_GROUPS


static bool
miai_2lib(board_t *b, group_t group, enum stone color)
{
	bool can_connect = false, can_pull_out = false;
	/* We have miai if we can either connect on both libs,
	 * or connect on one lib and escape on another. (Just
	 * having two escape routes can be risky.) We must make
	 * sure that we don't consider following as miai:
	 * X X X O
	 * X . . O
	 * O O X O - left dot would be pull-out, right dot connect */
	foreach_neighbor(b, board_group_info(b, group).lib[0], {
		enum stone cc = board_at(b, c);
		if (cc == S_NONE && cc != board_at(b, board_group_info(b, group).lib[1]))
			can_pull_out = true;
		else if (cc != color)
			continue;

		group_t cg = group_at(b, c);
		if (cg && cg != group && board_group_info(b, cg).libs > 1)
			can_connect = true;
	});
	
	foreach_neighbor(b, board_group_info(b, group).lib[1], {
		enum stone cc = board_at(b, c);
		if (c == board_group_info(b, group).lib[0])  continue;
		if (cc == S_NONE && can_connect)     return true;
		else if (cc != color)		     continue;

		group_t cg = group_at(b, c);
		if (cg && cg != group && board_group_info(b, cg).libs > 1)
			return (can_connect || can_pull_out);
	});
	return false;
}

static bool
defense_is_hopeless(board_t *b, group_t group, enum stone owner,
		    enum stone to_play, coord_t lib, coord_t otherlib,
		    bool use)
{
	/* If we are the defender not connecting out, do not
	 * escape with moves that do not gain liberties anyway
	 * - either the new extension has just single extra
	 * liberty, or the "gained" liberties are shared. */
	/* XXX: We do not check connecting to a short-on-liberty
	 * group (e.g. ourselves). */
	if (DEBUGL(7))  fprintf(stderr, "\tif_check %d and defending %d and uscount %d ilcount %d\n",
				use, to_play == owner,
				neighbor_count_at(b, lib, owner),
				immediate_liberty_count(b, lib));
	if (!use)  return false;
	
	if (to_play == owner && neighbor_count_at(b, lib, owner) == 1) {
		if (immediate_liberty_count(b, lib) == 1)	return true;
		if (immediate_liberty_count(b, lib) == 2
		    && coord_is_adjecent(lib, otherlib))	return true;
	}
	return false;
}

void
can_atari_group(board_t *b, group_t group, enum stone owner,
		enum stone to_play, move_queue_t *q,
		int tag, bool use_def_no_hopeless)
{
	bool have[2] = { false, false };
	bool preference[2] = { true, true };
	for (int i = 0; i < 2; i++) {
		coord_t lib = board_group_info(b, group).lib[i];
		assert(board_at(b, lib) == S_NONE);
		if (!board_is_valid_play(b, to_play, lib))  continue;

		if (DEBUGL(6))  fprintf(stderr, "- checking liberty %s of %s %s, filled by %s\n",
					coord2sstr(lib),
					stone2str(owner), coord2sstr(group),
					stone2str(to_play));

		/* Don't play at the spot if it is extremely short
		 * of liberties... */
		/* XXX: This looks harmful, could significantly
		 * prefer atari to throwin:
		 *
		 * XXXOOOOOXX
		 * .OO.....OX
		 * XXXOOOOOOX */
#if 0
		if (neighbor_count_at(b, lib, stone_other(owner)) + immediate_liberty_count(b, lib) < 2)
			continue;
#endif

		/* Prevent hopeless escape attempts. */
		if (defense_is_hopeless(b, group, owner, to_play, lib,
					board_group_info(b, group).lib[1 - i],
					use_def_no_hopeless))
			continue;

#ifdef NO_DOOMED_GROUPS
		/* If the owner can't play at the spot, we don't want
		 * to bother either. */
		if (is_bad_selfatari(b, owner, lib))  continue;
#endif

		/* Of course we don't want to play bad selfatari
		 * ourselves, if we are the attacker... */
		if (
#ifdef NO_DOOMED_GROUPS
		    to_play != owner &&
#endif
		    is_bad_selfatari(b, to_play, lib)) {
			if (DEBUGL(7))  fprintf(stderr, "\tliberty is selfatari\n");
			
			coord_t coord = pass;
			group_t bygroup = 0;
			if (to_play != owner) {
				/* Okay! We are attacker; maybe we just need
				 * to connect a false eye before atari - this
				 * is very common in the corner. */
				coord = selfatari_cousin(b, to_play, lib, &bygroup);
			}
			if (is_pass(coord))  continue;
			
			/* Ok, connect, but prefer not to. */
			enum stone byowner = board_at(b, bygroup);
			if (DEBUGL(7))  fprintf(stderr, "\treluctantly switching to cousin %s (group %s %s)\n",
						coord2sstr(coord), coord2sstr(bygroup), stone2str(byowner));
			/* One more thing - is the cousin sensible defense
			 * for the other group? */
			if (defense_is_hopeless(b, bygroup, byowner, to_play, coord, lib, use_def_no_hopeless))
				continue;
			lib = coord;
			preference[i] = false;

		/* By now, we must be decided we add the move to the
		 * queue!  [comment intentionally misindented] */
		}

		have[i] = true;

		/* If the move is too "lumpy", prefer the alternative:
		 *
		 * #######
		 * ..O.X.X <- always play the left one!
		 * OXXXXXX */
		if (neighbor_count_at(b, lib, to_play) + neighbor_count_at(b, lib, S_OFFBOARD) >= 3) {
			if (DEBUGL(7))  fprintf(stderr, "\tlumpy: mine %d + edge %d\n",
						neighbor_count_at(b, lib, to_play),
						neighbor_count_at(b, lib, S_OFFBOARD));
			preference[i] = false;
		}

		if (DEBUGL(6))  fprintf(stderr, "+ liberty %s ready with preference %d\n", coord2sstr(lib), preference[i]);

		/* If we prefer only one of the moves, pick that one. */
		if (i == 1 && have[0] && preference[0] != preference[1]) {
			if (!preference[0]) {
				if (q->move[q->moves - 1] == board_group_info(b, group).lib[0])
					q->moves--;
				/* ...else{ may happen, since we call
				 * mq_nodup() and the move might have
				 * been there earlier. */
			} else {
				assert(!preference[1]);
				continue;
			}
		}

		/* Tasty! Crispy! Good! */
		mq_add(q, lib, tag);
		mq_nodup(q);
	}

	if (DEBUGL(7)) {
		char label[256];
		snprintf(label, 256, "final %s %s liberties to play by %s = ",
			stone2str(owner), coord2sstr(group),
			stone2str(to_play));
		mq_print_line(label, q);
	}
}

void
group_2lib_check(board_t *b, group_t group, enum stone to_play, move_queue_t *q, int tag, bool use_miaisafe, bool use_def_no_hopeless)
{
	enum stone color = board_at(b, group_base(group));
	assert(color != S_OFFBOARD && color != S_NONE);

	if (DEBUGL(5))  fprintf(stderr, "[%s] 2lib check of color %d\n",
				coord2sstr(group), color);

	/* Do not try to atari groups that cannot be harmed. */
	if (use_miaisafe && miai_2lib(b, group, color))
		return;

	can_atari_group(b, group, color, to_play, q, tag, use_def_no_hopeless);

	/* Can we counter-atari another group, if we are the defender? */
	if (to_play != color)
		return;
	foreach_in_group(b, group) {
		foreach_neighbor(b, c, {
			if (board_at(b, c) != stone_other(color))  continue;
			
			group_t g2 = group_at(b, c);
			if (board_group_info(b, g2).libs == 1 &&
			    board_is_valid_play(b, to_play, board_group_info(b, g2).lib[0])) {
				/* We can capture a neighbor. */
				mq_add(q, board_group_info(b, g2).lib[0], tag);
				mq_nodup(q);
				continue;
			}
			if (board_group_info(b, g2).libs != 2)  continue;
			can_atari_group(b, g2, stone_other(color), to_play, q, tag, use_def_no_hopeless);
		});
	} foreach_in_group_end;
}


bool
can_capture_2lib_group(board_t *b, group_t g, move_queue_t *q, int tag)
{
	assert(board_group_info(b, g).libs == 2);
	for (int i = 0; i < 2; i++) {
		coord_t lib = board_group_info(b, g).lib[i];
		//fprintf(stderr, "can_capture_2lib_group(): checking %s\n", coord2sstr(lib));
		if (wouldbe_ladder_any(b, g, lib)) {
			if (q)  mq_add(q, lib, tag);
			return true;
		}
	}	
	return false;
}

void
group_2lib_capture_check(board_t *b, group_t group, enum stone to_play, move_queue_t *q, int tag, bool use_miaisafe, bool use_def_no_hopeless)
{
	enum stone color = board_at(b, group_base(group));
	assert(color != S_OFFBOARD && color != S_NONE);
	
	if (DEBUGL(5))  fprintf(stderr, "[%s] 2lib capture check of color %d\n",
				coord2sstr(group), color);

	if (to_play != color) {  /* Attacker */		
		can_capture_2lib_group(b, group, q, tag);
		return;
	}

	/* Can we counter-atari another group, if we are the defender? */
	foreach_in_group(b, group) {
		foreach_neighbor(b, c, {
			if (board_at(b, c) != stone_other(color))  continue;
			
			group_t g2 = group_at(b, c);
			if (board_group_info(b, g2).libs == 1 &&
			    board_is_valid_play(b, to_play, board_group_info(b, g2).lib[0])) {
				/* We can capture a neighbor. */
				mq_add(q, board_group_info(b, g2).lib[0], tag);
				mq_nodup(q);
				continue;
			}
			if (board_group_info(b, g2).libs != 2)  continue;
			can_capture_2lib_group(b, g2, q, tag);
		});
	} foreach_in_group_end;
}

