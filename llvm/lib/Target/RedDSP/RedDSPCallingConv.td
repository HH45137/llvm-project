def CC_RedDSP : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R2, R3, R4, R5]>>,
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def RetCC_RedDSP : CallingConv<[
  CCIfType<[i32], CCAssignToReg<[R2]>>
]>;