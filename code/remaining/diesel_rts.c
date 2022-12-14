/* diesel_rts.c */
#include <unistd.h>
// Compile with gcc -c diesel_rts.c -o diesel_rts.o -Wall -m64

/* Previously, getchar and putc were used, but these now cause
 * segmentation faults on Ubuntu 22.04
 */

#ifdef __cplusplus
extern "C" {
#endif

int mygetchar() {
    char res;
    return 1 == read(0, &res, 1) ? res : -1;
}

void myputchar(int ch) {
    char c = ch;
    write(1, &c, 1);
    fsync(1);
}

#ifdef __cplusplus
}
#endif
