#include "llvm/Pass.h"
#include "llvm/IR/Function.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

namespace
{
    class HelloWrold : public FunctionPass
    {
    public:
        static char ID;
        HelloWrold() : FunctionPass(ID) {}
        bool runOnFunction(Function &F);
    };
}

// runOnFunction 函数实现
bool HelloWrold::runOnFunction(Function &F)
{
    // TODO
    outs() << "Hello, " << F.getName() << "\n";
    return true;
}

char HelloWrold::ID = 0;

// 注册该Pass
static RegisterPass<HelloWrold> X("hlw", "hlw LLVM Pass");