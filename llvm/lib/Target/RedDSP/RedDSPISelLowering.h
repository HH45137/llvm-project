#ifndef LLVM_LIB_TARGET_REDDSP_REDDSPISELLOWERING_H
#define LLVM_LIB_TARGET_REDDSP_REDDSPISELLOWERING_H
#include "llvm/CodeGen/TargetLowering.h"
namespace llvm {
namespace RedDSPISD {
enum NodeType : unsigned {
  FIRST_NUMBER = ISD::BUILTIN_OP_END,
  RET_FLAG,
  BRCOND,
  CMP
};
}
class RedDSPSubtarget;
class RedDSPTargetLowering : public TargetLowering {
  const RedDSPSubtarget &STI;

public:
  RedDSPTargetLowering(const TargetMachine &TM, const RedDSPSubtarget &STI);
  SDValue LowerOperation(SDValue Op, SelectionDAG &DAG) const override;
  SDValue LowerFormalArguments(SDValue Chain, CallingConv::ID CallConv,
                               bool IsVarArg,
                               const SmallVectorImpl<ISD::InputArg> &Ins,
                               const SDLoc &DL, SelectionDAG &DAG,
                               SmallVectorImpl<SDValue> &InVals) const override;
  bool CanLowerReturn(CallingConv::ID CallConv, MachineFunction &MF,
                      bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      LLVMContext &Context, const Type *RetTy) const override;
  SDValue LowerReturn(SDValue Chain, CallingConv::ID CallConv, bool IsVarArg,
                      const SmallVectorImpl<ISD::OutputArg> &Outs,
                      const SmallVectorImpl<SDValue> &OutVals, const SDLoc &DL,
                      SelectionDAG &DAG) const override;
};
} // namespace llvm
#endif