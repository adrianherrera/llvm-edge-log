#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>

#include <vector>

#include <zlib.h>

using Edge = std::pair<uintptr_t, uintptr_t>;
using EdgeVector = std::vector<Edge>;

const char *const kEdgeLogEnv = "EDGE_LOG_PATH";
const char *const kEnableGZipEnv = "EDGE_LOG_GZIP";

static EdgeVector *Edges;
static std::uintptr_t BaseAddr;
static __thread std::uintptr_t PrevBB;

template <typename T, int PrintF(T, const char *, ...)>
static void WriteLog(T LogFile) {
  PrintF(LogFile, "# base address: %zu\nprev_addr,cur_addr\n", BaseAddr);
  for (const auto &Edge : *Edges) {
    PrintF(LogFile, "%zu,%zu\n", Edge.first, Edge.second);
  }
}

__attribute__((constructor)) static void Initialize() {
  Edges = new EdgeVector;
}

__attribute__((destructor)) static void AtExit() {
  const char *LogPath = getenv(kEdgeLogEnv);
  if (!LogPath) {
    goto Cleanup;
  }

  if (getenv(kEnableGZipEnv)) {
    gzFile Log = gzopen(LogPath, "w");
    if (!Log) {
      goto Cleanup;
    }
    WriteLog<gzFile, gzprintf>(Log);
    gzflush(Log, Z_SYNC_FLUSH);
    gzclose(Log);
  } else {
    FILE *Log = fopen(LogPath, "w");
    if (!Log) {
      goto Cleanup;
    }
    WriteLog<FILE *, fprintf>(Log);
    fflush(Log);
    fclose(Log);
  }

Cleanup:
  delete Edges;
}

extern "C" __attribute__((weak)) void __edge_log() {
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
