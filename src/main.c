#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h> // error management
#include <fcntl.h> // containst UNIX related constansts like O_RDWR
#include <unistd.h> // read(), write(), open(), close()

#define LF 0x0a
#define CR 0x0d

const char *path = "/dev/ttyUSB1"; 
const uint8_t code_uid[] = { 0x01, 0x08, 0x02, 0x20, 0x21, 0x22, 0x1d, 0xd4, CR, LF };

int main(void)
{
  hello();

  int serial_port = open(path, O_RDWR);
  if (serial_port < 0) {
    printf("Error %s\n", strerror(errno));
    return 1;
  }

  ssize_t bytes_written = write(serial_port, (void*)code_uid, sizeof(code_uid));
  if (bytes_written < 0) {
    printf("Error %s\n", strerror(errno));
    close(serial_port);
    return 1;
  }

  printf("%d Bytes written to %s\n", bytes_written, path);

  close(serial_port);
  return 0;
}

