/*
 * top - a top users display for Unix
 *
 * SYNOPSIS:  Linux 1.2.x, 1.3.x, using the /proc filesystem
 *
 * DESCRIPTION:
 * This is the machine-dependent module for Linux 1.2.x or 1.3.x.
 *
 * LIBS:
 *
 * CFLAGS: -DHAVE_GETOPT
 *
 * AUTHOR: Richard Henderson <rth@tamu.edu>
 */

#include "top.h"
#include "machine.h"
#include "utils.h"

#include <sys/types.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <dirent.h>
#include <string.h>
#include <math.h>
#include <ctype.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/vfs.h>

#include <sys/param.h>		/* for HZ */

#define PROC_SUPER_MAGIC 0x9fa0
#define NR_TASKS 1024;
#define PROCFS "/proc"
extern char *myname;
extern uid_t proc_owner(pid_t pid);

/*=STATE IDENT STRINGS==================================================*/

#define NPROCSTATES 7

static const char *procstatenames[NPROCSTATES+1] =
{
    "", " running, ", " sleeping, ", " uninterruptable, ",
    " zombie, ", " stopped, ", " swapping, ",
    NULL
};

#define NCPUSTATES 4
static const char *cpustatenames[NCPUSTATES+1] =
{
    "user", "nice", "system", "idle",
    NULL
};

#define NMEMSTATS 6
static const char *memorynames[NMEMSTATS+1] =
{
    "K used, ", "K free, ", "K shd, ", "K buf  Swap: ",
    "K used, ", "K free",
    NULL
};

/*=SYSTEM STATE INFO====================================================*/

/* these are for calculating cpu state percentages */
static long cp_time[NCPUSTATES];
static long cp_old[NCPUSTATES];
static long cp_diff[NCPUSTATES];

/* these are for keeping track of processes */
#define HASH_SIZE	(NR_TASKS * 3 / 2)

/* these are for passing data back to the machine independant portion */
static int cpu_states[NCPUSTATES];
static int memory_stats[NMEMSTATS];

#define PAGE_SHIFT	12

/* usefull macros */
#define bytetok(x)	(((x) + 512) >> 10)
#define pagetok(x)	((x) << (PAGE_SHIFT - 10))
#define HASH(x)		(((x) * 1686629713U) % HASH_SIZE)

/*======================================================================*/

static inline char *
skip_ws(const char *p)
{
    while (isspace(*p)) p++;
    return (char *)p;
}

static inline char *
skip_token(const char *p)
{
    while (isspace(*p)) p++;
    while (*p && !isspace(*p)) p++;
    return (char *)p;
}

int machine_init(struct statics *pstatics)
{
    /* make sure the proc filesystem is mounted */
    {
        struct statfs sb;
        if (statfs(PROCFS, &sb) < 0 || sb.f_type != PROC_SUPER_MAGIC)
        {
            fprintf(stderr, " proc filesystem not mounted on " PROCFS "\n");
            return -1;
        }
    }

    /* chdir to the proc filesystem to make things easier */
    chdir(PROCFS);

    /* fill in the statics information */
    pstatics->procstate_names = procstatenames;
    pstatics->cpustate_names = cpustatenames;
    pstatics->memory_names = memorynames;

    /* all done! */
    return 0;
}

void get_system_info(struct system_info *info)
{
    char buffer[4096+1];
    int fd, len;
    char *p;
    int i;

    /* get load averages */
    {
        fd = open("loadavg", O_RDONLY);
        len = read(fd, buffer, sizeof(buffer)-1);
        close(fd);
        buffer[len] = '\0';

        info->load_avg[0] = strtod(buffer, &p);
        info->load_avg[1] = strtod(p, &p);
        info->load_avg[2] = strtod(p, &p);
        p = skip_token(p);			/* skip running/tasks */
        p = skip_ws(p);
        if (*p)
            info->last_pid = atoi(p);
        else
            info->last_pid = -1;
    }

    /* get the cpu time info */
    {
        fd = open("stat", O_RDONLY);
        len = read(fd, buffer, sizeof(buffer)-1);
        close(fd);
        buffer[len] = '\0';

        p = skip_token(buffer);			/* "cpu" */
        cp_time[0] = strtoul(p, &p, 0);

        cp_time[1] = strtoul(p, &p, 0);
        cp_time[2] = strtoul(p, &p, 0);
        cp_time[3] = strtoul(p, &p, 0);

        /* convert cp_time counts to percentages */
        percentages(4, cpu_states, cp_time, cp_old, cp_diff);
    }

    /* get system wide memory usage */
    {
        char *p;

        fd = open("meminfo", O_RDONLY);
        len = read(fd, buffer, sizeof(buffer)-1);
        close(fd);
        buffer[len] = '\0';

        /* be prepared for extra columns to appear be seeking
           to ends of lines */

        p = buffer;
        p = skip_token(p);
        memory_stats[0] = strtoul(p, &p, 10); /* total memory */

        p = strchr(p, '\n');
        p = skip_token(p);
        memory_stats[1] = strtoul(p, &p, 10); /* free memory */


        p = strchr(p, '\n');
        p = skip_token(p);
        memory_stats[2] = strtoul(p, &p, 10); /* buffer memory */

        p = strchr(p, '\n');
        p = skip_token(p);
        memory_stats[3] = strtoul(p, &p, 10); /* cached memory */

        for(i = 0; i< 8 ; i++) {
            p++;
            p = strchr(p, '\n');
        }

        p = skip_token(p);
        memory_stats[4] = strtoul(p, &p, 10); /* total swap */

        p = strchr(p, '\n');
        p = skip_token(p);
        memory_stats[5] = strtoul(p, &p, 10); /* free swap */

    }

    /* set arrays and strings */
    info->cpustates = cpu_states;
    info->memory = memory_stats;
}

/*
 *  proc_compare - comparison function for "qsort"
 *	Compares the resource consumption of two processes using five
 *  	distinct keys.  The keys (in descending order of importance) are:
 *  	percent cpu, cpu ticks, state, resident set size, total virtual
 *  	memory usage.  The process states are ordered as follows (from least
 *  	to most important):  WAIT, zombie, sleep, stop, start, run.  The
 *  	array declaration below maps a process state index into a number
 *  	that reflects this ordering.
 */
int proc_compare (struct top_proc **pp1,struct top_proc ** pp2)
{
    static unsigned char sort_state[] =
    {
        0,	/* empty */
        6, 	/* run */
        3,	/* sleep */
        5,	/* disk wait */
        1,	/* zombie */
        2,	/* stop */
        4	/* swap */
    };

    struct top_proc *p1, *p2;
    int result;
    double dresult;

    /* remove one level of indirection */
    p1 = *pp1;
    p2 = *pp2;

    /* compare percent cpu (pctcpu) */
    dresult = p2->pcpu - p1->pcpu;
    if (dresult != 0.0)
        return dresult > 0.0 ? 1 : -1;

    /* use cputicks to break the tie */
    if ((result = p2->time - p1->time) == 0)
    {
        /* use process state to break the tie */
        if ((result = (sort_state[p2->state] - sort_state[p1->state])) == 0)
        {
            /* use priority to break the tie */
            if ((result = p2->pri - p1->pri) == 0)
            {
                /* use resident set size (rssize) to break the tie */
                if ((result = p2->rss - p1->rss) == 0)
                {
                    /* use total memory to break the tie */
                    result = (p2->size - p1->size);
                }
            }
        }
    }

    return result == 0 ? 0 : result < 0 ? -1 : 1;
}

