#ifndef LOG_H
#define LOG_H

#include <stdio.h>

void log_info(const char *fmt, ...);
void log_ok(const char *fmt, ...);
void log_err(const char *fmt, ...);
void log_recv(const char *fmt, ...);
void log_send(const char *fmt, ...);

#endif // LOG_H


