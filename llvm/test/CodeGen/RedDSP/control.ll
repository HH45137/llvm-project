; RUN: llc -mtriple=reddsp-unknown-none -filetype=asm < %s | FileCheck %s

define i32 @eq_branch(i32 %a, i32 %b) {
; CHECK-LABEL: eq_branch:
; CHECK: CMP
; CHECK: BNE {{R[0-9]+}} R0 X .LBB0_2
entry:
  %c = icmp eq i32 %a, %b
  br i1 %c, label %yes, label %no
yes:
  ret i32 1
no:
  ret i32 0
}

define i32 @ne_value(i32 %a, i32 %b) {
; CHECK-LABEL: ne_value:
; CHECK: CMP
; CHECK: XOR.IMM {{R[0-9]+}} {{R[0-9]+}} X 1
  %c = icmp ne i32 %a, %b
  %v = zext i1 %c to i32
  ret i32 %v
}

define i32 @eq_imm(i32 %a) {
; CHECK-LABEL: eq_imm:
; CHECK: CMP.IMM {{R[0-9]+}} R2 X 7
  %c = icmp eq i32 %a, 7
  %v = zext i1 %c to i32
  ret i32 %v
}

define i32 @ge_value(i32 %a, i32 %b) {
; CHECK-LABEL: ge_value:
; CHECK: CBE
  %c = icmp sge i32 %a, %b
  %v = zext i1 %c to i32
  ret i32 %v
}

define i32 @ge_branch(i32 %a, i32 %b) {
; CHECK-LABEL: ge_branch:
; CHECK: CBE
; CHECK: BNE
  %c = icmp sge i32 %a, %b
  br i1 %c, label %yes, label %no
yes:
  ret i32 1
no:
  ret i32 0
}

define i32 @le_value(i32 %a, i32 %b) {
; CHECK-LABEL: le_value:
; CHECK: CBE {{R[0-9]+}} R3 R2 X
  %c = icmp sle i32 %a, %b
  %v = zext i1 %c to i32
  ret i32 %v
}

define i32 @mac_reg(i32 %acc, i32 %a, i32 %b) {
; CHECK-LABEL: mac_reg:
; CHECK: MAC.INT
  %m = mul i32 %a, %b
  %r = add i32 %acc, %m
  ret i32 %r
}

define i32 @mac_imm(i32 %acc, i32 %a) {
; CHECK-LABEL: mac_imm:
; CHECK: MAC.INT.IMM
  %m = mul i32 %a, 7
  %r = add i32 %acc, %m
  ret i32 %r
}