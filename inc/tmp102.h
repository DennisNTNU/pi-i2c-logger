#ifndef TMP102_H
#define TMP102_H

int readTMP102(float * temp);
int readTMP102_fd(int fd_i2c, float * temp);

#endif /* TMP102_H */