#include "RedDSPFrameLowering.h"
#include "RedDSP.h"
#include "RedDSPInstrInfo.h"
#include "RedDSPSubtarget.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Support/ErrorHandling.h"

using namespace llvm;

void RedDSPFrameLowering::emitPrologue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  uint64_t Size = MF.getFrameInfo().getStackSize();
  if (!Size) return;
  if (!isInt<9>(Size)) report_fatal_error("RED DSP stack frame exceeds 255 bytes");
  const auto &TII = *MF.getSubtarget<RedDSPSubtarget>().getInstrInfo();
  BuildMI(MBB, MBB.begin(), DebugLoc(), TII.get(RedDSP::SUBri), RedDSP::R1)
      .addReg(RedDSP::R1).addImm(Size).setMIFlag(MachineInstr::FrameSetup);
}

void RedDSPFrameLowering::emitEpilogue(MachineFunction &MF,
                                        MachineBasicBlock &MBB) const {
  uint64_t Size = MF.getFrameInfo().getStackSize();
  if (!Size) return;
  const auto &TII = *MF.getSubtarget<RedDSPSubtarget>().getInstrInfo();
  auto I = MBB.getFirstTerminator();
  BuildMI(MBB, I, DebugLoc(), TII.get(RedDSP::ADDri), RedDSP::R1)
      .addReg(RedDSP::R1).addImm(Size).setMIFlag(MachineInstr::FrameDestroy);
}