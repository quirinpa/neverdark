#include "view.h"

#include "search.h"
#include "noise.h"
#include "geography.h"
#include "item.h"

#ifndef CLIENT
#include "db.h"
#include "externs.h"
#include "props.h"
#include "params.h"
#include "web.h"
#endif

/* #undef NDEBUG */
#include "debug.h"

#define BIOME_BG(i) (NIGHT_IS \
		? ANSI_RESET : biomes[i].bg)

// global buffer for map? // FIXME d bio_limit
static char view_buf[8 * BUFSIZ];
static const char * v = "|";
static const char * l = ANSI_FG_WHITE "+";
static const char *h_open	= "   ";
static const char *h_closed	= "---";

vtf_t vtf_map[] = {
	[VTF_SHOP] = {
		.pre = ANSI_BOLD ANSI_FG_GREEN,
		.emp = '$',
	},
	[VTF_NPC] = {
		.pre = ANSI_BOLD ANSI_FG_YELLOW,
		.emp = '!',
	},
	[VTF_POND] = {
		.pre = ANSI_BOLD ANSI_FG_BLUE,
		.emp = '~',
	},
	[VTF_PLAYER] = {
		.pre = ANSI_BOLD ANSI_FG_BLUE,
		.emp = '#',
	},
};

static inline char *
dr_tree(struct plant_data pd, int n, char *b) {
	if (PLANT_N(pd.n, n)) {
		plant_t *pl = PLANT(pd.id[n]);
		b = stpcpy(b, pl->pre);
		*b++ = PLANT_N(pd.n, n) > PLANT_HALF
			? pl->big : pl->small;
	} else
		*b++ = ' ';
	return b;
}

// TODO remove phantom trees

static inline char *
dr_room(int descr, dbref player, view_tile_t *t, char *buf, const char *bg)
{
	register char *b = buf;
	vtf_t *vtf = NULL;
	int i;

	if (t->exits & E_DOWN)
		b = stpcpy(b, ANSI_FG_WHITE "<");
	else
		b = dr_tree(t->pd, 0, b);


	for (i = 0; i < VTF_MAX; i++) {
		if (!(t->flags & (1 << i)))
			continue;

		vtf = &vtf_map[1 << i];
		break;
	}

	b = stpcpy(b, ANSI_RESET);
	b = stpcpy(b, bg);

	if (vtf) {
		b = stpcpy(b, vtf->pre);
		*b++ = vtf->emp;
	} else
		b = dr_tree(t->pd, 1, b);


	if (t->exits & E_UP)
		b = stpcpy(b, ANSI_FG_WHITE ">");
	else
		b = dr_tree(t->pd, 2, b);

	b = stpcpy(b, ANSI_RESET);
	b = stpcpy(b, bg);

	return b;
}

static inline unsigned
floor_get(dbref what) {
	unsigned floor = GETFLOOR(what);
	if (floor > BIOME_VOLCANIC)
		return BIOME_VOLCANIC;
	else
		return floor;
}

static inline const char *
wall_paint(int floor)
{
	switch (floor) {
	case -8:
		return ANSI_FG_RED;
	default:
		return ANSI_FG_YELLOW;
	}
}

static inline char *
dr_v(char *b, view_tile_t *t, enum exit exit,
		const char *wp) {
	if (t->doors & exit)
		return stpcpy(b, l);
	else if (t->exits & exit)
		return stpcpy(b, " ");
	else {
		b = stpcpy(b, wp);
		return stpcpy(b, v);
	}
}

static char *
dr_vs(char *b, int descr, dbref player, view_tile_t *t)
{
	view_tile_t * const t_max = &t[VIEW_SIZE - 1];
	const char *bg, *wp;
	view_tile_t *tp;
	unsigned floor;
	enum exit exit; // loop cache

	if (t->room < 0) {
		floor = t->bio_idx;
		wp = "";
	} else {
		floor = floor_get(t->room);
		wp = wall_paint(floor);
	}

	bg = BIOME_BG(floor);
	b = stpcpy(b, " ");
	for (;;) {
		b = stpcpy(b, bg);

		if (t->room >= 0)
			b = dr_room(descr, player, t, b, bg);
		else {
			b = dr_tree(t->pd, 0, b);
			b = dr_tree(t->pd, 1, b);
			b = dr_tree(t->pd, 2, b);
		}

		tp = t;
		t++;

		if (t >= t_max)
			break;

		if (t->room < 0 || GETTMP(t->room)) {
			floor = t->bio_idx;
			bg = BIOME_BG(floor);
		} else {
			floor = floor_get(t->room);
			wp = wall_paint(floor);
			bg = BIOME_BG(floor);
			b = stpcpy(b, bg);
		}

		if (t->room >= 0) {
			exit = E_WEST;
			tp = t;
		} else if (tp->room >= 0) {
			exit = E_EAST;
		} else {
			*b++ = ' ';
			continue;
		}

		b = dr_v(b, tp, exit, wp);
	}

        b = stpcpy(b, ANSI_RESET"\n");
        return b;
}

static inline char *
dr_h(char *b, char const *bg, char const *wp, char *w, int toggle)
{
	if (toggle) {
		b = stpcpy(b, " ");
		b = stpcpy(b, bg);
	} else {
		b = stpcpy(b, bg);
		b = stpcpy(b, " ");
	}
	b = stpcpy(b, wp);
	b = stpcpy(b, w);
	return b;
}

static inline char *
dr_hs_n(char *b, int descr, dbref player,
		view_tile_t *t)
{
	view_tile_t * const t_max = &t[VIEW_SIZE - 1];
	const char *wp, *bg;
	char wb[32], *w;
	unsigned floor;
	int toggle;

	for (wp = "",
	     bg = BIOME_BG(
		     t->room < 0
		     ? t->bio_idx
		     : floor_get(t->room)) ;
	     t < t_max ;
	     bg = BIOME_BG(floor),
	     b = dr_h(b, bg, wp, w, toggle),
	     t++)
	{
		view_tile_t *tn = &t[VIEW_SIZE];
		dbref curr = t->room,
		      next = tn->room;

		floor = t->bio_idx;
		if (next >= 0) {
			if (GETTMP(next)) {
				if (curr < 0 || GETTMP(curr)) {
					floor = t->bio_idx;
					toggle = 1;
				} else {
					floor = floor_get(curr);
					toggle = 0;
				}
			} else {
				floor = floor_get(next);
				toggle = 0;
			}
		} else if (curr >= 0 && !GETTMP(curr)) {
			floor = floor_get(curr);
			toggle = 0;
		} else {
			floor = t->bio_idx;
			w = (char *) h_open;
			toggle = 1;
			continue;
		}

		wp = wall_paint(floor);

		// TODO fix t exits
		//  if (!(t->exits & E_SOUTH)) should work
		// see FIXME below

		if (!(t->exits & E_SOUTH) || !(tn->exits & E_NORTH))
			w = (char *) h_closed;
		else if (t->doors & E_SOUTH) {
			snprintf(wb, sizeof(wb), "-"ANSI_FG_WHITE"+%s-", wp);
			w = wb;
		} else
			w = (char *) h_open;
	}

        b = stpcpy(b, ANSI_RESET"\n");
        return b;
}

static void
view_draw(int descr, dbref player, view_t view) {
	view_tile_t * const t_max = &view[VIEW_BDI];
	view_tile_t *t = view;
	char *p = view_buf;
        memset(view_buf, 0, sizeof(view_buf));

	p = dr_vs(p, descr, player, t);

	for (; t < t_max;) {
		p = dr_hs_n(p, descr, player, t);
		t += VIEW_SIZE;
		p = dr_vs(p, descr, player, t);
	}

}

static inline void
view_build_exit(view_tile_t *t, int descr, dbref player, dbref loc, enum exit e) {
	register dbref tmp
		= gexit_where(descr, player, loc, e);

	if (tmp > 0) {
		t->exits |= e;
		if (GETDOOR(tmp))
			t->doors |= e;
	}
}

static inline void
view_build_exit_z(view_tile_t *t, int descr, dbref player, dbref loc, enum exit e) {
	register dbref tmp
		= gexit_where(descr, player, loc, e);

	if (tmp > 0 && DBFETCH(tmp)->sp.exit.ndest
			&& DBFETCH(tmp)->sp.exit.dest[0] >= 0)

		t->exits |= e;
}

static inline void
view_build_exit_s(view_tile_t *t, int descr, dbref player, dbref loc, pos_t p, enum exit e) {
	pos_t pa;
	pos_move(pa, p, e);
	register dbref tmp;

	tmp = map_get(pa);

	// FIXME not returning correct room (at least for E_SOUTH)
	debug("here 0x%llx -> 0x%llx (%d)\n",
			* (morton_t *) p,
			* (morton_t *) pa,
			tmp);

	if (tmp > 0) {
		tmp = gexit_where(descr, player, tmp, e_simm(e));
		if (tmp > 0) {
			t->exits |= e;
			if (GETDOOR(tmp))
				t->doors |= e;
		}
	} else
		t->exits |= e;
}

static inline ucoord_t
view_build_flags(dbref loc) {
	dbref tmp = DBFETCH(loc)->contents;
	ucoord_t flags = 0;

	DOLIST(tmp, tmp) switch (Typeof(tmp)) {
		case TYPE_PLAYER:
			flags |= VTF_PLAYER;
			break;

		case TYPE_THING:
			if (GETSHOP(tmp))
				flags |= VTF_SHOP;

			else if (GETLID(tmp) >= 0)
				flags |= VTF_NPC;

			else if (GETDRINK(tmp) >= 0)
				flags |= VTF_POND;

			break;
	}

	return flags;
}

static void
view_build_tile(int descr, dbref player,
		struct bio *n, dbref loc,
		view_tile_t *t, pos_t p)
{
	if (loc > 0) {
		view_build_exit(t, descr, player, loc, E_EAST);
		view_build_exit(t, descr, player, loc, E_NORTH);
		view_build_exit(t, descr, player, loc, E_WEST);
		view_build_exit(t, descr, player, loc, E_SOUTH);
		view_build_exit_z(t, descr, player, loc, E_UP);
		view_build_exit_z(t, descr, player, loc, E_DOWN);
		t->flags = view_build_flags(loc);
	} else {
		view_build_exit_s(t, descr, player, loc, p, E_EAST);
		view_build_exit_s(t, descr, player, loc, p, E_NORTH);
		view_build_exit_s(t, descr, player, loc, p, E_WEST);
		view_build_exit_s(t, descr, player, loc, p, E_SOUTH);
		/* view_build_exit_sz(t, descr, player, loc, E_UP); */
		/* view_build_exit_sz(t, descr, player, loc, E_DOWN); */
	}

	t->room = loc;
	t->bio_idx = n->bio_idx;
	memcpy(&t->pd, &n->pd, sizeof(n->pd));
}

static inline view_tile_t *
_view_build(int descr, dbref player,
		struct bio *n, dbref *g,
		view_tile_t *b, pos_t p)
{
	dbref *g_max = g + VIEW_SIZE;

	// TODO vary p vertically and horizontally
	for (; g < g_max ; n++, g++, b++, p[1]++)
		view_build_tile(descr, player,
				n, *g, b, p);

        return b;
}

void
do_view(int descr, dbref player)
{
	struct bio bd[VIEW_M], *n_p = bd,
		   *n_max = &bd[VIEW_BDI + 1];
	pos_t pos, opos;
	map_where(opos, getloc(player));
	view_t view;
        view_tile_t *p = view;
	dbref o[VIEW_M], *o_p = o;

	memcpy(pos, opos, sizeof(pos));
	noise_view(bd, pos);
	map_search(o, pos, VIEW_AROUND);
	memset(view, 0, sizeof(view));

	for (; n_p < n_max;) {
		_view_build(descr, player, n_p,
				o_p, p, pos);
		p += VIEW_SIZE;
		o_p += VIEW_SIZE;
		n_p += VIEW_SIZE;
		pos[0]++;
		pos[1] = opos[1];
	}

	view_draw(descr, player, view);
        if (web_geo_view(descr, view_buf))
		notify(player, view_buf);
}

