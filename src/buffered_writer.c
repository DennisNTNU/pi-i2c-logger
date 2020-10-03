#include "buffered_writer.h"

#include <stdio.h> // for fopen(), fwrite(), fclose()
#include <stdlib.h> // for calloc()
#include <string.h> // for memcpy(), strerror();
#include <errno.h> // for errno

#include "debug_macros.h"

static char* buffer = NULL;
static char* buffer_file_path = NULL;
static unsigned int buffer_alloc_size = 0;
static unsigned int buffer_size_current = 0;

int br_alloc_buffer(unsigned int byte_count, const char* path)
{
    if (buffer == NULL)
    {
        buffer_size_current = 0;
        buffer_alloc_size = byte_count;
        // TODO Maybe check byte count is more than 4KB and less than ... i dont know 4MB?
        buffer = calloc(1, byte_count);
        if (buffer == NULL)
        {
            return -2; // could not allocate buffer
        }
        int pathlen = strlen(path);
        buffer_file_path = malloc(pathlen+1);
        if (buffer_file_path == NULL)
        {
            return -3; // could not allocate buffer file path buffer
        }
        strcpy(buffer_file_path, path);
        return 0;
    }
    return -1; // buffer already allocated
}

void br_dealloc_buffer()
{
    free(buffer);
    free(buffer_file_path);
    buffer_alloc_size = 0;
    buffer_size_current = 0;
}

//int add_data_to_buffer(char* data, int byte_count)
int br_data(char* data, int byte_count)
{
    if (buffer != NULL)
    {
        unsigned int new_size = buffer_size_current + byte_count;
        if (new_size > buffer_alloc_size)
        {
            // flush buffer to file
            if (br_flush_buffer() != 0)
            {
                printf("Probably lost some period of data just now!\n");
            }

            // clear buffer
            buffer_size_current = 0; // not necessary to deallocate or setting to zeros?
        }
        memcpy(&(buffer[buffer_size_current]), data, byte_count);
        buffer_size_current += byte_count;
        return 0;
    }
    return 1;
}

int br_flush_buffer()
{
    if (buffer_size_current != 0)
    {
        if (buffer_file_path != NULL)
        {
            if (buffer != NULL)
            {
                FILE* buffer_file_p = fopen(buffer_file_path, "a");
                if (buffer_file_p == NULL)
                {
                    printf("Error opening log file, errno %i: %s\n", errno, strerror(errno));
                    return -1;
                }
                else
                {
                    int bwrit = fwrite(buffer, 1, buffer_size_current, buffer_file_p);
                    if (bwrit != buffer_size_current)
                    {
                        printf("Error writing buffer to log file, ferror: %i\n", ferror(buffer_file_p));
                        clearerr(buffer_file_p);
                    }
                    int ret = fclose(buffer_file_p);
                    if (ret != 0)
                    {
                        printf("Error closing log file, errno: %i: %s\n", errno, strerror(errno));
                        return -2;
                    }
                }
            }
            else
            {
                printf("Buffer is NULL\n");
                return 1;
            }
        }
        else
        {
            printf("Buffer file path is NULL\n");
            return 2;
        }
    }
    return 0;
}