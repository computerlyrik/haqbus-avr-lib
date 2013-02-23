/* fnordRS485
   Version: 0.1
   Author: Christian Pohl
   Date: 2009-05-12
*/

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <termios.h>
#include <stdlib.h>

int openPort(char *port)
{
        int fh; /* filehandle */
        struct termios options;
        fh = open(port, O_RDWR | O_NOCTTY | O_NDELAY);
        if (fh == -1)
        {
                perror("openPort: Unable to open port");
                return (0);
        }
        else{
                fcntl(fh, F_SETFL, 0);
                /* read current options */
                tcgetattr(fh, &options);
                /* in/out 19200bd */
                cfsetispeed(&options, B19200);
                cfsetospeed(&options, B19200);
                /* Enable the receiver and set local mode */
                options.c_cflag |= (CLOCAL | CREAD);
                /* 8bit */
                options.c_cflag &= ~CSIZE; /* Mask the character size bits */
                options.c_cflag |= CS8;    /* Select 8 data bits */
                /* rts/cts to enable rs485 transmitter with rts */
                options.c_cflag |= CRTSCTS;
                /* one stop bit */
                options.c_cflag &= ~CSTOPB;
                /* enable parity */
                options.c_cflag |= PARENB;
                /* set new options */
                tcsetattr(fh, TCSANOW, &options);
                return (fh);
        }
}

/* return 0 if even number of 1,
   return 1 if odd number of 1 */
unsigned isOdd(unsigned char x)
{
      x = x ^ (x >> 4);
      x = x ^ (x >> 2);
      x = x ^ (x >> 1);

      return ((unsigned)(x & 1));
}

/* sendByte byte requires 9th bit (parity) to be parityBit
   addrcmd: 9th bit must be 1
   data: 9th bit must be 0
   returns number of sent bytes (1 on succes, 0 on error) */
int sendByte(int fh, unsigned char byte, unsigned int parityBit) {
        struct termios options;
        tcgetattr(fh, &options);

        /* precompute parity and set paritymode so that the paritybit will be parityBit */
        if(isOdd(byte) != parityBit) {
                options.c_cflag |= PARODD;
        printf( "8o1\n");
        }
        else {
                options.c_cflag &= ~PARODD;
        printf ("8e1\n");
        }
        tcsetattr(fh, TCSADRAIN, &options);
        return write(fh,&byte,1);
}

/* argv:
   1 addr
   2 cmd
   3 data 1
   4 data 2
   5 data 3
   6 ...
*/
int main(int argc, char *argv[]) {

        int fh;
        int i;
        unsigned char addr;

        if(argc<=2) {
                printf("Usage:\n");
                printf("%s address command [data1] [data2] ... \n",argv[0]);
                printf("Address: 0-255 (0 is broadcast)\n");
                printf("Command: 1 (softreset), 2 (set color, 3 databytes), 3 (fade color, 5 databytes)\n");
                printf("All databytes: 0-255\n");
                printf("No checks at all! Let's see what happens if you provide wrong data :-D\n");
                exit(1);
        }

        fh=openPort("/dev/ttyAMA0");
        if(fh == 0) {
                exit(2);
        }

        addr = atoi(argv[1]);
        printf("Sending to address %d\n",atoi(argv[1]));
        if(sendByte(fh,addr,1)<=0) {
                perror("error write addrcmd");
                exit(3);
        }
        for(i=2;i<argc;i++) {
                printf("Sending data: [%d]: %d\n",i-1,atoi(argv[i]));
                if(sendByte(fh,atoi(argv[i]),0)<=0) {
                        perror("error write data");
                        exit(4);
                }
        }
	
	unsigned char foo;
	while(1) {
	read(fh,&foo,2);
	printf("%s",foo);
        }
        close(fh);
        exit(0);
}
