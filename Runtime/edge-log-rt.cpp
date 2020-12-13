#include <dlfcn.h>
#include <stdint.h>
#include <stdio.h>

#include <map>
#include <vector>

using Edge = std::pair<uintptr_t, uintptr_t>;
using EdgeVector = std::vector<Edge>;

const char *const kEdgeLogEnv = "EDGE_LOG_PATH";

static EdgeVector *Edges;
static std::uintptr_t BaseAddr;
static __thread std::uintptr_t PrevBB;

__attribute__((constructor)) static void Initialize() {
  Edges = new EdgeVector;
}

__attribute__((destructor)) static void WriteLog() {
  FILE *LogFile;
  char *LogPath = getenv(kEdgeLogEnv);

  if (!LogPath) {
    LogFile = stderr;
  }

  LogFile = fopen(LogPath, "w");
  if (!LogFile) {
    LogFile = stderr;
  }

  fprintf(LogFile, "# base address: %zu\nprev_addr,cur_addr\n", BaseAddr);
  for (const auto &Edge : *Edges) {
    fprintf(LogFile, "%zu,%zu\n", Edge.first, Edge.second);
  }

  fclose(LogFile);
  delete Edges;
}

extern "C" void __edge_log() {
  void *Ret = __builtin_return_address(0);
  std::uintptr_t CurBB = reinterpret_cast<std::uintptr_t>(Ret);

  if (__builtin_expect(BaseAddr == 0, 0)) {
    Dl_info Info;
    dladdr(Ret, &Info);
    BaseAddr = reinterpret_cast<std::uintptr_t>(Info.dli_fbase);
  }

  Edges->push_back({PrevBB, CurBB});
  PrevBB = CurBB;
}
