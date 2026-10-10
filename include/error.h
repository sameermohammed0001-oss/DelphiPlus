
#ifndef DELPHIPLUS_ERROR_H
#define DELPHIPLUS_ERROR_H

void error_set(const char *message, int line, int column);
int error_count(void);
void error_reset(void);

#endif
