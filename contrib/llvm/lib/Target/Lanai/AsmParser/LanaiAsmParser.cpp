//===-- LanaiAsmParser.cpp - Parse Lanai assembly to MCInst instructions --===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Lanai.h"
#include "MCTargetDesc/LanaiMCExpr.h"
#include "MCTargetDesc/LanaiMCTargetDesc.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/MC/MCContext.h"
#include "llvm/MC/MCExpr.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCParser/MCAsmLexer.h"
#include "llvm/MC/MCParser/MCParsedAsmOperand.h"
#include "llvm/MC/MCParser/MCTargetAsmParser.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Support/MathExtras.h"
#include "llvm/Support/TargetRegistry.h"

namespace llvm {
namespace {
struct LanaiOperand;

class LanaiAsmParser : public MCTargetAsmParser {
  // Parse operands
  std::unique_ptr<LanaiOperand> parseRegister();

  std::unique_ptr<LanaiOperand> parseImmediate();

  std::unique_ptr<LanaiOperand> parseIdentifier();

  unsigned parseAluOperator(bool PreOp, bool PostOp);

  // Split the mnemonic stripping conditional code and quantifiers
  StringRef splitMnemonic(StringRef Name, SMLoc NameLoc,
                          OperandVector *Operands);

  bool parsePrePost(StringRef Type, int *OffsetValue);

  bool ParseDirective(AsmToken DirectiveID) override;

  bool ParseInstruction(ParseInstructionInfo &Info, StringRef Name,
                        SMLoc NameLoc, OperandVector &Operands) override;

  bool ParseRegister(unsigned &RegNum, SMLoc &StartLoc, SMLoc &EndLoc) override;

  bool MatchAndEmitInstruction(SMLoc IdLoc, unsigned &Opcode,
                               OperandVector &Operands, MCStreamer &Out,
                               uint64_t &ErrorInfo,
                               bool MatchingInlineAsm) override;

// Auto-generated instruction matching functions
#define GET_ASSEMBLER_HEADER
#include "LanaiGenAsmMatcher.inc"

  OperandMatchResultTy parseOperand(OperandVector *Operands,
                                    StringRef Mnemonic);

  OperandMatchResultTy parseMemoryOperand(OperandVector &Operands);

public:
  LanaiAsmParser(const MCSubtargetInfo &STI, MCAsmParser &Parser,
                 const MCInstrInfo &MII, const MCTargetOptions &Options)
      : MCTargetAsmParser(Options, STI), Parser(Parser),
        Lexer(Parser.getLexer()), SubtargetInfo(STI) {
    setAvailableFeatures(
        ComputeAvailableFeatures(SubtargetInfo.getFeatureBits()));
  }

private:
  MCAsmParser &Parser;
  MCAsmLexer &Lexer;

  const MCSubtargetInfo &SubtargetInfo;
};

// Auto-generated by TableGen
static unsigned MatchRegisterName(llvm::StringRef Name);

// LanaiOperand - Instances of this class represented a parsed machine
// instruction
struct LanaiOperand : public MCParsedAsmOperand {
  enum KindTy {
    TOKEN,
    REGISTER,
    IMMEDIATE,
    MEMORY_IMM,
    MEMORY_REG_IMM,
    MEMORY_REG_REG,
  } Kind;

  SMLoc StartLoc, EndLoc;

  struct Token {
    const char *Data;
    unsigned Length;
  };

  struct RegOp {
    unsigned RegNum;
  };

  struct ImmOp {
    const MCExpr *Value;
  };

  struct MemOp {
    unsigned BaseReg;
    unsigned OffsetReg;
    unsigned AluOp;
    const MCExpr *Offset;
  };

  union {
    struct Token Tok;
    struct RegOp Reg;
    struct ImmOp Imm;
    struct MemOp Mem;
  };

  explicit LanaiOperand(KindTy Kind) : MCParsedAsmOperand(), Kind(Kind) {}

public:
  // The functions below are used by the autogenerated ASM matcher and hence to
  // be of the form expected.

  // getStartLoc - Gets location of the first token of this operand
  SMLoc getStartLoc() const override { return StartLoc; }

  // getEndLoc - Gets location of the last token of this operand
  SMLoc getEndLoc() const override { return EndLoc; }

  unsigned getReg() const override {
    assert(isReg() && "Invalid type access!");
    return Reg.RegNum;
  }

  const MCExpr *getImm() const {
    assert(isImm() && "Invalid type access!");
    return Imm.Value;
  }

  StringRef getToken() const {
    assert(isToken() && "Invalid type access!");
    return StringRef(Tok.Data, Tok.Length);
  }

  unsigned getMemBaseReg() const {
    assert(isMem() && "Invalid type access!");
    return Mem.BaseReg;
  }

  unsigned getMemOffsetReg() const {
    assert(isMem() && "Invalid type access!");
    return Mem.OffsetReg;
  }

  const MCExpr *getMemOffset() const {
    assert(isMem() && "Invalid type access!");
    return Mem.Offset;
  }

  unsigned getMemOp() const {
    assert(isMem() && "Invalid type access!");
    return Mem.AluOp;
  }

  // Functions for testing operand type
  bool isReg() const override { return Kind == REGISTER; }

  bool isImm() const override { return Kind == IMMEDIATE; }

  bool isMem() const override {
    return isMemImm() || isMemRegImm() || isMemRegReg();
  }

  bool isMemImm() const { return Kind == MEMORY_IMM; }

  bool isMemRegImm() const { return Kind == MEMORY_REG_IMM; }

  bool isMemRegReg() const { return Kind == MEMORY_REG_REG; }

  bool isMemSpls() const { return isMemRegImm() || isMemRegReg(); }

  bool isToken() const override { return Kind == TOKEN; }

  bool isBrImm() {
    if (!isImm())
      return false;

    // Constant case
    const MCConstantExpr *MCE = dyn_cast<MCConstantExpr>(Imm.Value);
    if (!MCE)
      return true;
    int64_t Value = MCE->getValue();
    // Check if value fits in 25 bits with 2 least significant bits 0.
    return isShiftedUInt<23, 2>(static_cast<int32_t>(Value));
  }

  bool isBrTarget() { return isBrImm() || isToken(); }

  bool isCallTarget() { return isImm() || isToken(); }

  bool isHiImm16() {
    if (!isImm())
      return false;

    // Constant case
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value)) {
      int64_t Value = ConstExpr->getValue();
      return Value != 0 && isShiftedUInt<16, 16>(Value);
    }

    // Symbolic reference expression
    if (const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(Imm.Value))
      return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_HI;

    // Binary expression
    if (const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(Imm.Value))
      if (const LanaiMCExpr *SymbolRefExpr =
              dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS()))
        return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_HI;

    return false;
  }

  bool isHiImm16And() {
    if (!isImm())
      return false;

    const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value);
    if (ConstExpr) {
      int64_t Value = ConstExpr->getValue();
      // Check if in the form 0xXYZWffff
      return (Value != 0) && ((Value & ~0xffff0000) == 0xffff);
    }
    return false;
  }

  bool isLoImm16() {
    if (!isImm())
      return false;

    // Constant case
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value)) {
      int64_t Value = ConstExpr->getValue();
      // Check if value fits in 16 bits
      return isUInt<16>(static_cast<int32_t>(Value));
    }

    // Symbolic reference expression
    if (const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(Imm.Value))
      return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_LO;

    // Binary expression
    if (const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(Imm.Value))
      if (const LanaiMCExpr *SymbolRefExpr =
              dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS()))
        return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_LO;

    return false;
  }

  bool isLoImm16Signed() {
    if (!isImm())
      return false;

    // Constant case
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value)) {
      int64_t Value = ConstExpr->getValue();
      // Check if value fits in 16 bits or value of the form 0xffffxyzw
      return isInt<16>(static_cast<int32_t>(Value));
    }

    // Symbolic reference expression
    if (const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(Imm.Value))
      return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_LO;

    // Binary expression
    if (const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(Imm.Value))
      if (const LanaiMCExpr *SymbolRefExpr =
              dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS()))
        return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_LO;

    return false;
  }

  bool isLoImm16And() {
    if (!isImm())
      return false;

    const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value);
    if (ConstExpr) {
      int64_t Value = ConstExpr->getValue();
      // Check if in the form 0xffffXYZW
      return ((Value & ~0xffff) == 0xffff0000);
    }
    return false;
  }

  bool isImmShift() {
    if (!isImm())
      return false;

    const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value);
    if (!ConstExpr)
      return false;
    int64_t Value = ConstExpr->getValue();
    return (Value >= -31) && (Value <= 31);
  }

  bool isLoImm21() {
    if (!isImm())
      return false;

    // Constant case
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value)) {
      int64_t Value = ConstExpr->getValue();
      return isUInt<21>(Value);
    }

    // Symbolic reference expression
    if (const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(Imm.Value))
      return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_None;
    if (const MCSymbolRefExpr *SymbolRefExpr =
            dyn_cast<MCSymbolRefExpr>(Imm.Value)) {
      return SymbolRefExpr->getKind() == MCSymbolRefExpr::VK_None;
    }

    // Binary expression
    if (const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(Imm.Value)) {
      if (const LanaiMCExpr *SymbolRefExpr =
              dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS()))
        return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_None;
      if (const MCSymbolRefExpr *SymbolRefExpr =
              dyn_cast<MCSymbolRefExpr>(BinaryExpr->getLHS()))
        return SymbolRefExpr->getKind() == MCSymbolRefExpr::VK_None;
    }

    return false;
  }

  bool isImm10() {
    if (!isImm())
      return false;

    const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value);
    if (!ConstExpr)
      return false;
    int64_t Value = ConstExpr->getValue();
    return isInt<10>(Value);
  }

  bool isCondCode() {
    if (!isImm())
      return false;

    const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Imm.Value);
    if (!ConstExpr)
      return false;
    uint64_t Value = ConstExpr->getValue();
    // The condition codes are between 0 (ICC_T) and 15 (ICC_LE). If the
    // unsigned value of the immediate is less than LPCC::UNKNOWN (16) then
    // value corresponds to a valid condition code.
    return Value < LPCC::UNKNOWN;
  }

  void addExpr(MCInst &Inst, const MCExpr *Expr) const {
    // Add as immediates where possible. Null MCExpr = 0
    if (Expr == nullptr)
      Inst.addOperand(MCOperand::createImm(0));
    else if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Expr))
      Inst.addOperand(
          MCOperand::createImm(static_cast<int32_t>(ConstExpr->getValue())));
    else
      Inst.addOperand(MCOperand::createExpr(Expr));
  }

  void addRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getReg()));
  }

  void addImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addBrTargetOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addCallTargetOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addCondCodeOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addMemImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    const MCExpr *Expr = getMemOffset();
    addExpr(Inst, Expr);
  }

  void addMemRegImmOperands(MCInst &Inst, unsigned N) const {
    assert(N == 3 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getMemBaseReg()));
    const MCExpr *Expr = getMemOffset();
    addExpr(Inst, Expr);
    Inst.addOperand(MCOperand::createImm(getMemOp()));
  }

  void addMemRegRegOperands(MCInst &Inst, unsigned N) const {
    assert(N == 3 && "Invalid number of operands!");
    Inst.addOperand(MCOperand::createReg(getMemBaseReg()));
    assert(getMemOffsetReg() != 0 && "Invalid offset");
    Inst.addOperand(MCOperand::createReg(getMemOffsetReg()));
    Inst.addOperand(MCOperand::createImm(getMemOp()));
  }

  void addMemSplsOperands(MCInst &Inst, unsigned N) const {
    if (isMemRegImm())
      addMemRegImmOperands(Inst, N);
    if (isMemRegReg())
      addMemRegRegOperands(Inst, N);
  }

  void addImmShiftOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addImm10Operands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    addExpr(Inst, getImm());
  }

  void addLoImm16Operands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(getImm()))
      Inst.addOperand(
          MCOperand::createImm(static_cast<int32_t>(ConstExpr->getValue())));
    else if (isa<LanaiMCExpr>(getImm())) {
#ifndef NDEBUG
      const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(getImm());
      assert(SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_LO);
#endif
      Inst.addOperand(MCOperand::createExpr(getImm()));
    } else if (isa<MCBinaryExpr>(getImm())) {
#ifndef NDEBUG
      const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(getImm());
      assert(dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS()) &&
             dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS())->getKind() ==
                 LanaiMCExpr::VK_Lanai_ABS_LO);
#endif
      Inst.addOperand(MCOperand::createExpr(getImm()));
    } else
      assert(false && "Operand type not supported.");
  }

  void addLoImm16AndOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(getImm()))
      Inst.addOperand(MCOperand::createImm(ConstExpr->getValue() & 0xffff));
    else
      assert(false && "Operand type not supported.");
  }

  void addHiImm16Operands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(getImm()))
      Inst.addOperand(MCOperand::createImm(ConstExpr->getValue() >> 16));
    else if (isa<LanaiMCExpr>(getImm())) {
#ifndef NDEBUG
      const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(getImm());
      assert(SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_ABS_HI);
#endif
      Inst.addOperand(MCOperand::createExpr(getImm()));
    } else if (isa<MCBinaryExpr>(getImm())) {
#ifndef NDEBUG
      const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(getImm());
      assert(dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS()) &&
             dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS())->getKind() ==
                 LanaiMCExpr::VK_Lanai_ABS_HI);
#endif
      Inst.addOperand(MCOperand::createExpr(getImm()));
    } else
      assert(false && "Operand type not supported.");
  }

  void addHiImm16AndOperands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(getImm()))
      Inst.addOperand(MCOperand::createImm(ConstExpr->getValue() >> 16));
    else
      assert(false && "Operand type not supported.");
  }

  void addLoImm21Operands(MCInst &Inst, unsigned N) const {
    assert(N == 1 && "Invalid number of operands!");
    if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(getImm()))
      Inst.addOperand(MCOperand::createImm(ConstExpr->getValue() & 0x1fffff));
    else if (isa<LanaiMCExpr>(getImm())) {
#ifndef NDEBUG
      const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(getImm());
      assert(SymbolRefExpr &&
             SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_None);
#endif
      Inst.addOperand(MCOperand::createExpr(getImm()));
    } else if (isa<MCSymbolRefExpr>(getImm())) {
#ifndef NDEBUG
      const MCSymbolRefExpr *SymbolRefExpr =
          dyn_cast<MCSymbolRefExpr>(getImm());
      assert(SymbolRefExpr &&
             SymbolRefExpr->getKind() == MCSymbolRefExpr::VK_None);
#endif
      Inst.addOperand(MCOperand::createExpr(getImm()));
    } else if (isa<MCBinaryExpr>(getImm())) {
#ifndef NDEBUG
      const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(getImm());
      const LanaiMCExpr *SymbolRefExpr =
          dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS());
      assert(SymbolRefExpr &&
             SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_None);
#endif
      Inst.addOperand(MCOperand::createExpr(getImm()));
    } else
      assert(false && "Operand type not supported.");
  }

  void print(raw_ostream &OS) const override {
    switch (Kind) {
    case IMMEDIATE:
      OS << "Imm: " << getImm() << "\n";
      break;
    case TOKEN:
      OS << "Token: " << getToken() << "\n";
      break;
    case REGISTER:
      OS << "Reg: %r" << getReg() << "\n";
      break;
    case MEMORY_IMM:
      OS << "MemImm: " << *getMemOffset() << "\n";
      break;
    case MEMORY_REG_IMM:
      OS << "MemRegImm: " << getMemBaseReg() << "+" << *getMemOffset() << "\n";
      break;
    case MEMORY_REG_REG:
      assert(getMemOffset() == nullptr);
      OS << "MemRegReg: " << getMemBaseReg() << "+"
         << "%r" << getMemOffsetReg() << "\n";
      break;
    }
  }

  static std::unique_ptr<LanaiOperand> CreateToken(StringRef Str, SMLoc Start) {
    auto Op = make_unique<LanaiOperand>(TOKEN);
    Op->Tok.Data = Str.data();
    Op->Tok.Length = Str.size();
    Op->StartLoc = Start;
    Op->EndLoc = Start;
    return Op;
  }

  static std::unique_ptr<LanaiOperand> createReg(unsigned RegNum, SMLoc Start,
                                                 SMLoc End) {
    auto Op = make_unique<LanaiOperand>(REGISTER);
    Op->Reg.RegNum = RegNum;
    Op->StartLoc = Start;
    Op->EndLoc = End;
    return Op;
  }

  static std::unique_ptr<LanaiOperand> createImm(const MCExpr *Value,
                                                 SMLoc Start, SMLoc End) {
    auto Op = make_unique<LanaiOperand>(IMMEDIATE);
    Op->Imm.Value = Value;
    Op->StartLoc = Start;
    Op->EndLoc = End;
    return Op;
  }

  static std::unique_ptr<LanaiOperand>
  MorphToMemImm(std::unique_ptr<LanaiOperand> Op) {
    const MCExpr *Imm = Op->getImm();
    Op->Kind = MEMORY_IMM;
    Op->Mem.BaseReg = 0;
    Op->Mem.AluOp = LPAC::ADD;
    Op->Mem.OffsetReg = 0;
    Op->Mem.Offset = Imm;
    return Op;
  }

  static std::unique_ptr<LanaiOperand>
  MorphToMemRegReg(unsigned BaseReg, std::unique_ptr<LanaiOperand> Op,
                   unsigned AluOp) {
    unsigned OffsetReg = Op->getReg();
    Op->Kind = MEMORY_REG_REG;
    Op->Mem.BaseReg = BaseReg;
    Op->Mem.AluOp = AluOp;
    Op->Mem.OffsetReg = OffsetReg;
    Op->Mem.Offset = nullptr;
    return Op;
  }

  static std::unique_ptr<LanaiOperand>
  MorphToMemRegImm(unsigned BaseReg, std::unique_ptr<LanaiOperand> Op,
                   unsigned AluOp) {
    const MCExpr *Imm = Op->getImm();
    Op->Kind = MEMORY_REG_IMM;
    Op->Mem.BaseReg = BaseReg;
    Op->Mem.AluOp = AluOp;
    Op->Mem.OffsetReg = 0;
    Op->Mem.Offset = Imm;
    return Op;
  }
};

bool LanaiAsmParser::ParseDirective(AsmToken /*DirectiveId*/) { return true; }

bool LanaiAsmParser::MatchAndEmitInstruction(SMLoc IdLoc, unsigned &Opcode,
                                             OperandVector &Operands,
                                             MCStreamer &Out,
                                             uint64_t &ErrorInfo,
                                             bool MatchingInlineAsm) {
  MCInst Inst;
  SMLoc ErrorLoc;

  switch (MatchInstructionImpl(Operands, Inst, ErrorInfo, MatchingInlineAsm)) {
  case Match_Success:
    Out.EmitInstruction(Inst, SubtargetInfo);
    Opcode = Inst.getOpcode();
    return false;
  case Match_MissingFeature:
    return Error(IdLoc, "Instruction use requires option to be enabled");
  case Match_MnemonicFail:
    return Error(IdLoc, "Unrecognized instruction mnemonic");
  case Match_InvalidOperand: {
    ErrorLoc = IdLoc;
    if (ErrorInfo != ~0U) {
      if (ErrorInfo >= Operands.size())
        return Error(IdLoc, "Too few operands for instruction");

      ErrorLoc = ((LanaiOperand &)*Operands[ErrorInfo]).getStartLoc();
      if (ErrorLoc == SMLoc())
        ErrorLoc = IdLoc;
    }
    return Error(ErrorLoc, "Invalid operand for instruction");
  }
  default:
    break;
  }

  llvm_unreachable("Unknown match type detected!");
}

// Both '%rN' and 'rN' are parsed as valid registers. This was done to remain
// backwards compatible with GCC and the different ways inline assembly is
// handled.
// TODO: see if there isn't a better way to do this.
std::unique_ptr<LanaiOperand> LanaiAsmParser::parseRegister() {
  SMLoc Start = Parser.getTok().getLoc();
  SMLoc End = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  unsigned RegNum;
  // Eat the '%'.
  if (Lexer.getKind() == AsmToken::Percent)
    Parser.Lex();
  if (Lexer.getKind() == AsmToken::Identifier) {
    RegNum = MatchRegisterName(Lexer.getTok().getIdentifier());
    if (RegNum == 0)
      return 0;
    Parser.Lex(); // Eat identifier token
    return LanaiOperand::createReg(RegNum, Start, End);
  }
  return 0;
}

bool LanaiAsmParser::ParseRegister(unsigned &RegNum, SMLoc &StartLoc,
                                   SMLoc &EndLoc) {
  const AsmToken &Tok = getParser().getTok();
  StartLoc = Tok.getLoc();
  EndLoc = Tok.getEndLoc();
  std::unique_ptr<LanaiOperand> Op = parseRegister();
  if (Op != nullptr)
    RegNum = Op->getReg();
  return (Op == nullptr);
}

std::unique_ptr<LanaiOperand> LanaiAsmParser::parseIdentifier() {
  SMLoc Start = Parser.getTok().getLoc();
  SMLoc End = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
  const MCExpr *Res, *RHS = 0;
  LanaiMCExpr::VariantKind Kind = LanaiMCExpr::VK_Lanai_None;

  if (Lexer.getKind() != AsmToken::Identifier)
    return 0;

  StringRef Identifier;
  if (Parser.parseIdentifier(Identifier))
    return 0;

  // Check if identifier has a modifier
  if (Identifier.equals_lower("hi"))
    Kind = LanaiMCExpr::VK_Lanai_ABS_HI;
  else if (Identifier.equals_lower("lo"))
    Kind = LanaiMCExpr::VK_Lanai_ABS_LO;

  // If the identifier corresponds to a variant then extract the real
  // identifier.
  if (Kind != LanaiMCExpr::VK_Lanai_None) {
    if (Lexer.getKind() != AsmToken::LParen) {
      Error(Lexer.getLoc(), "Expected '('");
      return 0;
    }
    Lexer.Lex(); // lex '('

    // Parse identifier
    if (Parser.parseIdentifier(Identifier))
      return 0;
  }

  // If addition parse the RHS.
  if (Lexer.getKind() == AsmToken::Plus && Parser.parseExpression(RHS))
    return 0;

  // For variants parse the final ')'
  if (Kind != LanaiMCExpr::VK_Lanai_None) {
    if (Lexer.getKind() != AsmToken::RParen) {
      Error(Lexer.getLoc(), "Expected ')'");
      return 0;
    }
    Lexer.Lex(); // lex ')'
  }

  End = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
  MCSymbol *Sym = getContext().getOrCreateSymbol(Identifier);
  const MCExpr *Expr = MCSymbolRefExpr::create(Sym, getContext());
  Res = LanaiMCExpr::create(Kind, Expr, getContext());

  // Nest if this was an addition
  if (RHS)
    Res = MCBinaryExpr::createAdd(Res, RHS, getContext());

  return LanaiOperand::createImm(Res, Start, End);
}

std::unique_ptr<LanaiOperand> LanaiAsmParser::parseImmediate() {
  SMLoc Start = Parser.getTok().getLoc();
  SMLoc End = SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);

  const MCExpr *ExprVal;
  switch (Lexer.getKind()) {
  case AsmToken::Identifier:
    return parseIdentifier();
  case AsmToken::Plus:
  case AsmToken::Minus:
  case AsmToken::Integer:
  case AsmToken::Dot:
    if (!Parser.parseExpression(ExprVal))
      return LanaiOperand::createImm(ExprVal, Start, End);
  default:
    return 0;
  }
}

static unsigned AluWithPrePost(unsigned AluCode, bool PreOp, bool PostOp) {
  if (PreOp)
    return LPAC::makePreOp(AluCode);
  if (PostOp)
    return LPAC::makePostOp(AluCode);
  return AluCode;
}

unsigned LanaiAsmParser::parseAluOperator(bool PreOp, bool PostOp) {
  StringRef IdString;
  Parser.parseIdentifier(IdString);
  unsigned AluCode = LPAC::stringToLanaiAluCode(IdString);
  if (AluCode == LPAC::UNKNOWN) {
    Error(Parser.getTok().getLoc(), "Can't parse ALU operator");
    return 0;
  }
  return AluCode;
}

static int SizeForSuffix(StringRef T) {
  return StringSwitch<int>(T).EndsWith(".h", 2).EndsWith(".b", 1).Default(4);
}

bool LanaiAsmParser::parsePrePost(StringRef Type, int *OffsetValue) {
  bool PreOrPost = false;
  if (Lexer.getKind() == Lexer.peekTok(true).getKind()) {
    PreOrPost = true;
    if (Lexer.is(AsmToken::Minus))
      *OffsetValue = -SizeForSuffix(Type);
    else if (Lexer.is(AsmToken::Plus))
      *OffsetValue = SizeForSuffix(Type);
    else
      return false;

    // Eat the '-' '-' or '+' '+'
    Parser.Lex();
    Parser.Lex();
  } else if (Lexer.is(AsmToken::Star)) {
    Parser.Lex(); // Eat the '*'
    PreOrPost = true;
  }

  return PreOrPost;
}

bool shouldBeSls(const LanaiOperand &Op) {
  // The instruction should be encoded as an SLS if the constant is word
  // aligned and will fit in 21 bits
  if (const MCConstantExpr *ConstExpr = dyn_cast<MCConstantExpr>(Op.getImm())) {
    int64_t Value = ConstExpr->getValue();
    return (Value % 4 == 0) && (Value >= 0) && (Value <= 0x1fffff);
  }
  // The instruction should be encoded as an SLS if the operand is a symbolic
  // reference with no variant.
  if (const LanaiMCExpr *SymbolRefExpr = dyn_cast<LanaiMCExpr>(Op.getImm()))
    return SymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_None;
  // The instruction should be encoded as an SLS if the operand is a binary
  // expression with the left-hand side being a symbolic reference with no
  // variant.
  if (const MCBinaryExpr *BinaryExpr = dyn_cast<MCBinaryExpr>(Op.getImm())) {
    const LanaiMCExpr *LHSSymbolRefExpr =
        dyn_cast<LanaiMCExpr>(BinaryExpr->getLHS());
    return (LHSSymbolRefExpr &&
            LHSSymbolRefExpr->getKind() == LanaiMCExpr::VK_Lanai_None);
  }
  return false;
}

// Matches memory operand. Returns true if error encountered.
OperandMatchResultTy
LanaiAsmParser::parseMemoryOperand(OperandVector &Operands) {
  // Try to match a memory operand.
  // The memory operands are of the form:
  //  (1)  Register|Immediate|'' '[' '*'? Register '*'? ']' or
  //                            ^
  //  (2)  '[' '*'? Register '*'? AluOperator Register ']'
  //      ^
  //  (3)  '[' '--'|'++' Register '--'|'++' ']'
  //
  //  (4) '[' Immediate ']' (for SLS)

  // Store the type for use in parsing pre/post increment/decrement operators
  StringRef Type;
  if (Operands[0]->isToken())
    Type = static_cast<LanaiOperand *>(Operands[0].get())->getToken();

  // Use 0 if no offset given
  int OffsetValue = 0;
  unsigned BaseReg = 0;
  unsigned AluOp = LPAC::ADD;
  bool PostOp = false, PreOp = false;

  // Try to parse the offset
  std::unique_ptr<LanaiOperand> Op = parseRegister();
  if (!Op)
    Op = parseImmediate();

  // Only continue if next token is '['
  if (Lexer.isNot(AsmToken::LBrac)) {
    if (!Op)
      return MatchOperand_NoMatch;

    // The start of this custom parsing overlaps with register/immediate so
    // consider this as a successful match of an operand of that type as the
    // token stream can't be rewound to allow them to match separately.
    Operands.push_back(std::move(Op));
    return MatchOperand_Success;
  }

  Parser.Lex(); // Eat the '['.
  std::unique_ptr<LanaiOperand> Offset = nullptr;
  if (Op)
    Offset.swap(Op);

  // Determine if a pre operation
  PreOp = parsePrePost(Type, &OffsetValue);

  Op = parseRegister();
  if (!Op) {
    if (!Offset) {
      if ((Op = parseImmediate()) && Lexer.is(AsmToken::RBrac)) {
        Parser.Lex(); // Eat the ']'

        // Memory address operations aligned to word boundary are encoded as
        // SLS, the rest as RM.
        if (shouldBeSls(*Op)) {
          Operands.push_back(LanaiOperand::MorphToMemImm(std::move(Op)));
        } else {
          if (!Op->isLoImm16Signed()) {
            Error(Parser.getTok().getLoc(),
                  "Memory address is not word "
                  "aligned and larger than class RM can handle");
            return MatchOperand_ParseFail;
          }
          Operands.push_back(LanaiOperand::MorphToMemRegImm(
              Lanai::R0, std::move(Op), LPAC::ADD));
        }
        return MatchOperand_Success;
      }
    }

    Error(Parser.getTok().getLoc(),
          "Unknown operand, expected register or immediate");
    return MatchOperand_ParseFail;
  }
  BaseReg = Op->getReg();

  // Determine if a post operation
  if (!PreOp)
    PostOp = parsePrePost(Type, &OffsetValue);

  // If ] match form (1) else match form (2)
  if (Lexer.is(AsmToken::RBrac)) {
    Parser.Lex(); // Eat the ']'.
    if (!Offset) {
      SMLoc Start = Parser.getTok().getLoc();
      SMLoc End =
          SMLoc::getFromPointer(Parser.getTok().getLoc().getPointer() - 1);
      const MCConstantExpr *OffsetConstExpr =
          MCConstantExpr::create(OffsetValue, getContext());
      Offset = LanaiOperand::createImm(OffsetConstExpr, Start, End);
    }
  } else {
    if (Offset || OffsetValue != 0) {
      Error(Parser.getTok().getLoc(), "Expected ']'");
      return MatchOperand_ParseFail;
    }

    // Parse operator
    AluOp = parseAluOperator(PreOp, PostOp);

    // Second form requires offset register
    Offset = parseRegister();
    if (!BaseReg || Lexer.isNot(AsmToken::RBrac)) {
      Error(Parser.getTok().getLoc(), "Expected ']'");
      return MatchOperand_ParseFail;
    }
    Parser.Lex(); // Eat the ']'.
  }

  // First form has addition as operator. Add pre- or post-op indicator as
  // needed.
  AluOp = AluWithPrePost(AluOp, PreOp, PostOp);

  // Ensure immediate offset is not too large
  if (Offset->isImm() && !Offset->isLoImm16Signed()) {
    Error(Parser.getTok().getLoc(),
          "Memory address is not word "
          "aligned and larger than class RM can handle");
    return MatchOperand_ParseFail;
  }

  Operands.push_back(
      Offset->isImm()
          ? LanaiOperand::MorphToMemRegImm(BaseReg, std::move(Offset), AluOp)
          : LanaiOperand::MorphToMemRegReg(BaseReg, std::move(Offset), AluOp));

  return MatchOperand_Success;
}

// Looks at a token type and creates the relevant operand from this
// information, adding to operands.
// If operand was parsed, returns false, else true.
OperandMatchResultTy
LanaiAsmParser::parseOperand(OperandVector *Operands, StringRef Mnemonic) {
  // Check if the current operand has a custom associated parser, if so, try to
  // custom parse the operand, or fallback to the general approach.
  OperandMatchResultTy Result = MatchOperandParserImpl(*Operands, Mnemonic);

  if (Result == MatchOperand_Success)
    return Result;
  if (Result == MatchOperand_ParseFail) {
    Parser.eatToEndOfStatement();
    return Result;
  }

  // Attempt to parse token as register
  std::unique_ptr<LanaiOperand> Op = parseRegister();

  // Attempt to parse token as immediate
  if (!Op)
    Op = parseImmediate();

  // If the token could not be parsed then fail
  if (!Op) {
    Error(Parser.getTok().getLoc(), "Unknown operand");
    Parser.eatToEndOfStatement();
    return MatchOperand_ParseFail;
  }

  // Push back parsed operand into list of operands
  Operands->push_back(std::move(Op));

  return MatchOperand_Success;
}

// Split the mnemonic into ASM operand, conditional code and instruction
// qualifier (half-word, byte).
StringRef LanaiAsmParser::splitMnemonic(StringRef Name, SMLoc NameLoc,
                                        OperandVector *Operands) {
  size_t Next = Name.find('.');

  StringRef Mnemonic = Name;

  bool IsBRR = false;
  if (Name.endswith(".r")) {
    Mnemonic = Name.substr(0, Name.size() - 2);
    IsBRR = true;
  }

  // Match b?? and s?? (BR, BRR, and SCC instruction classes).
  if (Mnemonic[0] == 'b' ||
      (Mnemonic[0] == 's' && !Mnemonic.startswith("sel") &&
       !Mnemonic.startswith("st"))) {
    // Parse instructions with a conditional code. For example, 'bne' is
    // converted into two operands 'b' and 'ne'.
    LPCC::CondCode CondCode =
        LPCC::suffixToLanaiCondCode(Mnemonic.substr(1, Next));
    if (CondCode != LPCC::UNKNOWN) {
      Mnemonic = Mnemonic.slice(0, 1);
      Operands->push_back(LanaiOperand::CreateToken(Mnemonic, NameLoc));
      Operands->push_back(LanaiOperand::createImm(
          MCConstantExpr::create(CondCode, getContext()), NameLoc, NameLoc));
      if (IsBRR) {
        Operands->push_back(LanaiOperand::CreateToken(".r", NameLoc));
      }
      return Mnemonic;
    }
  }

  // Parse other instructions with condition codes (RR instructions).
  // We ignore .f here and assume they are flag-setting operations, not
  // conditional codes (except for select instructions where flag-setting
  // variants are not yet implemented).
  if (Mnemonic.startswith("sel") ||
      (!Mnemonic.endswith(".f") && !Mnemonic.startswith("st"))) {
    LPCC::CondCode CondCode = LPCC::suffixToLanaiCondCode(Mnemonic);
    if (CondCode != LPCC::UNKNOWN) {
      size_t Next = Mnemonic.rfind('.', Name.size());
      // 'sel' doesn't use a predicate operand whose printer adds the period,
      // but instead has the period as part of the identifier (i.e., 'sel.' is
      // expected by the generated matcher). If the mnemonic starts with 'sel'
      // then include the period as part of the mnemonic, else don't include it
      // as part of the mnemonic.
      if (Mnemonic.startswith("sel")) {
        Mnemonic = Mnemonic.substr(0, Next + 1);
      } else {
        Mnemonic = Mnemonic.substr(0, Next);
      }
      Operands->push_back(LanaiOperand::CreateToken(Mnemonic, NameLoc));
      Operands->push_back(LanaiOperand::createImm(
          MCConstantExpr::create(CondCode, getContext()), NameLoc, NameLoc));
      return Mnemonic;
    }
  }

  Operands->push_back(LanaiOperand::CreateToken(Mnemonic, NameLoc));
  if (IsBRR) {
    Operands->push_back(LanaiOperand::CreateToken(".r", NameLoc));
  }

  return Mnemonic;
}

bool IsMemoryAssignmentError(const OperandVector &Operands) {
  // Detects if a memory operation has an erroneous base register modification.
  // Memory operations are detected by matching the types of operands.
  //
  // TODO: This test is focussed on one specific instance (ld/st).
  // Extend it to handle more cases or be more robust.
  bool Modifies = false;

  int Offset = 0;

  if (Operands.size() < 5)
    return false;
  else if (Operands[0]->isToken() && Operands[1]->isReg() &&
           Operands[2]->isImm() && Operands[3]->isImm() && Operands[4]->isReg())
    Offset = 0;
  else if (Operands[0]->isToken() && Operands[1]->isToken() &&
           Operands[2]->isReg() && Operands[3]->isImm() &&
           Operands[4]->isImm() && Operands[5]->isReg())
    Offset = 1;
  else
    return false;

  int PossibleAluOpIdx = Offset + 3;
  int PossibleBaseIdx = Offset + 1;
  int PossibleDestIdx = Offset + 4;
  if (LanaiOperand *PossibleAluOp =
          static_cast<LanaiOperand *>(Operands[PossibleAluOpIdx].get()))
    if (PossibleAluOp->isImm())
      if (const MCConstantExpr *ConstExpr =
              dyn_cast<MCConstantExpr>(PossibleAluOp->getImm()))
        Modifies = LPAC::modifiesOp(ConstExpr->getValue());
  return Modifies && Operands[PossibleBaseIdx]->isReg() &&
         Operands[PossibleDestIdx]->isReg() &&
         Operands[PossibleBaseIdx]->getReg() ==
             Operands[PossibleDestIdx]->getReg();
}

static bool IsRegister(const MCParsedAsmOperand &op) {
  return static_cast<const LanaiOperand &>(op).isReg();
}

static bool MaybePredicatedInst(const OperandVector &Operands) {
  if (Operands.size() < 4 || !IsRegister(*Operands[1]) ||
      !IsRegister(*Operands[2]))
    return false;
  return StringSwitch<bool>(
             static_cast<const LanaiOperand &>(*Operands[0]).getToken())
      .StartsWith("addc", true)
      .StartsWith("add", true)
      .StartsWith("and", true)
      .StartsWith("sh", true)
      .StartsWith("subb", true)
      .StartsWith("sub", true)
      .StartsWith("or", true)
      .StartsWith("xor", true)
      .Default(false);
}

bool LanaiAsmParser::ParseInstruction(ParseInstructionInfo & /*Info*/,
                                      StringRef Name, SMLoc NameLoc,
                                      OperandVector &Operands) {
  // First operand is token for instruction
  StringRef Mnemonic = splitMnemonic(Name, NameLoc, &Operands);

  // If there are no more operands, then finish
  if (Lexer.is(AsmToken::EndOfStatement))
    return false;

  // Parse first operand
  if (parseOperand(&Operands, Mnemonic) != MatchOperand_Success)
    return true;

  // If it is a st instruction with one 1 operand then it is a "store true".
  // Transform <"st"> to <"s">, <LPCC:ICC_T>
  if (Lexer.is(AsmToken::EndOfStatement) && Name == "st" &&
      Operands.size() == 2) {
    Operands.erase(Operands.begin(), Operands.begin() + 1);
    Operands.insert(Operands.begin(), LanaiOperand::CreateToken("s", NameLoc));
    Operands.insert(Operands.begin() + 1,
                    LanaiOperand::createImm(
                        MCConstantExpr::create(LPCC::ICC_T, getContext()),
                        NameLoc, NameLoc));
  }

  // If the instruction is a bt instruction with 1 operand (in assembly) then it
  // is an unconditional branch instruction and the first two elements of
  // operands need to be merged.
  if (Lexer.is(AsmToken::EndOfStatement) && Name.startswith("bt") &&
      Operands.size() == 3) {
    Operands.erase(Operands.begin(), Operands.begin() + 2);
    Operands.insert(Operands.begin(), LanaiOperand::CreateToken("bt", NameLoc));
  }

  // Parse until end of statement, consuming commas between operands
  while (Lexer.isNot(AsmToken::EndOfStatement) && Lexer.is(AsmToken::Comma)) {
    // Consume comma token
    Lex();

    // Parse next operand
    if (parseOperand(&Operands, Mnemonic) != MatchOperand_Success)
      return true;
  }

  if (IsMemoryAssignmentError(Operands)) {
    Error(Parser.getTok().getLoc(),
          "the destination register can't equal the base register in an "
          "instruction that modifies the base register.");
    return true;
  }

  // Insert always true operand for instruction that may be predicated but
  // are not. Currently the autogenerated parser always expects a predicate.
  if (MaybePredicatedInst(Operands)) {
    Operands.insert(Operands.begin() + 1,
                    LanaiOperand::createImm(
                        MCConstantExpr::create(LPCC::ICC_T, getContext()),
                        NameLoc, NameLoc));
  }

  return false;
}

#define GET_REGISTER_MATCHER
#define GET_MATCHER_IMPLEMENTATION
#include "LanaiGenAsmMatcher.inc"
} // namespace

extern "C" void LLVMInitializeLanaiAsmParser() {
  RegisterMCAsmParser<LanaiAsmParser> x(getTheLanaiTarget());
}

} // namespace llvm
