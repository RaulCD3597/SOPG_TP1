#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>
#include <signal.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300
#define	MSG_SIZE (BUFFER_SIZE + 5)
#define SIGN_MSG_SIZE 6

char outputBuffer[BUFFER_SIZE];
char msg[MSG_SIZE];
uint32_t bytesWrote;
int32_t returnCode, fd;
struct sigaction sa1, sa2;

void sigusr1_handler(int sig) 
{
	if ((bytesWrote = write(fd, "SIGN:1", SIGN_MSG_SIZE)) == -1)
    {
		perror("write");
    }
}

void sigusr2_handler(int sig) 
{
	if ((bytesWrote = write(fd, "SIGN:2", SIGN_MSG_SIZE)) == -1)
    {
		perror("write");
    }
}

int main(void)
{
	sa1.sa_handler = sigusr1_handler;
	sa1.sa_flags = 0; // SA_RESTART; //
	sigemptyset(&sa1.sa_mask);

	sa2.sa_handler = sigusr2_handler;
	sa2.sa_flags = 0; // SA_RESTART; //
	sigemptyset(&sa2.sa_mask);

	if (sigaction(SIGUSR1, &sa1, NULL) == -1) 
	{
		perror("sigaction");
		exit(1);
	}

	if (sigaction(SIGUSR2, &sa2, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1 )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }

    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for readers...\n");
	if ( (fd = open(FIFO_NAME, O_WRONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a reader--type some stuff\n");

    /* Loop forever */
	while (1)
	{
        /* Get some text from console */
		if (NULL != fgets(outputBuffer, BUFFER_SIZE, stdin))
		{
			sprintf(msg, "DATA:%s", outputBuffer);
        
        	/* Write buffer to named fifo. Strlen - 1 to avoid sending \n char */
			if ((bytesWrote = write(fd, msg, strlen(msg)-1)) == -1)
        	{
				perror("write");
        	}
		}
		
	}
	return 0;
}