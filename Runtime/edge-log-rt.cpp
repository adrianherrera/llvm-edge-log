#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <dlfcn.h>

#include <vector>

#include <zlib.h>

using Edge = std::pair<std::uintptr_t, std::uintptr_t>;
using EdgeVector = std::vector<Edge>;

const char *const kEdgeLogEnv = "EDGE_LOG_PATH";
const char *const kEnableGZipEnv = "EDGE_LOG_GZIP";

static EdgeVector *Edges;
static __thread std::uintptr_t PrevBB;

template <typename T, T OpenF(const char *, const char *),
          int PrintF(T, const char *, ...), int CloseF(T)>
static void WriteLog(const char *LogPath) {
  Dl_info Info;

  T LogFile = OpenF(LogPath, "w");
  if (!LogFile) {
    return;
  }

  PrintF(LogFile, "shared_object,base_addr,prev_addr,cur_addr\n");
  for (const auto &Edge : *Edges) {
    const std::uintptr_t Prev = Edge.first;
    const std::uintptr_t Cur = Edge.second;

    dladdr(reinterpret_cast<void *>(Cur), &Info);
    const std::uintptr_t Base =
        reinterpret_cast<std::uintptr_t>(Info.dli_fbase);
    const char *SharedObj = Info.dli_fname;

    PrintF(LogFile, "%s,%zu,%zu,%zu\n", SharedObj, Base, Prev, Cur);
  }

  CloseF(LogFile);
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
    WriteLog<gzFile, gzopen, gzprintf, gzclose>(LogPath);
  } else {
    WriteLog<FILE *, fopen, fprintf, fclose>(LogPath);
  }

Cleanup:
  delete Edges;
}

extern "C" void __edge_log() {
  const void *Ret = __builtin_return_address(0);
  const std::uintptr_t CurBB = reinterpret_cast<std::uintptr_t>(Ret);

  Edges->push_back({PrevBB, CurBB});
  PrevBB = CurBB;
}
