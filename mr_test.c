#include "../mr_common.h"
#include "mr_test.h"


static key_t mt_qkey = -1;
static int   mt_qid = -1; 

static mt_tshare_t  share = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_COND_INITIALIZER
};

static pthread_t mrx_tid = -1;
static int rqs[MAX_NODE_CNT];
//???top.txt??
static U8  top[MAX_NODE_CNT][MAX_NODE_CNT];

//deleted by test
//fmd_t fmd[MAX_NODE_CNT];

int main(int argc, char* argv[])
{
	int len;
	int j;
	int rval, stop;
	void *result = NULL;
	
	EPT(stderr, "%s: main thread id = %ld\n", argv[0], pthread_self());
//	EPT(stderr, "sizeof(MADR) = %d\n", sizeof(MADR));

	if (argc != 2) {
		EPT(stderr, "mr_test: number of input paras is not correct\n");
		rval = 1;
		goto process_return;
	}

	/* this thread obtain all queues for rx and tx */
	rval = mt_queues_init();
	if (rval != 0)
	{
		EPT(stderr, "mr_test: creating queue fails");
		rval = 1;
		goto process_return;
	}

	/* initialize topology */
	//???参数是top.txt
	rval = mt_tinit(argv[1]);
	if (rval != 0) {
		EPT(stderr, "mr_test: errors occur in topology initialization\n");
		rval = 2;
		goto process_return;
	}

	/* create receiving msg thread */
	rval = pthread_create(&mrx_tid, NULL, mt_qrv_thread, &mt_qid);
	if (rval != 0) {
		EPT(stderr, "mr_test: can not open create msg receiving thread\n");
		rval = 3;
		goto process_return;
	}
//deleted by test
	/* start timer 
	rval = mt_mac_timer();
	memset(fmd, 0, sizeof(fmd));*/

	stop = 0;
	pthread_mutex_lock(&share.mutex);
	while(0 == stop) {
		pthread_cond_wait(&share.cond, &share.mutex); 

		/* do something */
	}
//	EPT(stderr, "%s: certain thread quit\n", argv[0]);
	pthread_mutex_unlock(&share.mutex); 

process_return:
	mt_queues_delete();
	exit(rval);
}

int mt_queues_init()
{
	int rval = 0;
	
	mt_qkey = ftok(PATH_CREATE_KEY, SN_MRTEST);
	mt_qid = msgget(mt_qkey, IPC_CREAT|QUEUE_MODE);
	if (mt_qid == -1) {
		EPT(stderr, "mr_test: can not get queue\n");
		rval = 1;
	}
	
	return rval;
}

int mt_queues_delete()
{
	if (mt_qid != -1);
		msgctl(mt_qid, IPC_RMID, NULL);
	return 0;
}


void* mt_qrv_thread(void *arg)
{
	int qid, rcnt;
	mmsg_t rx_msg;
	int rval, stop;
	
	pthread_detach(pthread_self());	

	qid = *(int *)arg;
	EPT(stderr, "mt_test: msg receiving thread id = %ld\n", pthread_self());
//	EPT(stderr, "mt_test: enter queue receiving thread, rqueue %d\n", qid);

	if (qid < 0) {
		EPT(stdout, "mr_test: wrong receive queue id %d", qid);
		rval = 1;
		goto thread_return;
	}
//deleted by test
//	srand(qid); 

	rval = 0;
	stop = 0;
	while(0 == stop) {
		memset(&rx_msg, 0, sizeof(rx_msg));
//		EPT(stdout, "mr_test: reveive msg queue at qid %d\n", qid);
		rcnt = msgrcv(qid, &rx_msg, MAX_DATA_LENGTH, 0, 0);
		if (rcnt < 0) {
			if (EIDRM != errno) {
				EPT(stderr, "mr_test: error in receiving msg, no:%d, meaning:%s\n", errno, strerror(errno));
			} 
			else {
				EPT(stderr, "mr_test: quit msg receiving thread\n");
			}
			rval = 2;
			break;
		}

		rval = mt_rmsg_proc(&rx_msg, rcnt);
		if (rval != 0) {
			/* report error */
			EPT(stderr, "mr_test: error occurs, the node of rx_msg  = %d", rx_msg.node);
		}
	}

thread_return:
/*	pthread_mutex_lock(&share.mutex);
	share.qr_run = 0;
	pthread_cond_signal(&share.cond);
	pthread_mutex_unlock(&share.mutex);
*/
	sleep(1);
	pthread_exit((void *)&rval);
}

int mt_rmsg_proc(mmsg_t *msg, int cnt)
{
	MADR node;
	int qid;
	int j, rval = 0;

	node = msg->node;
	if (msg->mtype == MMSG_MT_RQID) 		//node : sa
	{
		qid = *(int*)msg->data;						//ask???

		if (MR_ISUNI(node)) {
			rqs[node-1] = qid;
		}
		else {
			EPT(stderr, "mt_test: node error\n");
			rval = 1;
		}
	}
	else /* ²»Íù×ÔÉí·¢ËÍ£¬ÇÒÔÚÍØÆËµÄÔÊÐíÏÂ·¢ */		//from others sop packet
	{
		for (j = 0; j < MAX_NODE_CNT; j++) 	
		{
			if (j == node-1 || top[node-1][j] == 0)   /*if (j == node-1 || top[node-1][j].link == 0) */
				continue;
			if (rqs[j] == -1) 
			{
				EPT(stderr, "mr_test: can not get the qid of rx node %d\n", j+1);
				continue;
			}
//deleted by test, delete the condition 
//			if (mt_getlp() <= top[node-1][j].relb)			
													//	srand(qid)	rand()/RAND_MAX
			rval = msgsnd(rqs[j], (void *)msg, cnt, 0);	//只负责转发！！！
			EPT(stderr, "mt_test: msgsnd() write msg at qid %d of node %d\n", rqs[j], j+1);
			if ( rval < 0 ) 
			{  
				EPT(stderr, "mr_test: msgsnd() write msg failed,errno=%d[%s]\n", errno, strerror(errno));  
			}
//deleted by test, delete the condition 
/*
	else
			{
				EPT(stderr, "mt_test: msgsnd() do not write msg at qid %d of node %d due to link error\n", rqs[j], j+1);
			}
*/
		}
	}
//deleted by test
/*
	else if (MMSG_MT_MQID == msg->mtype)
	{
		qid = *(int*)msg->data;
		
		if (MR_ISUNI(node))
		{
			rqs[node-1] = qid;
		}
		else
		{
			EPT(stderr, "mt_test: node error\n");
			rval = 1;
		}
	}
	else if (wh_mac_msg(msg->mtype))			//MMSG_MT_MDT
	{
		msg_copy(&fmd[node-1].msg, msg, cnt);
		fmd[node-1].len = cnt;
	}
*/

	return rval;
}

//?把top.txt??????txt中的数据读入top数组??
int mt_tinit(char *name)
{
	int i, j;
	int rval = 0;
	FILE *fp = NULL;
	//a+方式打开，不清空源文件内容，只在末尾追加
	fp = fopen(name, "a+");
	if (NULL == fp) 
	{
		rval = 1;
		goto fexit;
	}

	/* clear */
	for (i = 0; i < MAX_NODE_CNT; i++) 
	{
		rqs[i] = -1;
		for (j = 0; j < MAX_NODE_CNT; j++)
			top[i][j] = 0;
//deleted by test
/*
			top[i][j].link = 0;
			top[i][j].relb = 0.0;
*/
	}

	while(!feof(fp)) 	//?????????判断文件是否结束。feof（fp）用于测试fp所指向的文件的当前状态是否为“文件结束”。如果是，函数则返回的值是1（真），否则为0（假）(fp)????fp??????????????“????”????,????????1(?),???0(?)
	{
		fscanf(fp, "%d %d", &i, &j); //从流中执行格式化输入的函数. fscanf遇到空格和换行时结束,返回值：整型，数值等于[argument...]的个数
		if (MR_ISUNI(i) && MR_ISUNI(j))
			top[i-1][j-1] = 1;
	}

	mt_show_top();
fexit:
	if (NULL != fp)
		fclose(fp);
	return rval;
}


void mt_show_top()
{
	int i,j;
	int vld = 0;

	for (i = 0; i < MAX_NODE_CNT; i++) 
	{
		vld = 0;
		for (j = 0; j < MAX_NODE_CNT; j++) 
		{
			if (0 == top[i][j])
				continue;
			vld = 1;			
			EPT(stderr, "top[%d][%d]=%d  ", i+1, j+1, top[i][j]);
		}
		if (1 == vld)
			EPT(stderr, "\n");
	}
}

void show_rqs()
{
	int i;
	for (i = 0; i < MAX_NODE_CNT; i++) {
		EPT(stderr, "%3d %d\n", i+1, rqs[i]);
	}
}


