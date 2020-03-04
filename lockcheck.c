#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "lockcheck.h"

#define	MAXPID	40
#define	MAXLOCK	30


typedef	struct	Btrace	Btrace;

struct	Btrace
{
	unsigned long	pid;
	int	deadlock;
	
};

enum
{
	btsize	 = 600,
};

	pthread_mutex_t lockcheck_lk = PTHREAD_MUTEX_INITIALIZER;
	pthread_mutex_t*	held[MAXPID][MAXLOCK];
	pthread_mutex_t*	lockid[MAXLOCK];
	pthread_t	pid_tab[MAXPID];
	Btrace*		follows[MAXLOCK][MAXLOCK];
	int		detected = 0;
	int		init = 0;
	int ok=1;

 void My_mutex_lock(void)
{
	pthread_mutex_lock(&lockcheck_lk);
}

 void My_mutex_unlock(void)
{
	pthread_mutex_unlock(&lockcheck_lk);
}

 int get_lockid(pthread_mutex_t *q)
{
	unsigned int i;
	int  found;
	
	found = 0;
	for(i = 0; i < MAXLOCK && lockid[i] != NULL; i++) 
	{
		if(lockid[i] == q) 
        {
			found = 1;
			break;
		}
	}
	if(found == 0)
		lockid[i] = q;

	return i;
}


 pthread_mutex_t *get_lock(unsigned int i)
{
	if(i < 0 || i >=MAXLOCK)
		return NULL;

	return lockid[i];
}


 int get_internal_pid()
{
	unsigned int i;
	int  found;
	pthread_t tid;
    /// returneaza id threadului curent
	tid = pthread_self();
	
	found = 0;
	for(i = 0; i < MAXPID && pid_tab[i] != -1; i++) 
	{
		if(pid_tab[i] == tid) 
        {
			found = 1;
			break;
		}
	}
	if(found == 0)
		pid_tab[i] = tid;

	return i;
}

 pthread_t get_pid( unsigned int i)
{
	if(i < 0 || i >=MAXPID)
		return (pthread_t) 0;

	return pid_tab[i];
}

 int does_follow(unsigned int a, unsigned int b)
{
	unsigned int i;

	if(a < 0 || a >= MAXLOCK || b < 0 || b >= MAXLOCK)
		return 0;

	if(follows[a][b])
		return 1;

	for(i=0; i<MAXLOCK; i++) {
		if(follows[a][i] && does_follow(i, b)) {
			printf("existing intermediate dependency \n");
			return 1;
		}
	}

	return 0;
}

 void lockcheck_init(void)
{
	unsigned int i, j;

#ifdef DEBUG
	printf("lockcheck_init <<< \n");
#endif
	if(init == 0) {
		for(i = 0; i < MAXLOCK; i++)
			for(j = 0; j < MAXLOCK; j++)
				follows[i][j] = NULL;

		for(i = 0; i < MAXLOCK; i++)
			lockid[i] = NULL;

		for(i = 0; i < MAXPID; i++)
			pid_tab[i] = -1;

		for(i = 0; i < MAXPID; i++)
			for(j = 0; j < MAXLOCK; j++)
				held[i][j] = NULL;

		init = 1;
	}

#ifdef DEBUG
	printf("lockcheck_init >>> \n");
#endif

}


 void dump_lockcheck(int dmpbt)
{
	unsigned int i, j;
	Btrace	*bt;
	pthread_mutex_t	*qlk;

	printf("current mutex with holding threads (PID)\n");
	printf("========================================\n");
	for(i = 0; i < MAXPID; i++)
		for(j = 0; j < MAXLOCK; j++)
			if(held[i][j] != NULL) 
            {
				qlk = get_lock(j);
				printf("pid 0x%x holds lock 0x%x (id=%d)\n",
					i, qlk, j);
			}

	printf("\n\n");

	printf("Recorded lock dependency\n");
	printf("========================================\n");
	for(i = 0; i < MAXLOCK; i++)
		for(j = 0; j < MAXLOCK; j++) {
			bt = follows[i][j];
			if(bt != NULL) 
{      
	if(bt->deadlock==1)
		{ok=0;}
	printf("pid 0x%x lock 0x%x (id=%d) -> lock 0x%x (id=%d) %s\n",
	bt->pid, get_lock(i), i, get_lock(j), j,
	bt->deadlock==1?"deadlock":"ok");
				
			}
		}

}

 int will_lock(pthread_mutex_t *mutex, int pid)
{
	int	lockid;
	unsigned int i;
	Btrace	*bt;

	if(pid < 0 || pid >= MAXPID || mutex == NULL)
		return NULL;

	My_mutex_lock();

#ifdef DEBUG
#endif

	lockid = get_lockid(mutex);

	for(i = 0; i < MAXLOCK; i++) 
        {
		if(held[pid][i] == NULL)
			continue;

		if(!follows[i][lockid]) 
            {
			bt = (Btrace*)malloc((sizeof(Btrace) + btsize));
			
			if(bt == NULL)
				printf("will_lock: * malloc error \n");

			follows[i][lockid] = bt;
			(follows[i][lockid])->pid = pid;

			if(does_follow(lockid, i)) 
                {

				if(detected == 0) 
                {
					
					printf(">>> will_lock: * pid 0x%x deadlock lock 0x%lux (id=%d) -> lock 0x%lux (id=%d)\n",
						get_pid(pid), (unsigned long)get_lock(i), i, (unsigned long)mutex, lockid);
					detected = 1;
					(follows[i][lockid])->deadlock = 1;
					 if((follows[i][lockid])->deadlock==1)
						ok=0;
			                

					dump_lockcheck(1);
				}

				break;
			    }
			else
			{
				(follows[i][lockid])->pid = pid;
				
			}
		}
	}
#ifdef DEBUG
#endif

	My_mutex_unlock();
        return 0;

}


 int locked(pthread_mutex_t *mutex, int pid)
{
	int	lockid;

#ifdef DEBUG
#endif

	if(pid < 0 || pid >= MAXPID || mutex == NULL)
		return NULL;

	My_mutex_lock();

	lockid = get_lockid(mutex);
	held[pid][lockid] = mutex;

	My_mutex_unlock();

#ifdef DEBUG

#endif

	return 0;
}

 int unlocked(pthread_mutex_t *mutex, int pid)
{

	int	lockid;

#ifdef DEBUG
#endif

	if(pid < 0 || pid >= MAXPID || mutex == NULL)
		return NULL;

	My_mutex_lock();

	lockid = get_lockid(mutex);
	held[pid][lockid] = NULL;

	My_mutex_unlock();

#ifdef DEBUG
#endif

	return 0;
}

int mutex_lock(pthread_mutex_t *mutex)
{
	int r, pid;

	if(init == 0)
		lockcheck_init();

	pid = get_internal_pid();
	will_lock(mutex, pid);
	r = pthread_mutex_lock(mutex);
	locked(mutex, pid);
	return r;
}

int mutex_unlock(pthread_mutex_t *mutex)
{
	int r, pid;

	if(init == 0)
		lockcheck_init();

	pid = get_internal_pid();
	r = pthread_mutex_unlock(mutex);
	unlocked(mutex, pid);
	return r;
}


int no_deadlock1()
{
if(ok==0)

return 0;


return 1;
}
