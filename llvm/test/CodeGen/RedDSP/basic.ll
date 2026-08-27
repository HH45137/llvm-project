; RUN: llc -mtriple=reddsp-unknown-none -filetype=asm < %s | FileCheck %s

define i32 @add_reg(i32 %a, i32 %b) {
; CHECK-LABEL: add_reg:
; CHECK: ADD.INT R2 R2 R3 X
; CHECK: RET R15 X X X
  %v = add i32 %a, %b
  ret i32 %v
}

define i32 @alu_imm(i32 %a) {
; CHECK-LABEL: alu_imm:
; CHECK: ADD.INT.IMM R2 R2 X 7
  %v = add i32 %a, 7
  ret i32 %v
}

define i32 @bit_ops(i32 %a, i32 %b) {
; CHECK-LABEL: bit_ops:
; CHECK: XOR
  %x = and i32 %a, %b
  %y = or i32 %x, 3
  %z = xor i32 %y, %b
  ret i32 %z
}

define i32 @constant() {
; CHECK-LABEL: constant:
; CHECK: ADD.INT.IMM R2 R0 X 42
  ret i32 42
}

define i32 @load_word(ptr %p) {
; CHECK-LABEL: load_word:
; CHECK: LD R2 R2 X 0
  %v = load i32, ptr %p, align 4
  ret i32 %v
}

define void @store_word(ptr %p, i32 %v) {
; CHECK-LABEL: store_word:
; CHECK: ST R2 R3 X 0
  store i32 %v, ptr %p, align 4
  ret void
}