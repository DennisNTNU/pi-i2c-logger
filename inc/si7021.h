#ifndef SI7021_H
#define SI7021_H

int readSi7021(float * data);
int readSi7021_fd(int fd_i2c, float * data);

#endif /* SI7021_H */