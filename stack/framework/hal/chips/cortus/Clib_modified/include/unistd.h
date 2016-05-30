#ifndef _UNISTD_H
#define _UNISTD_H

/* Increase the end of accessible data space by DELTA bytes.
   Returns a pointer to the start of the new area.
   On error, -1 is returned, and errno is set to ENOMEM.

   sbrk is not defined in the C Standard and is deliberately
   excluded from the POSIX.1 standard (see paragraphs B.1.1.1.3 and B.8.3.3).
*/
void* sbrk(int DELTA);

#ifndef __APS__
int read(int fd, void *buf, int count);
int write(int fd, void *buf, int count);
#endif

#endif
