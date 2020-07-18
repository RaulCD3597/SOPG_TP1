#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdint.h>

#define FIFO_NAME "myfifo"
#define BUFFER_SIZE 300
#define MSG_SIZE (BUFFER_SIZE + 2)
#define MSG_TYPE_LENGTH 4

int main(void)
{
	uint8_t inputBuffer[BUFFER_SIZE];
    char msg[MSG_SIZE];
	int32_t bytesRead, returnCode, fd;
    FILE *fp = NULL;
    
    /* Create named fifo. -1 means already exists so no action if already exists */
    if ( (returnCode = mknod(FIFO_NAME, S_IFIFO | 0666, 0) ) < -1  )
    {
        printf("Error creating named fifo: %d\n", returnCode);
        exit(1);
    }
    
    /* Open named fifo. Blocks until other process opens it */
	printf("waiting for writers...\n");
	if ( (fd = open(FIFO_NAME, O_RDONLY) ) < 0 )
    {
        printf("Error opening named fifo file: %d\n", fd);
        exit(1);
    }
    
    /* open syscalls returned without error -> other process attached to named fifo */
	printf("got a writer\n");

    /* Loop until read syscall returns a value <= 0 */
	do
	{
        /* read data into local buffer */
		if ((bytesRead = read(fd, inputBuffer, BUFFER_SIZE)) == -1)
        {
			perror("read");
        }
        else
		{
			inputBuffer[bytesRead] = '\0';
            sprintf(msg, "%s\r\n", inputBuffer);
            if (!(memcmp(inputBuffer, "DATA", MSG_TYPE_LENGTH)))
            {
                fp = fopen("Log.txt", "a");
                if(fp == NULL) 
                {
                    perror("Error opening file");
                }
                else
                {
                    fputs(msg, fp);
                    fclose(fp); 
                }
            }
            else if (!(memcmp(inputBuffer, "SIGN", MSG_TYPE_LENGTH)))
            {
                fp = fopen("Sign.txt", "a");
                if(fp == NULL) 
                {
                    perror("Error opening file");
                }
                else
                {
                    fputs(msg, fp);
                    fclose(fp);
                }
            }
		}
	}
	while (bytesRead > 0);

	return 0;
}