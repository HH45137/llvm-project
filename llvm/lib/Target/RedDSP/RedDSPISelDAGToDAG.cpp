#include "RedDSP.h"
#include "RedDSPTargetMachine.h"
#include "llvm/CodeGen/SelectionDAGISel.h"
#include "llvm/IR/PassManager.h"
using namespace llvm;

#define DEBUG_TYPE "reddsp-isel"
namespace {
class RedDSPDAGToDAGISel : public SelectionDAGISel {
public:
  RedDSPDAGToDAGISel(RedDSPTargetMachine &TM, CodeGenOptLevel OL)
      : SelectionDAGISel(TM, OL) {}
  void Select(SDNode *N) override {
    if (N->isMachineOpcode()) { N->setNodeId(-1); return; }
    SelectCode(N);
  }
#include "RedDSPGenDAGISel.inc"
};

class RedDSPDAGToDAGISelLegacy : public SelectionDAGISelLegacy {
public:
  static char ID;
  RedDSPDAGToDAGISelLegacy(RedDSPTargetMachine &TM, CodeGenOptLevel OL)
      : SelectionDAGISelLegacy(
            ID, std::make_unique<RedDSPDAGToDAGISel>(TM, OL)) {}
};

char RedDSPDAGToDAGISelLegacy::ID = 0;
}

FunctionPass *llvm::createRedDSPISelDag(RedDSPTargetMachine &TM,
                                        CodeGenOptLevel OL) {
  return new RedDSPDAGToDAGISelLegacy(TM, OL);
}