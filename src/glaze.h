#ifndef GLAZE__GLAZE_H_
#define GLAZE__GLAZE_H_

#define GLZ_VERSION "0.3.0"
#define err(...) glz_error(__FILE__, __LINE__, __VA_ARGS__)
#define fatal(...) glz_fatal(__FILE__, __LINE__, __VA_ARGS__)
#define bug(...) glz_bug(__FILE__, __LINE__, __VA_ARGS__)
#define array_sizeof(a) ((int)(sizeof(a)/sizeof(a[0])))
#define GLZ_SHADOW "___%"

void glz_error(const char* fname, unsigned int line, const char* fmt, ...);
void glz_fatal(const char* fname, unsigned int line, const char* fmt, ...);
void glz_bug(const char* fname, unsigned int line, const char* fmt, ...);

#define fpprintf(...) fprintf(__VA_ARGS__)
int fdprintf(int fd, const char *fmt, ...);

#endif // GLAZE__GLAZE_H_
