/*
 * USB serial connectivity tester
 *
 * based on: http://lists.uclibc.org/pipermail/uclibc/2008-January/039683.html
 * see: http://marc.info/?l=linux-serial&m=120661887111805&w=2
 *
 * TODO: Add a 250000 hack
 */
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>

#include <linux/serial.h>
#include <linux/termios.h>
//#include <sys/ioctl.h>

static char* portname;
static int baud_rate;
int currentSpeed;
static int serial_fd;
int lineBufferPos = 0;
char lineBuffer[1024];

typedef enum SET_SPEED_RESULT {
	SSR_OK = 0, SSR_IO_GET, SSR_IO_SET, SSR_IO_MGET, SSR_IO_MSET1, SSR_IO_MSET2
} SET_SPEED_RESULT;

int checkDelay = 5;
int recieveTimeout = 2;

enum LOG_LEVELS {
	QUIET = -1,
	ERROR = 0,
	WARNING = 1,
	INFO = 2,
	VERBOSE = 3,
	BULK = 4
};

int serverLogLevel = BULK; //default value, this is probably overridden in main()
FILE* serverLogFile = NULL;

void logger(int level, const char* format, ...) {
	va_list args;
	time_t ltime;
	struct tm* now;

	if (!serverLogFile || level > serverLogLevel) return;

	ltime = time(NULL);
	now = localtime(&ltime);
	fprintf(serverLogFile, "%02i-%02i %02i:%02i:%02i  ",now->tm_mday, now->tm_mon + 1, now->tm_hour, now->tm_min, now->tm_sec);

	va_start(args, format);
	vfprintf(serverLogFile, format, args);
	va_end(args);
}

/* based on setSerialSpeed in UltiFi */
static int setPortSpeed(int fd, int speed) {
	struct termios2 options;
	int modemBits;

	if (ioctl(fd, TCGETS2, &options) < 0) return SSR_IO_GET;

	cfmakeraw(&options);

	// Enable the receiver
	options.c_cflag |= CREAD;

	// Clear handshake, parity, stopbits and size
	options.c_cflag &= ~CLOCAL;
	options.c_cflag &= ~CRTSCTS;
	options.c_cflag &= ~PARENB;
	options.c_cflag &= ~CSTOPB;
	options.c_cflag &= ~CSIZE;

	//set speed
	options.c_ospeed = options.c_ispeed = speed;
	options.c_cflag &= ~CBAUD;
	options.c_cflag |= BOTHER;

	options.c_cflag |= CS8;
	options.c_cflag |= CLOCAL;

	if (ioctl(fd, TCSETS2, &options) < 0) return SSR_IO_SET;

	//toggle DTR
	if (ioctl(fd, TIOCMGET, &modemBits) < 0) return SSR_IO_MGET;
	modemBits |= TIOCM_DTR;
	if (ioctl(fd, TIOCMSET, &modemBits) < 0) return SSR_IO_MSET1;
	usleep(100 * 1000);
	modemBits &=~TIOCM_DTR;
	if (ioctl(fd, TIOCMSET, &modemBits) < 0) return SSR_IO_MSET2;

	currentSpeed = speed;

	return SSR_OK;
}

void send(const char* code) {
	logger(INFO,"send: '%s'\n", code);
	write(serial_fd, code, strlen(code));
}
void parseLine(const char* line) {
	logger(INFO, "|%s|\n", line);
	recieveTimeout	= 2;
}
int readSerial() {
	logger(BULK,"  Checking\n");
	fd_set fdset;
	struct timeval timeout;

	FD_ZERO(&fdset);
	FD_SET(serial_fd, &fdset);

	timeout.tv_sec = 0;
	timeout.tv_usec = 100 * 1000;//100ms timeout
	select(serial_fd + 1, &fdset, NULL, NULL, &timeout);

	if (FD_ISSET(serial_fd, &fdset)) {
		logger(BULK,"  Receiving\n");
		int len = read(serial_fd, lineBuffer + lineBufferPos, sizeof(lineBuffer) - lineBufferPos - 1);
		if (len < 1) {
			logger(INFO, "Connection closed.\n");
			exit(EXIT_FAILURE);
			return;
		}
		lineBufferPos += len;
		lineBuffer[lineBufferPos] = '\0';
		while(strchr(lineBuffer, '\n'))	{
			char* c = strchr(lineBuffer, '\n');
			*c++ = '\0';
			parseLine(lineBuffer);
			lineBufferPos -= strlen(lineBuffer) + 1;
			memmove(lineBuffer, c, strlen(c) + 1);
		}
		//On buffer overflow clear the buffer (this usually happens on the wrong baudrate)
		if (lineBufferPos == sizeof(lineBuffer) - 1) {
			lineBufferPos = 0;
		}
	}
}

int main(int argc, char** argv) {
	SET_SPEED_RESULT spdResult;

	//setup logger
	serverLogFile = stderr;

	//if (argc < 2) {
	//	fprintf(stderr, "%s: please supply a port name, optionally followed by the port speed\n", argv[0]);
	//	exit(1);
	//}

	if (argc >= 2) {
		serverLogLevel = strtol(argv[1], NULL, 10);
	} else {
		serverLogLevel = VERBOSE;
	}

	if (argc >= 3) {
		portname = argv[2];
	} else {
		portname = "/dev/ttyACM0";
	}

	if (argc >= 4) {
		baud_rate = strtol(argv[5], NULL, 10);
	} else {
		baud_rate = 115200;
	}

	serial_fd = open(portname, O_RDWR);
	if (serial_fd == -1) {
		logger(ERROR, "%s: could not open port %s (%s)\n", argv[0], portname, strerror(errno));
		exit(2);
	}

	logger(INFO,"using port %s with speed %i\n", portname, baud_rate);

	spdResult = setPortSpeed(serial_fd,baud_rate);
	switch (spdResult) {
		case SSR_OK:
			logger(INFO, "port opened ok\n");
			break;
		case SSR_IO_GET: logger(ERROR, "ioctl error in setPortSpeed() on TCGETS2 (%s)\n", strerror(errno));
		case SSR_IO_SET: logger(ERROR, "ioctl error in setPortSpeed() on TCSETS2 (%s)\n", strerror(errno));
		case SSR_IO_MGET: logger(ERROR, "ioctl error in setPortSpeed() on TIOCMGET (%s)\n", strerror(errno));
		case SSR_IO_MSET1: logger(ERROR, "ioctl error in setPortSpeed() on TIOCMSET1 (%s)\n", strerror(errno));
		case SSR_IO_MSET2: logger(ERROR, "ioctl error in setPortSpeed() on TIOCMSET2 (%s)\n", strerror(errno));
			exit(1);
			break;
	}

	while(1) {
		readSerial();

		if (checkDelay > 0) {
			checkDelay--;
		} else {
			send("M105\n");
			logger(VERBOSE, "new recieveTimeout: %d\n",recieveTimeout);
			if (recieveTimeout) {
				recieveTimeout--;
			} else {
				logger(ERROR, "Failed to connect\n");
				exit(1);
			}

			checkDelay = 5;
		}
	}
	close(serial_fd);
	exit(0);
}
