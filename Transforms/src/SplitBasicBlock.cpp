#include "SplitBasicBlock.h"
#include "Utils.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/IR/Instructions.h"
#include <vector>
using std::vector;
using namespace llvm;

// 可选的参数，指定一个基本块会被分裂成几个基本块，默认值为 2
static cl::opt<int> splitNum("split_num", cl::init(2),
                             cl::desc("Split<split_num> time(s) each BB"));
namespace
{
    class SplitBasicBlock : public FunctionPass
    {
    public:
        static char ID;
        SplitBasicBlock() : FunctionPass(ID) {
            PassPrintInfo(getPassName());
            outs() << "Split Number: " << splitNum << "\n";
        }

        // 入口函数
        bool runOnFunction(Function &F);
        // 判断一个基本块中是否包含PHI指令(PHINode)
        bool containsPHI(BasicBlock *BB);
        // 对单个基本块执行分裂操作
        void split(BasicBlock *BB);
    };
} // namespace

// runOnFunction 函数实现
bool SplitBasicBlock::runOnFunction(Function &F)
{
    // TODO
    outs() << "The function being processed is: " << F.getName() << "\n";
    // 第一步：保存原先的所有基本块
    vector<BasicBlock*> origBB;
    for (BasicBlock &BB : F)
    {
        // outs() << "Basic Block Name: " << BB.getName() << "\n";
        origBB.push_back(&BB);
    }
    // 第二步：对每个不包含 PHI 指令的基本块执行分裂操作
    for (BasicBlock *BB : origBB)
    {
        if (!containsPHI(BB))
        {
            split(BB);
        }
    }
    return true;

}

bool SplitBasicBlock::containsPHI(BasicBlock *BB)
{
    for (Instruction &I : *BB)
    {
        if (isa<PHINode>(&I))
        {
            return true;
        }
    }
    return false;
}

void SplitBasicBlock::split(BasicBlock *BB)
{
    // 计算分裂后每个基本块的大小
    // 原基本块的大小 / 分裂数目（向上取整）
    int splitSize = (BB->size() + splitNum - 1) / splitNum;
    BasicBlock *curBB = BB;
    for (int i = 1; i < splitNum; i++)
    {
        int ins_cnt = 0;
        for (Instruction &I : *curBB)
        {
            if (ins_cnt++ == splitSize)
            {
                // 在 I 指令处对基本块进行分割
                curBB = curBB->splitBasicBlock(&I);
                break;
            }
        }
    }
}

FunctionPass *llvm::createSplitBasicBlockPass()
{
    return new SplitBasicBlock();
}

char SplitBasicBlock::ID = 0;

// 注册该Pass
static RegisterPass<SplitBasicBlock>
    X("split", "Split a basic block intomultiple basic blocks.");