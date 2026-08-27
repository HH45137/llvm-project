def simm9 : Operand<i32>;
def uimm9 : Operand<i32>;
def uimm5 : Operand<i32>;
def brtarget : Operand<OtherVT>;
def calltarget : Operand<i32>;

def simm9_node : PatLeaf<(imm), [{ return isInt<9>(N->getSExtValue()); }]>;
def uimm9_node : PatLeaf<(imm), [{ return isUInt<9>(N->getZExtValue()); }]>;
def uimm5_node : PatLeaf<(imm), [{ return isUInt<5>(N->getZExtValue()); }]>;
def SDT_RedDSPCall : SDTypeProfile<0, 1, [SDTCisVT<0, i32>]>;
def RedDSPRet : SDNode<"RedDSPISD::RET_FLAG", SDTNone,
                       [SDNPHasChain, SDNPOptInGlue, SDNPVariadic]>;
def RedDSPCall : SDNode<"RedDSPISD::CALL", SDT_RedDSPCall,
                        [SDNPHasChain, SDNPOptInGlue, SDNPOutGlue, SDNPVariadic]>;
def RedDSPBrCond : SDNode<"RedDSPISD::BRCOND", SDTBrcond,
                          [SDNPHasChain]>;
def RedDSPCmp : SDNode<"RedDSPISD::CMP", SDTIntBinOp>;
def RedDSPCBE : SDNode<"RedDSPISD::CBE", SDTIntBinOp>;

class RedDSPInst<dag Outs, dag Ins, string Asm, list<dag> Pats = []>
    : Instruction {
  let Namespace = "RedDSP";
  let OutOperandList = Outs;
  let InOperandList = Ins;
  let AsmString = Asm;
  let Pattern = Pats;
  let Size = 4;
}

class RR<string Mnemonic, SDPatternOperator Op> :
  RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, GR32:$src2),
             Mnemonic#" $dst $src1 $src2 X",
             [(set GR32:$dst, (Op GR32:$src1, GR32:$src2))]>;

def ADDrr : RR<"ADD.INT", add>;
def SUBrr : RR<"SUB.INT", sub>;
def MULrr : RR<"MUL.INT", mul>;
def DIVrr : RR<"DIV.INT", sdiv>;
def ANDrr : RR<"AND", and>;
def ORrr  : RR<"OR", or>;
def XORrr : RR<"XOR", xor>;
def SFLrr : RR<"SFL", shl>;
def SFRrr : RR<"SFR", sra>;

def MACrr : RedDSPInst<(outs GR32:$dst),
    (ins GR32:$acc, GR32:$src1, GR32:$src2),
    "MAC.INT $dst $src1 $src2 X",
    [(set GR32:$dst,
      (add GR32:$acc, (mul GR32:$src1, GR32:$src2)))]> {
  let Constraints = "$dst = $acc";
}

def CMPrr : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, GR32:$src2),
  "CMP $dst $src1 $src2 X",
  [(set GR32:$dst, (RedDSPCmp GR32:$src1, GR32:$src2))]>;

def CBErr : RedDSPInst<
      (outs GR32:$dst),
      (ins GR32:$src1, GR32:$src2),
      "CBE $dst $src1 $src2 X",
      [(set GR32:$dst, (RedDSPCBE GR32:$src1, GR32:$src2))]
    >;

def SELECT : RedDSPInst<(outs GR32:$dst),
                        (ins GR32:$cond, GR32:$truev, GR32:$falsev),
                        "# SELECT PSEUDO",
                        [(set GR32:$dst, (select GR32:$cond, GR32:$truev, GR32:$falsev))]> {
  let usesCustomInserter = 1;
}

def ADDri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, simm9:$imm),
  "ADD.INT.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (add GR32:$src1, simm9_node:$imm))]>;
def SUBri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, simm9:$imm),
  "SUB.INT.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (sub GR32:$src1, simm9_node:$imm))]>;
def MULri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, simm9:$imm),
  "MUL.INT.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (mul GR32:$src1, simm9_node:$imm))]>;
def DIVri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, simm9:$imm),
  "DIV.INT.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (sdiv GR32:$src1, simm9_node:$imm))]>;
def ANDri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, uimm9:$imm),
  "AND.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (and GR32:$src1, uimm9_node:$imm))]>;
def ORri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, uimm9:$imm),
  "OR.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (or GR32:$src1, uimm9_node:$imm))]>;
def XORri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, uimm9:$imm),
  "XOR.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (xor GR32:$src1, uimm9_node:$imm))]>;
def SFLri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, uimm5:$imm),
  "SFL.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (shl GR32:$src1, uimm5_node:$imm))]>;
def SFRri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, uimm5:$imm),
  "SFR.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (sra GR32:$src1, uimm5_node:$imm))]>;

def : Pat<(srl GR32:$src1, GR32:$src2), (SFRrr GR32:$src1, GR32:$src2)>;
def : Pat<(srl GR32:$src1, uimm5_node:$imm), (SFRri GR32:$src1, uimm5_node:$imm)>;

def : Pat<(udiv GR32:$src1, GR32:$src2), (DIVrr GR32:$src1, GR32:$src2)>;
def : Pat<(udiv GR32:$src1, simm9_node:$imm), (DIVri GR32:$src1, simm9_node:$imm)>;
def : Pat<(srem GR32:$src1, GR32:$src2),
          (SUBrr GR32:$src1, (MULrr (DIVrr GR32:$src1, GR32:$src2), GR32:$src2))>;
def : Pat<(urem GR32:$src1, GR32:$src2),
          (SUBrr GR32:$src1, (MULrr (DIVrr GR32:$src1, GR32:$src2), GR32:$src2))>;
def : Pat<(srem GR32:$src1, simm9_node:$imm),
          (SUBrr GR32:$src1, (MULri (DIVri GR32:$src1, simm9_node:$imm), simm9_node:$imm))>;
def : Pat<(urem GR32:$src1, simm9_node:$imm),
          (SUBrr GR32:$src1, (MULri (DIVri GR32:$src1, simm9_node:$imm), simm9_node:$imm))>;

def MACri : RedDSPInst<(outs GR32:$dst),
    (ins GR32:$acc, GR32:$src1, simm9:$imm),
    "MAC.INT.IMM $dst $src1 X $imm",
    [(set GR32:$dst,
      (add GR32:$acc, (mul GR32:$src1, simm9_node:$imm)))]> {
  let Constraints = "$dst = $acc";
}

def CMPri : RedDSPInst<(outs GR32:$dst), (ins GR32:$src1, simm9:$imm),
  "CMP.IMM $dst $src1 X $imm",
  [(set GR32:$dst, (RedDSPCmp GR32:$src1, simm9_node:$imm))]>;

def MOVrr : RedDSPInst<(outs GR32:$dst), (ins GR32:$src),
  "ADD.INT $dst $src R0 X", []>;
def MOVi : RedDSPInst<(outs GR32:$dst), (ins simm9:$imm),
  "ADD.INT.IMM $dst R0 X $imm",
  [(set GR32:$dst, simm9_node:$imm)]>;

def LD : RedDSPInst<(outs GR32:$dst), (ins GR32:$base, simm9:$offset),
  "LD $dst $base X $offset", []> { let mayLoad = 1; }
def ST : RedDSPInst<(outs), (ins GR32:$base, GR32:$src, simm9:$offset),
  "ST $base $src X $offset", []> { let mayStore = 1; }

def : Pat<(i32 (load GR32:$base)), (LD GR32:$base, 0)>;
def : Pat<(store GR32:$src, GR32:$base), (ST GR32:$base, GR32:$src, 0)>;

def BEQ : RedDSPInst<(outs),
    (ins GR32:$src1, GR32:$src2, brtarget:$target),
    "BEQ $target $src1 $src2", []> {
  let isBranch = 1;
  let isTerminator = 1;
}
def BNE : RedDSPInst<(outs),
    (ins GR32:$src1, GR32:$src2, brtarget:$target),
    "BNE $target $src1 $src2", []> {
  let isBranch = 1;
  let isTerminator = 1;
}
def BR : RedDSPInst<(outs), (ins brtarget:$target),
    "BEQ $target R0 R0", [(br bb:$target)]> {
  let isBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
}
def : Pat<(RedDSPBrCond GR32:$cond, bb:$target),
          (BNE GR32:$cond, R0, bb:$target)>;

def JMP : RedDSPInst<(outs), (ins GR32:$target),
    "JMP $target X X X", []> {
  let isBranch = 1;
  let isIndirectBranch = 1;
  let isTerminator = 1;
  let isBarrier = 1;
}

def CALL : RedDSPInst<(outs), (ins GR32:$target),
    "CALL R15 $target X X", []> {
  let isCall = 1;
  let Defs = [R15];
}

def CALL_SYM : RedDSPInst<(outs), (ins calltarget:$target),
    "CALL R15 $target X X", [(RedDSPCall tglobaladdr:$target)]> {
  let isCall = 1;
  let Defs = [R15];
}
def : Pat<(RedDSPCall texternalsym:$target), (CALL_SYM texternalsym:$target)>;
def : Pat<(RedDSPCall GR32:$target), (CALL GR32:$target)>;

def NOP : RedDSPInst<(outs), (ins), "NOP", []>;

def RET : RedDSPInst<(outs), (ins), "RET R15 X X X", []> {
  let isReturn = 1;
  let isTerminator = 1;
  let isBarrier = 1;
  let Uses = [R15];
}
def : Pat<(RedDSPRet), (RET)>;