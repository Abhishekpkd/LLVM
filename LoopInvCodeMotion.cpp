#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/IR/PassManager.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Transforms/Utils/LoopUtils.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;
namespace {
    class LoopInvCodeMotion : public PassInfoMixin<LoopInvCodeMotion> {
    public:
        static PreservedAnalyses run(Loop &L, LoopAnalysisManager &AM, LoopStandardAnalysisResults &AR,LPMUpdater &Updater) {
            // Skip loops with no blocks
            if (L.getBlocks().empty())
                return PreservedAnalyses::all();

            bool modified = false;

            // Get preheader block
            BasicBlock *preheader = L.getLoopPreheader();
            if (!preheader)
                return PreservedAnalyses::all();

            // Iterate through instructions in the loop
            for (auto *BB : L.getBlocks()) {
                for (auto I = BB->begin(); I != BB->end(); ) {
                    Instruction *inst = &(*I);
                    ++I; // Advance iterator before potential removal

                    // Check if instruction is loop invariant
                    if (isLoopInvariant(inst, &L)) {
                        // Print message when invariant is found
                        llvm::outs() << "FoundInvariant: " << *inst << "\n";
                        //////errs() << "FoundInvariant: " << *inst << "\n";
                        // Move instruction to preheader
                        inst->removeFromParent();
                        inst->insertBefore(preheader->getTerminator());
                        //inst->moveBefore(preheader->getTerminator());
                       // preheader->getInstList().push_back(inst);
                        modified = true;
                    }
                }
            }

            return modified ? PreservedAnalyses::none() : PreservedAnalyses::all();
        }

    private:
        // Check if an instruction is loop invariant
        static bool isLoopInvariant(Instruction *inst, Loop *L) {
            // Skip certain instruction types
            if (inst->isTerminator() ||  // Preferred over isa<TerminatorInst>
            //if (isa<TerminatorInst>(inst) ||
                isa<PHINode>(inst) ||
                inst->mayHaveSideEffects())
                return false;

            // Check operands
            for (Use &operand : inst->operands()) {
                if (auto *opInst = dyn_cast<Instruction>(operand)) {
                    // If any operand is defined inside the loop, it's not invariant
                    if (L->contains(opInst))
                        return false;
                }
            }

            return true;
        }
    };
}
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION, "LoopInvariantCodeMotion", LLVM_VERSION_STRING,
        [](PassBuilder &PB) {
            PB.registerPipelineParsingCallback(
                [](StringRef Name, LoopPassManager &LPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "looaicm") {
                        LPM.addPass(LoopInvCodeMotion());
                        return true;
                    }
                    return false;
                });
        }
    };
}
