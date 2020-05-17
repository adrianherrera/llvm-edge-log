#ifndef EDGE_LOG_H
#define EDGE_LOG_H

#define EDGE_LOG_ENV "EDGE_LOG_PATH"

enum EdgeType {
  EdgeCall = 0,
  EdgeRet,
  EdgeCondBr,
  EdgeUncondBr,
  EdgeSwitch,
  EdgeUnreachable,
  EdgeUnknown,
};

#endif // EDGE_LOG_H
