#ifndef MIRCOMPILER_H
#define MIRCOMPILER_H

#include "mir.h"
#include <string>
#include <memory.h>

// https://github.com/grame-cncm/faust/blob/master-dev/
//          compiler/generator/interpreter/fbc_mir_compiler.hh,
//          see use of fSoundArrayTable, fSoundTableID and fMIRSoundTable.
//fSoundTable: <string,SoundTable*>
//fSoundTableID: <string,int(index)>.
//fSoundArrayTable: 显示创建数组。

/*
 MIR_disp_t: 地址偏移量
 MIR_scale_t: 内存中的索引单位长度, 比如 '索引int*' 单位长度就是4,
                                '索引double*' 单位长度是8
*/
namespace h7 {

    class MIRCompiler{

    public:
        using CString = const std::string&;
        using String = std::string;
        //获取函数参数
        //fMIRSoundTable = MIR_reg(fContext, "sound_table", fCompute->u.func);

        //-----------------------
        void testStruct(){
            /**
              struct Student{int age, bool male, char name[32]};
            */
            int len = 40;
            void* addr = malloc(len);
            memset(addr, 0, len);
            auto data_item = MIR_new_data(fContext, "ptr_stu",
                                          getInt8(), len, addr);
        }
        void newStruct(CString name){
            MIR_reg_t reg_struct = MIR_new_func_reg(fContext, fCompute->u.func,
                                             MIR_T_I64, name.data());
            //MIR_new_bss()// global var
            //MIR_new_data
            MIR_reg_t fMIRSoundTable;//TODO
            MIR_reg_t sf_reg = createIndexReg(0);//TODO
            MIR_reg_t sf_ptr = createVar(getInt64Ty(), "sf_ptr");
            MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext, MIR_MOV,
                                         MIR_new_reg_op(fContext, sf_ptr),
                                         MIR_new_mem_op(fContext,
                                                        getInt64Ty(), 0,
                                                        fMIRSoundTable, sf_reg,
                                                        sizeof(int64_t))));

        }
        MIR_reg_t createIndexReg(int index)
       {
           //---- Load index in a local
           //int64 array_id;
           MIR_reg_t index_reg = createVar(getInt64Ty(), "array_id");
           //array_id = index
           MIR_append_insn(fContext, fCompute,
                             MIR_new_insn(fContext, MIR_MOV,
                                MIR_new_reg_op(fContext, index_reg),
                                genInt64(index)
                             )
                           );
           return index_reg;
       }

        MIR_reg_t createVar(MIR_type_t type, const std::string& name)
        {
            return MIR_new_func_reg(fContext, fCompute->u.func,
                                    type, name.data());
        }
        //
        void pushValue(MIR_reg_t val) { fMIRStack[fMIRStackIndex++] = val; }
        MIR_reg_t popValue() { return fMIRStack[--fMIRStackIndex]; }
        //常量
        MIR_op_t genFloat(float num) { return MIR_new_float_op(fContext, num); }
        MIR_op_t genDouble(double num) { return MIR_new_double_op(fContext, num); }
       // MIR_op_t genReal(double num) { return (sizeof(REAL) == sizeof(double)) ? genDouble(num) : genFloat(num); }
        MIR_op_t genInt32(int num) { return MIR_new_int_op(fContext, num); }
        MIR_op_t genInt64(int64_t num) { return MIR_new_int_op(fContext, num); }

        //类型
        MIR_type_t getFloatTy() { return MIR_T_F; }
        MIR_type_t getDoubleTy() { return MIR_T_D; }
       // MIR_type_t getRealTy() { return (sizeof(REAL) == sizeof(double)) ? getDoubleTy() : getFloatTy(); }
        MIR_type_t getInt32Ty() { return MIR_T_I32; }
        MIR_type_t getInt64Ty() { return MIR_T_I64; }
        MIR_type_t getInt8() { return MIR_T_I8; }
        MIR_type_t getInt16() { return MIR_T_I16; }
        MIR_type_t getBLK() { return MIR_T_BLK; }

    private:
         MIR_context_t fContext;
         MIR_item_t fCompute; // the function

         MIR_reg_t fMIRStack[512];
         int fMIRStackIndex{0};
    };
}

#endif // MIRCOMPILER_H
