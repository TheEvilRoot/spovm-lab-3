#include <unistd.h>
#include <cstdio>
#include <string>
#include <sys/sem.h>
#include <sys/shm.h>


std::string getString(size_t maxSize) {
  std::string str;
  while (true) {
    int c = getchar();
    if (c <= 0 || c == '\n' || str.size() >= maxSize) break;

    if (c <= 255) {
      str += static_cast<char>(c);
    }
  }
  return str;
}

pid_t startClient() {
  pid_t clientPid = fork();
  if (clientPid == 0) {
    execlp("./client", nullptr);
  }
  return clientPid;
}

int setReadSemaphore(int sid) {
  struct sembuf ops[1] = { 0, 1, 0 };
  return semop(sid, ops, 1);
}

int waitClientSemaphore(int sid) {
  struct sembuf ops[1] = { 1, -1, 0 };
  return semop(sid, ops, 1);
}

void writeToShared(char *shared, const std::string &string) {
  if (string.size() > 255) return;

  shared[0] = static_cast<char>(string.size());
  for (char i = 0; i < shared[0]; i++)
    shared[1 + i] = string[i];
}

void killClient(int sid) {
  struct sembuf ops[1] = { 2, 1, 0 };
  semop(sid, ops, 1);
}

int main(const int argc, const char *argv[]) {
  int smid = shmget(6741,256, IPC_CREAT | 0666);
  if (smid < 0) { perror("smid"); }
  int sid = semget(6741, 3, IPC_CREAT | 0655);
  if (sid < 0) { perror("sid"); }
  pid_t client = startClient();
  if (client < 0) { perror("fork"); }

  if (sid >= 0 && smid >= 0) {
    char *mem = (char*) shmat(smid, nullptr, 0);
    while (true) {
      auto string = getString(255);
      if (string == "#q") break;
      writeToShared(mem, string);
      setReadSemaphore(sid);
      waitClientSemaphore(sid);
    }
    shmdt(mem);
  }

  killClient(sid);
  if (sid >= 0)
    semctl(sid, 0, IPC_RMID);
  if (smid >= 0)
    shmctl(smid, IPC_RMID, nullptr);
  fprintf(stderr, "Waiting client to die...\n");
  waitpid(client, nullptr, 0);
  return 0;
}