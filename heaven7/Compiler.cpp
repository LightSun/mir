#include "Compiler.h"


void Compiler::CompileBlock(MIR_label_t code_block){

//InstructionIT it  = block->fInstructions.begin();
//        bool          end = false;

//        // Push the block label
//        MIR_append_insn(fContext, fCompute, code_block);

//        while ((it != block->fInstructions.end()) && !end) {
//            //(*it)->write(&std::cout);
//            //MIR_output(fContext, stderr);

//            switch ((*it)->fOpcode) {
//                // Numbers
//                case FBCInstruction::kRealValue: {
//                    MIR_reg_t var_real = createVar(getRealTy(), "var_real");
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     typedReal(MIR_FMOV, MIR_DMOV),
//                                                                     MIR_new_reg_op(fContext, var_real),
//                                                                     genReal((*it)->fRealValue)));
//                    pushValue(var_real);
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kInt32Value: {
//                    MIR_reg_t var_i64 = createVar(getInt64Ty(), "var_i64");
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     MIR_MOV,
//                                                                     MIR_new_reg_op(fContext, var_i64),
//                                                                     genInt64((*it)->fIntValue)));
//                    pushValue(var_i64);
//                    it++;
//                    break;
//                }

//                // Memory load/store
//                case FBCInstruction::kLoadReal:
//                    pushLoadRealArray((*it)->fOffset1);
//                    it++;
//                    break;

//                case FBCInstruction::kLoadInt:
//                    pushLoadIntArray((*it)->fOffset1);
//                    it++;
//                    break;

//                case FBCInstruction::kStoreReal:
//                    pushStoreRealArray((*it)->fOffset1);
//                    it++;
//                    break;

//                case FBCInstruction::kStoreInt:
//                    pushStoreIntArray((*it)->fOffset1);
//                    it++;
//                    break;

//                // Indexed memory load/store: constant values are added at generation time by CreateBinOp...
//                case FBCInstruction::kLoadIndexedReal: {
//                    pushLoadRealArrayImp(createAddOffsetReg((*it)->fOffset1));
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kLoadIndexedInt: {
//                    pushLoadIntArrayImp(createAddOffsetReg((*it)->fOffset1));
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kLoadSoundFieldInt:
//                    throw faustexception("ERROR : kLoadSoundFieldInt not yet supported in FBCMIRCompiler\n");
//                    it++;
//                    break;

//                case FBCInstruction::kLoadSoundFieldReal:
//                    throw faustexception("ERROR : kLoadSoundFieldReal not yet supported in FBCMIRCompiler\n");
//                    it++;
//                    break;

//                case FBCInstruction::kStoreIndexedReal: {
//                    pushStoreRealArrayImp(createAddOffsetReg((*it)->fOffset1));
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kStoreIndexedInt: {
//                    pushStoreIntArrayImp(createAddOffsetReg((*it)->fOffset1));
//                    it++;
//                    break;
//                }

//                // Memory shift
//                case FBCInstruction::kBlockShiftReal: {
//                    for (int i = (*it)->fOffset1; i > (*it)->fOffset2; i -= 1) {
//                        pushLoadRealArray(i - 1);
//                        pushStoreRealArray(i);
//                    }
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kBlockShiftInt: {
//                    for (int i = (*it)->fOffset1; i > (*it)->fOffset2; i -= 1) {
//                        pushLoadIntArray(i - 1);
//                        pushStoreIntArray(i);
//                    }
//                    it++;
//                    break;
//                }

//                // Input/output
//                case FBCInstruction::kLoadInput:
//                    pushLoadInput((*it)->fOffset1);
//                    it++;
//                    break;

//                case FBCInstruction::kStoreOutput:
//                    pushStoreOutput((*it)->fOffset1);
//                    it++;
//                    break;

//                // Cast
//                case FBCInstruction::kCastReal: {
//                    MIR_reg_t ext32 = createVar(MIR_T_I64, "ext32");
//                    // Take lower 32 bits
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     MIR_EXT32,
//                                                                     MIR_new_reg_op(fContext, ext32),
//                                                                     MIR_new_reg_op(fContext, popValue())));
//                    // Generate the cast
//                    MIR_reg_t cast_real = createVar(getRealTy(), "cast_real");
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     typedReal(MIR_I2F, MIR_I2D),
//                                                                     MIR_new_reg_op(fContext, cast_real),
//                                                                     MIR_new_reg_op(fContext, ext32)));

//                    pushValue(cast_real);
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kCastInt: {
//                    // Generate the cast
//                    MIR_reg_t cast_i64 = createVar(getInt64Ty(), "cast_i64");
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     typedReal(MIR_F2I, MIR_D2I),
//                                                                     MIR_new_reg_op(fContext, cast_i64),
//                                                                     MIR_new_reg_op(fContext, popValue())));
//                    // Take lower 32 bits
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     MIR_EXT32,
//                                                                     MIR_new_reg_op(fContext, cast_i64),
//                                                                     MIR_new_reg_op(fContext, cast_i64)));
//                    pushValue(cast_i64);
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kBitcastInt: {
//                    // MIR_reg_t val = popValue();
//                    // it++;
//                    // TODO
//                    faustassert(false);
//                    break;
//                }

//                case FBCInstruction::kBitcastReal: {
//                    // MIR_reg_t val = popValue();
//                    // it++;
//                    // TODO
//                    faustassert(false);
//                    break;
//                }

//                // Binary math
//                case FBCInstruction::kAddReal:
//                    pushBinop(typedReal(MIR_FADD, MIR_DADD), getRealTy());
//                    it++;
//                    break;

//                case FBCInstruction::kAddInt:
//                    pushBinop(MIR_ADDS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kSubReal:
//                    pushBinop(typedReal(MIR_FSUB, MIR_DSUB), getRealTy());
//                    it++;
//                    break;

//                case FBCInstruction::kSubInt:
//                    pushBinop(MIR_SUBS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kMultReal:
//                    pushBinop(typedReal(MIR_FMUL, MIR_DMUL), getRealTy());
//                    it++;
//                    break;

//                case FBCInstruction::kMultInt:
//                    pushBinop(MIR_MULS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kDivReal:
//                    pushBinop(typedReal(MIR_FDIV, MIR_DDIV), getRealTy());
//                    it++;
//                    break;

//                case FBCInstruction::kDivInt:
//                    pushBinop(MIR_DIVS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kRemReal:
//                    pushBinaryRealCall("mir_remainder");
//                    it++;
//                    break;

//                case FBCInstruction::kRemInt:
//                    pushBinop(MIR_MODS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kLshInt:
//                    pushBinop(MIR_LSHS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kARshInt:
//                    pushBinop(MIR_RSHS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kGTInt:
//                    pushIntComp(MIR_GTS);
//                    it++;
//                    break;

//                case FBCInstruction::kLTInt:
//                    pushIntComp(MIR_LTS);
//                    it++;
//                    break;

//                case FBCInstruction::kGEInt:
//                    pushIntComp(MIR_GES);
//                    it++;
//                    break;

//                case FBCInstruction::kLEInt:
//                    pushIntComp(MIR_LES);
//                    it++;
//                    break;

//                case FBCInstruction::kEQInt:
//                    pushIntComp(MIR_EQS);
//                    it++;
//                    break;

//                case FBCInstruction::kNEInt:
//                    pushIntComp(MIR_NES);
//                    it++;
//                    break;

//                case FBCInstruction::kGTReal:
//                    pushRealComp(typedReal(MIR_FGT, MIR_DGT));
//                    it++;
//                    break;

//                case FBCInstruction::kLTReal:
//                    pushRealComp(typedReal(MIR_FLT, MIR_DLT));
//                    it++;
//                    break;

//                case FBCInstruction::kGEReal:
//                    pushRealComp(typedReal(MIR_FGE, MIR_DGE));
//                    it++;
//                    break;

//                case FBCInstruction::kLEReal:
//                    pushRealComp(typedReal(MIR_FLE, MIR_DLE));
//                    it++;
//                    break;

//                case FBCInstruction::kEQReal:
//                    pushRealComp(typedReal(MIR_FEQ, MIR_DEQ));
//                    it++;
//                    break;

//                case FBCInstruction::kNEReal:
//                    pushRealComp(typedReal(MIR_FNE, MIR_DNE));
//                    it++;
//                    break;

//                case FBCInstruction::kANDInt:
//                    pushBinop(MIR_ANDS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kORInt:
//                    pushBinop(MIR_ORS, getInt64Ty());
//                    it++;
//                    break;

//                case FBCInstruction::kXORInt:
//                    pushBinop(MIR_XORS, getInt64Ty());
//                    it++;
//                    break;

//                    // Extended unary math
//                case FBCInstruction::kAbs:
//                    pushUnaryIntCall("mir_abs", false);
//                    it++;
//                    break;

//                case FBCInstruction::kAbsf:
//                    pushUnaryRealCall("mir_fabs");
//                    it++;
//                    break;

//                case FBCInstruction::kAcosf:
//                    pushUnaryRealCall("mir_acos");
//                    it++;
//                    break;

//                case FBCInstruction::kAcoshf:
//                    pushUnaryRealCall("mir_acosh");
//                    it++;
//                    break;

//                case FBCInstruction::kAsinf:
//                    pushUnaryRealCall("mir_asin");
//                    it++;
//                    break;

//                case FBCInstruction::kAsinhf:
//                    pushUnaryRealCall("mir_asinh");
//                    it++;
//                    break;

//                case FBCInstruction::kAtanf:
//                    pushUnaryRealCall("mir_atan");
//                    it++;
//                    break;

//                case FBCInstruction::kAtanhf:
//                    pushUnaryRealCall("mir_atanh");
//                    it++;
//                    break;

//                case FBCInstruction::kCeilf:
//                    pushUnaryRealCall("mir_ceil");
//                    it++;
//                    break;

//                case FBCInstruction::kCosf:
//                    pushUnaryRealCall("mir_cos");
//                    it++;
//                    break;

//                case FBCInstruction::kCoshf:
//                    pushUnaryRealCall("mir_cosh");
//                    it++;
//                    break;

//                case FBCInstruction::kExpf:
//                    pushUnaryRealCall("mir_exp");
//                    it++;
//                    break;

//                case FBCInstruction::kFloorf:
//                    pushUnaryRealCall("mir_floor");
//                    it++;
//                    break;

//                case FBCInstruction::kLogf:
//                    pushUnaryRealCall("mir_log");
//                    it++;
//                    break;

//                case FBCInstruction::kLog10f:
//                    pushUnaryRealCall("mir_log10");
//                    it++;
//                    break;

//                case FBCInstruction::kRintf:
//                    pushUnaryRealCall("mir_rint");
//                    it++;
//                    break;

//                case FBCInstruction::kRoundf:
//                    pushUnaryRealCall("mir_round");
//                    it++;
//                    break;

//                case FBCInstruction::kSinf:
//                    pushUnaryRealCall("mir_sin");
//                    it++;
//                    break;

//                case FBCInstruction::kSinhf:
//                    pushUnaryRealCall("mir_sinh");
//                    it++;
//                    break;

//                case FBCInstruction::kSqrtf:
//                    pushUnaryRealCall("mir_sqrt");
//                    it++;
//                    break;

//                case FBCInstruction::kTanf:
//                    pushUnaryRealCall("mir_tan");
//                    it++;
//                    break;

//                case FBCInstruction::kTanhf:
//                    pushUnaryRealCall("mir_tanh");
//                    it++;
//                    break;

//                case FBCInstruction::kIsnanf:
//                    // TODO
//                    faustassert(false);
//                    it++;
//                    break;

//                case FBCInstruction::kIsinff:
//                    // TODO
//                    faustassert(false);
//                    it++;
//                    break;

//                case FBCInstruction::kCopysignf:
//                    // TODO
//                    faustassert(false);
//                    it++;
//                    break;

//                    // Extended binary math
//                case FBCInstruction::kAtan2f:
//                    pushBinaryRealCall("mir_atan2");
//                    it++;
//                    break;

//                case FBCInstruction::kFmodf:
//                    pushBinaryRealCall("mir_fmod");
//                    it++;
//                    break;

//                case FBCInstruction::kPowf:
//                    pushBinaryRealCall("mir_pow");
//                    it++;
//                    break;

//                case FBCInstruction::kMax: {
//                    pushBinaryIntCall("mir_max_i", false);
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kMaxf: {
//                    pushBinaryRealCall("mir_max");
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kMin: {
//                    pushBinaryIntCall("mir_min_i", false);
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kMinf: {
//                    pushBinaryRealCall("mir_min");
//                    it++;
//                    break;
//                }

//                // Control
//                case FBCInstruction::kReturn:
//                    end = true;
//                    break;

//                case FBCInstruction::kIf: {
//                    // TODO
//                    faustassert(false);
//                    break;
//                }

//                case FBCInstruction::kSelectInt: {
//                    // Create typed local variable
//                    createSelectBlock(it, createVar(getInt64Ty(), "select_i64"), MIR_MOV);
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kSelectReal: {
//                    // Create typed local variable
//                    createSelectBlock(it, createVar(getRealTy(), "select_real"), typedReal(MIR_FMOV, MIR_DMOV));
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kCondBranch: {
//                    MIR_reg_t cond_res = createVar(getInt64Ty(), "cond_i64");
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, MIR_MOV, MIR_new_reg_op(fContext, cond_res), genInt64(0)));
//                    MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     MIR_BNES,
//                                                                     MIR_new_label_op(fContext, code_block),
//                                                                     MIR_new_reg_op(fContext, cond_res),
//                                                                     MIR_new_reg_op(fContext, popValue())));
//                    it++;
//                    break;
//                }

//                case FBCInstruction::kLoop: {
//                    MIR_label_t init_block = MIR_new_label(fContext);
//                    MIR_label_t loop_body_block = MIR_new_label(fContext);

//                    // Compile init branch (= branch1)
//                    CompileBlock((*it)->fBranch1, init_block);

//                    // Compile loop branch (= branch2)
//                    CompileBlock((*it)->fBranch2, loop_body_block);
//                    it++;
//                    break;
//                }

//                default:
//                    (*it)->write(&std::cout);
//                    faustassert(false);
//                    break;
//            }
//        }
//    }
}
