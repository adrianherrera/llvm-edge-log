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

__attribute__((weak)) EdgeVector *Edges = nullptr;
__attribute__((weak)) std::uintptr_t BaseAddr;
__attribute__((weak)) __thread std::uintptr_t PrevBB;

template <typename T, T OpenF(const char *, const char *),
          int PrintF(T, const char *, ...), int CloseF(T)>
static void WriteLog(const char *LogPath) {
  T LogFile = OpenF(LogPath, "w");
  if (!LogFile) {
    return;
  }

  PrintF(LogFile, "# base address: %zu\nprev_addr,cur_addr\n", BaseAddr);
  for (const auto &Edge : *Edges) {
    PrintF(LogFile, "%zu,%zu\n", Edge.first, Edge.second);
  }

  CloseF(LogFile);
}

__attribute__((constructor)) static void Initialize() {
  if (Edges) {
    return;
  }

  Edges = new EdgeVector;
}

__attribute__((destructor)) static void AtExit() {
  if (!Edges) {
    return;
  }

  const char *LogPath = getenv(kEdgeLogEnv);
  if (!LogPath) {
    goto Cleanup;
  }

  if (getenv(kEnableGZipEnv)) {
    WriteLog<gzFile, gzopen, gzprintf, gzclose>(LogPath);
  } else {
    WriteLog<FILE *, fopen, fprintf, fclose>(LogPath);
  }

Cleanup:
  delete Edges;
  Edges = nullptr;
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
