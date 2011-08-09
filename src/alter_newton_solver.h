#ifndef ALTER_NEWTON_SOLVER_H
#define ALTER_NEWTON_SOLVER_H

void newton_log(char* str);

#define info(...) {char buf[100]; \
                   sprintf(buf, __VA_ARGS__); \
                   char* buf2 = buf; \
                   if(strncmp(buf, " ----", 5)) buf2 = buf+5; \
                   newton_log(buf2);}

#endif
