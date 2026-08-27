#ifndef LLVM_LIB_TARGET_REDDSP_REDDSP_H
#define LLVM_LIB_TARGET_REDDSP_REDDSP_H

#include "llvm/Support/CodeGen.h"

namespace llvm {
class FunctionPass;
class RedDSPTargetMachine;
class Target;
Target &getTheRedDSPTarget();
FunctionPass *createRedDSPISelDag(RedDSPTargetMachine &TM,
                                  CodeGenOptLevel OptLevel);
} // namespace llvm

#define GET_INSTRINFO_ENUM
#include "RedDSPGenInstrInfo.inc"

#endif