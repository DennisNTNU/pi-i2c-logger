#ifndef BUFFERRED_WRITER_H
#define BUFFERRED_WRITER_H

int br_alloc_buffer(unsigned int byte_count, const char* path);
void br_dealloc_buffer();
int br_data(char* data, int byte_count);
int br_flush_buffer();

#endif /* BUFFERRED_WRITER_H */