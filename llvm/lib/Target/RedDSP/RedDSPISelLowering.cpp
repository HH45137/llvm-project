#include "RedDSPISelLowering.h"
#include "RedDSPSubtarget.h"
#include "RedDSPTargetMachine.h"
#include "llvm/CodeGen/CallingConvLower.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/SelectionDAG.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include <utility>

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
  setOperationAction(ISD::SREM, MVT::i32, Expand);
  setOperationAction(ISD::UREM, MVT::i32, Expand);
  setOperationAction(ISD::SDIV, MVT::i32, Legal);
  setOperationAction(ISD::UDIV, MVT::i32, Legal);
  setOperationAction(ISD::SDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::UDIVREM, MVT::i32, Expand);
  setOperationAction(ISD::SETCC, MVT::i32, Custom);
  setOperationAction(ISD::BRCOND, MVT::Other, Custom);
  setOperationAction(ISD::BR_CC, MVT::i32, Custom);
  setOperationAction(ISD::SELECT, MVT::i32, Legal);
  setOperationAction(ISD::SELECT_CC, MVT::i32, Custom);
  setOperationAction(ISD::BR_JT, MVT::Other, Expand);
  setOperationAction(ISD::DYNAMIC_STACKALLOC, MVT::i32, Expand);
  setOperationAction(ISD::STACKSAVE, MVT::Other, Expand);
  setOperationAction(ISD::STACKRESTORE, MVT::Other, Expand);
}

SDValue RedDSPTargetLowering::LowerOperation(SDValue Op,
                                             SelectionDAG &DAG) const {
  SDLoc DL(Op);
  auto lowerSignedComparison = [&](ISD::CondCode CC, SDValue LHS,
                                   SDValue RHS) -> SDValue {
    bool Invert = false;
    switch (CC) {
    case ISD::SETGE:
      break;
    case ISD::SETLE:
      std::swap(LHS, RHS);
      break;
    case ISD::SETLT:
      Invert = true;
      break;
    case ISD::SETGT:
      std::swap(LHS, RHS);
      Invert = true;
      break;
    default:
      return SDValue();
    }

    SDValue Result = DAG.getNode(RedDSPISD::CBE, DL, MVT::i32, LHS, RHS);
    if (Invert)
      Result = DAG.getNode(ISD::XOR, DL, MVT::i32, Result,
                           DAG.getConstant(1, DL, MVT::i32));
    return Result;
  };

  switch (Op.getOpcode()) {
  case ISD::SELECT_CC: {
    SDValue LHS = Op.getOperand(0);
    SDValue RHS = Op.getOperand(1);
    SDValue TrueVal = Op.getOperand(2);
    SDValue FalseVal = Op.getOperand(3);
    ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(4))->get();

    SDValue Cond = lowerSignedComparison(CC, LHS, RHS);
    if (!Cond) {
      if (CC == ISD::SETEQ || CC == ISD::SETNE) {
        Cond = DAG.getNode(RedDSPISD::CMP, DL, MVT::i32, LHS, RHS);
        if (CC == ISD::SETNE)
          Cond = DAG.getNode(ISD::XOR, DL, MVT::i32, Cond,
                             DAG.getConstant(1, DL, MVT::i32));
      } else {
        report_fatal_error(
            "RED DSP does not support this comparison for select_cc");
      }
    }
    return DAG.getNode(ISD::SELECT, DL, Op.getValueType(), Cond, TrueVal,
                       FalseVal);
  }
  case ISD::BR_CC: {
    ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(1))->get();

    if (SDValue Cond =
            lowerSignedComparison(CC, Op.getOperand(2), Op.getOperand(3))) {
      return DAG.getNode(RedDSPISD::BRCOND, DL, MVT::Other, Op.getOperand(0),
                         Cond, Op.getOperand(4));
    }

    if (CC == ISD::SETEQ || CC == ISD::SETNE) {
      SDValue Cond = DAG.getNode(RedDSPISD::CMP, DL, MVT::i32,
                                 Op.getOperand(2), Op.getOperand(3));
      if (CC == ISD::SETNE)
        Cond = DAG.getNode(ISD::XOR, DL, MVT::i32, Cond,
                           DAG.getConstant(1, DL, MVT::i32));
      return DAG.getNode(RedDSPISD::BRCOND, DL, MVT::Other, Op.getOperand(0),
                         Cond, Op.getOperand(4));
    }

    report_fatal_error("RED DSP does not support this branch condition");
  }
  case ISD::SETCC: {
    ISD::CondCode CC = cast<CondCodeSDNode>(Op.getOperand(2))->get();

    if (SDValue Result =
            lowerSignedComparison(CC, Op.getOperand(0), Op.getOperand(1)))
      return Result;

    if (CC == ISD::SETEQ) {
      SDValue Cmp = DAG.getNode(RedDSPISD::CMP, DL, MVT::i32, Op.getOperand(0),
                                Op.getOperand(1));
      return Cmp;
    }

    if (CC == ISD::SETNE) {
      SDValue Cmp = DAG.getNode(RedDSPISD::CMP, DL, MVT::i32, Op.getOperand(0),
                                Op.getOperand(1));

      return DAG.getNode(ISD::XOR, DL, MVT::i32, Cmp,
                         DAG.getConstant(1, DL, MVT::i32));
    }

    report_fatal_error("RED DSP does not support this integer comparison");
  }
  case ISD::BRCOND:
    return DAG.getNode(RedDSPISD::BRCOND, DL, MVT::Other, Op.getOperand(0),
                       Op.getOperand(1), Op.getOperand(2));
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

MachineBasicBlock *
RedDSPTargetLowering::EmitInstrWithCustomInserter(MachineInstr &MI,
                                                  MachineBasicBlock *BB) const {
  const TargetInstrInfo &TII = *BB->getParent()->getSubtarget().getInstrInfo();
  DebugLoc DL = MI.getDebugLoc();
  assert(MI.getOpcode() == RedDSP::SELECT && "Unexpected instr to insert");

  Register Dst = MI.getOperand(0).getReg();
  Register Cond = MI.getOperand(1).getReg();
  Register TrueVal = MI.getOperand(2).getReg();
  Register FalseVal = MI.getOperand(3).getReg();

  const BasicBlock *LLVM_BB = BB->getBasicBlock();
  MachineFunction::iterator It = ++BB->getIterator();
  MachineFunction *F = BB->getParent();
  MachineBasicBlock *Copy0MBB = F->CreateMachineBasicBlock(LLVM_BB);
  MachineBasicBlock *Copy1MBB = F->CreateMachineBasicBlock(LLVM_BB);
  F->insert(It, Copy0MBB);
  F->insert(It, Copy1MBB);

  Copy1MBB->splice(Copy1MBB->begin(), BB,
                   std::next(MachineBasicBlock::iterator(MI)), BB->end());
  Copy1MBB->transferSuccessorsAndUpdatePHIs(BB);

  BB->addSuccessor(Copy0MBB);
  BB->addSuccessor(Copy1MBB);
  BuildMI(BB, DL, TII.get(RedDSP::BNE)).addReg(Cond).addReg(RedDSP::R0).addMBB(Copy1MBB);

  Copy0MBB->addSuccessor(Copy1MBB);

  BuildMI(*Copy1MBB, Copy1MBB->begin(), DL, TII.get(RedDSP::PHI), Dst)
      .addReg(FalseVal)
      .addMBB(Copy0MBB)
      .addReg(TrueVal)
      .addMBB(BB);

  MI.eraseFromParent();
  return Copy1MBB;
}