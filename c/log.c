#include "log.h"

#include <stdarg.h>
#include <stdio.h>
#include <time.h>

static void ts(char *buf, size_t n) {
	time_t now = time(NULL);
	struct tm t;
	localtime_r(&now, &t);
	strftime(buf, n, "%H:%M:%S", &t);
}

static void vlog(const char *level, const char *color, const char *fmt, va_list ap) {
	char timebuf[16];
	ts(timebuf, sizeof timebuf);
	fprintf(stdout, "%s[%s]%s %s ", color, level, "\x1b[0m", timebuf);
	vfprintf(stdout, fmt, ap);
	fprintf(stdout, "\n");
	fflush(stdout);
}

void log_info(const char *fmt, ...) {
	va_list ap; va_start(ap, fmt);
	vlog("INFO", "\x1b[36m", fmt, ap); // cyan
	va_end(ap);
}

void log_ok(const char *fmt, ...) {
	va_list ap; va_start(ap, fmt);
	vlog(" OK ", "\x1b[32m", fmt, ap); // green
	va_end(ap);
}

void log_err(const char *fmt, ...) {
	va_list ap; va_start(ap, fmt);
	vlog("ERR ", "\x1b[31m", fmt, ap); // red
	va_end(ap);
}

void log_recv(const char *fmt, ...) {
	va_list ap; va_start(ap, fmt);
	vlog("RECV", "\x1b[33m", fmt, ap); // yellow
	va_end(ap);
}

void log_send(const char *fmt, ...) {
	va_list ap; va_start(ap, fmt);
	vlog("SEND", "\x1b[35m", fmt, ap); // magenta
	va_end(ap);
}


