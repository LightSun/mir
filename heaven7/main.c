
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

// test ok.
// gen mir: ./c2m -S test.c   -> test.mir
int main(int argc, char *argv[]){
    const char* file = "/home/heaven7/heaven7/temp/"
                       "build-mir-Desktop_Qt_5_14_2_GCC_64bit-Debug/test.mir";
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
    MIR_load_external (ctx, "printf", printf);

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
    //execute
    MIR_link (ctx, MIR_set_gen_interface, NULL);
    Addr fun_addr = MIR_gen (ctx, 0, main_func);
    int res = fun_addr (1, 2);
    fprintf (stderr, "%s: %d\n", file, res);

    MIR_gen_finish (ctx);

    MIR_finish (ctx);
    our_exit(0);
    return 0;
}
