#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <vector>
#include "../mir.h"
#include "../mir-gen.h"

//full example
//https://github.com/theMackabu/ant/blob/0950cdbaf937cc11dabdb23beb5f2ac5df6aa7ff/src/silver/swarm.c

// 我们要在 MIR 里操作的结构体
typedef struct {
  int   a;    // 偏移 0  (4 字节)
  float b;    // 偏移 4  (4 字节)
  char  c;    // 偏移 8  (1 字节)
} MyStruct;

static MIR_item_t createMallocFunc(MIR_context_t ctx){
  MIR_type_t malloc_ret = MIR_T_P; // void*
  MIR_type_t malloc_arg = MIR_T_U32; // size_t
  MIR_item_t malloc_func = MIR_new_func(
    ctx, "malloc",
    1, &malloc_ret,  // 1个返回值
    1, &malloc_arg    // 1个参数
    );
  MIR_finish_func(ctx); // 无体，即外部函数
  return malloc_func;
}

static MIR_item_t createFreeFunc(MIR_context_t ctx){
  MIR_type_t malloc_arg = MIR_T_P; // void*
  MIR_item_t malloc_func = MIR_new_func(
    ctx, "free",
    0, NULL,
    1, &malloc_arg
    );
  MIR_finish_func(ctx);
  return malloc_func;
}

static MIR_item_t newMallocProto(MIR_context_t ctx){
  MIR_type_t malloc_ret = MIR_T_P; // void*
  //MIR_type_t malloc_arg = MIR_T_U32; // size_t
  MIR_item_t proto = MIR_new_proto(
    ctx, "malloc0",
    1, &malloc_ret,  // 1个返回值
    1, // 1个参数
    MIR_T_U32, "s"
    );
  return proto;
}
static MIR_item_t newFreeProto(MIR_context_t ctx){
  MIR_item_t proto = MIR_new_proto(
    ctx, "free0",
    0, NULL,
    1,
    MIR_T_P, "ptr"
    );
  return proto;
}

static void mir_i64_to_d(MIR_context_t ctx, MIR_item_t fn,
                          MIR_reg_t dst_d, MIR_reg_t src_i64,
                          MIR_reg_t slot) {
  MIR_append_insn(ctx, fn,
                   MIR_new_insn(ctx, MIR_MOV,
                                 MIR_new_mem_op(ctx, MIR_T_I64, 0, slot, 0, 1),
                                 MIR_new_reg_op(ctx, src_i64)));
  MIR_append_insn(ctx, fn,
                   MIR_new_insn(ctx, MIR_DMOV,
                                 MIR_new_reg_op(ctx, dst_d),
                                 MIR_new_mem_op(ctx, MIR_T_D, 0, slot, 0, 1)));
}

static void mir_d_to_i64(MIR_context_t ctx, MIR_item_t fn,
                          MIR_reg_t dst_i64, MIR_reg_t src_d,
                          MIR_reg_t slot) {
  MIR_append_insn(ctx, fn,
                   MIR_new_insn(ctx, MIR_DMOV,
                                 MIR_new_mem_op(ctx, MIR_T_D, 0, slot, 0, 1),
                                 MIR_new_reg_op(ctx, src_d)));
  MIR_append_insn(ctx, fn,
                   MIR_new_insn(ctx, MIR_MOV,
                                 MIR_new_reg_op(ctx, dst_i64),
                                 MIR_new_mem_op(ctx, MIR_T_I64, 0, slot, 0, 1)));
}

/**
 mir_call_helper2(ctx, jit_func, rd,
                           helper2_proto, imp_add,
                           r_vm, r_js, rl, rr);
 */
//not done.
void test_mir_struct() {
  // 1. 初始化 MIR 上下文
  MIR_context_t ctx = MIR_init();

  // 2. 创建模块
  MIR_module_t mod = MIR_new_module(ctx, "struct_demo");

  // 3. 声明外部函数 malloc 和 free
  //    malloc:  size_t -> void*   (size_t 用 MIR_T_U64 表示)
  //    free:    void* -> void
//  MIR_item_t malloc_import = MIR_new_import(ctx, "malloc");
//  MIR_item_t free_import = MIR_new_import(ctx, "free");

  MIR_load_external (ctx, "abort", (void*)abort);
  MIR_load_external (ctx, "exit", (void*)exit);
  MIR_load_external (ctx, "printf", (void*)printf);
  MIR_load_external (ctx, "malloc", (void*)malloc);
  MIR_load_external (ctx, "free", (void*)free);
  //auto func_malloc = MIR_get_item_func(ctx, free_item);
  auto proto_malloc = newMallocProto(ctx);
  auto proto_free = newFreeProto(ctx);
  //MIR_link() //
  auto malloc_item = MIR_new_import(ctx, "malloc");
  auto free_item = MIR_new_import(ctx, "free");

//  MIR_type_t params_malloc_proto[] = {MIR_T_P};
//  MIR_item_t malloc_proto = MIR_new_proto(ctx, "malloc_proto",
//                                           1, params_malloc_proto,
//                                           0);                 // variadic_kind = 0

//  MIR_item_t free_proto = MIR_new_proto(ctx, "free_proto",
//                                         0, nullptr,
//                                         1, params_malloc_proto);
  //    MIR_set_func_proto(ctx, malloc_import, malloc_proto);


  // 4. 创建 JIT 函数：int create_and_sum_point(void)
  //    无参数，返回 int (两个成员的和)
  MIR_type_t ret_proto[] = {MIR_T_I32};
  MIR_item_t func = MIR_new_func(ctx, "create_and_sum_point", 1, ret_proto, 0,
                                   NULL);

  // 5. 定义函数内部使用的寄存器
  //    ptr       : 指向 Point 结构体的指针 (MIR_T_P 即 void*)
  //    val_x, val_y : 存放读取的字段值
  //    sum       : 临时和
  //    result    : 返回值
  MIR_reg_t reg_ptr   = MIR_new_func_reg(ctx, func->u.func, MIR_T_P, "ptr");
  MIR_reg_t reg_val_x = MIR_new_func_reg(ctx, func->u.func, MIR_T_I32, "val_x");
  MIR_reg_t reg_val_y = MIR_new_func_reg(ctx, func->u.func, MIR_T_I32, "val_y");
  MIR_reg_t reg_sum   = MIR_new_func_reg(ctx, func->u.func, MIR_T_I32, "sum");
  MIR_reg_t reg_res   = MIR_new_func_reg(ctx, func->u.func, MIR_T_I32, "res");

  // 6. 创建基本块
  //MIR_label_t entry = MIR_new_label(ctx);

  // 7. 生成指令序列
  MIR_insn_t insns[20];
  int icnt = 0;

  // 7.1 调用 malloc(8) 分配 8 字节 (两个 int)
  //     准备参数：立即数 8
  //MIR_new_ref_op, MIR_new_reg_op
  //返回值存入 reg_ptr
  MIR_op_t malloc_arg = MIR_new_uint_op(ctx, 8);
  MIR_insn_t call_malloc = MIR_new_call_insn(ctx, 4,
                                              MIR_new_ref_op(ctx, proto_malloc),
                                              MIR_new_ref_op(ctx, malloc_item),
                                              MIR_new_reg_op(ctx, reg_ptr),
                                              malloc_arg
                                              );
  insns[icnt++] = call_malloc;

  // 7.2 检查分配是否成功 (可选，本例略)

  // 7.3 向结构体写入 x = 10 (偏移 0)
  //     先构造内存操作数 [reg_ptr + 0]
  //最终地址 = disp + base_reg + (index_reg * scale)
  MIR_op_t mem_x = MIR_new_mem_op(ctx, MIR_T_I32, 0, reg_ptr, -1, 0);
  MIR_insn_t store_x = MIR_new_insn(ctx, MIR_MOV, mem_x, MIR_new_int_op(ctx, 10));
  insns[icnt++] = store_x;

  // 7.4 向结构体写入 y = 20 (偏移 4)
  MIR_op_t mem_y = MIR_new_mem_op(ctx, MIR_T_I32, 4, reg_ptr, -1, 0);
  MIR_insn_t store_y = MIR_new_insn(ctx, MIR_MOV, mem_y, MIR_new_int_op(ctx, 20));
  insns[icnt++] = store_y;

  // 7.5 从结构体读取 x 到 reg_val_x
  MIR_insn_t load_x = MIR_new_insn(ctx, MIR_MOV, reg_val_x, mem_x);
  insns[icnt++] = load_x;

  // 7.6 从结构体读取 y 到 reg_val_y
  MIR_insn_t load_y = MIR_new_insn(ctx, MIR_MOV, reg_val_y, mem_y);
  insns[icnt++] = load_y;

  // 7.7 加法: reg_sum = reg_val_x + reg_val_y
  MIR_insn_t add_insn = MIR_new_insn(ctx, MIR_ADD,
                                      reg_sum, reg_val_x, reg_val_y);
  insns[icnt++] = add_insn;

  // 7.8 将 reg_sum 移动到返回值寄存器 reg_res
  MIR_insn_t mov_insn = MIR_new_insn(ctx, MIR_MOV, reg_res, reg_sum);
  insns[icnt++] = mov_insn;

  // 7.9 调用 free(ptr) 释放内存. 无返回值
  MIR_op_t free_arg = MIR_new_reg_op(ctx, reg_ptr);
  MIR_insn_t call_free = MIR_new_call_insn(ctx, 3,
                                            MIR_new_ref_op(ctx, proto_free),
                                            MIR_new_ref_op(ctx, free_item),
                                            free_arg
                                            );
  insns[icnt++] = call_free;

  // 7.10 返回 reg_res
  MIR_insn_t ret_insn = MIR_new_ret_insn(ctx, 1, MIR_new_reg_op(ctx, reg_res));
  insns[icnt++] = ret_insn;

  // 8. 将指令添加到基本块
  for(int i = 0; i < icnt ; ++i){
    MIR_append_insn(ctx, func, insns[i]);
  }
  MIR_finish_func(ctx);
  MIR_finish_module(ctx);

  // 11. JIT 编译
  MIR_load_module(ctx, mod);
  MIR_output (ctx, stderr);
  MIR_link(ctx, MIR_set_gen_interface, NULL);

  // 12. 获取 JIT 函数指针
  std::vector<MIR_item_t> funcs;
  MIR_module_t m = DLIST_TAIL(MIR_module_t, *MIR_get_module_list(ctx));
  for (MIR_item_t item = DLIST_HEAD(MIR_item_t, m->items); item != NULL;
       item = DLIST_NEXT(MIR_item_t, item)) {
    if (item->item_type == MIR_func_item)
      funcs.push_back(item);
  }
  if (funcs.empty()) {
    fprintf(stderr, "找不到 JIT 函数\n");
    MIR_finish(ctx);
    return;
  }
  auto func_item = funcs[0];
  int (*jit_func)(void) = (int (*)(void)) MIR_gen(ctx, 0, func_item);

  // 13. 调用 JIT 函数并输出结果
  int result = jit_func();
  printf("JIT 函数返回的和: %d\n", result);   // 预期输出 30

  // 14. 清理 MIR 上下文
  MIR_finish(ctx);
}
