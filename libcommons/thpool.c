/********************************** 
 * @author     wallwind@yeah.net
 * @date        2012/06/13
 * Last update: 2012/06/13
 * License:     LGPL
 * 
 **********************************/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <string.h>

#include "ulog.h"
#include "thpool.h"

static	sigset_t signal_mask;

static void th_nosing()
{
	sigemptyset(&signal_mask);
	sigaddset(&signal_mask,SIGPIPE);//send errno
	//sigaddset(&signal_mask,SIGQUIT);//ctr+c errno

	int rc = pthread_sigmask(SIG_BLOCK, &signal_mask, NULL);//NULL屏蔽信号处理函数
	if (rc != 0) {
		debug("thpool_add_work new:block sigpipe error");
	}
}

//开辟的线程--一直运行
static void thpool_thread_do(void* v)
{
	th_node_t *th_node = (th_node_t *)v;
	//debug("thpool_thread_do begin pthread_t:%ld lock:%d",th_node->thread,th_node->lock);
	
	while(1)
	{
		//debug("thpool_thread_do loop pthread_t:%ld lock:%d",th_node->thread,th_node->lock);
		pthread_mutex_lock(&(th_node->mutex));

		//检测队列就绪--执行
		//debug("thpool_thread_do loop pthread_t:%ld lock:%d",th_node->thread,th_node->lock);
		if(th_node->lock && th_node->function !=NULL)
		{
			th_node->function(th_node->arg);
			th_node->lock = 0;//清空队列
			th_node->function = NULL;
			th_node->arg = NULL;
		}

		//exit
		if(th_node->lock == -1)
		{
			pthread_mutex_unlock(&(th_node->mutex));
			break;
		}
		pthread_mutex_unlock(&(th_node->mutex));

		usleep(20*1000);//微秒	20毫秒
	}
	debug("thpool_thread_do exit pthread_t:%ld lock:%d",th_node->thread,th_node->lock);
}

//将消息加入线程池
pthread_t thpool_add_work(thpool_t* thpool, void* (*function)(void*), void* arg)
{
	th_node_t *p = thpool->head;
	int busy = 0;
	int k=0;

	again:
	k=0;
	do
	{
		if(!p->lock)
		{
			pthread_mutex_lock(&(p->mutex));
			p->function = function;
			p->arg = arg;
			p->lock = 1;
			pthread_mutex_unlock(&(p->mutex));

			//debug("thpool_add_work exits:%ld (0x%x) k=%d lock:%d",p->thread,p->thread,k,p->lock);
			return p->thread;
		}
		//debug("k=%d lock:%d",k,!(p->lock));
		k++;
	}while((p=p->next) != NULL);

	//debug("thpool_add_work for k=%d max_threads=%d",k,thpool->max_threads);
	if(k==thpool->max_threads)
	{
		if(busy==10)
		{
			debug("thpool_add_work is full max_threads:%ld k=%d",thpool->max_threads,k);
			//debug("thpool_add_work for k=%d max_threads=%d",k,thpool->max_threads);
			return -1;
		}
		debug("thpool_add_work loop add:busy=%d",busy);
		
		busy++;
		p = thpool->head;
		sleep(1);
		goto again;
	}

	//新分配空间
	//debug("thpool_add_work for NEW");
	p = (th_node_t*) malloc(sizeof(th_node_t));
	p->function = function;
	p->arg = arg;
	p->next = NULL;
	p->prev = NULL;
	pthread_mutex_init(&(p->mutex),NULL);

	//add to last node
	thpool->last->next = p;
	p->prev = thpool->last;
	thpool->last = p;
	p->lock = 1;

	//debug("thpool_add_work for NEW");
	th_nosing();
	pthread_create(&(p->thread),NULL,(void *)thpool_thread_do,(void*)p);
	//usleep(1000);
	debug("thpool_add_work new:%u (0x%x)",p->thread,p->thread);
	return p->thread;
}

int thpool_init(thpool_t* thpool,const int max_threads,const int min_threads)
{
	//最大线程数
	if(!max_threads || max_threads < 1)
		thpool->max_threads = 1;
	else
		thpool->max_threads = max_threads;

	//最小线程数
	if(!min_threads || min_threads >= max_threads)
		thpool->min_threads = max_threads;
	else
		thpool->min_threads = min_threads;

	thpool->head = NULL;
	thpool->last = NULL;

	for(int i = 0;i<thpool->min_threads;i++)
	{
		th_node_t *p = (th_node_t*) malloc(sizeof(th_node_t));
		p->function = NULL;
		p->arg = NULL;
		p->next = NULL;
		p->prev = NULL;
		p->lock = 0;
		pthread_mutex_init(&(p->mutex),NULL);
		if(i==0)
		{
			thpool->head = p;
			thpool->last = p;
		}
		else
		{
			//add to last node
			thpool->last->next = p;
			p->prev = thpool->last;
			thpool->last = p;
		}

		th_nosing();
		pthread_create(&(p->thread),NULL,(void *)thpool_thread_do,(void*)p);
		//debug("thpool_init:%ld",p->thread);
	}
	return 0;
}

void thpool_destroy(thpool_t* thpool)
{
	//debug("thpool_destroy begin");
	th_node_t *p = thpool->head;
	th_node_t *p1 = NULL;
	do
	{
		//unsigned long td = p->thread;

		pthread_mutex_lock(&(p->mutex));
		p->lock = -1;
		pthread_mutex_unlock(&(p->mutex));

		//usleep(500);
		pthread_cancel(p->thread);
		//debug("wait thpool_destroy pthread_t:%ld stop....",td);
		pthread_join(p->thread,NULL);
		pthread_mutex_destroy(&(p->mutex));

		p1 = p->next;
		free(p);
		//debug("thpool_destroy already pthread_t:%ld",td);
	}while((p = p1)!= NULL);

	thpool->head = NULL;
	thpool->last = NULL;
}
