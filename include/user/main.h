#ifndef MAIN_H
#define MAIN_H
void do_data_abort(void);

void do_prefetch_abort(void);

void do_svc(void);

void do_undef(void);
void main(void *args);
#endif
