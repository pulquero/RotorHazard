#include <sys/time.h>

inline int sprints(char*const dest, const char*const src) {
  strcpy(dest, src);
  return strlen(src);
}

inline int jsonStart(char *const buf) {
    buf[0] = '{';
    buf[1] = '\0';
    return 1;
}

inline int jsonPropertyString(char *const buf, const char *const key, const char *const value) {
    int n = 0;
    buf[n++] = '\"';
    n += sprints(buf+n, key);
    buf[n++] = '\"';
    buf[n++] = ':';
    buf[n++] = ' ';
    buf[n++] = '\"';
    n += sprints(buf+n, value);
    buf[n++] = '\"';
    buf[n++] = '\0';
    return n;
}

inline int jsonPropertyUInt(char *buf, const char *key, const int value) {
    int n = 0;
    buf[n++] = '\"';
    n += sprints(buf+n, key);
    buf[n++] = '\"';
    buf[n++] = ':';
    buf[n++] = ' ';
    n += sprintf(buf+n, "%u", value);
    return n;
}

inline int jsonPropertyNegUInt(char *const buf, const char *const key, const int value) {
    int n = 0;
    buf[n++] = '\"';
    n += sprints(buf+n, key);
    buf[n++] = '\"';
    buf[n++] = ':';
    buf[n++] = ' ';
    n += sprintf(buf+n, "-%u", value);
    return n;
}

inline int jsonPropertyULong(char *const buf, const char *const key, const uint32_t value) {
    int n = 0;
    buf[n++] = '\"';
    n += sprints(buf+n, key);
    buf[n++] = '\"';
    buf[n++] = ':';
    buf[n++] = ' ';
    n += sprintf(buf+n, "%lu", value);
    return n;
}

inline int jsonPropertyTime(char *const buf, const char *const key, const timeval *const t) {
    int n = 0;
    buf[n++] = '\"';
    n += sprints(buf+n, key);
    buf[n++] = '\"';
    buf[n++] = ':';
    buf[n++] = ' ';
    n += sprintf(buf+n, "%ld%ld", t->tv_sec, t->tv_usec/1000L);
    return n;
}

inline int jsonEnd(char *const buf) {
    buf[0] = '}';
    buf[1] = '\0';
    return 1;
}
