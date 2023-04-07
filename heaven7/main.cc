
#include <string.h>
#include "../mir.h"
#include "../mir-gen.h"
#include "test-read.h"

static char *program_str = NULL;
static void our_exit (int code) {
  if (program_str != NULL) free (program_str);
  exit (code);
}
typedef int(*Addr)(int,int);
typedef unsigned long (*jit_loop_t)(unsigned long);


void test_struct0(){
    MIR_module_t mir_module;
    MIR_context_t ctx;
    MIR_item_t mir_func;
    //
    ctx = MIR_init ();
    //gen
    MIR_gen_init(ctx, 0);
    MIR_gen_set_optimize_level(ctx, 0, 2);
    MIR_link(ctx, MIR_set_gen_interface, NULL);

    mir_module = MIR_new_module(ctx, "test_struct");
}

/**
unsigned long loop(unsigned long n)
{
  unsigned long sum = 0;
  for(unsigned long i = 0; i <= n; ++i)
    sum += i;

   return sum;
}
 */
void test_loop(){
    MIR_module_t mir_module;
    MIR_context_t ctx;
    MIR_item_t mir_func;
    //
    ctx = MIR_init ();

    //gen
    MIR_gen_init(ctx, 0);
    MIR_gen_set_optimize_level(ctx, 0, 2);
    MIR_link(ctx, MIR_set_gen_interface, NULL);

    mir_module = MIR_new_module(ctx, "m");

    MIR_type_t res_type[1] = {MIR_T_U64};
    mir_func = MIR_new_func(ctx, "loop", 1, res_type, 1, MIR_T_U64, "n");
    //get param 'n'
    MIR_reg_t n = MIR_reg(ctx, "n", mir_func->u.func);
    MIR_reg_t i = MIR_new_func_reg(ctx, mir_func->u.func, MIR_T_I64, "i");
    MIR_reg_t sum = MIR_new_func_reg(ctx, mir_func->u.func, MIR_T_I64, "sum");

    MIR_label_t cond = MIR_new_label(ctx);
    MIR_label_t out = MIR_new_label(ctx);

    /* sum = 0 */
    MIR_append_insn(ctx, mir_func,
            MIR_new_insn(ctx, MIR_MOV,
                MIR_new_reg_op(ctx, sum),
                MIR_new_uint_op(ctx, 0)));
    /* i = 0 */
    MIR_append_insn(ctx, mir_func,
            MIR_new_insn(ctx, MIR_MOV,
                MIR_new_reg_op(ctx, i),
                MIR_new_uint_op(ctx, 0)));

    /* loop top */
    // if i > n , jump to out.
    MIR_append_insn(ctx, mir_func, cond);
    MIR_append_insn(ctx, mir_func,
            MIR_new_insn(ctx, MIR_UBGT,// >
                MIR_new_label_op(ctx, out),
                MIR_new_reg_op(ctx, i),
                MIR_new_reg_op(ctx, n)));

    /* sum += i */
    MIR_append_insn(ctx, mir_func,
            MIR_new_insn(ctx, MIR_ADD,
                MIR_new_reg_op(ctx, sum),
                MIR_new_reg_op(ctx, sum),
                MIR_new_reg_op(ctx, i)));

    /* ++i */
    MIR_append_insn(ctx, mir_func,
            MIR_new_insn(ctx, MIR_ADD,
                MIR_new_reg_op(ctx, i),
                MIR_new_reg_op(ctx, i),
                MIR_new_uint_op(ctx, 1)));

    /* jump to cond */
    MIR_append_insn(ctx, mir_func,
            MIR_new_insn(ctx, MIR_JMP, MIR_new_label_op(ctx, cond)));
    //out:
    MIR_append_insn(ctx, mir_func, out);

    MIR_append_insn(ctx, mir_func,
            MIR_new_ret_insn(ctx, 1, MIR_new_reg_op(ctx, sum)));

    MIR_finish_func(ctx);
    //export symbol
    MIR_new_export(ctx, mir_func->u.func->name);
    //end module
    MIR_finish_module(ctx);
    MIR_load_module(ctx, mir_func->module);

    //log
    MIR_output (ctx, stderr);
    //
    jit_loop_t  func_addr = (jit_loop_t)MIR_gen(ctx, 0, mir_func);
    int res = func_addr(5);//15
    fprintf (stdout, "test_loop add result: %d\n", res);
    //final
    MIR_gen_finish (ctx);
    MIR_finish (ctx);
}

/**
// mir detail: https://github.com/vnmakarov/mir/blob/master/MIR.md
int add(int a, int b){
    return a + b;
}
*/
void test_mir_gen(){
    MIR_module_t mir_module;
    MIR_context_t ctx;
    MIR_item_t func;
    //
    ctx = MIR_init ();

    //gen
    MIR_gen_init(ctx, 0);
    MIR_gen_set_optimize_level(ctx, 0, 2);
    MIR_link(ctx, MIR_set_gen_interface, NULL);

    mir_module = MIR_new_module(ctx, "test_mir_gen");
    //func sig
    MIR_type_t res_type[1] = {MIR_T_I32};
    func = MIR_new_func(ctx, "add", 1, res_type, 2,
                 MIR_T_I32, "a",
                 MIR_T_I32, "b"
                 );
    //get func arg
    MIR_reg_t a = MIR_reg(ctx, "a", func->u.func);
    MIR_reg_t b = MIR_reg(ctx, "b", func->u.func);
    //local var. here should use int64
    MIR_reg_t ret = MIR_new_func_reg(ctx, func->u.func, MIR_T_I64, "ret");
    //...MIR_new_uint_op(ctx, 0) create value op.
    //ret = a +b
    MIR_append_insn(ctx, func,
                MIR_new_insn(ctx, MIR_ADDS,
                    MIR_new_reg_op(ctx, ret),
                    MIR_new_reg_op(ctx, a),
                    MIR_new_reg_op(ctx, b))
                    );
    //return ret
    MIR_append_insn(ctx, func,
                MIR_new_ret_insn(ctx, 1, MIR_new_reg_op(ctx, ret)));
    //post
    MIR_finish_func(ctx);   // endfunc
    MIR_finish_module(ctx); // endmodule
    // gen func
    MIR_load_module(ctx, func->module);
    //log
    MIR_output (ctx, stderr);

    Addr add = (Addr)MIR_gen(ctx, 0, func);
    int res = add(1, 2);
    fprintf (stdout, "test_mir_gen add result: %d\n", res);
    //final
    MIR_gen_finish (ctx);
    MIR_finish (ctx);
}

void test_read_struct(){
    const char* file = "/home/heaven7/heaven7/study/github/mine_temp/"
                       "mir/heaven7/struct_test.mir";
    MIR_module_t mir_module;
    MIR_context_t ctx;
    MIR_item_t f;
    ctx = MIR_init ();
    program_str = read_file (file);
    MIR_scan_string (ctx, program_str);
    mir_module = DLIST_HEAD (MIR_module_t, *MIR_get_module_list (ctx));
    //
    for (f = DLIST_HEAD (MIR_item_t, mir_module->items); f != NULL;
         f = DLIST_NEXT (MIR_item_t, f)){
      switch (f->item_type) {
      case  MIR_func_item:{
          printf("func: name = %s\n", f->u.func->name);
      }break;

      case  MIR_proto_item:{
          printf("proto: name = %s\n", f->u.proto->name);
      }break;

      case  MIR_import_item:{
          printf("import: name = %s\n", f->u.import_id);
      }break;
      case  MIR_export_item:{
          printf("export: name = %s\n", f->u.export_id);
      }break;
      case  MIR_forward_item:{
          printf("forward: name = %s\n", f->u.forward_id);
      }break;
      case  MIR_data_item:{
          printf("data: name = %s\n", f->u.data->name);
      }break;

      case  MIR_ref_data_item:{
          printf("ref_data: name = %s\n", f->u.ref_data->name);
      }break;
      case  MIR_expr_data_item:{
          printf("expr_data: name = %s\n", f->u.expr_data->name);
      }break;
      case  MIR_bss_item:{
          printf("bss: name = %s\n", f->u.bss->name);
      }break;
      }
    }
    MIR_finish (ctx);
    our_exit(0);
}

static int test_hello(int argc, char **argv);

// test ok.
// gen mir: ./c2m -S test.c   -> test.mir
int main(int argc, char *argv[]){
    setbuf(stdout, NULL);
    //test_read_struct();
    //test_mir_gen();
    test_loop();

    const char* file = "/home/heaven7/heaven7/study/github/"
                       "mine_temp/mir/heaven7/test.mir";
    MIR_module_t mir_module;
    MIR_context_t ctx;
    MIR_item_t f, main_func;
     //

    ctx = MIR_init ();
    program_str = read_file (file);
    MIR_scan_string (ctx, program_str);
    mir_module = DLIST_HEAD (MIR_module_t, *MIR_get_module_list (ctx));
    //
    for (f = DLIST_HEAD (MIR_item_t, mir_module->items); f != NULL; f = DLIST_NEXT (MIR_item_t, f))
      if (f->item_type == MIR_func_item && strcmp (f->u.func->name, "add") == 0) main_func = f;
    if (main_func == NULL) {
      fprintf (stderr, "%s: cannot execute program w/o add function\n", argv[0]);
      our_exit (1);
    }

    /* ctx is a context created by MIR_init */
    MIR_load_module (ctx, mir_module);
    MIR_load_external (ctx, "printf", (void*)printf);

    MIR_output (ctx, stderr);
   // MIR_link (ctx, MIR_set_interp_interface, import_resolver);

    /* or use MIR_set_gen_interface to generate and use the machine code */
    /* or use MIR_set_lazy_gen_interface to generate function code on its 1st call */
    /* use MIR_gen (ctx, func) to explicitly generate the function machine code */
   // MIR_interp (ctx, func, &result, 0); /* zero here is arguments number  */
    /* or ((void (*) (void)) func->addr) (); to call interpr. or gen. code through the interface */
    MIR_gen_init (ctx, 1);
    //debug if need
    MIR_gen_set_debug_file (ctx, 0, stderr);
    //gen and execute
    MIR_link (ctx, MIR_set_gen_interface, NULL);
    Addr fun_addr = (Addr)MIR_gen (ctx, 0, main_func);
    int res = fun_addr (1, 2);
    fprintf (stderr, "%s: %d\n", file, res);

    MIR_gen_finish (ctx);

    MIR_finish (ctx);
    our_exit(0);
    return 0;
}

int test_hello(int argc, char **argv)
{
  int interp_p = 0, gen_p = 0;
  switch (argc) {
  case 1:
    break;
  case 2:
    interp_p = !strcmp(argv[1], "-i");
    if (interp_p)
      break;
    gen_p = !strcmp(argv[1], "-g");
    if (gen_p)
      break;
    fprintf (stderr, "%s: unknown option %s\n", argv[0], argv[1]);
    return 1;
  default:
    fprintf (stderr, "%s: [-i|-g]\n", argv[0]);
    return 1;
  }

  MIR_context_t ctx = MIR_init();
  MIR_module_t mir_module = MIR_new_module(ctx, "hello");
  MIR_item_t gv = MIR_new_data(ctx, "greetings", MIR_T_U8, 12,
                               "world\0all\0\0");
  MIR_type_t i = MIR_T_I32;
  MIR_item_t callback = MIR_new_proto(ctx, "cb", 1, &i, 1, MIR_T_P, "string");
  MIR_item_t func = MIR_new_func(ctx, "hello", 1, &i, 3,
                                 MIR_T_P, "string", MIR_T_P, "callback",
                                 MIR_T_I32, "id");
  MIR_reg_t temp = MIR_new_func_reg(ctx, func->u.func, MIR_T_I64, "$temp");
  MIR_reg_t ret = MIR_new_func_reg(ctx, func->u.func, MIR_T_I64, "$ret");
  MIR_reg_t string = MIR_reg(ctx, "string", func->u.func),
    cb = MIR_reg(ctx, "callback", func->u.func),
    id = MIR_reg(ctx, "id", func->u.func);
  MIR_append_insn(ctx, func,
                  MIR_new_insn(ctx, MIR_MOV,
                               MIR_new_reg_op(ctx, temp),
                               MIR_new_ref_op(ctx, gv)));
  MIR_append_insn(ctx, func,
                  MIR_new_insn(ctx, MIR_MUL, MIR_new_reg_op(ctx, id),
                               MIR_new_reg_op(ctx, id),
                               MIR_new_int_op(ctx, 6)));
  MIR_append_insn(ctx, func,
                  MIR_new_insn(ctx, MIR_ADD, MIR_new_reg_op(ctx, id),
                               MIR_new_reg_op(ctx, id),
                               MIR_new_reg_op(ctx, temp)));
  MIR_append_insn(ctx, func,
                  MIR_new_call_insn(ctx, 4, MIR_new_ref_op(ctx, callback),
                                    MIR_new_reg_op(ctx, cb),
                                    MIR_new_reg_op(ctx, ret),
                                    MIR_new_reg_op(ctx, string)));
  MIR_append_insn(ctx, func,
                  MIR_new_call_insn(ctx, 4, MIR_new_ref_op(ctx, callback),
                                    MIR_new_reg_op(ctx, cb),
                                    MIR_new_reg_op(ctx, temp),
                                    MIR_new_reg_op(ctx, id)));
  MIR_append_insn(ctx, func,
                  MIR_new_insn(ctx, MIR_ADD, MIR_new_reg_op(ctx, ret),
                               MIR_new_reg_op(ctx, ret),
                               MIR_new_reg_op(ctx, temp)));
  MIR_append_insn(ctx, func,
                  MIR_new_ret_insn(ctx, 1, MIR_new_reg_op(ctx, ret)));
  MIR_finish_func(ctx);
  MIR_finish_module(ctx);
  // MIR_output(ctx, stderr);
  MIR_load_module(ctx, mir_module);
  // MIR_output(ctx, stderr);
  MIR_gen_init(ctx, 1);
  typedef int (*Callback)(const char*);
  typedef int (*Func_boo) (const char *, Callback, unsigned);
  Func_boo boo = NULL;
  if (interp_p) {
    MIR_link(ctx, MIR_set_interp_interface, NULL);
    boo = (Func_boo)func->addr;
  } else if (gen_p) {
    MIR_link(ctx, MIR_set_gen_interface, NULL);
    boo = (Func_boo)MIR_gen(ctx, 0, func);
  } else {
    MIR_output(ctx, stderr);
    goto cleanup;
  }
  printf("%s: %d\n", interp_p ? "interpreted" : "compiled",
         boo("hello", puts, 0) + boo("goodbye", puts, 1));
cleanup:
  MIR_gen_finish(ctx);
  MIR_finish(ctx);
  return 0;
}
