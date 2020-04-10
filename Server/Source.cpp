#include <iostream>
#include <string>
#include <functional>

#include <Windows.h>

typedef PROCESS_INFORMATION ProcessInfo;

DWORD getProcessExitCode(HANDLE& handle) { 
  DWORD exitCode;
  GetExitCodeProcess(handle, &exitCode);
  return exitCode;
}

std::string getString(size_t maxSize) { 
  std::string string;
  while (true) {
    int c = getchar();
    if (c <= 0 || c == '\n' || string.size() > maxSize - 1)
      break;
    if (c <= 255)
      string += static_cast<char>(c);
  }
  return string;
}

ProcessInfo* createClient() { 
  STARTUPINFO startUpInfo;
  ZeroMemory(&startUpInfo, sizeof(startUpInfo));

  auto *processInfo = new ProcessInfo;

  startUpInfo.cb = sizeof(startUpInfo);
  TCHAR name[11];
  wcscpy_s(name, 11, L"Client.exe");

  CreateProcessW(nullptr, name, nullptr, nullptr, false,
                 CREATE_NEW_CONSOLE, nullptr, nullptr, &startUpInfo,
                 processInfo);

  return processInfo;
}

HANDLE createDuplexPipe() { 
  return CreateNamedPipeA("\\\\.\\pipe\\server2clientpipe",
    PIPE_ACCESS_DUPLEX,
    PIPE_TYPE_MESSAGE, 
    2, 255, 255, 0, nullptr);
}

bool checkClient(HANDLE &client) { 
  return getProcessExitCode(client) == STILL_ACTIVE;  
}

HANDLE pipe;
ProcessInfo* clientHandle;
HANDLE toWrite;
HANDLE writeComplete;

void destruct() {
  CloseHandle(pipe);
  CloseHandle(toWrite);
  CloseHandle(writeComplete);
  if (clientHandle != nullptr) {
    TerminateProcess(clientHandle->hProcess, 0);
    WaitForSingleObject(clientHandle->hProcess, INFINITE); 
  }
  delete clientHandle;
}

bool consoleHandler(DWORD event) {
  if (event == CTRL_CLOSE_EVENT || event == CTRL_C_EVENT) 
     destruct();
  return true;
}

int main(const int argc, const char* argv[]) {
  DWORD error = 0;

  SetConsoleCtrlHandler((PHANDLER_ROUTINE)consoleHandler, true);
  fprintf(stderr, "chE:\t%d\n", GetLastError());
  error |= GetLastError();

  pipe = createDuplexPipe();
  fprintf(stderr, "pE:\t%d\n", GetLastError());
  error |= GetLastError();

  clientHandle = createClient();
  fprintf(stderr, "cE:\t%d\n", GetLastError());
  error |= GetLastError();

  toWrite = CreateSemaphoreA(nullptr, 0, 1, "toWrite");
  fprintf(stderr, "twE:\t%d\n", GetLastError());
  error |= GetLastError();

  CreateSemaphoreA(nullptr, 0, 1, "writeComplete");

  writeComplete = OpenSemaphoreA(SYNCHRONIZE, false, "writeComplete");
  fprintf(stderr, "cwcE:\t%d\n", GetLastError());
  error |= GetLastError();

  fprintf(stderr, "Server side!\n");
  fprintf(stderr, "Enter #q to exit and stop client\n");
  fprintf(stderr, "Enter message to send to client\n");
  fprintf(stderr, "Max message length is 255\n");
  fprintf(stderr, "Up to 255-characters messages will be splitted into multiple messages\n");

  if (error == 0) {
    DWORD writed = 0;

    while (true) {
      fprintf(stderr, "> ");
      auto string = getString(255);
      if (string == "#q")
        break;

      if (!checkClient(clientHandle->hProcess)) {
        fprintf(stderr, "Wow. Client died. Stopping server!\n");
        break;
      }

      WriteFile(pipe, string.c_str(), string.size(), &writed, nullptr);
      ReleaseSemaphore(toWrite, 1, nullptr);
      fprintf(stderr, "Writed %d/%d bytes. Waiting client...\n", writed, string.size());
      WaitForSingleObject(writeComplete, INFINITE);
    }
  }

  destruct();
  return 0;
}