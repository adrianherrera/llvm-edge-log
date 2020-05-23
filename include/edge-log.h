#ifndef EDGE_LOG_H
#define EDGE_LOG_H

#define EDGE_LOG_ENV "EDGE_LOG_PATH"

enum EdgeType {
  EdgeDirectCall = 0,
  EdgeIndirectCall,
  EdgeRet,
  EdgeCondBr,
  EdgeUncondBr,
  EdgeSwitch,
  EdgeUnreachable,
  EdgeUnknown,
};

#endif // EDGE_LOG_H
