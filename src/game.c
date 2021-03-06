/* $Header$ */

#include "copyright.h"
#include "config.h"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <string.h>

#include <sys/wait.h>

#include "db.h"
#include "props.h"
#include "params.h"
#include "defaults.h"
#include "interface.h"
#include "match.h"
#include "externs.h"
#include "fbstrings.h"
#include "debug.h"
#include "geography.h"
#include "kill.h"
#include "view.h"
#include "web.h"
#include "search.h"
#include "mob.h"

/* declarations */
static const char *dumpfile = 0;
static int epoch = 0;
time_t last_monolithic_time = 0;
static int forked_dump_process_flag = 0;
FILE *input_file;
FILE *delta_infile;
FILE *delta_outfile;
char *in_filename = NULL;

extern void do_showextver(dbref player);
void fork_and_dump(void);
void dump_database(void);

void
do_dump(dbref player, const char *newfile)
{
	char buf[BUFFER_LEN];

	if (Wizard(player)) {
#ifndef DISKBASE
		if (global_dumper_pid != 0) {
			notify(player, "Sorry, there is already a dump currently in progress.");
			return;
		}
#endif
		if (*newfile
#ifdef GOD_PRIV
			&& God(player)
#endif							/* GOD_PRIV */
				) {
			if (dumpfile)
				free((void *) dumpfile);
			dumpfile = alloc_string(newfile);
			snprintf(buf, sizeof(buf), "Dumping to file %s...", dumpfile);
		} else {
			snprintf(buf, sizeof(buf), "Dumping...");
		}
		notify(player, buf);
		dump_db_now();
	} else {
		notify(player, "Sorry, you are in a no dumping zone.");
	}
}

void
do_delta(dbref player)
{
	if (Wizard(player)) {
#ifdef DELTADUMPS
		notify(player, "Dumping deltas...");
		delta_dump_now();
#else
		notify(player, "Sorry, this server was compiled without DELTADUMPS.");
#endif
	} else {
		notify(player, "Sorry, you are in a no dumping zone.");
	}
}

void
do_shutdown(dbref player)
{
	if (Wizard(player)) {
		log_status("SHUTDOWN: by %s", unparse_object(player, player));
		shutdown_flag = 1;
	} else {
		notify(player, "Your delusions of grandeur have been duly noted.");
		log_status("ILLEGAL SHUTDOWN: tried by %s", unparse_object(player, player));
	}
}

#ifdef DISKBASE
extern long propcache_hits;
extern long propcache_misses;
#endif

static void
dump_database_internal(void)
{
	char tmpfile[2048];
	FILE *f;

	snprintf(tmpfile, sizeof(tmpfile), "%s.#%d#", dumpfile, epoch - 1);
	(void) unlink(tmpfile);		/* nuke our predecessor */

	snprintf(tmpfile, sizeof(tmpfile), "%s.#%d#", dumpfile, epoch);

	if ((f = fopen(tmpfile, "wb")) != NULL) {
		db_write(f);
		fclose(f);

#ifdef DISKBASE
		fclose(input_file);
#endif

#ifdef DELTADUMPS
		fclose(delta_outfile);
		fclose(delta_infile);
#endif

		if (rename(tmpfile, dumpfile) < 0)
			perror(tmpfile);

#ifdef DISKBASE
		free((void *) in_filename);
		in_filename = string_dup(dumpfile);
		if ((input_file = fopen(in_filename, "rb")) == NULL)
			perror(dumpfile);
#endif

#ifdef DELTADUMPS
		if ((delta_outfile = fopen(DELTAFILE_NAME, "wb")) == NULL)
			perror(DELTAFILE_NAME);

		if ((delta_infile = fopen(DELTAFILE_NAME, "rb")) == NULL)
			perror(DELTAFILE_NAME);
#endif

	} else {
		perror(tmpfile);
	}

	/* Write out the macros */

	snprintf(tmpfile, sizeof(tmpfile), "%s.#%d#", MACRO_FILE, epoch - 1);
	(void) unlink(tmpfile);

	snprintf(tmpfile, sizeof(tmpfile), "%s.#%d#", MACRO_FILE, epoch);

	if ((f = fopen(tmpfile, "wb")) != NULL) {
		macrodump(macrotop, f);
		fclose(f);
		if (rename(tmpfile, MACRO_FILE) < 0)
			perror(tmpfile);
	} else {
		perror(tmpfile);
	}
	sync();

#ifdef DISKBASE
	/* Only show dumpdone mesg if not doing background saves. */
	DUMPDONE_WARN();
	propcache_hits = 0L;
	propcache_misses = 1L;
#endif
}

void
panic(const char *message)
{
	char panicfile[2048];
	FILE *f;

	log_status("PANIC: %s", message);
	fprintf(stderr, "PANIC: %s\n", message);

	/* shut down interface */
	if (!forked_dump_process_flag) {
		emergency_shutdown();
	}

	/* dump panic file */
	snprintf(panicfile, sizeof(panicfile), "%s.PANIC", dumpfile);
	if ((f = fopen(panicfile, "wb")) == NULL) {
		perror("CANNOT OPEN PANIC FILE, YOU LOSE");
		sync();

#ifdef NOCOREDUMP
		exit(135);
#else							/* !NOCOREDUMP */
# ifdef SIGIOT
		signal(SIGIOT, SIG_DFL);
# endif
		abort();
#endif							/* NOCOREDUMP */
	} else {
		log_status("DUMPING: %s", panicfile);
		fprintf(stderr, "DUMPING: %s\n", panicfile);
		db_write(f);
		fclose(f);
		log_status("DUMPING: %s (done)", panicfile);
		fprintf(stderr, "DUMPING: %s (done)\n", panicfile);
		(void) unlink(DELTAFILE_NAME);
	}

	/* Write out the macros */
	snprintf(panicfile, sizeof(panicfile), "%s.PANIC", MACRO_FILE);
	if ((f = fopen(panicfile, "wb")) != NULL) {
		macrodump(macrotop, f);
		fclose(f);
	} else {
		perror("CANNOT OPEN MACRO PANIC FILE, YOU LOSE");
		sync();
#ifdef NOCOREDUMP
		exit(135);
#else							/* !NOCOREDUMP */
#ifdef SIGIOT
		signal(SIGIOT, SIG_DFL);
#endif
		abort();
#endif							/* NOCOREDUMP */
	}

	sync();

#ifdef NOCOREDUMP
	exit(136);
#else							/* !NOCOREDUMP */
#ifdef SIGIOT
	signal(SIGIOT, SIG_DFL);
#endif
	abort();
#endif							/* NOCOREDUMP */
}

void
dump_database(void)
{
	epoch++;

	log_status("DUMPING: %s.#%d#", dumpfile, epoch);
	dump_database_internal();
	log_status("DUMPING: %s.#%d# (done)", dumpfile, epoch);
}

/*
 * Named "fork_and_dump()" mostly for historical reasons...
 */
void
fork_and_dump(void)
{
	epoch++;

#ifndef DISKBASE
	if (global_dumper_pid != 0) {
		wall_wizards("## Dump already in progress.  Skipping redundant scheduled dump.");
		return;
	}
#endif

	last_monolithic_time = time(NULL);
	log_status("CHECKPOINTING: %s.#%d#", dumpfile, epoch);

	DBDUMP_WARN();

#ifdef DISKBASE
	dump_database_internal();
#else
	if ((global_dumper_pid=fork())==0) {
	/* We are the child. */
		forked_dump_process_flag = 1;
# ifdef NICEVAL
	/* Requested by snout of SPR, reduce the priority of the
	 * dumper child. */
		nice(NICEVAL);
# endif /* NICEVAL */
		set_dumper_signals();
		dump_database_internal();
		_exit(0);
	}
	if (global_dumper_pid < 0) {
	    global_dumper_pid = 0;
	    wall_wizards("## Could not fork for database dumping.  Possibly out of memory.");
	    wall_wizards("## Please restart the server when next convenient.");
	}
#endif
}

#ifdef DELTADUMPS
extern int deltas_count;

int
time_for_monolithic(void)
{
	dbref i;
	int count = 0;
	long a, b;

	if (!last_monolithic_time)
		last_monolithic_time = time(NULL);
	if (time(NULL) - last_monolithic_time >= (MONOLITHIC_INTERVAL - DUMP_WARNTIME)
			) {
		return 1;
	}

	for (i = 0; i < db_top; i++)
		if (FLAGS(i) & (SAVED_DELTA | OBJECT_CHANGED))
			count++;
	if (((count * 100) / db_top) > MAX_DELTA_OBJS) {
		return 1;
	}

	fseek(delta_infile, 0L, 2);
	a = ftell(delta_infile);
	fseek(input_file, 0L, 2);
	b = ftell(input_file);
	if (a >= b) {
		return 1;
	}
	return 0;
}
#endif

void
dump_warning(void)
{
#if DBDUMP_WARNING
#ifdef DELTADUMPS
	if (time_for_monolithic())
		wall_and_flush(DUMPWARN_MESG);
	else
		DELTADUMP_WARN();
#else
	wall_and_flush(DUMPWARN_MESG);
#endif
#endif
}

#ifdef DELTADUMPS
void
dump_deltas(void)
{
	if (time_for_monolithic()) {
		fork_and_dump();
		deltas_count = 0;
		return;
	}

	epoch++;
	log_status("DELTADUMP: %s.#%d#", dumpfile, epoch);

#if DBDUMP_WARNING
	DELTADUMP_WARN();
#endif

	db_write_deltas(delta_outfile);

#if DELTADUMP_WARNING
	DUMPDONE_WARN();
#endif

#ifdef DISKBASE
	propcache_hits = 0L;
	propcache_misses = 1L;
#endif
}
#endif

extern short db_conversion_flag;

int
init_game(const char *infile, const char *outfile)
{
	FILE *f;

	if ((f = fopen(MACRO_FILE, "rb")) == NULL)
		log_status("INIT: Macro storage file %s is tweaked.", MACRO_FILE);
	else {
		macroload(f);
		fclose(f);
	}

	in_filename = (char *) string_dup(infile);
	if ((input_file = fopen(infile, "rb")) == NULL)
		return -1;

#ifdef DELTADUMPS
	if ((delta_outfile = fopen(DELTAFILE_NAME, "wb")) == NULL)
		return -1;

	if ((delta_infile = fopen(DELTAFILE_NAME, "rb")) == NULL)
		return -1;
#endif

	db_free();
	init_primitives();			/* init muf compiler */
	mesg_init();				/* init mpi interpreter */
	SRANDOM(getpid());			/* init random number generator */

	/* ok, read the db in */
	log_status("LOADING: %s", infile);
	fprintf(stderr, "LOADING: %s\n", infile);
	if (db_read(input_file) < 0)
		return -1;
	log_status("LOADING: %s (done)", infile);
	fprintf(stderr, "LOADING: %s (done)\n", infile);

	/* set up dumper */
	if (dumpfile)
		free((void *) dumpfile);
	dumpfile = alloc_string(outfile);

	if (!db_conversion_flag) {
		/* initialize the _sys/startuptime property */
		add_property((dbref) 0, "_sys/startuptime", NULL, (int) time((time_t *) NULL));
		add_property((dbref) 0, "_sys/maxpennies", NULL, MAX_PENNIES);
		add_property((dbref) 0, "_sys/dumpinterval", NULL, DUMP_INTERVAL);
		add_property((dbref) 0, "_sys/max_connects", NULL, 0);
	}

	return 0;
}

void
cleanup_game()
{
	if (dumpfile)
		free((void *) dumpfile);
	free((void *) in_filename);
}

extern short wizonly_mode;
void
do_restrict(dbref player, const char *arg)
{
	if (!Wizard(player)) {
		notify(player, "Permission Denied.");
		return;
	}

	if (!strcmp(arg, "on")) {
		wizonly_mode = 1;
		notify(player, "Login access is now restricted to wizards only.");
	} else if (!strcmp(arg, "off")) {
		wizonly_mode = 0;
		notify(player, "Login access is now unrestricted.");
	} else {
		notify_fmt(player, "Restricted connection mode is currently %s.",
			wizonly_mode ? "on" : "off"
		);
	}
}

/* use this only in process_command */
#define Matched(string) if (!string_prefix(string, command)) \
	goto bad; else matched = 1;

int force_level = 0;
dbref force_prog = NOTHING; /* Set when a program is the source of FORCE */

static inline int
do_v(int descr, dbref player, char const *cmd)
{
	int ofs = 1;
	char const *s = cmd;

	for (; *s && ofs > 0; s += ofs) {
		switch (*s) {
		case COMMAND_TOKEN:
			return s - cmd;
		case SAY_TOKEN:
			do_say(player, s + 1);
			return s + strlen(s) - cmd;
		case POSE_TOKEN:
			do_pose(player, s + 1);
			return s + strlen(s) - cmd;
		}

		ofs = geo_v(descr, player, s);
		if (ofs < 0)
			ofs = - ofs;
		s += ofs;
		ofs = kill_v(descr, player, s);
	}

	return s - cmd;
}

void
process_command(int descr, dbref player, char *command)
{
	char *arg1;
	char *arg2;
	char *full_command;
	char *p;					/* utility */
	char xbuf[BUFFER_LEN];
	char ybuf[BUFFER_LEN];
	struct timeval starttime;
	struct timeval endtime;
	double totaltime;
	pos_t pos;
	map_where(pos, getloc(player));

        // set current descriptor (needed for death)
        CBUG(GETLID(player) < 0);
        mobi_t *liv = MOBI(player);
        liv->descr = descr;

	if (command == 0)
		abort();

	/* robustify player */
	if (player < 0 || player >= db_top ||
		(Typeof(player) != TYPE_PLAYER && Typeof(player) != TYPE_THING)) {
		log_status("process_command: bad player %d", player);
		return;
	}

#if LOG_COMMANDS
	if (Wizard(OWNER(player))) {
		if (!(FLAGS(player) & (INTERACTIVE | READMODE))) {
			dbref here;
			if (!*command)
				goto out;
			here = getloc(player);
			log_command("%s%s%s%s(%d) in %s(%d):%s %s",
						Wizard(OWNER(player)) ? "WIZ: " : "",
						(Typeof(player) != TYPE_PLAYER) ? NAME(player) : "",
						(Typeof(player) != TYPE_PLAYER) ? " owned by " : "",
						NAME(OWNER(player)), (int) player,
						
						here == NOTHING ? "NOWHERE" : NAME(here),
						(int) DBFETCH(player)->location, " ", command);
#if LOG_INTERACTIVE
		} else {
			log_command("%s%s%s%s(%d) in %s(%d):%s %s",
				    Wizard(OWNER(player)) ? "WIZ: " : "",
				    (Typeof(player) != TYPE_PLAYER) ? NAME(player) : "",
				    (Typeof(player) != TYPE_PLAYER) ? " owned by " : "",
				    NAME(OWNER(player)), (int) player,
				    NAME(DBFETCH(player)->location),
				    (int) DBFETCH(player)->location,
				    (FLAGS(player) & (READMODE)) ? " [READ] " : " [INTERP] ", command);
#endif
		}
	}
#endif /* LOG_COMMANDS */

	if (FLAGS(player) & INTERACTIVE) {
		interactive(descr, player, command);
		return;
	}
	/* eat leading whitespace */
	while (*command && isspace(*command))
		command++;

	/* Disable null command once past READ line */
	if (!*command)
		goto out;

	/* profile how long command takes. */
	gettimeofday(&starttime, NULL);

	/* if player is a wizard, and uses overide token to start line... */
	/* ... then do NOT run actions, but run the command they specify. */
	if (!(TrueWizard(OWNER(player)) && (*command == OVERIDE_TOKEN))
	    && can_move(descr, player, command, 0))
	{
		do_move(descr, player, command, 0);	/* command is exact match for exit */
		*match_args = 0;
		*match_cmdname = 0;
		goto out;
	}

	{
		int matched;
		command += do_v(descr, player, command);
		if (*command == COMMAND_TOKEN) {
			command++;
			matched = 1;
		} else if (*command == '\0')
			goto end_of_command;
		else
			goto bad;

		if (TrueWizard(OWNER(player)) && (*command == OVERIDE_TOKEN))
			command++;
		full_command = strcpyn(xbuf, sizeof(xbuf), command);
		for (; *full_command && !isspace(*full_command); full_command++) ;
		if (*full_command)
			full_command++;

		/* find arg1 -- move over command word */
		command = strcpyn(ybuf, sizeof(ybuf), command);
		for (arg1 = command; *arg1 && !isspace(*arg1); arg1++) ;
		/* truncate command */
		if (*arg1)
			*arg1++ = '\0';

		/* remember command for programs */
		strcpyn(match_args, sizeof(match_args), full_command);
		strcpyn(match_cmdname, sizeof(match_cmdname), command);

		/* move over spaces */
		while (*arg1 && isspace(*arg1))
			arg1++;

		/* find end of arg1, start of arg2 */
		for (arg2 = arg1; *arg2 && *arg2 != ARG_DELIMITER; arg2++) ;

		/* truncate arg1 */
		for (p = arg2 - 1; p >= arg1 && isspace(*p); p--)
			*p = '\0';

		/* go past delimiter if present */
		if (*arg2)
			*arg2++ = '\0';
		while (*arg2 && isspace(*arg2))
			arg2++;

		switch (command[0]) {
		case '@':
			switch (command[1]) {
			case 'a':
			case 'A':
				/* @action, @advitam, @armageddon, @attach */
				switch (command[2]) {
				case 'c':
				case 'C':
					Matched("@action");
					do_action(descr, player, arg1, arg2);
					break;
				case 'd':
				case 'D':
					Matched("@advitam");
					do_advitam(descr, player, arg1);
					break;
				case 'r':
				case 'R':
					if (strcmp(command, "@armageddon"))
						goto bad;
					do_armageddon(player, full_command);
					break;
				case 't':
				case 'T':
					Matched("@attach");
					do_attach(descr, player, arg1, arg2);
					break;
				default:
					goto bad;
				}
				break;
			case 'b':
			case 'B':
				/* @bless, @boot */
				switch (command[2]) {
				case 'l':
				case 'L':
					Matched("@bless");
					do_bless(descr, player, arg1, arg2);
					break;
				case 'o':
				case 'O':
					Matched("@boot");
					do_boot(player, arg1);
					break;
				default:
					goto bad;
				}
				break;
			case 'c':
			case 'C':
				/* @chlock, @chown, @chown_lock, @clone,
				   @conlock, @contents, @create, @credits */
				switch (command[2]) {
				case 'h':
				case 'H':
					switch (command[3]) {
					case 'l':
					case 'L':
						Matched("@chlock");
						do_chlock(descr, player, arg1, arg2);
						break;
					case 'o':
					case 'O':
						if(strlen(command) < 7) {
							Matched("@chown");
							do_chown(descr, player, arg1, arg2);
						} else {
							Matched("@chown_lock");
							do_chlock(descr, player, arg1, arg2);
						}
						break;
					default:
						goto bad;
					}
					break;
				case 'l':
				case 'L':
					Matched("@clone");
					do_clone(descr, player, arg1);
					break;
				case 'o':
				case 'O':
					switch (command[4]) {
					case 'l':
					case 'L':
						Matched("@conlock");
						do_conlock(descr, player, arg1, arg2);
						break;
					case 't':
					case 'T':
						Matched("@contents");
						do_contents(descr, player, arg1, arg2);
						break;
					default:
						goto bad;
					}
					break;
				case 'r':
				case 'R':
					if (string_compare(command, "@credits")) {
						Matched("@create");
						do_create(player, arg1, arg2);
					} else {
						do_credits(player);
					}
					break;
				default:
					goto bad;
				}
				break;
			case 'd':
			case 'D':
				/* @dbginfo, @delta, @describe, @dig, @dlt,
				   @doing, @drop, @dump */
				switch (command[2]) {
				case 'b':
				case 'B':
					Matched("@dbginfo");
					do_serverdebug(descr, player, arg1, arg2);
					break;
				case 'e':
				case 'E':
					if(command[3] == 'l' || command[3] == 'L') {
						Matched("@delta");
						do_delta(player);
					} else {
						Matched("@describe");
						do_describe(descr, player, arg1, arg2);
					}
					break;
				case 'i':
				case 'I':
					Matched("@dig");
					do_dig(descr, player, arg1, arg2);
					break;
				case 'l':
				case 'L':
					Matched("@dlt");
					do_delta(player);
					break;
#if WHO_DOING
				case 'o':
				case 'O':
					Matched("@doing");
					do_doing(descr, player, arg1, arg2);
					break;
#endif
				case 'r':
				case 'R':
					Matched("@drop");
					do_drop_message(descr, player, arg1, arg2);
					break;
				case 'u':
				case 'U':
					Matched("@dump");
					do_dump(player, full_command);
					break;
				default:
					goto bad;
				}
				break;
			case 'e':
			case 'E':
				/* @edit, @entrances, @examine */
				switch (command[2]) {
				case 'd':
				case 'D':
					Matched("@edit");
					do_edit(descr, player, arg1);
					break;
				case 'n':
				case 'N':
					Matched("@entrances");
					do_entrances(descr, player, arg1, arg2);
					break;
				case 'x':
				case 'X':
					Matched("@examine");
					sane_dump_object(player, arg1);
					break;
				default:
					goto bad;
				}
				break;
			case 'f':
			case 'F':
				/* @fail, @find, @flock, @force, @force_lock */
				switch (command[2]) {
				case 'a':
				case 'A':
					Matched("@fail");
					do_fail(descr, player, arg1, arg2);
					break;
				case 'i':
				case 'I':
					Matched("@find");
					do_find(player, arg1, arg2);
					break;
				case 'l':
				case 'L':
					Matched("@flock");
					do_flock(descr, player, arg1, arg2);
					break;
				case 'o':
				case 'O':
					if(strlen(command) < 7) {
						Matched("@force");
						do_force(descr, player, arg1, arg2);
					} else {
						Matched("@force_lock");
						do_flock(descr, player, arg1, arg2);
					}
					break;
				default:
					goto bad;
				}
				break;
			case 'h':
				/* @heal */
				Matched("@heal");
				do_heal(descr, player, arg1);
				break;
			case 'i':
			case 'I':
				/* @idescribe */
				Matched("@idescribe");
				do_idescribe(descr, player, arg1, arg2);
				break;
			case 'k':
			case 'K':
				/* @kill */
				Matched("@kill");
				do_dequeue(descr, player, arg1);
				break;
			case 'l':
			case 'L':
				/* @link, @list, @lock */
				switch (command[2]) {
				case 'i':
				case 'I':
					switch (command[3]) {
					case 'n':
					case 'N':
						Matched("@link");
						do_link(descr, player, arg1, arg2);
						break;
					case 's':
					case 'S':
						Matched("@list");
						match_and_list(descr, player, arg1, arg2);
						break;
					default:
						goto bad;
					}
					break;
				case 'o':
				case 'O':
					Matched("@lock");
					do_lock(descr, player, arg1, arg2);
					break;
				default:
					goto bad;
				}
				break;
			case 'm':
			case 'M':
				/* @mcpedit, @mcpprogram, @memory, @mpitops,
				   @muftops */
				switch (command[2]) {
				case 'c':
				case 'C':
					if (string_prefix("@mcpedit", command)) {
						Matched("@mcpedit");
						do_mcpedit(descr, player, arg1);
						break;
					} else {
						Matched("@mcpprogram");
						do_mcpprogram(descr, player, arg1);
						break;
					}
				case 'e':
				case 'E':
					Matched("@memory");
					do_memory(player);
					break;
				case 'p':
			    case 'P':
			        Matched("@mpitops");
			        do_mpi_topprofs(player, arg1);
			        break;
			    case 'u':
			    case 'U':
			        Matched("@muftops");
			        do_muf_topprofs(player, arg1);
			        break;
				default:
					goto bad;
				}
				break;
			case 'n':
			case 'N':
				/* @name, @newpassword */
				switch (command[2]) {
				case 'a':
				case 'A':
					Matched("@name");
					do_name(descr, player, arg1, arg2);
					break;
				case 'e':
				case 'E':
					if (strcmp(command, "@newpassword"))
						goto bad;
					do_newpassword(player, arg1, arg2);
					break;
				default:
					goto bad;
				}
				break;
			case 'o':
			case 'O':
				/* @odrop, @oecho, @ofail, @open, @osuccess,
				   @owned */
				switch (command[2]) {
				case 'd':
				case 'D':
					Matched("@odrop");
					do_odrop(descr, player, arg1, arg2);
					break;
				case 'e':
				case 'E':
					Matched("@oecho");
					do_oecho(descr, player, arg1, arg2);
					break;
				case 'f':
				case 'F':
					Matched("@ofail");
					do_ofail(descr, player, arg1, arg2);
					break;
				case 'p':
				case 'P':
					Matched("@open");
					do_open(descr, player, arg1, arg2);
					break;
				case 's':
				case 'S':
					Matched("@osuccess");
					do_osuccess(descr, player, arg1, arg2);
					break;
				case 'w':
				case 'W':
					Matched("@owned");
					do_owned(player, arg1, arg2);
					break;
				default:
					goto bad;
				}
				break;
			case 'p':
			case 'P':
				/* @password, @pcreate, @pecho, @program, 
				   @propset, @ps */
				switch (command[2]) {
				case 'a':
				case 'A':
					Matched("@password");
					do_password(player, arg1, arg2);
					break;
				case 'c':
				case 'C':
					Matched("@pcreate");
					do_pcreate(player, arg1, arg2);
					break;
				case 'e':
				case 'E':
					Matched("@pecho");
					do_pecho(descr, player, arg1, arg2);
					break;
				case 'r':
				case 'R':
					if (string_prefix("@program", command)) {
						Matched("@program");
						do_prog(descr, player, arg1);
						break;
					} else {
						Matched("@propset");
						do_propset(descr, player, arg1, arg2);
						break;
					}
				case 's':
				case 'S':
					Matched("@ps");
					list_events(player);
					break;
				default:
					goto bad;
				}
				break;
			case 'r':
			case 'R':
				/* @recycle, @relink, @restrict */
				switch (command[3]) {
				case 'c':
				case 'C':
					Matched("@recycle");
					do_recycle(descr, player, arg1);
					break;
				case 'l':
				case 'L':
					Matched("@relink");
					do_relink(descr, player, arg1, arg2);
					break;
				case 's':
				case 'S':
					Matched("@restrict");
					do_restrict(player, arg1);
					break;
				default:
					goto bad;
				}
				break;
			case 's':
			case 'S':
				/* @sanity, @sanchange, @sanfix, @set, @showextver,
				   @shutdown, @stats, @success, @sweep */
				switch (command[2]) {
				case 'a':
				case 'A':
					if (!strcmp(command, "@sanity")) {
						sanity(player);
					} else if (!strcmp(command, "@sanchange")) {
						sanechange(player, full_command);
					} else if (!strcmp(command, "@sanfix")) {
						sanfix(player);
					} else {
						goto bad;
					}
					break;
				case 'e':
				case 'E':
					Matched("@set");
					do_set(descr, player, arg1, arg2);
					break;
				case 'h':
				case 'H':
					if (!strcmp(command, "@showextver")) {
						do_showextver(player);
						break;
					} else if (strcmp(command, "@shutdown"))
						goto bad;
					do_shutdown(player);
					break;
				case 't':
				case 'T':
					Matched("@stats");
					do_stats(player, arg1);
					break;
				case 'u':
				case 'U':
					Matched("@success");
					do_success(descr, player, arg1, arg2);
					break;
				case 'w':
				case 'W':
					Matched("@sweep");
					do_sweep(descr, player, arg1);
					break;
				default:
					goto bad;
				}
				break;
			case 't':
			case 'T':
				/* @teleport, @toad, @trace, @tune */
				switch (command[2]) {
				case 'e':
				case 'E':
					Matched("@teleport");
					do_teleport(descr, player, arg1, arg2);
					break;
				case 'o':
				case 'O':
					if (!strcmp(command, "@toad")) {
						do_toad(descr, player, arg1, arg2);
					} else if (!strcmp(command, "@tops")) {
						do_all_topprofs(player, arg1);
					} else {
						goto bad;
					}
					break;
				case 'r':
				case 'R':
					Matched("@trace");
					do_trace(descr, player, arg1, atoi(arg2));
					break;
				default:
					goto bad;
				}
				break;
			case 'u':
			case 'U':
				/* @unbless, @unlink,
				 * @unlock, @uncompile, @usage */
				switch (command[2]) {
				case 'N':
				case 'n':
					if (string_prefix(command, "@unb")) {
						Matched("@unbless");
						do_unbless(descr, player, arg1, arg2);
					} else if (string_prefix(command, "@unli")) {
						Matched("@unlink");
						do_unlink(descr, player, arg1);
					} else if (string_prefix(command, "@unlo")) {
						Matched("@unlock");
						do_unlock(descr, player, arg1);
					} else if (string_prefix(command, "@uncom")) {
						Matched("@uncompile");
						do_uncompile(player);
					} else {
						goto bad;
					}
					break;

				case 'S':
				case 's':
					Matched("@usage");
					do_usage(player);
					break;

				default:
					goto bad;
					break;
				}
				break;
			case 'v':
			case 'V':
				/* @version */
				Matched("@version");
				do_version(player);
				break;
			case 'w':
			case 'W':
				/* @wall */
				if (strcmp(command, "@wall"))
					goto bad;
				do_wall(player, full_command);
				break;
			default:
				goto bad;
			}
			break;
		case 'b':
			/* buy */
			Matched("buy");
			do_buy(descr, player, arg1, arg2);
			break;
		case 'd':
		case 'D':
			/* disembark, drink, drop */
			switch (command[1]) {
			case 'i':
			case 'I':
				Matched("disembark");
				do_leave(descr, player);
				break;
			case 'r':
			case 'R':
				switch (command[2]) {
				case 'i':
				case 'I':
					Matched("drink");
					do_drink(descr, player, arg1);
					break;
				case 'o':
				case 'O':
					Matched("drop");
					do_drop(descr, player, arg1, arg2);
					break;
				default:
					goto bad;
				}
				break;
			default:
				goto bad;
			}
			break;
		case 'e':
		case 'E':
			/* equip, examine */
			switch (command[1]) {
			case 'a':
				Matched("eat");
				do_eat(descr, player, arg1);
				break;
			case 'x':
				Matched("examine");
				do_examine(descr, player, arg1, arg2);
				break;
			default:
				Matched("equip");
				do_equip(descr, player, arg1);
			}
			break;
		case 'f':
		case 'F':
			/* fill */
			Matched("fill");
			do_fill(descr, player, arg1, arg2);
			break;
		case 'g':
		case 'G':
			/* get, give, goto, gripe */
			switch (command[1]) {
			case 'e':
			case 'E':
				Matched("get");
				do_get(descr, player, arg1, arg2);
				break;
			case 'i':
			case 'I':
				Matched("give");
				do_give(descr, player, arg1, atoi(arg2));
				break;
			case 'o':
			case 'O':
				Matched("goto");
				do_move(descr, player, arg1, 0);
				break;
			case 'r':
			case 'R':
				if (string_compare(command, "gripe"))
					goto bad;
				do_gripe(player, full_command);
				break;
			default:
				goto bad;
			}
			break;
		case 'h':
		case 'H':
			/* help */
			Matched("help");
			do_help(player, arg1, arg2);
			break;
		case 'i':
		case 'I':
			/* inventory, info */
			if (string_compare(command, "info")) {
				Matched("inventory");
				do_inventory(player);
			} else {
				Matched("info");
				do_info(player, arg1, arg2);
			}
			break;
		case 'k':
		case 'K':
			/* kill */
			Matched("kill");
			do_kill(descr, player, arg1);
			break;
		case 'l':
		case 'L':
			/* leave, look */
			if (string_prefix("look", command)) {
				Matched("look");
				do_look_at(descr, player, arg1, arg2);
				break;
			} else {
				Matched("leave");
				do_leave(descr, player);
				break;
			}
		case 'm':
		case 'M':
			/* man, motd, move, mpi */
			if (string_prefix(command, "move")) {
				do_move(descr, player, arg1, 0);
				break;
			} else if (!string_compare(command, "motd")) {
				do_motd(player, full_command);
				break;
			} else if (!string_compare(command, "mpi")) {
				do_mpihelp(player, arg1, arg2);
				break;
			} else if (!string_compare(command, "map")) {
				do_view(descr, player);
				break;
			} else if (!string_compare(command, "meme")) {
                                Matched("meme");
                                do_meme(descr, player, arg1);
			} else {
				if (string_compare(command, "man"))
					goto bad;
				do_man(player, (!*arg1 && !*arg2 && arg1 != arg2) ? "=" : arg1, arg2);
			}
			break;
		case 'n':
		case 'N':
			/* news */
			Matched("news");
			do_news(player, arg1, arg2);
			break;
		case 'p':
		case 'P':
			/* page, pose, put */
			switch (command[1]) {
			case 'a':
			case 'A':
				Matched("page");
				do_page(player, arg1, arg2);
				break;
			case 'o':
			case 'O':
				Matched("pose");
				do_pose(player, full_command);
				break;
			case 'u':
			case 'U':
				Matched("put");
				do_drop(descr, player, arg1, arg2);
				break;
			default:
				goto bad;
			}
			break;
		case 'r':
		case 'R':
			/* read, rob */
			switch (command[1]) {
			case 'e':
			case 'E':
				Matched("read");	/* undocumented alias for look */
				do_look_at(descr, player, arg1, arg2);
				break;
			case 'o':
			case 'O':
				Matched("rob");
				do_rob(descr, player, arg1);
				break;
			default:
				goto bad;
			}
			break;
		case 's':
		case 'S':
			/* say, score, sell, shop, sit, stand, status */
			switch (command[1]) {
			case 'a':
			case 'A':
				Matched("say");
				do_say(player, full_command);
				break;
			case 'c':
			case 'C':
				Matched("score");
				do_score(player);
				break;
			case 'e':
			case 'E':
				switch (command[3]) {
				case 'l':
					Matched("sell");
					do_sell(descr, player, arg1, arg2);
					break;
				case 'e':
					Matched("select");
					do_select(player, arg1);
					break;
				default:
					goto bad;
				}
				break;
			case 'h':
			case 'H':
				Matched("shop");
				do_shop(player);
				break;
			case 'i':
			case 'I':
				Matched("sit");
				do_sit(descr, player, arg1);
				break;
			case 't':
			case 'T':
				switch (command[3]) {
				case 'n':
					Matched("stand");
					do_stand(player);
					break;
				case 't':
					Matched("status");
					do_status(player);
					break;
				default: goto bad;
				}
				break;
			default:
				goto bad;
			}
			break;
		case 't':
		case 'T':
			/* take, throw */
			switch (command[1]) {
			case 'a':
			case 'A':
				Matched("take");
				do_get(descr, player, arg1, arg2);
				break;
			case 'h':
			case 'H':
				Matched("throw");
				do_drop(descr, player, arg1, arg2);
				break;
			case 'r':
			case 'R':
				Matched("train");
				do_train(player, arg1, arg2);
				break;
			default:
				goto bad;
			}
			break;
		case 'u':
		case 'U':
			/* unequip */
			Matched("unequip");
			do_unequip(descr, player, arg1);
			break;
		case 'w':
		case 'W':
			/* whisper */
			Matched("whisper");
			do_whisper(descr, player, arg1, arg2);
			break;
		default:
		  bad:
#if M3_HUH
			{
				char hbuf[BUFFER_LEN];
				snprintf(hbuf,BUFFER_LEN,"HUH? %s", command);
				if(can_move(descr, player, hbuf, 3)) {
					do_move(descr, player, hbuf, 3);
					*match_args = 0;
					*match_cmdname = 0;
					break;
				}
			}	
#endif
			notify(player, HUH_MESSAGE);
#if LOG_FAILED_COMMANDS
			if (!controls(player, DBFETCH(player)->location)) {
				log_status("HUH from %s(%d) in %s(%d)[%s]: %s %s",
						   NAME(player), player, NAME(DBFETCH(player)->location),
						   DBFETCH(player)->location,
						   NAME(OWNER(DBFETCH(player)->location)), command, full_command);
			}
#endif
			matched = 1;
			break;
		}

	}

end_of_command:
	/* calculate time command took. */
	gettimeofday(&endtime, NULL);
	if (starttime.tv_usec > endtime.tv_usec) {
		endtime.tv_usec += 1000000;
		endtime.tv_sec -= 1;
	}
	endtime.tv_usec -= starttime.tv_usec;
	endtime.tv_sec -= starttime.tv_sec;

	totaltime = endtime.tv_sec + (endtime.tv_usec * 1.0e-6);
	if (totaltime > (CMD_LOG_THRESHOLD_MSEC / 1000.0)) {
		log2file(LOG_CMD_TIMES, "%6.3fs, %.16s: %s%s%s%s(%d) in %s(%d):%s %s",
					totaltime, ctime((time_t *)&starttime.tv_sec),
					Wizard(OWNER(player)) ? "WIZ: " : "",
					(Typeof(player) != TYPE_PLAYER) ? NAME(player) : "",
					(Typeof(player) != TYPE_PLAYER) ? " owned by " : "",
					NAME(OWNER(player)), (int) player,
					NAME(DBFETCH(player)->location),
					(int) DBFETCH(player)->location, " ", command);
	}
out:
	{
		pos_t pos2;
		map_where(pos2, getloc(player));
		if (MORTON_READ(pos2) != MORTON_READ(pos))
			do_view(descr, player);
	}
}

#undef Matched
static const char *game_c_version = "$RCSfile$ $Revision: 1.50 $";
const char *get_game_c_version(void) { return game_c_version; }
