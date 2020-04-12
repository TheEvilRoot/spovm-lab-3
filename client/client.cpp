#include <unistd.h>
#include <csignal>
#include <sys/wait.h>
#include <cstdio>
#include <sys/sem.h>
#include <sys/shm.h>

#define LOG(a) fprintf(stderr, "Client: %s\n", a)

int waitServer(int sid) {
  struct sembuf ops[1] = { 0, -1, 0 };
  return semop(sid, ops, 1);
}

int setDone(int sid) {
  struct sembuf ops[1] = { 1, 1, 0 };
  return semop(sid, ops, 1);
}

int main(const int argc, const char *argv[]) {
  int smid = shmget(6741, 256, 0666);
  int sid = semget(6741, 2, 0655);
  fprintf(stderr, "sid: %d\n", sid);
  if (sid >= 0 && smid >= 0){
    char *mem = (char*) shmat(smid, nullptr, 0);
    while (true) {
      LOG("Waiting server...");
      waitServer(sid);
      LOG("Working...");
      fprintf(stderr, "got length %d\n", (int)mem[0]);
      for (int i = 0; i < mem[0]; i++)
        fprintf(stderr, "%c", mem[1 + i]);
      fprintf(stderr, "\n");
      LOG("Done");
      setDone(sid);
      LOG("Done setted");
    }
  } else {
    LOG("Semaphore failed");
    return 0;
  }
  return 0;
}