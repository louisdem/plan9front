/*
 * Posix stdio -- fdopen
 */
#include "iolib.h"
#include <errno.h>
/*
 * Open the named file with the given mode, using the given FILE
 * Legal modes are given below, `additional characters may follow these sequences':
 * r rb		open to read
 * w wb		open to write, truncating
 * a ab		open to write positioned at eof, creating if non-existant
 * r+ r+b rb+	open to read and write, creating if non-existant
 * w+ w+b wb+	open to read and write, truncating
 * a+ a+b ab+	open to read and write, positioned at eof, creating if non-existant.
 */
FILE *fdopen(const int fd, const char *mode){
	FILE *f;

	if(fd < 0){
		errno = EBADF;
		return NULL;
	}
	if((f = _IO_newfile()) == NULL)
		return NULL;
	f->fd=fd;
	if(mode[0]=='a')
		lseek(f->fd, 0L, 2);
	f->flags=0;
	f->state=OPEN;
	f->buf=0;
	f->rp=0;
	f->wp=0;
	f->lp=0;
	return f;
}
