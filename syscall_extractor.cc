#include <iostream>
#include <fstream>
// This is the first gcc header to be included
#include "gcc-plugin.h"
#include "plugin-version.h"

#include "tree-pass.h"
#include "context.h"
#include "function.h"
#include "tree.h"
#include "tree-ssa-alias.h"
#include "internal-fn.h"
#include "is-a.h"
#include "predict.h"
#include "basic-block.h"
#include "gimple-expr.h"
#include "gimple.h"
#include "gimple-pretty-print.h"
#include "gimple-iterator.h"
#include "gimple-walk.h"
#include "cgraph.h"
#include "rtl.h"

/*
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
*/

#include <vector>
#include <algorithm>


// We must assert that this plugin is GPL compatible
int plugin_is_GPL_compatible;

static struct plugin_info my_gcc_plugin_info = { "1.0", "This is a very simple plugin" };
static struct plugin_info rtl_plugin_info = { "1.0", "This is a RTL pass" };



namespace
{
    const pass_data my_first_pass_data = 
    {
        SIMPLE_IPA_PASS,
        "my_first_pass",        /* name */
        OPTGROUP_NONE,          /* optinfo_flags */
        TV_NONE,                /* tv_id */
        PROP_gimple_any | PROP_gimple_lcf,        /* properties_required */
        0,                      /* properties_provided */
        0,                      /* properties_destroyed */
        0,                      /* todo_flags_start */
        0                       /* todo_flags_finish */
    };

    struct my_first_pass : simple_ipa_opt_pass
    {
        private:
            std::ofstream outfile;

        public:

        my_first_pass(gcc::context *ctx)
            : simple_ipa_opt_pass(my_first_pass_data, ctx)
        {
            //std::cerr << "Created gimple pass\n";
        }

        
        virtual unsigned int execute(function *fun) override
        {

            //cgraph_node::dump_cgraph(stderr);
            // The asm doesn't exist here
            cgraph_node* node = nullptr;
            bool processed_toplev_asm = false;
            FOR_EACH_DEFINED_FUNCTION(node) {
                
                if (node->alias) {
                    cgraph_node* alias_node = node->ultimate_alias_target();
                    function* const alias_fn = alias_node->get_fun();
                    if (!outfile.is_open()) {
                        const char* file_name = LOCATION_FILE(DECL_SOURCE_LOCATION(alias_fn->decl));
                        // just so we don't screw up file_name
                        char* base = new char[strlen(file_name)+10];
                        strcpy(base, file_name);
                        char* outfile_name = strcat((char*)base, ".confine");
                        outfile.open(outfile_name, std::ios_base::app);
                    }
                    outfile << get_name(node->decl) << " : " << get_name(alias_fn->decl) << "\n";
                }

                function* const fn = node->get_fun();


                if (fn) {
                    std::cerr << "Function: " << get_name(fn->decl) << "\n";

                    if (!processed_toplev_asm) {
                        processed_toplev_asm = true;
                        asm_node *anode;

                        for (anode = symtab->first_asm_symbol (); anode; anode = anode->next) {
                            const char* asm_tree = TREE_STRING_POINTER(anode->asm_str);
                            char* copy = new char[strlen(asm_tree)+1];
                            strcpy(copy, asm_tree);

                            std::cerr << "found asm node: " << copy << "\n";

                            char* token = strtok((char*)copy, " ,@");
                            if (token != NULL) {
                                if (strncmp(token, ".symver", 7) == 0) {
                                    if (!outfile.is_open()) {
                                        const char* file_name = LOCATION_FILE(DECL_SOURCE_LOCATION(fn->decl));
                                        // just so we don't screw up file_name
                                        char* base = new char[strlen(file_name)+10];
                                        strcpy(base, file_name);
                                        char* outfile_name = strcat((char*)base, ".confine");
                                        outfile.open(outfile_name, std::ios_base::app);
                                    }

                                    // This is the symbol
                                    char* sym = strtok(NULL, " ,@");

                                    char* versym = strtok(NULL, " ,@");

                                    if (sym != NULL && versym != NULL) {
                                        std::cerr << "Found a symver\n";
                                        outfile << sym << " : " << versym << "\n";
                                        outfile << versym << " : " << sym << "\n";
                                    }
                                }
                            }
                        }
                    }

                    // Do this in RTL because some things seem missing
                    /*

                    struct cgraph_edge *edge;
                    for (edge = node->callees; edge; edge = edge->next_callee) {
                        struct cgraph_node* callee_node = edge->callee;
                        if (!outfile.is_open()) {
                            const char* file_name = LOCATION_FILE(DECL_SOURCE_LOCATION(node->get_fun()->decl));
                            // just so we don't screw up file_name
                            char* base = new char[strlen(file_name)+10];
                            strcpy(base, file_name);
                            char* outfile_name = strcat((char*)base, ".confine");
                            outfile.open(outfile_name, std::ios_base::app);
                        }
                        outfile << get_name(node->get_fun()->decl) << " : " << get_name(callee_node->decl) << "\n";
                    }
                    */
                    basic_block bb;
                    FOR_EACH_BB_FN(bb, fn) {
                        gimple_stmt_iterator si;
                        for (si = gsi_start_bb (bb); !gsi_end_p (si); gsi_next (&si)) {
#if __GNUC__ <= 5
                            gimple stmt = gsi_stmt (si);
#else
                            gimple* stmt = gsi_stmt(si);
#endif
                            
                            if (gimple_code(stmt) == GIMPLE_ASSIGN) {
                               
                                tree t2 = gimple_op(stmt, 1);
                                if (TREE_CODE(t2) == ADDR_EXPR) {
                                    // Get the operand, and need to verify if
                                    // it is a function
                                    tree type = TREE_TYPE(t2);
                                    if (TREE_CODE(type) == POINTER_TYPE) {
                                        tree typetype = TREE_TYPE(type);
                                        if (TREE_CODE(typetype) == FUNCTION_TYPE) {
                                            outfile << get_name(node->get_fun()->decl) <<  " : " << get_name(t2) << "\n";
                                        }
                                    }
                                }
                            }
                            //print_gimple_stmt(stderr, stmt, 0,0);
                        }
                    }
                }
                if (outfile.is_open()) {
                    outfile.close();
                }
            }

            // Nothing special todo
            return 0;
        }

        virtual my_first_pass* clone() override
        {
            // We do not clone ourselves
            return this;
        }
    };

    const pass_data pass_data_syscall_rtl =
    {
        RTL_PASS, /* type */
        "syscall_rtl", /* name */
        OPTGROUP_NONE, /* optinfo_flags */
        TV_NONE, /* tv_id */
        0, /* properties_required */
        0, /* properties_provided */
        0, /* properties_destroyed */
        0, /* todo_flags_start */
        0 /* todo_flags_finish */
    };

    class pass_syscall_rtl : public rtl_opt_pass
    {
        private:
            std::ofstream outfile;
        public:
            pass_syscall_rtl (gcc::context *ctxt)
                : rtl_opt_pass (pass_data_syscall_rtl, ctxt)
            {}

            void handle_insn(rtx_insn* insn, function* f) {
                if (!outfile.is_open()) {
                    const char* file_name = LOCATION_FILE(DECL_SOURCE_LOCATION(f->decl));
                    if (!file_name) {
                        file_name = get_name(f->decl);
                    }
                    // just so we don't screw up file_name
                    char* base = new char[strlen(file_name)+10];
                    strcpy(base, file_name);
                    char* outfile_name = strcat((char*)base, ".confine");
                    outfile.open(outfile_name, std::ios_base::app);
                }
                rtx def = PATTERN(insn);
                if (GET_CODE (def) == SET) 
                    def = XEXP (def, 1);
                if (GET_CODE (def) == CALL) {
                    rtx sym_to_call =  XEXP(def, 0);
                    rtx sym_ref = XEXP(sym_to_call, 0);
                    if (GET_CODE (sym_ref) == SYMBOL_REF) {

                        // This can be many types here: check out rtl.def for
                        // the correct compiler version to see more

                        tree sym_decl = SYMBOL_REF_DECL (sym_ref);
                        rtx op2 = XEXP(sym_ref, 1);
                        rtx op1 = XEXP(sym_ref, 0);

                        if (sym_decl) {
                            if (TREE_CODE(sym_decl) == FUNCTION_DECL) {
                                tree sym_decl = SYMBOL_REF_DECL (sym_ref);
                                outfile << get_name (f->decl) << " : " << get_name(sym_decl) << "\n";
                            }
                            /*
                        else if (TREE_CODE(op2) == CONST_STRING) {
                                const char* func_name = TREE_STRING_POINTER(op2);
                                outfile << get_name (f->decl) << " : " << func_name << "\n";
                            }
                            */
                        } else if (op1) {
                                const char* func_name = XSTR(sym_ref, 0);
                                outfile << get_name (f->decl) << " : " << func_name << "\n";

                        }
                    }

                }
                if (GET_CODE (def) == PARALLEL) {
                    int op_len = XVECLEN(def, 0);
                    for (int i = 0; i < op_len; i++) {
                        rtx temp = XVECEXP(def, 0, i);
                        if (temp && GET_CODE (temp) == SET) {
                            rtx body = XEXP(temp, 1);
                            // https://github.com/gcc-mirror/gcc/blob/master/gcc/rtl.def
                            // for operands
                            if (body && GET_CODE (body) == ASM_OPERANDS) {
                                // if this is a syscall
                                const char* templ  = XSTR(body, 0);
                                int syscall_reg_num = -1;
                                if (strncmp(templ, "syscall", 7) == 0) {
                                    // Which register number do we care about?
                                    rtx input_vec = XEXP(body, 3);
                                    rtx first_input = XVECEXP(body, 3, 0);
                                    syscall_reg_num =  REGNO(first_input);
                                    // then find the previous set instruction
                                    // that wrote to it, by backward
                                    // analysis
                                    bool found_si_set = false;
                                    const rtx_insn* curr = insn;
                                    while (!found_si_set) {
                                        const rtx_insn* prev = PREV_INSN(curr);
                                        if (!prev) break;

                                        rtx def2 = PATTERN(prev);
                                        if (def2) {
                                            if (GET_CODE (def2) == SET) {
                                                // operand 1 is location
                                                // (REG/MEM/PC) assigned to
                                                // operand 2 is the value
                                                rtx reg = XEXP(def2, 0);
                                                if (GET_CODE(reg) == REG) {
                                                    if (REGNO(reg) == syscall_reg_num) {

                                                        // operand?
                                                        int value = XINT(XEXP(def2, 1), 0);
                                                        outfile << get_name(f->decl) << " : " << " syscall ( " << value << " )\n";
                                                        found_si_set = true;
                                                    }
                                                }
                                            }
                                        }

                                        curr = prev;
                                    }
                                }
                            }
                        }
                    }
                }
                outfile.close();
            }

            virtual unsigned int execute (function *f) { 
                symtab_node* snode;

                /*
                FOR_EACH_SYMBOL (snode) {
                    snode->debug();

                    tree value = lookup_attribute ("symver", DECL_ATTRIBUTES (snode->decl));
                    if (value) {
                        std::cerr << "is a symver\n";
                    }
                }
                */

                if (f) {
                    //std::cerr << "RTL function: " << get_name(f->decl) << "\n";
                    basic_block bb;
                    FOR_EACH_BB_FN(bb, f) {
                        rtx_insn* instruction;
                        FOR_BB_INSNS(bb, instruction) {
                            if (!INSN_P (instruction))
                                continue;


                            handle_insn(instruction, f);
                            
                        }
                    }

                }
                outfile.close();
                return 0;
            }

    }; // class pass_regrename


}

void on_rtl_plugin_finish(void *gcc_data, void *user_data) {
    std::cerr << "Invoked plugin finish\n";
}

int plugin_init (struct plugin_name_args *plugin_info,
		struct plugin_gcc_version *version) {
	// We check the current gcc loading this plugin against the gcc we used to
	// created this plugin
	if (!plugin_default_version_check (version, &gcc_version)) {
        std::cerr << "This GCC plugin is for version " << GCCPLUGIN_VERSION_MAJOR << "." << GCCPLUGIN_VERSION_MINOR << "\n";
		return 1;
    }

    std::cerr << "Initialized plugin\n";
    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_INFO,
            /* callback */ NULL, /* user_data */ &my_gcc_plugin_info);


    // Register the phase right after cfg
    struct register_pass_info pass_info;

    pass_info.pass = new my_first_pass(g);
    pass_info.reference_pass_name = "*free_lang_data";
    pass_info.ref_pass_instance_number = 1;
    pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &pass_info);


    // For RTL
    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_INFO,
            /* callback */ NULL, /* user_data */ &rtl_plugin_info);

    register_callback(plugin_info->base_name,
            /* event */ PLUGIN_FINISH,
            /* callback */ on_rtl_plugin_finish, /* user_data */ NULL);


    // Register the phase right after eh_ranges
    struct register_pass_info rtl_pass_info;

    rtl_pass_info.pass = new pass_syscall_rtl(g);
    rtl_pass_info.reference_pass_name = "expand";
    rtl_pass_info.ref_pass_instance_number = 1;
    rtl_pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &rtl_pass_info);

    return 0;
}
