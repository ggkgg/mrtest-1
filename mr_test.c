#include "../mr_common.h"
#include "mr_test.h"
#include <stdio.h>
#include <stdlib.h>

static key_t mt_qkey = -1;
static int   mt_qid = -1; 

static mt_tshare_t  share = {
	PTHREAD_MUTEX_INITIALIZER,
	PTHREAD_COND_INITIALIZER
};

int write_id1(int id)
{
	FILE *fp;
    char *path = "/home/kip/Desktop/wanghao";

    fp = fopen(path,"w");
    fprintf(fp, "%d\n", id);
    fclose(fp);
}

int write_id2(int id)
{
	FILE *fp;
    char *path = "/home/kip/Desktop/wanghao";

    fp = fopen(path,"a");
    fprintf(fp, "%d\n", id);
    fclose(fp);
}

static pthread_t mrx_tid = -1;
static int rqs[MAX_NET_CNT][MAX_DATA_LENGTH];

//此值从top*.txt读入
static U8  top[MAX_NET_CNT][MAX_NODE_CNT][MAX_NODE_CNT];


//deleted by test
//fmd_t fmd[MAX_NODE_CNT];

int main(int argc, char* argv[])
{
	int len;
	int i,j;
	int rval, stop;
	void *result = NULL;
	
	EPT(stderr, "%s: main thread id = %ld\n", argv[0], pthread_self());
//	EPT(stderr, "sizeof(MADR) = %d\n", sizeof(MADR));

	if (argc < 2) {
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

	write_id1(mt_qid);

	/* initialize topology */
	//参数是top.txt
	top_init();
	for(i = 1; i < argc; i++)
	{
		rval = mt_tinit(argv[i], i);
		if (rval != 0) {
			EPT(stderr, "mr_test: errors occur in topology %d initialization\n", i);
			rval = 2;
			goto process_return;
		}
		mt_show_top();
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
	test_mmsg tstmsg;
//	mmsg_t rx_msg;
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
		memset(&tstmsg, 0, sizeof(tstmsg));

		rcnt = msgrcv(qid, &tstmsg, MAX_DATA_LENGTH, 0, 0);
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

		rval = mt_rmsg_proc(&tstmsg, rcnt);
		if (rval != 0) {
			/* report error */
			EPT(stderr, "mr_test: error occurs, the node of tstmsg  = %d", tstmsg.mesg.node);
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

int mt_rmsg_proc(test_mmsg *tstmsg, int cnt)
{
	MADR node;
	int qid;
	int j, rval = 0;
	int net;

	net = (int)tstmsg->mtype;
	node = tstmsg->mesg.node;
	if (tstmsg->mesg.mtype == MMSG_MT_RQID) 		//node : sa
	{
		qid = *(int*)tstmsg->mesg.data;			//ask???

		if (NET_ISUNI(net) && MR_ISUNI(node)) {
			rqs[net-1][node-1] = qid;
			write_id2(qid);
		}
		else {
			EPT(stderr, "mt_test: node[%d][%d] error\n",net, node);
			rval = 1;
		}
	}
	else 		//from others sop packet
	{

		EPT(stderr, "receive a mesg from node[%d][%d]\n", net, node);
		cnt -= sizeof(long);
		
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			if (j == node-1 || top[net-1][node-1][j] == 0)   /*if (j == node-1 || top[node-1][j].link == 0) */
				continue;

			if (rqs[net-1][j] == -1) 
			{
				EPT(stderr, "mr_test: can not get the qid of rx node[%d][%d]\n",net,j+1);
				continue;
			}
//deleted by test, delete the condition 
//			if (mt_getlp() <= top[node-1][j].relb)			
													//	srand(qid)	rand()/RAND_MAX
			
			EPT(stderr, "mt_test: msgsnd() write msg(size:%d) at qid %d of node[%d][%d]\n", cnt, rqs[net-1][j], net, j+1);

			rval = msgsnd(rqs[net-1][j], (void *)&(tstmsg->mesg), cnt, 0);	//?????!!!
			
			if (rval < 0) 
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

void top_init()
{
	int i, j, k;
/* initiate */
	for (i = 0; i < MAX_NET_CNT; i++) 
	{
		for (j = 0; j < MAX_NODE_CNT; j++)
		{
			rqs[i][j] = -1;
			for(k = 0; k < MAX_NODE_CNT; k++)
			{
				top[i][j][k] = 0;
				//deleted by test
				/*
							top[i][j].link = 0;
							top[i][j].relb = 0.0;
				*/
			}
		}
	}
}

//把top.txt中的数据读入top数组
int mt_tinit(char *name, int num)
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
	while(!feof(fp)) 	//判断文件是否结束。feof（fp）用于测试fp所指向的文件的当前状态是否为“文件结束”。如果是，函数则返回的值是1（真），否则为0（假）
	{
		fscanf(fp, "%d %d", &i, &j); //从流中执行格式化输入的函数. fscanf遇到空格和换行时结束,返回值：整型，数值等于[argument...]的个数
		if (MR_ISUNI(i) && MR_ISUNI(j))
		{
			if(1 == num)
				top[0][i-1][j-1] = 1;
			else
				top[num+2][i-1][j-1] = 1;
		}
	}
	


fexit:
	if (NULL != fp)
		fclose(fp);
	return rval;
}



void mt_show_top()
{
	int i, j, k;
	int vld = 0;


	for(k = 0; k < MAX_NET_CNT; k++)
	{
		if( (k > 0) && (k < 4) )
			continue;

		for (i = 0; i < MAX_NODE_CNT; i++) 
		{
			vld = 0;
			for (j = 0; j < MAX_NODE_CNT; j++) 
			{
				if (0 == top[k][i][j])
				{
					continue;
				}
				vld = 1;			
				EPT(stderr, "top[%d][%d][%d]=%d", k+1, i+1, j+1, top[k][i][j]);
			}
			if (1 == vld)
				EPT(stderr, "\n");
		}
		if (1 == vld)
			EPT(stderr, "\n");
	}
	
}


void show_rqs()
{
	int i, j;
	for (i = 0; i < MAX_NET_CNT; i++)
	{
		for(j = 0; j < MAX_NODE_CNT; j++)
			EPT(stderr, "%3d.%3d %d\n", i+1, j+1, rqs[i][j]);
	}
}


