#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>

int util_load_file (char* buf, char* file_name);
int util_check_flag (char* flag_name);
void util_clear_flag (char* flag_name);
void util_set_flag (char* flag_name);
#endif
