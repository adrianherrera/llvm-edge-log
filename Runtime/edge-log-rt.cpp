#include <stdint.h>
#include <stdio.h>

#include <map>
#include <vector>

using Edge = std::pair<uintptr_t, uintptr_t>;
using EdgeVector = std::vector<Edge>;

const char *const kEdgeLogEnv = "EDGE_LOG_PATH";

static EdgeVector *Edges;
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

  fprintf(LogFile, "prev_addr,cur_addr\n");
  for (const auto &Edge : *Edges) {
    fprintf(LogFile, "%zu,%zu\n", Edge.first, Edge.second);
  }

  fclose(LogFile);
  delete Edges;
}

extern "C" void __edge_log() {
  std::uintptr_t CurBB =
      reinterpret_cast<std::uintptr_t>(__builtin_return_address(0));

  Edges->push_back({PrevBB, CurBB});
  PrevBB = CurBB;
}
