#ifndef _MR_TEST_H
#define _MR_TEST_H

//#include "mr_common.h"

/* struct for the data sharing among thread */
typedef struct _mt_tshare_t {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
//	int qr_run;
//	int gi_run;
} mt_tshare_t;

typedef struct _mtop_t {
	U8		link;		/* 1: connected */
	float	relb;		/* successful distribution probability */
} mtop_t;

typedef struct _fmd_t {
	mmsg_t	msg;
	int		len;
} fmd_t;

int   mt_tinit(char*);
int   mt_queues_init();
int   mt_queues_delete();
void* mt_qrv_thread();
int   mt_rmsg_proc(mmsg_t*, int);
int   mt_tmsg_node(MADR, int, mmsg_t*);
float mt_getlp();
void  show_top1();
void  show_top2();
void  show_rqs();

int   wh_rp_msg(long);
int   wh_mac_msg(long);
void  msg_copy(mmsg_t*, mmsg_t*, int);

#endif
