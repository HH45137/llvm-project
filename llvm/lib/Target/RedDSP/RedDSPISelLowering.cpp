#include "RedDSPISelLowering.h"
#include "RedDSPSubtarget.h"
#include "RedDSPTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAG.h"

using namespace llvm;
#define GET_CALLING_CONV_IMPL
#include "RedDSPGenCallingConv.inc"

RedDSPTargetLowering::RedDSPTargetLowering(const TargetMachine &TM,
                                           const RedDSPSubtarget &STI)
    : TargetLowering(TM, STI), STI(STI) {
  addRegisterClass(MVT::i32, &RedDSP::GR32RegClass);
  computeRegisterProperties(STI.getRegisterInfo());
  setStackPointerRegisterToSaveRestore(RedDSP::R1);
  setBooleanContents(ZeroOrOneBooleanContent);
  setOperationAction(ISD::ROTL, MVT::i32, Expand);
  setOperationAction(ISD::ROTR, MVT::i32, Expand);
  setOperationAction(ISD::SRL, MVT::i32, Expand);
  setOperationAction(ISD::SETCC, MVT::i32, Custom);
  setOperationAction(ISD::BRCOND, MVT::Other, Custom);
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
}

SDValue RedDSPTargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDLoc DL(Op);
  switch (Op.getOpcode()) {
  case ISD::SETCC: {
    ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();
    if (CC != ISD::SETEQ && CC != ISD::SETNE)
      report_fatal_error("RED DSP only supports integer equality comparisons");
    SDValue Cmp = DAG.getNode(RedDSPISD::CMP, DL, MVT::i32, Op.getOperand(0),
                              Op.getOperand(1));
    if (CC == ISD::SETNE)
      return DAG.getNode(ISD::XOR, DL, MVT::i32, Cmp,
                         DAG.getConstant(1, DL, MVT::i32));
    return Cmp;
  }
  case ISD::BRCOND:
    return DAG.getNode(RedDSPISD::BRCOND, DL, MVT::Other, Op.getOperand(0),
                       Op.getOperand(1), Op.getOperand(2));
  case ISD::BR_CC: {
    ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();
    if (CC != ISD::SETEQ && CC != ISD::SETNE)
      report_fatal_error("RED DSP only supports integer equality branches");
    SDValue Cmp = DAG.getNode(RedDSPISD::CMP, DL, MVT::i32, Op.getOperand(2),
                              Op.getOperand(3));
    if (CC == ISD::SETNE)
      Cmp = DAG.getNode(ISD::XOR, DL, MVT::i32, Cmp,
                        DAG.getConstant(1, DL, MVT::i32));
    return DAG.getNode(RedDSPISD::BRCOND, DL, MVT::Other, Op.getOperand(0), Cmp,
                       Op.getOperand(4));
  }
  default:
    llvm_unreachable("unsupported RED DSP custom lowering");
  }
}

SDValue RedDSPTargetLowering::LowerFormalArguments(
    SDValue Chain, CallingConv::ID CC, bool IsVarArg,
    const SmallVectorImpl<ISD::InputArg> &Ins, const SDLoc &DL,
    SelectionDAG &DAG, SmallVectorImpl<SDValue> &InVals) const {
  MachineFunction &MF = DAG.getMachineFunction();
  if (IsVarArg)
    report_fatal_error("RED DSP does not support variadic functions");
  SmallVector<CCValAssign, 8> Locs;
  CCState State(CC, IsVarArg, MF, Locs, *DAG.getContext());
  State.AnalyzeFormalArguments(Ins, CC_RedDSP);
  for (const CCValAssign &VA : Locs) {
    if (VA.isRegLoc()) {
      Register VReg = MF.addLiveIn(VA.getLocReg(), &RedDSP::GR32RegClass);
      InVals.push_back(DAG.getCopyFromReg(Chain, DL, VReg, MVT::i32));
    } else {
      int FI =
          MF.getFrameInfo().CreateFixedObject(4, VA.getLocMemOffset(), true);
      SDValue FIN = DAG.getFrameIndex(FI, MVT::i32);
      InVals.push_back(DAG.getLoad(MVT::i32, DL, Chain, FIN,
                                   MachinePointerInfo::getFixedStack(MF, FI)));
    }
  }
  return Chain;
}

bool RedDSPTargetLowering::CanLowerReturn(
    CallingConv::ID CC, MachineFunction &MF, bool IsVarArg,
    const SmallVectorImpl<ISD::OutputArg> &Outs, LLVMContext &Context,
    const Type *) const {
  SmallVector<CCValAssign, 4> Locs;
  CCState State(CC, IsVarArg, MF, Locs, Context);
  return State.CheckReturn(Outs, RetCC_RedDSP);
}

SDValue
RedDSPTargetLowering::LowerReturn(SDValue Chain, CallingConv::ID CC,
                                  bool IsVarArg,
                                  const SmallVectorImpl<ISD::OutputArg> &Outs,
                                  const SmallVectorImpl<SDValue> &OutVals,
                                  const SDLoc &DL, SelectionDAG &DAG) const {
  MachineFunction &MF = DAG.getMachineFunction();
  SmallVector<CCValAssign, 4> Locs;
  CCState State(CC, IsVarArg, MF, Locs, *DAG.getContext());
  State.AnalyzeReturn(Outs, RetCC_RedDSP);
  SDValue Glue;
  SmallVector<SDValue, 4> Ops(1, Chain);
  for (unsigned I = 0; I < Locs.size(); ++I) {
    Chain = DAG.getCopyToReg(Chain, DL, Locs[I].getLocReg(), OutVals[I], Glue);
    Glue = Chain.getValue(1);
    Ops.push_back(DAG.getRegister(Locs[I].getLocReg(), Locs[I].getLocVT()));
  }
  Ops[0] = Chain;
  if (Glue)
    Ops.push_back(Glue);
  return DAG.getNode(RedDSPISD::RET_FLAG, DL, MVT::Other, Ops);
}