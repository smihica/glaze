#ifndef GLAZE__GC_H_
#define GLAZE__GC_H_

void* gc_realloc (void* ob, size_t os, size_t ns);
void* gc_newblock(void* ob, size_t os, size_t ns);

#endif // GLAZE__GC_H_
