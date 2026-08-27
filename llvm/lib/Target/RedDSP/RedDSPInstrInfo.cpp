#include "RedDSPInstrInfo.h"
#include "RedDSPSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"

using namespace llvm;
#define GET_INSTRINFO_CTOR_DTOR
#include "RedDSPGenInstrInfo.inc"

RedDSPInstrInfo::RedDSPInstrInfo(const RedDSPSubtarget &STI)
    : RedDSPGenInstrInfo(STI, RI, ~0U, ~0U, ~0U, RedDSP::RET), RI() {}

void RedDSPInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I,
                                  const DebugLoc &DL, Register Dst,
                                  Register Src, bool KillSrc, bool,
                                  bool) const {
  BuildMI(MBB, I, DL, get(RedDSP::MOVrr), Dst)
      .addReg(Src, getKillRegState(KillSrc));
}

void RedDSPInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator I,
                                          Register Src, bool IsKill, int FI,
                                          const TargetRegisterClass *, Register,
                                          MachineInstr::MIFlag Flags) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOStore,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  BuildMI(MBB, I, DebugLoc(), get(RedDSP::ST))
      .addFrameIndex(FI)
      .addReg(Src, getKillRegState(IsKill))
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlag(Flags);
}

void RedDSPInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator I,
                                           Register Dst, int FI,
                                           const TargetRegisterClass *,
                                           Register, unsigned,
                                           MachineInstr::MIFlag Flags) const {
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = MF.getFrameInfo();
  MachineMemOperand *MMO = MF.getMachineMemOperand(
      MachinePointerInfo::getFixedStack(MF, FI), MachineMemOperand::MOLoad,
      MFI.getObjectSize(FI), MFI.getObjectAlign(FI));
  BuildMI(MBB, I, DebugLoc(), get(RedDSP::LD), Dst)
      .addFrameIndex(FI)
      .addImm(0)
      .addMemOperand(MMO)
      .setMIFlag(Flags);
}

bool RedDSPInstrInfo::analyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify) const {
  TBB = FBB = nullptr;
  Cond.clear();
  auto I = MBB.getLastNonDebugInstr();
  if (I == MBB.end() || !I->isTerminator())
    return false;
  if (I->getOpcode() == RedDSP::BR) {
    MachineBasicBlock *UncondTarget = I->getOperand(0).getMBB();
    if (I == MBB.begin()) {
      TBB = UncondTarget;
      return false;
    }
    auto Prev = std::prev(I);
    if (Prev->getOpcode() != RedDSP::BEQ && Prev->getOpcode() != RedDSP::BNE) {
      TBB = UncondTarget;
      return false;
    }
    TBB = Prev->getOperand(2).getMBB();
    FBB = UncondTarget;
    Cond.push_back(MachineOperand::CreateImm(Prev->getOpcode()));
    Cond.push_back(Prev->getOperand(0));
    Cond.push_back(Prev->getOperand(1));
    return false;
  }
  if (I->getOpcode() != RedDSP::BEQ && I->getOpcode() != RedDSP::BNE)
    return true;
  TBB = I->getOperand(2).getMBB();
  Cond.push_back(MachineOperand::CreateImm(I->getOpcode()));
  Cond.push_back(I->getOperand(0));
  Cond.push_back(I->getOperand(1));
  return false;
}

unsigned RedDSPInstrInfo::removeBranch(MachineBasicBlock &MBB,
                                       int *BytesRemoved) const {
  unsigned Count = 0;
  while (!MBB.empty()) {
    auto I = MBB.getLastNonDebugInstr();
    if (I == MBB.end() ||
        (I->getOpcode() != RedDSP::BR && I->getOpcode() != RedDSP::BEQ &&
         I->getOpcode() != RedDSP::BNE))
      break;
    I->eraseFromParent();
    ++Count;
  }
  if (BytesRemoved)
    *BytesRemoved = Count * 4;
  return Count;
}

unsigned RedDSPInstrInfo::insertBranch(
    MachineBasicBlock &MBB, MachineBasicBlock *TBB, MachineBasicBlock *FBB,
    ArrayRef<MachineOperand> Cond, const DebugLoc &DL, int *BytesAdded) const {
  assert(TBB && "branch target required");
  unsigned Count = 0;
  if (Cond.empty()) {
    BuildMI(&MBB, DL, get(RedDSP::BR)).addMBB(TBB);
    Count = 1;
  } else {
    BuildMI(&MBB, DL, get(Cond[0].getImm()))
        .add(Cond[1])
        .add(Cond[2])
        .addMBB(TBB);
    Count = 1;
    if (FBB) {
      BuildMI(&MBB, DL, get(RedDSP::BR)).addMBB(FBB);
      ++Count;
    }
  }
  if (BytesAdded)
    *BytesAdded = Count * 4;
  return Count;
}

bool RedDSPInstrInfo::reverseBranchCondition(
    SmallVectorImpl<MachineOperand> &Cond) const {
  if (Cond.empty())
    return true;
  Cond[0].setImm(Cond[0].getImm() == RedDSP::BEQ ? RedDSP::BNE : RedDSP::BEQ);
  return false;
}