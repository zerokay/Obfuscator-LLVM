#include "SplitBasicBlock.h"
#include "Utils.h"


#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Transforms/Utils/ValueMapper.h"
#include "llvm/Transforms/Utils/Cloning.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Instructions.h"

#include <vector>
#include <cstdlib>
#include <ctime>

using std::vector;
using namespace llvm;

// 混淆次数，混淆次数越多混淆结果越复杂
static cl::opt<int> obfuTimes("bcf_loop", cl::init(1), cl::desc("Obfuscate a function <bcf_loop> time(s)."));

namespace{
    class BogusControlFlow : public FunctionPass {
        public:
            static char ID;
            BogusControlFlow() : FunctionPass(ID) {
                srand(time(NULL));
                PassPrintInfo(getPassName());
            }

            bool runOnFunction(Function &F);

            // 对基本块 BB 进行混淆
            void bogus(BasicBlock *BB);

            // 创建条件恒为真的 ICmpInst*
            // 该比较指令的条件为：y < 10 || x * (x + 1) % 2 == 0
            // 其中 x, y 为恒为0的全局变量
            Value* createBogusCmp(BasicBlock *insertAfter);
    };

}

bool BogusControlFlow::runOnFunction(Function &F){
    INIT_CONTEXT(F);

    outs() << "Use SplitBasicBlockPass!\n";
    FunctionPass *pass = createSplitBasicBlockPass();
    pass->runOnFunction(F);
    
    // 虚假控制流是对基本块进行操作的
    for(int i = 0; i < obfuTimes; i ++){
        vector<BasicBlock*> origBB;
        for(BasicBlock &BB : F){
            origBB.push_back(&BB);
        }
        for(BasicBlock *BB : origBB){
            bogus(BB);
        }
    }
    return true;
}

Value* BogusControlFlow::createBogusCmp(BasicBlock *insertAfter){
    // if((y < 10 || x * (x + 1) % 2 == 0))
    // 等价于 if(true)
    Module *M = insertAfter->getModule();
    // 全局变量x,y
    GlobalVariable *xptr = new GlobalVariable(*M, TYPE_I32, false, GlobalValue::CommonLinkage, CONST_I32(0), "x");
    GlobalVariable *yptr = new GlobalVariable(*M, TYPE_I32, false, GlobalValue::CommonLinkage, CONST_I32(0), "y");
    // load x y
    LoadInst *x = new LoadInst(TYPE_I32, xptr, "", insertAfter);
    LoadInst *y = new LoadInst(TYPE_I32, yptr, "", insertAfter);
    // y < 10
    ICmpInst *cond1 = new ICmpInst(*insertAfter, CmpInst::ICMP_SLT, y, CONST_I32(10));
    // x + 1
    BinaryOperator *op1 = BinaryOperator::CreateAdd(x, CONST_I32(1), "", insertAfter);
    // x * (x + 1)
    BinaryOperator *op2 = BinaryOperator::CreateMul(op1, x, "", insertAfter);
    // x * (x + 1) % 2
    BinaryOperator *op3 = BinaryOperator::CreateURem(op2, CONST_I32(2), "", insertAfter);
    // x * (x + 1) % 2 == 0
    ICmpInst *cond2 = new ICmpInst(*insertAfter, CmpInst::ICMP_EQ, op3, CONST_I32(0));
    return BinaryOperator::CreateOr(cond1, cond2, "", insertAfter);
}

void BogusControlFlow::bogus(BasicBlock *entryBB){
    // 第一步，拆分基本块BB得到 entryBB, bodyBB, endBB
    // 其中所有的 PHI 指令都在 entryBB(如果有的话)
    // endBB 只包含一条终结指令
    BasicBlock *bodyBB = entryBB->splitBasicBlock(entryBB->getFirstNonPHI(), "bodyBB");
    BasicBlock *endBB = bodyBB->splitBasicBlock(bodyBB->getTerminator(), "endBB");
    
    // 第二步，克隆 bodyBB 得到克隆块 cloneBB
    BasicBlock *cloneBB = createCloneBasicBlock(bodyBB);

    // 第三步，构造虚假跳转
    // (1). 将 entryBB, bodyBB, cloneBB 末尾的绝对跳转移除
    entryBB->getTerminator()->eraseFromParent();
    bodyBB->getTerminator()->eraseFromParent();
    cloneBB->getTerminator()->eraseFromParent();

    // (2). 在 entryBB 和 bodyBB 的末尾插入条件恒为真的虚假比较指令
    Value *cond1 = createBogusCmp(entryBB); 
    Value *cond2 = createBogusCmp(bodyBB); 

    // (3). 将 entryBB 到 bodyBB 的绝对跳转改为条件跳转
    BranchInst::Create(bodyBB, cloneBB, cond1, entryBB);

    // (4). 将 bodyBB 到 endBB的绝对跳转改为条件跳转
    BranchInst::Create(endBB, cloneBB, cond2, bodyBB);

    // (5). 添加 bodyBB.clone 到 bodyBB 的绝对跳转
    BranchInst::Create(bodyBB, cloneBB);
}

char BogusControlFlow::ID = 0;
static RegisterPass<BogusControlFlow> X("bcf", "Add bogus control flow to each function.");