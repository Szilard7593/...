#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <ctype.h>
#include <time.h>
#include <local.h>
#include <signal.h>

#define MAX_PROCESSES 512
#define MAX_PROC_NAME 256
#define REFRESH_INTERVAL 1

enum {
	CP_HEADER = 1,
	CP_CPU,
	CP_MEM,
	CP_SWAP,
	CP_TEXT,
	CP_WARNING,
	CP_CRITICAL,
	CP_BORDER,
	CP_PROCESS
};

typedef struct {
	int pid;
	char name[MAX_PROC_NAME];
	float cpu_percent;
	float mem_percent;
	char state;
} Process;	

typedef struct {
	float cpu_percent;
	unsigned long mem_total;
	unsigned long mem_free;
	unsigned long mem_available;
	unsigned long swap_total;
	unsigned long swap_free;
	float load_avg[3];
	long uptime_seconds;
	Process processes[MAX_PROCESSES];
	int process_count;
} SystemStats;

static unsigned long prev_idle = 0;
static unsigned long prev_total = 0;

typedef struct {
	int pid;
	unsigned long total_time;
} ProcCpuSample;

static ProcCpuSample prev_sample[MAX_PROCESSES];
static int pre_sample_count = 0;
static struct timespec prev_sample_ts;
static int have_prev_sample_ts = 0;

static int find_prev_time(int pid, unsigned long* out_time) {
	for (int i = 0
