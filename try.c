
#include <stdio.h>

#include <omp.h> 
 
void process(const char *msg, int *id, int pid) 
{
	while (*id)
		if (pid == *id) {
			printf("%s\n", msg);
			*id = -1;
		}
}


void master_process(int *pid) 
{
	while (*pid)
		scanf("%d", pid);

	printf("bye!\n");
}

int main ()  {
	
	int pid = -1;	

	
	#pragma omp parallel shared(pid) num_threads(4)
    {
	#pragma omp sections nowait
	{
	#pragma omp section 
	process("A", &pid, 1);

	#pragma omp section 
	process("B", &pid, 2);

	#pragma omp section 
	process("C", &pid, 3);

	#pragma omp section
	master_process(&pid);
	}
	}

	return 0;
}



