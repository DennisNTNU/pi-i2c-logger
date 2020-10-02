#ifndef LOG_LOCAL_H
#define LOG_LOCAL_H

int log_alloc_buffer(unsigned int byte_count, const char* path);
void log_dealloc_buffer();
int log_data(char* data, int byte_count);
int log_flush_buffer();

#endif /* LOG_LOCAL_H */