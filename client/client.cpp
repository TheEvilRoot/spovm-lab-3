#include <cstdio>
#include <sys/sem.h>
#include <sys/shm.h>

int waitServer(int sid) {
  struct sembuf ops[1] = { 0, -1, 0 };
  return semop(sid, ops, 1);
}

int setDone(int sid) {
  struct sembuf ops[1] = { 1, 1, 0 };
  return semop(sid, ops, 1);
}

bool checkShouldDie(int sid) {
  struct sembuf ops[1] = { 2, 0, IPC_NOWAIT };
  int res = semop(sid, ops, 1);
  return res == -1;
}

void readFromShared(char *shared, FILE* file) {
  for (int i = 0; i < shared[0]; i++)
    fprintf(file, "%c", shared[1 + i]);
  fprintf(file, "\n");
  fflush(file);
}

int main(const int argc, const char *argv[]) {
  int smid = shmget(6741, 256, 0666);
  if (smid < 0) { perror("smid"); }
  int sid = semget(6741, 3, 0655);
  if (sid < 0) { perror("sid"); }
  FILE *file = fopen("client.log", "w+");
  if (file == nullptr) { perror("file"); }

  if (sid >= 0 && smid >= 0 && file != nullptr) {
    char *mem = (char*) shmat(smid, nullptr, 0);
    while (true) {
      waitServer(sid);
      if (checkShouldDie(sid)) break;
      readFromShared(mem, file);
      setDone(sid);
    }
  }

  fclose(file);
  return 0;
}