

## spec doc
- MIR_insn_code_t: 操作数(汇编内的概念)
```
大多数 MIR insn 具有目标操作数和一两个源操作数。 目标只能是寄存器或内存。
```

## func spec
- MIR_context_t
```
//init
MIR_context_t MIR_init (void)
//destroy
MIR_finish (MIR_context_t ctx)
// 输出mir 到指定的文件
MIR_output (MIR_context_t ctx, FILE *f) 
// 扫描文本的mir 到ctx
MIR_scan_string (MIR_context_t ctx, const char *str)
// write/read 二进制的 mir
MIR_write (MIR_context_t ctx, FILE *f),
MIR_read (MIR_context_t ctx, FILE *f)
MIR_write_with_func (MIR_context_t ctx, const int (*writer_func) (MIR_context_t, uint8_t))
MIR_read_with_func (MIR_context_t ctx, const int (*reader_func) (MIR_context_t))
```

- data type
```
MIR_T_I8 and MIR_T_U8 -- signed and unsigned 8-bit integer values
MIR_T_I16 and MIR_T_U16 -- signed and unsigned 16-bit integer values
MIR_T_I32 and MIR_T_U32 -- signed and unsigned 32-bit integer values
MIR_T_I64 and MIR_T_U64 -- signed and unsigned 64-bit integer values
MIR_T_F and MIR_T_D -- IEEE single and double precision floating point values
MIR_T_LD - long double values.
MIR_T_BLK .. MIR_T_BLK + MIR_BLK_NUM - 1 -- 
	给定情况下的块数据。 该类型只能用于函数的参数。 
	不同的案例编号可以表示在特定目标上传递块数据以实现目标调用 ABI 的不同方式。
	目前有 6 种块类型 (MIR_BLK_NUM = 5)
MIR_T_RBLK -- return block data. This type can be used only for argument of function
MIR textual representation of the types are correspondingly i8, u8, i16, u16, i32, u32, i64, u64, f, d, ld, p, and blk
Function int MIR_int_type_p (MIR_type_t t) returns TRUE if given type is an integer one (it includes pointer type too)
Function int MIR_fp_type_p (MIR_type_t t) returns TRUE if given type is a floating point type
```

- MIR_module_t 
```
//create
MIR_module_t MIR_new_module (const char *name)
//finish/end
MIR_finish_module
//get all module
DLIST (MIR_module_t) *MIR_get_module_list (MIR_context_t ctx)

//item types (and function for their creation):
Function: MIR_func_item

Import: MIR_import_item (MIR_item_t MIR_new_import (MIR_context_t ctx, const char *name))

Export: MIR_export_item (MIR_item_t MIR_new_export (MIR_context_t ctx, const char *name))

Forward declaration: MIR_forward_item (MIR_item_t MIR_new_forward (MIR_context_t ctx, const char *name))

//Prototype: MIR_proto_item 
(MIR_new_proto_arr, MIR_new_proto, MIR_new_vararg_proto_arr, MIR_new_vararg_proto 
analogous to MIR_new_func_arr, MIR_new_func, MIR_new_vararg_func_arr and MIR_new_vararg_func -- see below). 
The only difference is that two or more prototype argument names can be the same

//Data: MIR_data_item with optional name 
(MIR_item_t MIR_new_data (MIR_context_t ctx, const char *name, MIR_type_t el_type, size_t nel, const void *els) 
or MIR_item_t MIR_new_string_data (MIR_context_t ctx, const char *name, MIR_str_t str))

Reference data: MIR_ref_data_item with optional name 
(MIR_item_t MIR_new_ref_data (MIR_context_t ctx, const char *name, MIR_item_t item, int64_t disp)

链接后的item地址加上disp用来初始化数据
Expression Data: MIR_expr_data_item with optional name 
(`MIR_item_t MIR_new_expr_data (MIR_context_t ctx, const char *****name, MIR_item_t expr_item))

并非所有 MIR 函数都可用于表达式数据。 表达式函数应该只有一个结果，没有参数，不使用任何调用或任何有内存的指令
在链接期间调用表达式函数，其结果用于初始化数据
Memory segment: MIR_bss_item with optional name 
(MIR_item_t MIR_new_bss (MIR_context_t ctx, const char *name, size_t len))

Names of MIR functions, imports, and prototypes should be unique in a module

API functions MIR_output_item (MIR_context_t ctx, FILE *f, MIR_item_t item) and 
MIR_output_module (MIR_context_t ctx, FILE *f, MIR_module_t module) output item 
or module textual representation into given file
```

- MIR_new_insn(ctx, MIR_MOV/etc, reg_op, value)
```
MIR_reg_t var_real = createVar(getRealTy(), "var_real");
 MIR_append_insn(fContext, fCompute, MIR_new_insn(fContext,
//                                                                     MIR_FMOV),
//                                                                     MIR_new_reg_op(fContext, var_real),
//                                                                     genReal((*it)->fRealValue)))
```
## jit compile 技术
```
libjit
libgccjit
GNU Lightning
MIR
//asmjit

除了单个测试点不足以得出结论外，我们仍然可以看到一些模式。 GNU Lightning 是最低级别的 JIT 框架，它编译代码的速度最快，
几乎没有优化开销。这导致代码执行速度相对较慢，尽管仍然比解释器领先几英里。 GNU Lightning 还具有被移植到大量架构的优势，
尽管它与 GCC 相比仍然相形见绌，GCC 可以为 GCC 支持的基本上所有架构编译代码。
但是请注意，此示例中的 JIT 在一定程度上是手动优化的，如果在脚本语言中使用，实施者将负责更合理的 JIT 使用，
并尽量避免不必要的加载/存储等.其他框架更宽容，具有更高级别的抽象和一些优化通道。

GCC 拥有所有经过测试的框架中最好的优化例程，尽管在这个简单的示例中这并不是很明显。显而易见的是，
优化和成熟的编译器会导致代码编译速度变慢。 （如果您发现我使用 GCCJIT 的方式会导致这种大幅减速，请告诉我）。
当您绝对需要生成尽可能快的代码并且编译速度几乎无关紧要时，GCCJIT 可能是合适的。这确实不适合脚本语言使用。

LibJIT 提供了相当快的编译时间，并将其与不错的代码执行时间配对。该框架的一些缺点包括有限的机器代码架构支持，
但它确实带有与架构无关的解释器。目前似乎完全支持 x86/64 和 arm，alpha 支持在 alpha 中。
看到 RISC-V 或其他东西会很酷，但我离题了。

MIR 编译是三个纯 JIT 项目中最慢的，但确实提供了比 LibJIT 更多的优化。架构机器代码支持似乎也好很多，
尽管目前似乎只支持 Linux 作为操作系统。 MIR 目前非常具有实验性，但看起来确实很有希望。

对于一个较小的类似爱好的项目，我可能会选择 LibJIT，因为它相当容易使用，并且在以编程方式使用时似乎相当宽容。
我不喜欢它不支持那么多架构的事实，如果我的目标是更广泛使用但仍然很小的软件项目，我可能会选择 Lightning。 
MIR 似乎最适合大型项目，其中有足够的资源来缓解使用某种混合解释/编译系统的较慢编译。
 MIR 似乎提供了这个 OOTB，虽然我不知道目前解释的 MIR 有什么样的延迟。
``` 
- from https://github.com/Kimplul/jit-comparison
 
## abi
- __aarch64__: armv8
- s390x:  IBM System z 系列 (zSeries)大型机 (mainframe) 硬件平台
- ppc64： Linux和GCC开源软件社区内常用，指向目标架构为64位PowerPC和Power Architecture处理器，
	并对此优化的应用程序的标识符，编译源代码时经常使用。
- __riscv： 基于精简指令集（RISC）原则的开源指令集架构（ISA）
