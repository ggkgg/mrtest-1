#include "mr_common.h"
#include "mr_test.h"
#include "mr_timer.h"

tsche_t mt_tsch = {
	0,
	0,
	{
		{"mac test: start slot", 0, 1000,  mt_mac_sslot}
	}
};

const int mt_tfs = sizeof(mt_tsch.procs)/sizeof(mt_tsch.procs[0]);

struct itimerval new_value = {
{MT_TDELAY_S, MT_TDELAY_US},
{MT_TINTVL_S, MT_TINTVL_US}
};

static int start_slot = 1;

extern int rqs[MAX_NODE_CNT];
extern mtop_t top[MAX_NODE_CNT][MAX_NODE_CNT];
extern fmd_t fmd[MAX_NODE_CNT];

void mt_tsch_init()
{
	int i, cnt, sopcnt = 0;;
	tproc_t *p;

	ASSERT(mt_tsch.tmap == 0);
	ASSERT(mt_tsch.tmask == 0);

	for (i = 0; i < mt_tfs; i++)
	{
		p = &mt_tsch.procs[i];
		if (p->period == 0)
		{
			ASSERT(mt_tsch.procs[i].pf == NULL);
			continue;
		}

		mt_tsch.tmap = mt_tsch.tmap|(1<<i);
		cnt = (p->period * 1000)/MT_TINTVL_US;
		if (cnt < 1)
			p->period = 1;
		else
			p->period = cnt;

		p->wait = p->period;
	}
}


int mt_mac_timer()
{
	int rval;

	mt_tsch_init();

	signal(SIGALRM, mt_timer_sche);
	rval = setitimer(ITIMER_REAL, &new_value, NULL);
	if (-1 == rval)	{
		/* failure */
		EPT(stderr, "error occurs in setting timer %d[%s]\n", errno, strerror(errno));
	}
	else {
		/* success */
		rval = 0;
	}
	return rval;
}

void mt_timer_sche(int signo)
{
	int i;
	U32 emap;

	if (SIGALRM != signo)
	{
		EPT(stderr, "Caught the other signal %d\n", signo);
		return;
	}

//	EPT(stderr, "Caught the SIGALRM signal\n");

	emap = mt_tsch.tmap&(mt_tsch.tmap^mt_tsch.tmask);
	for (i = 0; i < mt_tfs; i++)
	{
		if (!((1<<i)&emap))
			continue;

		mt_tsch.procs[i].wait -= 1;
		if (mt_tsch.procs[i].wait <= 0)
		{
			mt_tsch.procs[i].wait = mt_tsch.procs[i].period;
			(*mt_tsch.procs[i].pf)(&i);
		}
	}

}

void mt_mac_sslot(void *data)
{
	mmsg_t msg;
	int cnt = 0;
	int i, j, k;
	int nin[MAX_NODE_CNT], rval;


	if (1 == start_slot) {
		EPT(stderr, "mt_mac_slot: slot start\n");

		msg.mtype = MMSG_MT_SIN;
		msg.node = 0;
		cnt += sizeof(msg.node);
		for (i = 0; i < MAX_NODE_CNT; i++) {
			if (-1 == rqs[i])
				continue;
			rval = msgsnd(rqs[i], (void *)&msg, cnt, 0);
			EPT(stderr, "mt_mac_slot: msgsnd() write msg at qid %d of node %d\n", rqs[i], i+1);
			if ( rval < 0 ) {
				EPT(stderr, "mr_mac_sslot: msgsnd() write msg failed,errno=%d[%s]\n", errno, strerror(errno));
			}
		}

		start_slot = 0;
	}
	else {
		EPT(stderr, "mt_mac_slot: slot half\n");

		k = 0;
		for (i = 0; i < MAX_NODE_CNT; i++) {
			if (MMSG_NULL == fmd[i].msg.mtype)
				nin[i] = 0;
			else {
				nin[i] = 1;
				k += 1;
			}
		}

		if (k > 0) {
			for (i = 0; i < MAX_NODE_CNT; i++) {
				if (1 == nin[i])
					continue;
				k = -1;
				for (j = 1; j < MAX_NODE_CNT; i++) {
					if (1 == top[j][i].link && 1 == nin[j]) {
						if (-1 == k)
							k = j;
						else {
							k = MAX_NODE_CNT;
							break;
						}
					}
				}
				if (k >=0 && k < MAX_NODE_CNT) {
					rval = msgsnd(rqs[i], (void *)&fmd[k].msg, fmd[k].len, 0);
				if (rval < 0 ) {
					EPT(stderr, "mr_mac_sslot: msgsnd() write msg failed,errno=%d[%s]\n", errno, strerror(errno));
				}				}
			}
		}

		start_slot = 1;
	}

}


