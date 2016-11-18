#include "util.h"
#include <unistd.h>
#include <stdio.h>

int util_load_file (char* buf, char* file_name) {
	FILE* p_file = fopen (file_name, "r");
	if (!p_file)
		return -1;

	fseek (p_file, 0, SEEK_END);
	long s = ftell (p_file);
	rewind (p_file);

	fread (buf, s, 1, p_file);
	fclose (p_file);

	return 0;
}

int util_check_flag (char* flag_name) {
	return (access(flag_name, F_OK) != -1);
}

void util_clear_flag (char* flag_name) {
	remove (flag_name);
}

void util_set_flag (char* flag_name) {
	FILE* p_file = fopen (flag_name, "w");
	fclose(p_file);
}
