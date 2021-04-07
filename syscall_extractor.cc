#include <iostream>

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

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>

#include <vector>
#include <algorithm>
#include <regex.h>


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
        my_first_pass(gcc::context *ctx)
            : simple_ipa_opt_pass(my_first_pass_data, ctx)
        {
            std::cerr << "Created gimple pass\n";
        }

        /*
        int derive_syscall(gasm* asm_stmt) {
            const char* str = gimple_asm_string(asm_stmt);
            std::string asm_str(str);

            std::vector<std::string> asm_vec;

            // Does the string contain "syscall"
            if (boost::contains(asm_str, "syscall")) {

                // We want to track all inline assembly snippets right before
                // this one
                gimple curr_asm = asm_stmt;
                do {
                    gasm* curr_gimple = as_a <gasm *> (curr_asm);
                    asm_vec.push_back(gimple_asm_string(curr_gimple));
                    curr_asm = curr_asm->prev;
                } while (gimple_code(curr_asm) == GIMPLE_ASM);

                std::reverse(asm_vec.begin(), asm_vec.end());


                bool foundRAX = false;
                int storedVal = -1;

                auto it = asm_vec.begin();
                for (; it != asm_vec.end(); it++) {
                    boost::char_separator<char> sep("\n", "+");
                    boost::tokenizer< boost::char_separator<char> > tok(*it, sep);
                    for(boost::tokenizer< boost::char_separator<char> >::iterator beg = tok.begin(); beg != tok.end(); ++beg)
                    {
                        std::string asm_line = boost::trim_copy(*beg);
                        // We're interested in mov $0, %rax ... style
                        // instructions
                        // And of course the syscall

                        regex_t r1, r2;
                        int rresult;

                        char *p1 = "\\s*mov\\s*\$[,\\w\\s]*rax\\s*";

                        char *p2 = "mov[ $,0-9]*%rax";

                        if (regcomp (&r1, p2 , REG_EXTENDED | REG_NOSUB) != 0) {
                            std::cerr << "Regex compilation error!\n";
                            return -1;
                        }

                        rresult = regexec(&r1, asm_line.c_str(), 0, NULL, 0); 

                        if (rresult == 0) {
                            // Match - storing to %rax
                            std::cerr << "Found store to rax " << asm_line << "\n";
                        }


                        if (regcomp (&r2, "\\s*syscall\\s*", REG_EXTENDED | REG_NOSUB) != 0) {
                            std::cerr << "Regex compilation error!\n";
                            return -1;
                        }

                        rresult = regexec(&r2, asm_line.c_str(), 0, NULL, 0); 
                        if (rresult == 0) {
                            // Match;
                            std::cerr << "Found syscall: " << asm_line << "\n";
                            std::cerr << "Full asm: " << asm_str << "\n";
                        }


                    }    
                }
                return -1;
                
            } else {
                return -1;
            }

        }

        */
        virtual unsigned int execute(function *fun) override
        {
            cgraph_node* node = nullptr;
            std::cerr << "Ran GIMPLE pass\n";
            FOR_EACH_DEFINED_FUNCTION(node) {
                function* const fn = node->get_fun();
                if (fn) {

                    struct cgraph_edge *edge;

                    for (edge = node->callees; edge; edge = edge->next_callee) {
                        struct cgraph_node* callee_node = edge->callee;
                        std::cerr <<  get_name(node->get_fun()->decl) << " ---> " << get_name(callee_node->decl) << "\n";
                    }
                    basic_block bb;
                    FOR_EACH_BB_FN(bb, fn) {
                        gimple_stmt_iterator si;
                        for (si = gsi_start_bb (bb); !gsi_end_p (si); gsi_next (&si)) {
                            gimple stmt = gsi_stmt (si);
                            /*
                             * if stmt.code == GIMPLE_ASSIGN ... or something
                             p gimple_get_lhs(stmt)->var_decl
                             */
                            if (gimple_code(stmt) == GIMPLE_ASSIGN) {
                               
                                tree t2 = gimple_op(stmt, 1);
                                if (TREE_CODE(t2) == ADDR_EXPR) {
                                    // Get the operand, and need to verify if
                                    // it is a function
                                    tree type = TREE_TYPE(t2);
                                    if (TREE_CODE(type) == POINTER_TYPE) {
                                        tree typetype = TREE_TYPE(type);
                                        if (TREE_CODE(typetype) == FUNCTION_TYPE) {
                                            std::cerr << get_name(node->get_fun()->decl) << " ---- " << get_name(t2) << "\n";
                                        }
                                    }
                                }
                            }
                            // We handle the syscall in the RTL pass
                            /*else if (gimple_code(stmt) == GIMPLE_ASM) {
                                gasm *asm_stmt = as_a <gasm *> (stmt);

                                int syscallNo = derive_syscall(asm_stmt);
                                if (syscallNo > -1) {
                                    // It's a valid
                                    std::cerr << get_name(node->get_fun()->decl) << " **** " << syscallNo << "\n";
                                }
                            }
                            */

                            print_gimple_stmt(stderr, stmt, 0,0);
                        }
                    }
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
        public:
            pass_syscall_rtl (gcc::context *ctxt)
                : rtl_opt_pass (pass_data_syscall_rtl, ctxt)
            {}


            virtual unsigned int execute (function *f) { 
                if (f) {
                    std::cerr << "RTL function: " << get_name(f->decl) << "\n";
                    basic_block bb;
                    FOR_EACH_BB_FN(bb, f) {
                        rtx_insn* instruction;
                        FOR_BB_INSNS(bb, instruction) {
                            if (!INSN_P (instruction))
                                continue;
                            //std::cerr << "Printed one instruction\n";
                            if (GET_CODE (PATTERN (instruction)) == ASM_INPUT) {

                                print_rtl(stderr, instruction);
                            }
                            //std::cerr << "Instruction code: " << INSN_CODE(instruction) << "\n";
                        }
                    }

                }
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
    rtl_pass_info.reference_pass_name = "eh_ranges";
    rtl_pass_info.ref_pass_instance_number = 1;
    rtl_pass_info.pos_op = PASS_POS_INSERT_AFTER;

    register_callback (plugin_info->base_name, PLUGIN_PASS_MANAGER_SETUP, NULL, &rtl_pass_info);

    return 0;
}
