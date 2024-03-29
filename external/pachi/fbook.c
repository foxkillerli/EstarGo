#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG

#include "board.h"
#include "debug.h"
#include "fbook.h"
#include "random.h"


#define HASH_VMIRROR     1
#define HASH_HMIRROR     2
#define HASH_XYFLIP      4

static coord_t
coord_transform(board_t *b, coord_t coord, int i)
{
	int stride = board_stride(b);
	int x = coord_x(coord);  int y = coord_y(coord);	
	if (i & HASH_VMIRROR)  coord = coord_xy(x, stride - 1 - y);
	if (i & HASH_HMIRROR)  coord = coord_xy(stride - 1 - x, y);
	if (i & HASH_XYFLIP)   coord = coord_xy(y, x);
	return coord;
}

/* Check if we can make a move along the fbook right away.
 * Otherwise return pass. */
coord_t
fbook_check(board_t *board)
{
	if (!board->fbook) return pass;

	hash_t hi = board->hash;
	coord_t cf = pass;
	while (!is_pass(board->fbook->moves[hi & fbook_hash_mask])) {
		if (board->fbook->hashes[hi & fbook_hash_mask] == board->hash) {
			cf = board->fbook->moves[hi & fbook_hash_mask];
			break;
		}
		hi++;
	}
	if (!is_pass(cf)) {
		if (DEBUGL(1))
			fprintf(stderr, "fbook match %" PRIhash ":%" PRIhash "\n", board->hash, board->hash & fbook_hash_mask);
	} else {
		/* No match, also prevent further fbook usage
		 * until the next clear_board. */
		if (DEBUGL(4))
			fprintf(stderr, "fbook out %" PRIhash ":%" PRIhash "\n", board->hash, board->hash & fbook_hash_mask);
		fbook_done(board->fbook);
		board->fbook = NULL;
	}
	return cf;
}

static fbook_t *fbcache;

fbook_t *
fbook_init(char *filename, board_t *b)
{
	if (fbcache && fbcache->bsize == board_rsize(b)
	    && fbcache->handicap == b->handicap)
		return fbcache;

	FILE *f = fopen_data_file(filename, "r");
	if (!f) {
		perror(filename);
		return NULL;
	}

	fbook_t *fbook = calloc2(1, fbook_t);
	fbook->bsize = board_rsize(b);
	fbook->handicap = b->handicap;
	/* We do not set handicap=1 in case of too low komi on purpose;
	 * we want to go with the no-handicap fbook for now. */
	for (int i = 0; i < 1<<fbook_hash_bits; i++)
		fbook->moves[i] = pass;

	if (DEBUGL(1))
		fprintf(stderr, "Loading opening fbook %s...\n", filename);

	/* Scratch board where we lay out the sequence;
	 * one for each transposition. */
	board_t *bs[8];
	for (int i = 0; i < 8; i++)
		bs[i] = board_new(fbook->bsize, NULL);
	
	char linebuf[1024];
	while (fgets(linebuf, sizeof(linebuf), f)) {
		char *line = linebuf;
		linebuf[strlen(linebuf) - 1] = 0; // chop

		/* Format of line is:
		 * BSIZE COORD COORD COORD... | COORD
		 * BSIZE/HANDI COORD COORD COORD... | COORD */
		int bsize = strtol(line, &line, 10);
		if (bsize != fbook->bsize)
			continue;
		int handi = 0;
		if (*line == '/') {
			line++;
			handi = strtol(line, &line, 10);
		}
		if (handi != fbook->handicap)
			continue;
		while (isspace(*line)) line++;

		for (int i = 0; i < 8; i++) {
			board_clear(bs[i]);
			last_move(bs[i]).color = S_WHITE;
		}

		while (*line != '|') {
			coord_t c = str2coord(line);

			for (int i = 0; i < 8; i++) {
				coord_t coord = coord_transform(b, c, i);
				move_t m = move_(coord, stone_other(last_move(bs[i]).color));
				int ret = board_play(bs[i], &m);
				assert(ret >= 0);
			}

			while (!isspace(*line)) line++;
			while (isspace(*line)) line++;
		}

		line++;
		while (isspace(*line)) line++;

		/* In case of multiple candidates, pick one with
		 * exponentially decreasing likelihood. */
		while (strchr(line, ' ') && fast_random(2)) {
			line = strchr(line, ' ');
			while (isspace(*line)) line++;
			// fprintf(stderr, "<%s> skip to %s\n", linebuf, line);
		}

		coord_t c = str2coord(line);
		for (int i = 0; i < 8; i++) {
			coord_t coord = coord_transform(b, c, i);
#if 0
			char conflict = is_pass(fbook->moves[bs[i]->hash & fbook_hash_mask]) ? '+' : 'C';
			if (conflict == 'C')
				for (int j = 0; j < i; j++)
					if (bs[i]->hash == bs[j]->hash)
						conflict = '+';
			if (conflict == 'C') {
				hash_t hi = bs[i]->hash;
				while (!is_pass(fbook->moves[hi & fbook_hash_mask]) && fbook->hashes[hi & fbook_hash_mask] != bs[i]->hash)
					hi++;
				if (fbook->hashes[hi & fbook_hash_mask] == bs[i]->hash)
					hi = 'c';
			}
			fprintf(stderr, "%c %" PRIhash ":%" PRIhash " (<%d> %s)\n", conflict,
			        bs[i]->hash & fbook_hash_mask, bs[i]->hash, i, linebuf);
#endif
			hash_t hi = bs[i]->hash;
			while (!is_pass(fbook->moves[hi & fbook_hash_mask]) && fbook->hashes[hi & fbook_hash_mask] != bs[i]->hash)
				hi++;
			fbook->moves[hi & fbook_hash_mask] = coord;
			fbook->hashes[hi & fbook_hash_mask] = bs[i]->hash;
			fbook->movecnt++;
		}
	}

	for (int i = 0; i < 8; i++)
		board_delete(&bs[i]);

	fclose(f);

	if (!fbook->movecnt) {
		/* Empty book is not worth the hassle. */
		fbook_done(fbook);
		return NULL;
	}

	fbook_t *fbold = fbcache;
	fbcache = fbook;
	if (fbold)
		fbook_done(fbold);

	return fbook;
}

void fbook_done(fbook_t *fbook)
{
	if (fbook != fbcache)
		free(fbook);
}
