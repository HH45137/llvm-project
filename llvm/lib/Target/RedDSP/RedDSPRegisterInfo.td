class RedDSPReg<bits<16> Enc, string Name> : Register<Name> {
  let Namespace = "RedDSP";
  let HWEncoding = Enc;
}

foreach I = 0-15 in {
  def R#I : RedDSPReg<I, "R"#I>, DwarfRegNum<[I]>;
}

def GR32 : RegisterClass<"RedDSP", [i32], 32,
  (add (sequence "R%i", 2, 15))>;