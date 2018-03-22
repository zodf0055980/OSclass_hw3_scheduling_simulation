#include "scheduling_simulator.h"
#include <sys/time.h>
#include <sys/queue.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>

void shell(void);
void simulate(void);

ucontext_t MAIN;
ucontext_t shellctx;
ucontext_t simuctx;
ucontext_t ctrctx;
int count = 0;
struct itimerval value, ovalue;
int t_pid =1;
int stop =0;
ucontext_t store;

struct task_t {
	char 	stack[1000];
	char 	name[8];
	char	length;
	int 	task_pid;
	int 	tcount;
	int 	rcount;
	int 	stime;
	TAILQ_ENTRY(task_t) entry;
	ucontext_t 	taskcontext;
	enum TASK_STATE state;
	int suspend;
};

TAILQ_HEAD(,task_t) whead=TAILQ_HEAD_INITIALIZER(whead);
TAILQ_HEAD(,task_t) rhead=TAILQ_HEAD_INITIALIZER(rhead);
TAILQ_HEAD(,task_t) thead=TAILQ_HEAD_INITIALIZER(thead);

/*
void sig(int sigal)
{
	printf("insig\n");
	//	struct task_t *now;
	//	now = TAILQ_FIRST(&rhead);
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL,&value,&ovalue);

	if(sigal==SIGALRM) {
		count++;
		struct task_t *search = malloc(sizeof(struct task_t));
		TAILQ_FOREACH(search,&whead,entry) {
			int find = count - (search->tcount);
			if (search->stime == find) {
				printf("wakeup whead\n");
				TAILQ_REMOVE(&whead, search, entry);
				TAILQ_INSERT_TAIL(&rhead, search, entry);
			}
		}

		printf("find\n");
		struct task_t *now;
		now = TAILQ_FIRST(&rhead);
		now->rcount++;
		if ((now->length == 'S' && now->rcount == 1) || (now->length == 'L'
		        && now->rcount == 2)) {
			printf("timeout\n");

			TAILQ_REMOVE(&rhead, now, entry);
			TAILQ_INSERT_TAIL(&rhead, now, entry);
			printf("end remove\n");
			swapcontext(&now->taskcontext, &MAIN);
		}
		swapcontext(&now->taskcontext, &MAIN);
	} else {
		stop = 1;
		getcontext(&store);
		setcontext(&shell);
		getcontext(&store);
		setcontext(&shellctx);
		printf("aaa\n");
		struct task_t *now;
		now = TAILQ_FIRST(&rhead);
		printf("name=%s", now->name);
		ctrlz_task->suspend = 1;
		getcontext(&ctrlz_task->taskcontext);
		swapcontext(&now->taskcontext, &MAIN);
		printf("in stp\n");
		makecontext(&now->taskcontext, shell,0);
		swapcontext(&MAIN,&now->taskcontext);
		getcontext(&now->taskcontext);
		printf("set ctx\n");
		setcontext(&shellctx);
		swapcontext(&now->taskcontext, &MAIN);
	}
}
*/
void hw_suspend(int msec_10)
{
	printf("hw_suspend\n");
	struct task_t *suspend_task = malloc(sizeof(struct task_t));
	printf("malloc\n");
	suspend_task = TAILQ_FIRST(&rhead);

	TAILQ_REMOVE(&rhead,suspend_task,entry);
	TAILQ_INSERT_TAIL(&whead,suspend_task, entry);

	suspend_task->tcount =	count;
	suspend_task->stime = msec_10;

	printf("do swap\n");
	getcontext(&suspend_task->taskcontext);
	setcontext(&simuctx);
	return;
}

void hw_wakeup_pid(int pid)
{
	printf("wakeup pid\n");
	struct task_t *sherch_pid= malloc(sizeof(struct task_t));;
	TAILQ_FOREACH(sherch_pid,&whead,entry) {
		if(sherch_pid->task_pid == pid) {
			TAILQ_REMOVE(&whead,sherch_pid,entry);
			TAILQ_INSERT_TAIL(&rhead,sherch_pid, entry);
		}
	}
	return;
}

int hw_wakeup_taskname(char *task_name)
{
	printf("wakeupname\n");
	struct task_t *sherch_taskname = malloc(sizeof(struct task_t));
	TAILQ_FOREACH(sherch_taskname,&whead,entry) {
		if(sherch_taskname->name == task_name) {
			TAILQ_REMOVE(&whead,sherch_taskname,entry);
			TAILQ_INSERT_TAIL(&rhead,sherch_taskname, entry);
		}
	}
	return 0;
}

int hw_task_create(char *task_name)
{
	printf("create\n");
	struct task_t *create = malloc(sizeof(struct task_t));
	strcpy(create->name, task_name);
	create->name[0] =  'T';
	create->length = 'L';
	create->task_pid = t_pid;
	create->tcount= count;
	t_pid++;
	getcontext(&create->taskcontext);
	create->taskcontext.uc_stack.ss_sp = create->stack;
	create->taskcontext.uc_stack.ss_size = sizeof(create->stack);
	create->taskcontext.uc_link=&MAIN;

	if(strcmp(create->name,"Task1")==0) {
		makecontext(&create->taskcontext, task1, 0);
	} else if (strcmp(create->name,"Task2")==0) {
		makecontext(&create->taskcontext, task2, 0);
	} else if (strcmp(create->name,"Task3")==0) {
		makecontext(&create->taskcontext, task3, 0);
	} else if (strcmp(create->name,"Task4")==0) {
		makecontext(&create->taskcontext, task4, 0);
	} else if (strcmp(create->name,"Task5")==0) {
		makecontext(&create->taskcontext, task5, 0);
	} else if (strcmp(create->name,"Task6")==0) {
		makecontext(&create->taskcontext, task6, 0);
	}


	TAILQ_INSERT_TAIL(&whead,create, entry);
	return create->task_pid; // the pid of created task name
}


void shell(void)
{
	while(1) {
/*		value.it_value.tv_sec = 0;
		value.it_value.tv_usec = 0;
		value.it_interval.tv_sec = 0;
		value.it_interval.tv_usec = 0;
		setitimer(ITIMER_REAL,&value,&ovalue);
*/
		printf("shell\n");

		char shell_chosse[28];
		memset(shell_chosse,'\0',sizeof(shell_chosse));
		fgets(shell_chosse,28,stdin);
		char choose[8],want[8],d[8];
		char timeq;
		memset(choose,'\0',sizeof(choose));
		memset(want,'\0',sizeof(want));
		memset(d,'\0',sizeof(d));
		memset(&timeq,'\0',sizeof(timeq));

		sscanf(shell_chosse, "%s %s %s %c ", choose, want, d, &timeq);
		if (strcmp(choose, "add") == 0) {
			struct task_t *temp = malloc(sizeof(struct task_t));
			temp->length='L';
			temp->task_pid = t_pid;
			temp->tcount = count;
			temp->suspend = 0;
			t_pid++;

			strcpy(temp->name,want);
			if(strcmp(d,"")!=0) {
				temp->length = timeq;
			}
			temp->taskcontext.uc_stack.ss_sp = temp->stack;
			temp->taskcontext.uc_stack.ss_size = sizeof(temp->stack);
			temp->taskcontext.uc_link=&MAIN;
			getcontext(&temp->taskcontext);

			if(strcmp(temp->name,"Task1")==0) {
				makecontext(&temp->taskcontext, task1, 0);
			} else if (strcmp(temp->name,"Task2")==0) {
				makecontext(&temp->taskcontext, task2, 0);
			} else if (strcmp(temp->name,"Task3")==0) {
				makecontext(&temp->taskcontext, task3, 0);
			} else if (strcmp(temp->name,"Task4")==0) {
				makecontext(&temp->taskcontext, task4, 0);
			} else if (strcmp(temp->name,"Task5")==0) {
				makecontext(&temp->taskcontext, task5, 0);
			} else if (strcmp(temp->name,"Task6")==0) {
				makecontext(&temp->taskcontext, task6, 0);
			}

			printf("insert\n");
			TAILQ_INSERT_TAIL(&rhead, temp, entry);

		} else if (strcmp(choose,"remove")==0) {
			struct task_t *listptr;
			struct task_t *wlistptr;
			listptr = malloc(sizeof(struct task_t));
			wlistptr= malloc(sizeof(struct task_t));
			int gpid = atoi(want);

			TAILQ_FOREACH(listptr,&rhead,entry)
			if(listptr->task_pid==gpid) {
				TAILQ_REMOVE(&rhead,listptr,entry);
			}
			TAILQ_FOREACH(wlistptr,&whead,entry)
			if(wlistptr->task_pid==gpid) {
				TAILQ_REMOVE(&whead,wlistptr,entry);
			}
		} else if (strcmp(choose,"start")==0) {
			return;
		} else if (strcmp(choose,"ps")==0) {
			struct task_t *ps_rlistptr;
			struct task_t *ps_wlistptr;
			struct task_t *ps_tlistptr;
			printf("ready\n");
			TAILQ_FOREACH(ps_rlistptr, &rhead, entry) {
				int showcount = (count-ps_rlistptr->tcount)*10;
				printf("%d %s TASK_READY %d\n", ps_rlistptr->task_pid, ps_rlistptr->name,
				       showcount);
			}

			printf("wait\n");
			TAILQ_FOREACH(ps_wlistptr, &whead, entry) {
				int showcount = (count-ps_wlistptr->tcount)*10;
				printf("%d %s TASK_WAITING %d\n",ps_wlistptr->task_pid,ps_wlistptr->name,
				       showcount);
			}

			printf("terminated\n");
			TAILQ_FOREACH(ps_tlistptr,&thead,entry) {
				int showcount = (count-ps_tlistptr->tcount)*10;
				printf("%d %s TASK_TERMINATED %d\n",ps_tlistptr->task_pid,	ps_tlistptr->name,
				       showcount);
			}
		}
	}
	return;
}

void simulate(void)
{
	if(stop==0) {
		while(!TAILQ_EMPTY(&rhead)) {
			struct task_t *simulate_task;
			simulate_task = TAILQ_FIRST(&rhead);
			simulate_task->rcount=0;
			value.it_value.tv_sec=0;
			value.it_value.tv_usec=10000;
			value.it_interval.tv_sec=0;
			value.it_interval.tv_usec = 10000;
			setitimer(ITIMER_REAL, &value, &ovalue);
			swapcontext(&MAIN,&simulate_task->taskcontext);

			TAILQ_REMOVE(&rhead,simulate_task,entry);
			TAILQ_INSERT_TAIL(&thead, simulate_task, entry);
		}

	} else {
		stop = 0;
		setcontext(&store);
	}
	printf("rq empty");
	setcontext(&shellctx);
}

int main()
{
	value.it_value.tv_sec = 0;
	value.it_value.tv_usec = 0;
	value.it_interval.tv_sec = 0;
	value.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL,&value,&ovalue);
/*
	signal(SIGTSTP,sig);
	signal(SIGALRM,sig);
*/
	getcontext(&shellctx);
	shell();
	getcontext(&simuctx);
	simulate();

}
