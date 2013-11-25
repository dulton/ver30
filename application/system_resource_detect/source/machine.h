#include <unistd.h>

/*
 *  This file defines the interface between top and the machine-dependent
 *  module.  It is NOT machine dependent and should not need to be changed
 *  for any specific machine.
 */

/*
 * the statics struct is filled in by machine_init
 */
struct statics
{
    const char **procstate_names;
    const char **cpustate_names;
    const char **memory_names;
#ifdef ORDER
    const char **order_names;
#endif
};

/*
 * the system_info struct is filled in by a machine dependent routine.
 */


struct system_info
{
    int    last_pid;
    double load_avg[3];
    int    p_total;
    int    p_active;     /* number of procs considered "active" */
    int    *procstates;
    int    *cpustates;
    int    *memory;
};

/* cpu_states is an array of percentages * 10.  For example,
   the (integer) value 105 is 10.5% (or .105).
 */

/*
 * the process_select struct tells get_process_info what processes we
 * are interested in seeing
 */

struct process_select
{
    int idle;		/* show idle processes */
    int system;		/* show system processes */
    int uid;		/* only this uid (unless uid == -1) */
    char *command;	/* only this command (unless == NULL) */
};

/* routines defined by the machine dependent module */

char *format_header();
char *format_next_process();

/* non-int routines typically used by the machine dependent module */
char *printable();


/*=PROCESS INFORMATION==================================================*/

struct top_proc
{
    pid_t pid;
    uid_t uid;
    char name[64];
    int pri, nice;
    unsigned long size, rss;	/* in k */
    int state;
    unsigned long time;
    double pcpu, wcpu;
};

