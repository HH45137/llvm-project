#include "RedDSPRegisterInfo.h"
#include "RedDSP.h"
#include "RedDSPFrameLowering.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;
#define GET_REGINFO_TARGET_DESC
#include "RedDSPGenRegisterInfo.inc"

RedDSPRegisterInfo::RedDSPRegisterInfo() : RedDSPGenRegisterInfo(RedDSP::R15) {}

const MCPhysReg *RedDSPRegisterInfo::getCalleeSavedRegs(const MachineFunction *) const {
  static const MCPhysReg Regs[] = {RedDSP::R10, RedDSP::R11, RedDSP::R12,
                                   RedDSP::R13, 0};
  return Regs;
}

BitVector RedDSPRegisterInfo::getReservedRegs(const MachineFunction &) const {
  BitVector Reserved(getNumRegs());
  Reserved.set(RedDSP::R0);
  Reserved.set(RedDSP::R1);
  Reserved.set(RedDSP::R15);
  return Reserved;
}

const TargetRegisterClass *RedDSPRegisterInfo::getPointerRegClass(unsigned) const {
  return &RedDSP::GR32RegClass;
}

bool RedDSPRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                              int SPAdj, unsigned FIOperandNum,
                                              RegScavenger *) const {
  assert(SPAdj == 0 && "unexpected stack adjustment");
  MachineInstr &MI = *II;
  MachineFunction &MF = *MI.getParent()->getParent();
  int FI = MI.getOperand(FIOperandNum).getIndex();
  int64_t Offset = MF.getFrameInfo().getObjectOffset(FI) +
                   MF.getFrameInfo().getStackSize() +
                   MI.getOperand(FIOperandNum + 1).getImm();
  if (!isInt<9>(Offset))
    report_fatal_error("RED DSP stack offset exceeds signed 9-bit range");
  MI.getOperand(FIOperandNum).ChangeToRegister(RedDSP::R1, false);
  MI.getOperand(FIOperandNum + 1).ChangeToImmediate(Offset);
  return false;
}

Register RedDSPRegisterInfo::getFrameRegister(const MachineFunction &) const {
  return RedDSP::R1;
}