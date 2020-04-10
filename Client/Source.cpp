#include <iostream>
#include <string>

#include <Windows.h>

void checkpoint(const char *name) {
  fprintf(stderr, "%sE:\t%d\n", name, GetLastError());
  if (GetLastError() != 0) {
    fprintf(stderr, "Terminating on %s\n", name);
    ExitProcess(1);
  }
}

int main(const int argc, const char *argv[]) {
  DWORD error = 0;

  HANDLE pipeFile =
      CreateFileA("\\\\.\\pipe\\server2clientpipe",
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr, OPEN_EXISTING, 
        FILE_ATTRIBUTE_NORMAL, nullptr);
  checkpoint("p");
  HANDLE toWrite =
      OpenSemaphoreA(SYNCHRONIZE, false, "toWrite");
  checkpoint("tw");
  HANDLE writeComplete =
    OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, false, "writeComplete");
  checkpoint("wc");

  fprintf(stderr, "Client side!\n");
  fprintf(stderr, "Waiting messages from server\n");

  char *buffer = new char[255];
  buffer[254] = 0;
  DWORD readed = 0;
  DWORD i = 0;

  while (true) {
    if (WaitForSingleObject(toWrite, 500) != WAIT_TIMEOUT) {
      fprintf(stderr, "\r");
      ReadFile(pipeFile, buffer, 255, &readed, nullptr);
      fprintf(stderr, "%d < ", readed);
      buffer[readed] = 0;
      fprintf(stderr, "%s\n", buffer);
      ReleaseSemaphore(writeComplete, 1, nullptr);
    } else {
      if (i++ >= 3) {
        fprintf(stderr, "\r");
        for (; i != 0; i--)
          fprintf(stderr, " ");
        fprintf(stderr, "\r");
      }
      fprintf(stderr, ".");
    }
  }

  return 0;
}