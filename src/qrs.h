#ifndef _qrs_h
#define _qrs_h

#include <stdint.h>
#include <stdbool.h>
#include "core.h"
#include "piecedef.h"
#include "grid.h"
#include "timer.h"

#define PSINACTIVE 		0x0000

#define PSARE 			0x0001
#define PSLINEARE 		0x0002
#define PSFALL 			0x0004
#define PSDASLEFT		0x0008
#define PSDASRIGHT		0x0010
#define PSLOCK	 		0x0020
#define PSLINECLEAR		0x0040
#define PSFKICKED		0x0080
#define PSLOCKED		0x0100
#define PSPRELOCKED		0x0200
#define PSLOCKFLASH1	0x0400
#define PSLOCKFLASH2	0x0800
#define PSLOCKPROTECT	0x1000
#define PSLOCKFLASH3	0x2000
#define PSLOCKFLASH4	0x4000
#define PSPRELOCKFLASH1 0x8000
#define PSSPAWN			0x10000
#define PSLOCKPRESSED	0x20000
#define PSUSEDHOLD		0x40000
#define PSHOLDFLASH		0x80000

#define MOVE_LEFT		-1
#define MOVE_RIGHT		1

#define QRS_FIELD_EDIT  2

#define FIELD_EDITOR_PALETTE_X 6
#define FIELD_EDITOR_PALETTE_Y 96

// IJLXSZNGUTFaFbPQWYaYbV I4T4J4L4OS4Z4 - felicity's arbitrary pentomino+tetromino ordering
#define QRS_I 0
#define QRS_J 1
#define QRS_L 2
#define QRS_X 3
#define QRS_S 4
#define QRS_Z 5
#define QRS_N 6
#define QRS_G 7
#define QRS_U 8
#define QRS_T 9
#define QRS_Fa 10
#define QRS_Fb 11
#define QRS_P 12
#define QRS_Q 13
#define QRS_W 14
#define QRS_Ya 15
#define QRS_Yb 16
#define QRS_V 17
#define QRS_I4 18
#define QRS_T4 19
#define QRS_J4 20
#define QRS_L4 21
#define QRS_O 22
#define QRS_S4 23
#define QRS_Z4 24

// ITJLOSZ - felicity's arbitrary tetromino ordering
#define QRS_ARS_I 0
#define QRS_ARS_T 1
#define QRS_ARS_J 2
#define QRS_ARS_L 3
#define QRS_ARS_O 4
#define QRS_ARS_S 5
#define QRS_ARS_Z 6

// IZSJLOT - arkia's arbitrary tetromino ordering
#define ARS_I 0
#define ARS_Z 1
#define ARS_S 2
#define ARS_J 3
#define ARS_L 4
#define ARS_O 5
#define ARS_T 6

#define PIECE_ID_INVALID 255

/* */

#define SUBSET_ALL 0
#define SUBSET_TETS 1
#define SUBSET_PENTS 2

#define QRS_PIECE_GARBAGE -5
#define QRS_PIECE_RAINBOW 512
#define QRS_PIECE_BRACKETS 1024
#define QRS_FIELD_W_LIMITER -30
#define QRS_WALL QRS_FIELD_W_LIMITER

#define SEQUENCE_REPEAT_START 2048
#define SEQUENCE_REPEAT_END	4096
#define SEQUENCE_REPEAT_INF 8192

#define QRS_FIELD_W 12
#define QRS_FIELD_H 22

#define GAMESTATE_INVISIBLE 		0x0001
#define GAMESTATE_BRACKETS 			0x0002
#define GAMESTATE_RISING_GARBAGE 	0x0004
#define GAMESTATE_CREDITS			0x0008

#define GARBAGE_COPY_BOTTOM_ROW		0x0001

#define LOCKPIECE_BRACKETS 0x0001
#define LOCKPIECE_RAINBOW 0x0002

#define SPAWNX_QRS 5
#define SPAWNY_QRS 1

#define ANCHORX_QRS		1
#define ANCHORY_QRS		1
#define SPAWNX_ARS		4
#define SPAWNY_ARS		2

#define TAP_ARS 0
#define TI_ARS 1

#define BRONZE 1
#define SILVER 2
#define GOLD 3
#define PLATINUM 4

#define YTOROW(n)   (n/256)
#define ROWTOY(n)   (n*256)
#define IS_STACK(n)	((!n || n == -2 || n == GRID_OOB || n == QRS_FIELD_W_LIMITER) ? 0 : 1)
#define IS_INBOUNDS(n) (IS_STACK(n) || !n || n == -2)
#define IS_QRS_PIECE(n) (n >= 0 && n < 25)

extern const char *qrs_piece_names[25];

typedef uint8_t piece_id;

typedef struct {
	int level;

	int grav;
	int lock;
	int das;
	int are;
	int lineare;
	int lineclear;
} qrs_timings;

typedef struct {
	int init;

	int lock;
	int are;
	int lineare;
	int lineclear;
	unsigned int floorkicks;
	int hold_flash;
} qrs_counters;

typedef struct {
	piecedef *def;
	qrs_timings *speeds;
	unsigned int state;		// rename?

	int x;
	int y;
	int orient;
} qrs_player;

struct pracdata {
	int game_type;	// mirrors of values in qrsdata; these are just here so that..
	int field_w;	// ..backed up pracdata structs can be used to restore their values

	//int *long_history;		// old idea, just history of all pieces placed,
								// going to re-add long_history but with placement locations included,
								// as well as game time/level, to allow piece-by-piece rewinds

	int usr_sequence[2000];
	int usr_seq_expand[4000];
	int usr_seq_len;
	int usr_seq_expand_len;

	grid_t **usr_field_undo;
	grid_t **usr_field_redo;
	int usr_field_undo_len;
	int usr_field_redo_len;
	bool field_edit_in_progress;

	grid_t *usr_field;
	int palette_selection;
	int field_selection;
	int field_selection_vertex1_x;
	int field_selection_vertex1_y;
	int field_selection_vertex2_x;
	int field_selection_vertex2_y;

	qrs_timings *usr_timings;

	int paused;
	bool grid_lines_shown;
	bool brackets;
	bool invisible;

	int hist_index;
	int lock_protect;
	bool infinite_floorkicks;
	int piece_subset;

	long randomizer_seed;
};

typedef struct {
    piecedef **piecepool;
	struct randomizer *randomizer;
	struct pracdata *pracdata;
	struct replay *replay;
	grid_t *garbage;
	piece_id *piece_seq;

	nz_timer *timer;
	qrs_player *p1;
    qrs_counters *p1counters;
	piecedef *previews[3];
	piecedef *hold;

// fields which are assumed to be read-only during normal gameplay

	unsigned int piece_seq_len;
	bool using_piece_seq;

	bool is_practice;
	bool recording;
	bool playback;

	int game_type;
	int mode_type;
	unsigned int mode_flags;
	int randomizer_type;

	long randomizer_seed;
	bool tetromino_only;
	bool pentomino_only;

	int field_x; // in pixels (position of top-left of field)
	int field_y; // ^^
	int field_w; // in cells (only player-accessible ones)

	unsigned int max_floorkicks;
	int num_previews;

	bool lock_delay_enabled;
	bool lock_protect;
	bool hold_enabled;
	bool special_irs;

// fields which are assumed to be mutated during normal gameplay

	piece_id cur_piece_qrs_id;

	unsigned int state_flags;	// things like invisible mode, bracket mode, etc. go here

	int piece_seq_index;
	int garbage_row_index;	// which row of the garbage grid to spawn next
	int playback_index;		// equivalent to number of frames that input has been handled in the game so far

	// increments for each piece that doesnt clear lines (shirase: for each piece spawned & decrements for each line cleared)
	int garbage_counter;
	// = 13 - q->section for TGM+. shirase: 500-599=20, 600-699=18, 700-799=10, 800-899=9, 900-999=8; checks during ARE
	int garbage_delay;

    int level;
    int section;
	int grade;

	// values: 1 = set to 2 next time a rotate happens.
	//		   2 = lock during THIS frame ( handled by qs_process_lock() )
	int lock_on_rotate;

	bool lock_held;
	int locking_row;
    int lvlinc;
    int lastclear;
	int combo;
	int combo_simple;
	int singles;	//
	int doubles;	//
	int triples;	//
	int tetrises;	//
	int pentrises;
	int recoveries;
	bool is_recovering;

	int medal_re; // medal values range from 0 to BRONZE (1) to PLATINUM (4)
	int medal_sk;
	int medal_st;
	int medal_co;

    int speed_curve_index;
    int music;
    bool mute;
    //int history[10];    // 0, 1, 2, 3, 4, 5 are in the past; 6 is the current piece; 7, 8, 9 are previews
} qrsdata;

const char *get_qrspiece_name(int n);

void qrsdata_destroy(qrsdata *q);
void pracdata_destroy(struct pracdata *d);

piecedef *qrspent_create(int pent);
piecedef **qrspool_create();
piecedef *arstet_create(int tet);
piecedef **arspool_create(piecedef **piecepool, int type);
piecedef *qrspiece_cpy(piecedef **piecepool, int index);

grid_t *qrsfield_create();
int qrsfield_set_w(grid_t *field, int w);
int qrsfield_clear(grid_t *field);

int ufu_not_exists(coreState *cs);

int usr_field_bkp(coreState *cs, struct pracdata *d);
int usr_field_undo(coreState *cs, struct pracdata *d);
int usr_field_redo(coreState *cs, struct pracdata *d);
int push_undo_clear_confirm(coreState *cs, void *data);
int undo_clear_confirm_yes(coreState *cs, void *data);
int undo_clear_confirm_no(coreState *cs, void *data);
int usr_field_undo_clear(coreState *cs, void *data);

int qrs_input(game_t *g);

int qrs_start_record(game_t *g);
int qrs_end_record(game_t *g);

int qrs_load_replay(game_t *g, char *filename);
int qrs_start_playback(game_t *g);
int qrs_end_playback(game_t *g);

int qrs_move(game_t *g, qrs_player *p, int offset);
int qrs_rotate(game_t *g, qrs_player *p, int direction);
int qrs_proc_initials(game_t *g);
int qrs_irs(game_t *g);
int qrs_wallkick(game_t *g, qrs_player *p);
int qrs_floorkick(game_t *g, qrs_player *p);
int qrs_hold(game_t *g, qrs_player *p);
//int qrs_ihs(game_t *g, int playerid);

int qrs_fall(game_t *g, qrs_player *p, int grav);
int qrs_lock(game_t *g, qrs_player *p, unsigned int flags);
int qrs_chkcollision(game_t *g, qrs_player *p);
int qrs_isonground(game_t *g, qrs_player *p);

int qrs_lineclear(game_t *g, qrs_player *p);
int qrs_dropfield(game_t *g);
int qrs_spawn_garbage(game_t *g, unsigned int flags);

#endif
