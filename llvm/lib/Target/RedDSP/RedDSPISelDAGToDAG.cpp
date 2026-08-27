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

    if (N->getOpcode() == ISD::Constant) {
      uint32_t Val = cast<ConstantSDNode>(N)->getZExtValue();
      if (!isInt<9>(static_cast<int32_t>(Val))) {
        // Materialize > 9-bit constants using chunk construction:
        // Build 32-bit constant via 4-bit / 8-bit shifts and ORs
        SDLoc DL(N);
        SDValue Zero = CurDAG->getRegister(RedDSP::R0, MVT::i32);
        
        // Chunk 0: Bits [31:24]
        uint32_t B3 = (Val >> 24) & 0xFF;
        uint32_t B2 = (Val >> 16) & 0xFF;
        uint32_t B1 = (Val >> 8) & 0xFF;
        uint32_t B0 = Val & 0xFF;

        SDNode *Res = CurDAG->getMachineNode(RedDSP::ADDri, DL, MVT::i32, Zero,
                                             CurDAG->getTargetConstant(B3, DL, MVT::i32));
        SDValue Cur = SDValue(Res, 0);

        auto appendByte = [&](uint32_t Byte) {
          SDNode *Shl = CurDAG->getMachineNode(RedDSP::SFLri, DL, MVT::i32, Cur,
                                               CurDAG->getTargetConstant(8, DL, MVT::i32));
          if (Byte != 0) {
            SDNode *Or = CurDAG->getMachineNode(RedDSP::ORri, DL, MVT::i32, SDValue(Shl, 0),
                                                CurDAG->getTargetConstant(Byte, DL, MVT::i32));
            Cur = SDValue(Or, 0);
          } else {
            Cur = SDValue(Shl, 0);
          }
        };

        appendByte(B2);
        appendByte(B1);
        appendByte(B0);

        ReplaceNode(N, Cur.getNode());
        return;
      }
    }

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