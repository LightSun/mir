#ifndef COMPILER_H
#define COMPILER_H

#include <map>
#include <string>

extern "C"{
#include "../mir.h"
#include "../mir-gen.h"
}

#ifndef REAL
#define REAL float
#endif

//#define typedReal(op1, op2) ((sizeof(REAL) == sizeof(float)) ? op1 : op2)
//just as float
#define typedReal(op1, op2) op1

class Compiler
{
public:
    Compiler(){
        //gMathLib["mir_acosf"] = (void*)mir_acosf;
        fContext = MIR_init();
        MIR_module_t module = MIR_new_module(fContext, "Faust");

        // Create 'compute' function
        fCompute = MIR_new_func(fContext, "compute", 0,
                                NULL, 4,
                                MIR_T_P, "int_heap",
                                MIR_T_P, "real_heap",
                                MIR_T_P, "inputs",
                                MIR_T_P, "outputs");

        // Get the 4 function parameters
        fMIRIntHeap = MIR_reg(fContext, "int_heap", fCompute->u.func);
        fMIRRealHeap = MIR_reg(fContext, "real_heap", fCompute->u.func);
        fMIRInputs = MIR_reg(fContext, "inputs", fCompute->u.func);
        fMIROutputs = MIR_reg(fContext, "outputs", fCompute->u.func);

        // Compile compute body
        MIR_label_t entry_block = MIR_new_label(fContext);
       //TODO CompileBlock(fbc_block, entry_block);

        // Add return
        MIR_append_insn(fContext, fCompute, MIR_new_ret_insn(fContext, 0));

        // Finish function
        MIR_finish_func(fContext);

        // Finish module
        MIR_finish_module(fContext);

        // Load module and link
        MIR_load_module(fContext, module);
        MIR_link(fContext, MIR_set_interp_interface, importResolver);

        // Code generation
        MIR_gen_init(fContext, 1);

        // Optimize the code (default = 2)
        MIR_gen_set_optimize_level(fContext, 0, 2);

        fCompiledFun = (compiledFun)MIR_gen(fContext, 0, fCompute);
        MIR_gen_finish(fContext);

        // Print module
        //MIR_output(fContext, stderr);
    }

    char* getFreshID(const std::string& prefix)
   {
       if (fIDCounters.find(prefix) == fIDCounters.end()) {
           fIDCounters[prefix] = 0;
       }
       int n = fIDCounters[prefix];
       fIDCounters[prefix] = n + 1;
       fIdent = prefix + std::to_string(n);
       return (char*)fIdent.c_str();
   }

    MIR_op_t genFloat(float num) { return MIR_new_float_op(fContext, num); }
    MIR_op_t genDouble(double num) { return MIR_new_double_op(fContext, num); }
   // MIR_op_t genReal(double num) { return (sizeof(REAL) == sizeof(double)) ? genDouble(num) : genFloat(num); }
    MIR_op_t genInt32(int num) { return MIR_new_int_op(fContext, num); }
    MIR_op_t genInt64(int64_t num) { return MIR_new_int_op(fContext, num); }

    MIR_type_t getFloatTy() { return MIR_T_F; }
    MIR_type_t getDoubleTy() { return MIR_T_D; }
   // MIR_type_t getRealTy() { return (sizeof(REAL) == sizeof(double)) ? getDoubleTy() : getFloatTy(); }
    MIR_type_t getInt32Ty() { return MIR_T_I32; }
    MIR_type_t getInt64Ty() { return MIR_T_I64; }

    MIR_reg_t createVar(MIR_type_t type, const std::string& name)
    {
        return MIR_new_func_reg(fContext, fCompute->u.func, type, getFreshID(name));
    }

    std::string getMathName(const std::string& name) {
        //return (sizeof(REAL) == sizeof(float)) ? (name + "f") : name;
        return (name + "f");
    }

    void pushValue(MIR_reg_t val) {
        fMIRStack[fMIRStackIndex++] = val;
    }
    MIR_reg_t popValue() { return fMIRStack[--fMIRStackIndex]; }

    void pushBinop(MIR_insn_code_t op, MIR_type_t res_type, const std::string& res_name = "binop")
    {
        MIR_reg_t binop_res = createVar(res_type, res_name);
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, op,
                                                         MIR_new_reg_op(fContext, binop_res),
                                                         MIR_new_reg_op(fContext, popValue()),
                                                         MIR_new_reg_op(fContext, popValue())));
        pushValue(binop_res);
    }

    void pushIntComp(MIR_insn_code_t op)
    {
        pushBinop(op, getInt64Ty(), "comp_i64");
    }

    void pushRealComp(MIR_insn_code_t op)
    {
        pushBinop(op, getInt64Ty(), "comp_real");
    }

    void pushUnaryCall(const std::string& name_aux, MIR_type_t type, bool rename)
    {
        std::string name = (rename) ? getMathName(name_aux) : name_aux;
        std::string name_proto = name + "_proto";

        // Generate call
        MIR_type_t res_type = type;
        MIR_var_t var[1];
        var[0].name = "val";
        var[0].type = type;
        MIR_reg_t call_res = createVar(type, "unary_call");

        // Check proto
        if (fFunProto.find(name_proto) == fFunProto.end()) {
            fFunProto[name_proto] = MIR_new_proto_arr(fContext, name_proto.c_str(), 1, &res_type, 1, var);
        }
        MIR_item_t proto = fFunProto[name_proto];

        MIR_item_t fun = MIR_new_import(fContext, name.c_str());
        MIR_append_insn(fContext, fCompute, MIR_new_call_insn(fContext, 4,
                                                              MIR_new_ref_op(fContext, proto),
                                                              MIR_new_ref_op(fContext, fun),
                                                              MIR_new_reg_op(fContext, call_res),
                                                              MIR_new_reg_op(fContext, popValue())));
        pushValue(call_res);
    }

    void pushUnaryIntCall(const std::string& name, bool rename = true)
    {
        return pushUnaryCall(name, getInt64Ty(), rename);
    }

    void pushUnaryRealCall(const std::string& name, bool rename = true)
    {
        return pushUnaryCall(name, getFloatTy(), rename);
    }

    void pushBinaryCall(const std::string& name_aux, MIR_type_t type, bool rename)
    {
        std::string name = (rename) ? getMathName(name_aux) : name_aux;
        std::string name_proto = name + "_proto";

        // Generate call
        MIR_type_t res_type = type;
        MIR_var_t var[2];
        var[0].name = "val1";
        var[0].type = type;
        var[1].name = "val2";
        var[1].type = type;
        MIR_reg_t call_res = createVar(type, "binary_call");

        // Check proto
        if (fFunProto.find(name_proto) == fFunProto.end()) {
            fFunProto[name_proto] = MIR_new_proto_arr(fContext, name_proto.c_str(), 1, &res_type, 2, var);
        }
        MIR_item_t proto = fFunProto[name_proto];

        MIR_item_t fun = MIR_new_import(fContext, name.c_str());
        MIR_append_insn(fContext, fCompute, MIR_new_call_insn(fContext, 5,
                                                              MIR_new_ref_op(fContext, proto),
                                                              MIR_new_ref_op(fContext, fun),
                                                              MIR_new_reg_op(fContext, call_res),
                                                              MIR_new_reg_op(fContext, popValue()),
                                                              MIR_new_reg_op(fContext, popValue())));
        pushValue(call_res);
    }

    void pushBinaryIntCall(const std::string& name, bool rename = true)
    {
        pushBinaryCall(name, getInt64Ty(), rename);
    }

    void pushBinaryRealCall(const std::string& name, bool rename = true)
    {
        pushBinaryCall(name, getFloatTy(), rename);
    }

    MIR_reg_t createIndexReg(int index)
    {
        // Load index in a local
        MIR_reg_t index_reg = createVar(getInt64Ty(), "array_id");
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
                                                         MIR_MOV,
                                                         MIR_new_reg_op(fContext, index_reg),
                                                         genInt64(index)));
        return index_reg;
    }

    MIR_reg_t createAddOffsetReg(int index)
    {
        // Add offset
        MIR_reg_t index_reg = createIndexReg(index);
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, MIR_ADDS,
                                                         MIR_new_reg_op(fContext, index_reg),
                                                         MIR_new_reg_op(fContext, index_reg),
                                                         MIR_new_reg_op(fContext, popValue())));
        return index_reg;
    }

    void pushLoadIntArray(int index)
    {
        // Load index in a local
        pushLoadIntArrayImp(createIndexReg(index));
    }

    void pushLoadIntArrayImp(MIR_reg_t index_reg)
    {
        // Create a local res
        MIR_reg_t load_res = createVar(getInt64Ty(), "load_i64");
        // Load the int32 value
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, MIR_MOV,
                                                         MIR_new_reg_op(fContext, load_res),
                                                         MIR_new_mem_op(fContext,
                                                                        MIR_T_I32, 0,
                                                                        fMIRIntHeap,
                                                                        index_reg,
                                                                        sizeof(int))));
        pushValue(load_res);
    }

    void pushLoadRealArray(int index)
    {
        // Load index in a local
        pushLoadRealArrayImp(createIndexReg(index));
    }

    void pushLoadRealArrayImp(MIR_reg_t index_reg)
    {
        // Create a local res
        MIR_reg_t load_res = createVar(getFloatTy(), "load_real");
        // Load the value
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, typedReal(MIR_FMOV, MIR_DMOV),
                                                         MIR_new_reg_op(fContext, load_res),
                                                         MIR_new_mem_op(fContext,
                                                                        getFloatTy(), 0,
                                                                        fMIRRealHeap,
                                                                        index_reg,
                                                                        sizeof(float))));
        pushValue(load_res);
    }

    void pushStoreIntArray(int index)
    {
        // Load index in a local
        pushStoreIntArrayImp(createIndexReg(index));
    }

    void pushStoreIntArrayImp(MIR_reg_t index_reg)
    {
        // Store the value
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, MIR_MOV,
                                                         MIR_new_mem_op(fContext,
                                                                        MIR_T_I32, 0,
                                                                        fMIRIntHeap, index_reg,
                                                                        sizeof(int)),
                                                         MIR_new_reg_op(fContext, popValue())));
    }

    void pushStoreRealArray(int index)
    {
        // Load index in a local
        pushStoreRealArrayImp(createIndexReg(index));
    }

    void pushStoreRealArrayImp(MIR_reg_t index_reg)
    {
        // Store the value
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, typedReal(MIR_FMOV, MIR_DMOV),
                                                         MIR_new_mem_op(fContext,
                                                                        getFloatTy(), 0,
                                                                        fMIRRealHeap, index_reg,
                                                                        sizeof(float)),
                                                         MIR_new_reg_op(fContext, popValue())));
    }

    void pushLoadInput(int index)
    {
        // Load index in a local
        MIR_reg_t index_reg = createIndexReg(index);
        MIR_reg_t load_res_ptr = createVar(getInt64Ty(), "load_ptr");

        // Load the buffer
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, MIR_MOV,
                                                         MIR_new_reg_op(fContext, load_res_ptr),
                                                         MIR_new_mem_op(fContext,
                                                                        getInt64Ty(), 0,
                                                                        fMIRInputs, index_reg,
                                                                        sizeof(int64_t))));

        // Create a local res
        MIR_reg_t load_res = createVar(getFloatTy(), "load_real");
        // Load the value
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, typedReal(MIR_FMOV, MIR_DMOV),
                                                         MIR_new_reg_op(fContext, load_res),
                                                         MIR_new_mem_op(fContext,
                                                                        getFloatTy(), 0,
                                                                        load_res_ptr, popValue(),
                                                                        sizeof(float))));
        pushValue(load_res);
    }

    void pushStoreOutput(int index)
    {
        // Load index in a local
        MIR_reg_t index_reg = createIndexReg(index);
        MIR_reg_t store_res_ptr = createVar(getInt64Ty(), "store_ptr");

        // Load the buffer
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, MIR_MOV,
                                                         MIR_new_reg_op(fContext, store_res_ptr),
                                                         MIR_new_mem_op(fContext,
                                                                        getInt64Ty(), 0,
                                                                        fMIROutputs, index_reg,
                                                                        sizeof(int64_t))));

        // Store the value
        MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, typedReal(MIR_FMOV, MIR_DMOV),
                                                         MIR_new_mem_op(fContext,
                                                                        getFloatTy(), 0,
                                                                        store_res_ptr, popValue(),
                                                                        sizeof(float)),
                                                         MIR_new_reg_op(fContext, popValue())));
    }

    // Select that only computes one branch: a local variable is create and written in 'then' and 'else' blocks, and loaded in the 'merge' block
    void createSelectBlock(/*InstructionIT it, */MIR_reg_t typed_res, MIR_insn_code_t mov_code)
    {
        MIR_label_t then_block = MIR_new_label(fContext);
        MIR_label_t else_block = MIR_new_label(fContext);
        MIR_label_t merge_block = MIR_new_label(fContext);

        // Branch in 'else_block' if false
        MIR_append_insn(fContext, fCompute,
                        MIR_new_insn(fContext, MIR_BFS,
                                     MIR_new_label_op(fContext, else_block),
                                     MIR_new_reg_op(fContext, popValue())));

        // Compile then branch (= branch1)
        //TODO CompileBlock((*it)->fBranch1, then_block);

        // Keep result in 'typed_res'
        MIR_append_insn(fContext, fCompute,
                        MIR_new_insn(fContext, mov_code,
                                     MIR_new_reg_op(fContext, typed_res),
                                     MIR_new_reg_op(fContext, popValue())));

        // Branch in 'merge_block'
        MIR_append_insn(fContext, fCompute,
                        MIR_new_insn(fContext, MIR_JMP,
                                     MIR_new_label_op(fContext, merge_block)));

        // Compile else branch (= branch2)
        //TODO CompileBlock((*it)->fBranch2, else_block);

        // Keep result in 'typed_res'
        MIR_append_insn(fContext, fCompute,
                        MIR_new_insn(fContext, mov_code,
                                     MIR_new_reg_op(fContext, typed_res),
                                     MIR_new_reg_op(fContext, popValue())));

        // Merge block
        MIR_append_insn(fContext, fCompute, merge_block);

        // Push result
        pushValue(typed_res);
    }

    void CompileBlock(MIR_label_t code_block);
//    {
//        MIR_append_insn(fContext, fCompute, code_block);
//    }

    void Execute(int* int_heap, REAL* real_heap, REAL** inputs, REAL** outputs)
        {
            fCompiledFun((MIR_val_t){.a = (void*)int_heap},
                         (MIR_val_t){.a = (void*)real_heap},
                         (MIR_val_t){.a = (void*)inputs},
                         (MIR_val_t){.a = (void*)outputs});
            /*
            // Interpreter mode
            MIR_interp(fContext, fCompute, NULL, 4,
                       (MIR_val_t){.a = (void*)int_heap},
                       (MIR_val_t){.a = (void*)real_heap},
                       (MIR_val_t){.a = (void*)inputs},
                       (MIR_val_t){.a = (void*)outputs});
            */
        }

    static void* importResolver(const char* name)
    {
        assert(gMathLib.find(name) != gMathLib.end());
        return gMathLib[name];
    }

private:
    typedef void (*compiledFun)(MIR_val_t int_heap, MIR_val_t real_heap,
                                MIR_val_t inputs, MIR_val_t outputs);
    compiledFun fCompiledFun;

    MIR_context_t fContext;
    MIR_item_t fCompute;

    MIR_reg_t fMIRStack[512];

    int fMIRStackIndex;
    int fAddrStackIndex;

    MIR_reg_t fMIRIntHeap;
    MIR_reg_t fMIRRealHeap;
    MIR_reg_t fMIRInputs;
    MIR_reg_t fMIROutputs;
    //
    std::map<std::string, int> fIDCounters;
    std::map<std::string, MIR_item_t> fFunProto;
    static std::map<std::string, void*> gMathLib;
    std::string fIdent;
};

#endif // COMPILER_H
