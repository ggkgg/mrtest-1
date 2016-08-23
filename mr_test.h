#ifndef _MR_TEST_H
#define _MR_TEST_H

/* struct for the data sharing among thread */
typedef struct _mt_tshare_t {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
//	int qr_run;
//	int gi_run;
} mt_tshare_t;

int   mt_tinit(char*);
int   mt_queues_init();
int   mt_queues_delete();
void* mt_qrv_thread();
int   mt_rmsg_proc(mmsg_t*, int);
int   mt_tmsg_node(MADR, int, mmsg_t*);
void  mt_show_top();
void  show_rqs();

#endif
