#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h> // error management
#include <fcntl.h> // containst UNIX related constansts like O_RDWR
#include <unistd.h> // read(), write(), open(), close()
#include <termios.h> // terminal settings handler

#define FRAME_LEN 11

const char *path = "/dev/ttyUSB0"; 

typedef enum {
  FRAME_1,
  FRAME_2,
  FRAME_MAX
} card_t;

const uint8_t cards[FRAME_MAX][FRAME_LEN] = {
  { 0x01, 0x0b, 0x13, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x7d, 0x50 },
  { 0x01, 0x0b, 0x13, 0x00, 0x0a, 0x0b, 0x0c, 0x0d, 0x0e, 0x37, 0x68 },
};

int main(int argc, char *argv[])
{
  if (argc != 3)
    printf("Set serial port as argument and choose valid or invalid card [0/1].");

  path = argv[1];

  uint8_t code_uid[FRAME_LEN];
  uint8_t target = atoi(argv[2]);
  switch(target)
  {
    case FRAME_1:
      memcpy(code_uid, cards[FRAME_1], FRAME_LEN);
      break;
    case FRAME_2:
      memcpy(code_uid, cards[FRAME_2], FRAME_LEN);
      break;
    default:
      break;
  }

  int serial_port = open(path, O_RDWR);
  if (serial_port < 0) {
    printf("Error %s\n", strerror(errno));
    return 1;
  }

  // Create new termios struct, we call it 'tty' for convention
  struct termios tty;

  // Read in existing settings, and handle any error
  if(tcgetattr(serial_port, &tty) != 0) {
      printf("Error %i from tcgetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  tty.c_cflag &= ~PARENB; // Clear parity bit, disabling parity (most common)
  tty.c_cflag &= ~CSTOPB; // Clear stop field, only one stop bit used in communication (most common)
  tty.c_cflag &= ~CSIZE; // Clear all bits that set the data size 
  tty.c_cflag |= CS8; // 8 bits per byte (most common)
  tty.c_cflag &= ~CRTSCTS; // Disable RTS/CTS hardware flow control (most common)
  tty.c_cflag |= CREAD | CLOCAL; // Turn on READ & ignore ctrl lines (CLOCAL = 1)

  tty.c_lflag &= ~ICANON;
  tty.c_lflag &= ~ECHO; // Disable echo
  tty.c_lflag &= ~ECHOE; // Disable erasure
  tty.c_lflag &= ~ECHONL; // Disable new-line echo
  tty.c_lflag &= ~ISIG; // Disable interpretation of INTR, QUIT and SUSP
  tty.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
  tty.c_iflag &= ~(IGNBRK|BRKINT|PARMRK|ISTRIP|INLCR|IGNCR|ICRNL); // Disable any special handling of received bytes

  tty.c_oflag &= ~OPOST; // Prevent special interpretation of output bytes (e.g. newline chars)
  tty.c_oflag &= ~ONLCR; // Prevent conversion of newline to carriage return/line feed
  // tty.c_oflag &= ~OXTABS; // Prevent conversion of tabs to spaces (NOT PRESENT ON LINUX)
  // tty.c_oflag &= ~ONOEOT; // Prevent removal of C-d chars (0x004) in output (NOT PRESENT ON LINUX)

  tty.c_cc[VTIME] = 10;    // Wait for up to 1s (10 deciseconds), returning as soon as any data is received.
  tty.c_cc[VMIN] = 0;

  // Set in/out baud rate to be 9600
  cfsetispeed(&tty, B115200);

  // Save tty settings, also checking for error
  if (tcsetattr(serial_port, TCSANOW, &tty) != 0) {
      printf("Error %i from tcsetattr: %s\n", errno, strerror(errno));
      return 1;
  }

  ssize_t bytes_written = write(serial_port, (void*)code_uid, sizeof(code_uid));
  if (bytes_written < 0) {
    printf("Error %s\n", strerror(errno));
    close(serial_port);
    return 1;
  }

  printf("%zd Bytes written to %s\n", bytes_written, path);

  close(serial_port);
  return 0;
}

