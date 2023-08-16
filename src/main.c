#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h> // error management
#include <fcntl.h> // containst UNIX related constansts like O_RDWR
#include <unistd.h> // read(), write(), open(), close()
#include <termios.h>

#define LF 0x0a
#define CR 0x0d

const char *path = "/dev/ttyUSB0"; 
const uint8_t code_uid[] = { 0x01, 0x08, 0x02, 0x20, 0x21, 0x22, 0xd4, 0x1d };

int main(int argc, char *argv[])
{
  if (argc != 2)
    printf("Set serial port as argument.");

  path = argv[1];

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

