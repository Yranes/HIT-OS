#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <sys/times.h>
#include <sys/types.h>

#define HZ	100

void cpuio_bound(int last, int cpu_time, int io_time);

int main(int argc, char * argv[]){
	pid_t n_proc[10]; /*10个子进程 PID*/
	int i;
	for(i = 0; i < 10; i++){
		n_proc[i] = fork();     //fork创建子进程
		if(n_proc[i] == 0){
			cpuio_bound(20, 2 * i, 20 - 2 * i); //每个子进程都占用20
			return 0;   //结束子进程
		}
		/*fork 失败*/
		else if(n_proc[i] < 0){
			printf("Failed to fork child process %d!\n", i + 1);
			return -1;
		}
	}
	/*父进程打印所有子进程PID*/
	for(i = 0; i < 10; i++)
		printf("Child PID: %d\n", n_proc[i]);
	/*并等待所有子进程完成*/
	wait(&i);
	return 0;
}

void cpuio_bound(int last, int cpu_time, int io_time){
	struct tms start_time, current_time;
	clock_t utime, stime;
	int sleep_time;

	while(last > 0){
		/* CPU Burst */
		times(&start_time);   //记录进程开始时间（时钟数）
		do{
			times(&current_time);   //记录当前时间（时钟数）
			utime = current_time.tms_utime - start_time.tms_utime;
			stime = current_time.tms_stime - start_time.tms_stime;
		}while(((utime + stime) / HZ)  < cpu_time);     //时钟数 * (1/hz) = t
		last -= cpu_time;
		if(last <= 0)
			break;
		/* IO Burst */
		/* 用sleep(1)模拟1秒钟的I/O操作 */
		sleep_time = 0;
		while (sleep_time < io_time){
			sleep(1);
			sleep_time++;
		}
		last -= sleep_time;
	}
}
