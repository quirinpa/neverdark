/* $Header$ */

#ifndef _EXTERNS_H
#define _EXTERNS_H
#include "copyright.h"

/* Definition of 'dbref' */
#include "db.h"
/* Definition of 'McpFrame' */
#include "mcp.h"
/* Definition of PropPtr, among other things */
#include "props.h"
/* Definition of match_data */
#include "match.h"
/* Auto-generated list of extern functions */
#include "externs-auto.h"

/* Prototypes for externs not defined elsewhere */

extern char match_args[];
extern char match_cmdname[];

/* from event.c */
extern long next_muckevent_time(void);
extern void next_muckevent(void);

/* from timequeue.c */
extern void handle_read_event(int descr, dbref player, const char *command);
extern int add_muf_read_event(int descr, dbref player, dbref prog, struct frame *fr);
extern int add_muf_queue_event(int descr, dbref player, dbref loc, dbref trig, dbref prog,
						   const char *argstr, const char *cmdstr, int listen_p);
extern int add_event(int event_type, int subtyp, int dtime, int descr, dbref player,
					 dbref loc, dbref trig, dbref program, struct frame *fr,
					 const char *strdata, const char *strcmd, const char *str3);
extern void next_timequeue_event(void);
extern int in_timequeue(int pid);
extern struct frame* timequeue_pid_frame(int pid);
extern long next_event_time(void);
extern void list_events(dbref program);
extern int dequeue_prog(dbref program, int sleeponly);
extern int dequeue_process(int procnum);
extern int dequeue_timers(int procnum, char* timerid);
extern int control_process(dbref player, int procnum);
extern void do_dequeue(int descr, dbref player, const char *arg1);
extern void propqueue(int descr, dbref player, dbref where, dbref trigger, dbref what,
					  dbref xclude, const char *propname, const char *toparg,

					  int mlev, int mt);
extern void envpropqueue(int descr, dbref player, dbref where, dbref trigger, dbref what,
						 dbref xclude, const char *propname, const char *toparg,

						 int mlev, int mt);
extern int scan_instances(dbref program);

/* from db.c */
extern int number(const char *s);
extern int ifloat(const char *s);
extern void putproperties(FILE * f, int obj);
extern void getproperties(FILE * f, int obj, const char *pdir);
extern void free_line(struct line *l);
extern void db_free_object(dbref i);
extern void db_clear_object(dbref i);
extern void macrodump(struct macrotable *node, FILE * f);
extern void macroload(FILE * f);
extern void free_prog_text(struct line *l);
extern struct line *read_program(dbref i);
extern void write_program(struct line *first, dbref i);
extern char *show_line_prims(dbref program, struct inst *pc, int maxprims, int markpc);
extern dbref db_write_deltas(FILE * f);


/* From create.c */
extern void do_open(int descr, dbref player, const char *direction, const char *linkto);
extern void do_link(int descr, dbref player, const char *name, const char *room_name);
extern void do_dig(int descr, dbref player, const char *name, const char *pname);
extern void do_create(dbref player, char *name, char *cost);
extern void do_prog(int descr, dbref player, const char *name);
extern void do_attach(int descr, dbref player, const char *action_name, const char *source_name);
extern int unset_source(dbref player, dbref loc, dbref action);
extern int link_exit(int descr, dbref player, dbref exit, char *dest_name, dbref * dest_list);
extern void do_action(int descr, dbref player, const char *action_name, const char *source_name);

/* from edit.c */
extern struct macrotable
*new_macro(const char *name, const char *definition, dbref player);
extern char *macro_expansion(struct macrotable *node, const char *match);
extern void match_and_list(int descr, dbref player, const char *name, char *linespec);
extern void do_list(dbref player, dbref program, int oarg[], int argc);

/* From game.c */
extern void do_dump(dbref player, const char *newfile);
extern void do_shutdown(dbref player);
extern void dump_warning(void);
extern void dump_deltas(void);
extern void fork_and_dump(void);

/* From hashtab.c */
extern hash_data *find_hash(const char *s, hash_tab * table, unsigned size);
extern hash_entry *add_hash(const char *name, hash_data data, hash_tab * table, unsigned size);
extern int free_hash(const char *name, hash_tab * table, unsigned size);
extern void kill_hash(hash_tab * table, unsigned size, int freeptrs);

/* From help.c */
extern void spit_file(dbref player, const char *filename);
extern void do_help(dbref player, char *topic, char *seg);
extern void do_mpihelp(dbref player, char *topic, char *seg);
extern void do_news(dbref player, char *topic, char *seg);
extern void do_man(dbref player, char *topic, char *seg);
extern void do_motd(dbref player, char *text);
extern void do_info(dbref player, const char *topic, const char *seg);

/* From look.c */
extern void look_room(int descr, dbref player, dbref room, int verbose);

/* extern void look_room_simple(dbref player, dbref room); */
extern void do_look_around(int descr, dbref player);
extern void do_look_at(int descr, dbref player, const char *name, const char *detail);
extern void do_examine(int descr, dbref player, const char *name, const char *dir);
extern void do_inventory(dbref player);
extern void do_find(dbref player, const char *name, const char *flags);
extern void do_owned(dbref player, const char *name, const char *flags);
extern void do_trace(int descr, dbref player, const char *name, int depth);
extern void do_entrances(int descr, dbref player, const char *name, const char *flags);
extern void do_contents(int descr, dbref player, const char *name, const char *flags);
extern void exec_or_notify(int descr, dbref player, dbref thing,

						   const char *message, const char *whatcalled);

/* From move.c */
extern void moveto(dbref what, dbref where);
extern void enter_room(int descr, dbref player, dbref loc, dbref exit);
extern void send_home(int descr, dbref thing, int homepuppet);
extern int parent_loop_check(dbref source, dbref dest);
extern int can_move(int descr, dbref player, const char *direction, int lev);
extern void do_move(int descr, dbref player, const char *direction, int lev);
extern void do_get(int descr, dbref player, const char *what, const char *obj);
extern void do_drop(int descr, dbref player, const char *name, const char *obj);
extern void do_recycle(int descr, dbref player, const char *name);
extern void recycle(int descr, dbref player, dbref thing);

/* From player.c */
extern dbref lookup_player(const char *name);
extern void do_password(dbref player, const char *old, const char *newobj);
extern void add_player(dbref who);
extern void delete_player(dbref who);
extern void clear_players(void);

/* From predicates.c */
extern int can_link_to(dbref who, object_flag_type what_type, dbref where);
extern int can_link(dbref who, dbref what);
extern int could_doit(int descr, dbref player, dbref thing);
extern int can_doit(int descr, dbref player, dbref thing, const char *default_fail_msg);
extern int can_see(dbref player, dbref thing, int can_see_location);
extern int controls(dbref who, dbref what);
extern int controls_link(dbref who, dbref what);
extern int restricted(dbref player, dbref thing, object_flag_type flag);
extern int payfor(dbref who, int cost);
extern int ok_name(const char *name);
extern int isancestor(dbref parent, dbref child);

/* From rob.c */
extern void do_kill(int descr, dbref player, const char *what, int cost);
extern void do_give(int descr, dbref player, const char *recipient, int amount);
extern void do_rob(int descr, dbref player, const char *what);

/* From set.c */
extern void do_name(int descr, dbref player, const char *name, char *newname);
extern void do_describe(int descr, dbref player, const char *name, const char *description);
extern void do_idescribe(int descr, dbref player, const char *name, const char *description);
extern void do_fail(int descr, dbref player, const char *name, const char *message);
extern void do_success(int descr, dbref player, const char *name, const char *message);
extern void do_drop_message(int descr, dbref player, const char *name, const char *message);
extern void do_osuccess(int descr, dbref player, const char *name, const char *message);
extern void do_ofail(int descr, dbref player, const char *name, const char *message);
extern void do_odrop(int descr, dbref player, const char *name, const char *message);
extern int setlockstr(int descr, dbref player, dbref thing, const char *keyname);
extern void do_lock(int descr, dbref player, const char *name, const char *keyname);
extern void do_unlock(int descr, dbref player, const char *name);
extern void do_unlink(int descr, dbref player, const char *name);
extern void do_chown(int descr, dbref player, const char *name, const char *newobj);
extern void do_set(int descr, dbref player, const char *name, const char *flag);
extern void do_chlock(int descr, dbref player, const char *name, const char *keyname);
extern void do_conlock(int descr, dbref player, const char *name, const char *keyname);

/* From speech.c */
extern void do_wall(dbref player, const char *message);
extern void do_gripe(dbref player, const char *message);
extern void do_say(dbref player, const char *message);
extern void do_page(dbref player, const char *arg1, const char *arg2);
extern void notify_listeners(dbref who, dbref xprog, dbref obj, dbref room, const char *msg,

							 int isprivate);
extern void notify_except(dbref first, dbref exception, const char *msg, dbref who);

/* From stringutil.c */
extern int alphanum_compare(const char *s1, const char *s2);
extern int string_compare(const char *s1, const char *s2);
extern int string_prefix(const char *string, const char *prefix);
extern const char *string_match(const char *src, const char *sub);
extern char *pronoun_substitute(int descr, dbref player, const char *str);
extern char *intostr(int i);

#if !defined(MALLOC_PROFILING)
extern char *string_dup(const char *s);
#endif


/* From utils.c */
extern int member(dbref thing, dbref list);
extern dbref remove_first(dbref first, dbref what);

/* From wiz.c */
extern void do_teleport(int descr, dbref player, const char *arg1, const char *arg2);
extern void do_force(int descr, dbref player, const char *what, char *command);
extern void do_stats(dbref player, const char *name);
extern void do_toad(int descr, dbref player, const char *name, const char *recip);
extern void do_boot(dbref player, const char *name);
extern void do_pcreate(dbref player, const char *arg1, const char *arg2);
extern void do_usage(dbref player);
extern void do_serverdebug(int descr, dbref player, const char *arg1, const char *arg2);
extern void do_muf_topprofs(dbref player, char *arg1);
extern void do_mpi_topprofs(dbref player, char *arg1);
extern void do_all_topprofs(dbref player, char *arg1);

extern time_t mpi_prof_start_time;
extern time_t sel_prof_start_time;
extern long sel_prof_idle_sec;
extern long sel_prof_idle_usec;
extern unsigned long sel_prof_idle_use;



/* From boolexp.c */
extern int eval_boolexp(int descr, dbref player, struct boolexp *b, dbref thing);
extern struct boolexp *parse_boolexp(int descr, dbref player, const char *string, int dbloadp);
extern struct boolexp *copy_bool(struct boolexp *old);
extern struct boolexp *getboolexp(FILE * f);
extern struct boolexp *negate_boolexp(struct boolexp *b);
extern void free_boolexp(struct boolexp *b);

/* From unparse.c */
extern const char *unparse_object(dbref player, dbref object);
extern const char *unparse_boolexp(dbref player, struct boolexp *b, int fullname);

/* From compress.c */
#ifdef COMPRESS
extern void save_compress_words_to_file(FILE * f);
extern void init_compress_from_file(FILE * dicto);
extern const char *compress(const char *);
extern const char *uncompress(const char *);
extern void init_compress(void);
#endif							/* COMPRESS */

/* From edit.c */
extern void interactive(int descr, dbref player, const char *command);

/* From compile.c */
extern void uncompile_program(dbref i);
extern void do_uncompile(dbref player);
extern void free_unused_programs(void);
extern int get_primitive(const char *);
extern void do_compile(int descr, dbref in_player, dbref in_program, int force_err_disp);
extern void clear_primitives(void);
extern void init_primitives(void);

/* From interp.c */
extern struct inst *interp_loop(dbref player, dbref program, struct frame *fr, int rettyp);
extern struct frame *interp(int descr, dbref player, dbref location, dbref program,
							dbref source, int nosleeping, int whichperms);
extern void purge_for_pool(void);

/* declared in log.c */
extern void log2file(char *myfilename, char *format, ...);
extern void log_error(char *format, ...);
extern void log_gripe(char *format, ...);
extern void log_muf(char *format, ...);
extern void log_conc(char *format, ...);
extern void log_status(char *format, ...);
extern void log_other(char *format, ...);
extern void notify_fmt(dbref player, char *format, ...);
extern void log_program_text(struct line *first, dbref player, dbref i);
extern void log_command(char *format, ...);

/* From timestamp.c */
extern void ts_newobject(struct object *thing);
extern void ts_useobject(dbref thing);
extern void ts_lastuseobject(dbref thing);
extern void ts_modifyobject(dbref thing);

/* From smatch.c */
extern int equalstr(char *s, char *t);

extern void CrT_summarize(dbref player);

extern int force_level;

extern void do_credits(dbref player);

extern void disassemble(dbref player, dbref program);

/* from random.c */
void *init_seed(char *seed);
void delete_seed(void *buffer);
unsigned long rnd(void *buffer);

/* from mcppkgs.c */
extern void show_mcp_error(McpFrame * mfr, char *topic, char *text);

/* from diskprop.c */
extern void dispose_all_oldprops(void);

/* from rwho.c */
extern int rwhocli_shutdown(void);
extern int rwhocli_setup(const char *server, const char *serverpw, const char *myname, const char *comment);

/* from interface.c */
extern void update_rwho(void);
extern void do_armageddon(dbref player, const char *msg);

/* from events.c */
extern void dump_db_now(void);
extern void delta_dump_now(void);

/* from mesgparse.c */
extern void mesg_init(void);

/* from tune.c */
extern void tune_load_parmsfile(dbref player);

void dump_status(void);
void log_status(char *format, ...);
void kill_resolver(void);

#endif /* _EXTERNS_H */
