import angr

class ObjectAnalyzer:
    def __init__(self, bin_path):
        self.p = angr.Project(bin_path, load_options={'auto_load_libs': False})
        self.cfg = self.p.analyses.CFGFast()
        self.binPath = bin_path
        self.symbolAddrMap = {}

    def populateFunctionSyscalls(self):
        for addr,fun in self.cfg.kb.functions.items():
            # To maintain consistency use the symbolAddrMap to get the symbol name
            # Sometimes angr seems to give dummy names to the functions 
            # like sub_70070()

            # This part should be architecture-specific
            rax_val = 0x0;
            if addr in self.symbolAddrMap:
                fun_name = list(self.symbolAddrMap[addr])[0]
                for bb in list(fun.blocks):
                    capstoneBlock = bb.capstone
                    for instruction in capstoneBlock.insns:
                        if instruction.mnemonic == "mov" or instruction.mnemonic == "movq":
                            # TODO, handle this in a better way
                            operand0, operand1 = instruction.op_str.split(',')
                            if operand0 == "rax" or operand0 == "eax":
                                rax_val = int(operand1,16)
                        if instruction.mnemonic == "syscall":
                            print fun_name, " ### ", rax_val

    
    def populateAliases(self):
        for so in self.p.loader.all_objects:
            if so is self.p.loader._extern_object:
                continue
            for sym_key in so._symbol_cache.keys():
                sym = so._symbol_cache[sym_key]
                sym_addr = sym.rebased_addr
                sym_name = sym.name
                if len(sym_name) == 0 or sym.is_import:
                    continue
                if sym_addr not in self.symbolAddrMap:
                    self.symbolAddrMap[sym_addr] = set()
                self.symbolAddrMap[sym_addr].add(sym_name)
    
    def printAliases(self):
        for sym_addr in self.symbolAddrMap:
            aliasSet = self.symbolAddrMap[sym_addr]
            lenSet = len(aliasSet)
            assert lenSet <= 2;
            if lenSet > 1:
                idx = 0
                for alias in aliasSet:
                    # For the last alias don't print the arrow
                    if idx == lenSet - 1:
                        print alias,
                    else:
                        print alias, " <---> ",
                    idx = idx + 1
            print ""


if __name__ == "__main__":
    objAnalyzer = ObjectAnalyzer('/morespace/glibc-analysis/glibc-build/signal/kill.o')
    objAnalyzer.populateAliases()
    objAnalyzer.printAliases()
    objAnalyzer.populateFunctionSyscalls()
