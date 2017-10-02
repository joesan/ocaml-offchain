
#include <stdlib.h>

char inputName(int, int);
int inputSize(int);
char inputData(int, int);

struct system {
  int next_fd;
  int ptr[1024]; // Pointers to the data blocks for each fd
  int pos[1024]; // Location inside the block
  int closed[1024];
  char *file_name[1024];
  char *file_data[1024];
  int file_size[1024];
};

// Global variable that will store our system
struct system *sys;

int getNameLength(int ptr) {
  int res = 0;
  while (inputName(ptr, res) != 0) res++;
  return res;
}

char *getName(int ptr) {
  int sz = getNameLength(ptr);
  char *res = malloc(sz+1);
  for (int i = 0; i < sz; i++) res[i] = inputName(ptr, i);
  res[sz] = 0;
  return res;
}

char *getData(int ptr) {
  int sz = inputSize(ptr);
  char *res = malloc(sz+1);
  for (int i = 0; i < sz; i++) res[i] = inputData(ptr, i);
  return res;
}

void initSystem() {
  struct system *s = malloc(sizeof(struct system));
  s->next_fd = 3; // 0:stdin, 1:stdout, 2:stderr
  // Actually we should here have a list of file names?
  // Read input byte by byte, it includes file names and data
  int loc = 0;
  int index = 0;
  int nextLength = getNameLength(index);
  while (nextLength > 0) {
     s->file_name[index] = getName(index);
     s->file_size[index] = inputSize(index);
     s->file_data[index] = getData(index);
     index++;
     nextLength = getNameLength(index);
  }
  s->file_name[index] = 0;
  sys = s;
}

int str_eq(char *s1, char *s2) {
   while (*s1 == *s2) {
     if (!s1[0] && !s2[0]) return 1;
     s1++;
     s2++;
   }
   return 0;
}

// Open file
int env____syscall5(int which, int *varargs) {
  char *name = (char*)varargs[0];
  int flags = varargs[1];
  int mode = varargs[2];
  // No empty names allowed
  if (!name || !name[0]) return -1;
  int index = 0;
  if (!sys) return -1;
  while (sys->file_name[index]) {
      if (str_eq(sys->file_name[index], name)) {
              int fd = sys->next_fd;
              sys->ptr[fd] = index;
              sys->pos[fd] = 0;
              sys->closed[fd] = 0;
              sys->next_fd++;
              return fd;
      }
      index++;
  }
  // No such file
  return -1;
}

void debugSeek();

// Seeking
int env____syscall140(int which, int *varargs) {
  int fd = varargs[0];
  int offset_high = varargs[1];
  int offset_low = varargs[2];
  int *result = (int*)varargs[3];
  int whence = varargs[4];
  // llseek(stream, offset_low, whence)
  if (whence == 1) {
    sys->pos[fd] += offset_low;
  }
  else if (whence == 2) {
    debugSeek();
  }
  else return -1;
  *result = sys->pos[fd];
  if (sys->pos[fd] < 0) return -1;
  return 0;
}

// Close
int env____syscall6(int which, int *varargs) {
  int fd = varargs[0];
  sys->closed[fd] = 1;
  return 0;
}

// Read
int env____syscall3(int which, int *varargs) {
  int fd = varargs[0];
  char *buf = (char*)varargs[1];
  int count = varargs[2];
  // read
  int index = sys->ptr[fd];
  int pos = sys->pos[fd];
  int i;
  for (i = 0; i < count && i+pos < sys->file_size[index]; i++) {
    buf[i] = sys->file_data[index][pos+i];
  }
  return i;
}

