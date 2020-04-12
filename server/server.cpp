#include <unistd.h>
#include <cstdio>
#include <string>
#include <csignal>
#include <sys/sem.h>
#include <sys/shm.h>

#define LOG(a) fprintf(stderr, "Server: %s\n", a)

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

int main(const int argc, const char *argv[]) {
  int smid = shmget(6741,256, IPC_CREAT | 0666);
  int sid = semget(6741, 2, IPC_CREAT | 0655);
  pid_t client = startClient();

  if (sid >= 0 && smid >= 0) {
    char *mem = (char*) shmat(smid, nullptr, 0);
    while (true) {
      fprintf(stderr, "> ");
      auto string = getString(8);
      if (string == "#q") break;
      writeToShared(mem, string);
      setReadSemaphore(sid);
      waitClientSemaphore(sid);
    }
    shmdt(mem);
  } else {
    LOG("Semaphore or shared memory failed");
  }
  if (sid >= 0)
    semctl(sid, 0, IPC_RMID);
  if (smid >= 0)
    shmctl(smid, IPC_RMID, nullptr);
  kill(client, SIGKILL);
  waitpid(client, nullptr, 0);
  return 0;
}