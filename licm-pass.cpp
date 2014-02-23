// CS380C S14 Assignment 4: licm.cpp
// 
// Based on code from Todd C. Mowry
// Modified by Arthur Peters
// Modified by Jianyu Huang (UT EID: jh57266)
////////////////////////////////////////////////////////////////////////////////

#include "llvm/Pass.h"
#include "llvm/PassManager.h"
#include "llvm/Transforms/IPO/PassManagerBuilder.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

#include "llvm/IR/BasicBlock.h"
#include "llvm/Support/InstIterator.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/Instructions.h"
#include "llvm/Assembly/AssemblyAnnotationWriter.h"

#include "llvm/Analysis/LoopInfo.h"
#include "llvm/Analysis/LoopPass.h"

#include "llvm/Analysis/Dominators.h"
#include "llvm/Support/CFG.h"
#include "llvm/Transforms/Scalar.h"

#include "llvm/Analysis/ValueTracking.h"

//#include "licmAnalysis.h"

#include <ostream>
#include <fstream>
#include <iostream>

using namespace llvm;

namespace {
	class LICM : public LoopPass {
		public:
			static char ID;

			BasicBlock *preheader;
			Loop *myloop;
			LoopInfo *LI;
			DominatorTree *DT;
			bool isChanged;
			//LICM() : LoopPass(ID) {}
			LICM() : LoopPass(ID) {
				initializeLICMPass(*PassRegistry::getPassRegistry());
			}
			virtual bool runOnLoop(Loop *L, LPPassManager &LPM) {
				isChanged = false;
				LI = &getAnalysis<LoopInfo>();
				//DT = &getAnalysis<DominatorTreeWrapperPass>().getDomTree();
 				DT = &getAnalysis<DominatorTree>();
				myloop = L;
				preheader = L->getLoopPreheader();

			//	for (


				if (preheader)
					putAboveHandler(DT->getNode(L->getHeader()));
				return isChanged;
			}

			bool inInnerLoop(BasicBlock *BB) {
				assert(myloop->contains(BB) && "only valid if BB is in the loop)");
				return LI->getLoopFor(BB) != myloop;
			}

			//depth-first order traverse on the dominatorTree
			void putAboveHandler(DomTreeNode *N) {
				assert(N != 0 && "Null dominator tree node?");
				BasicBlock *BB = N->getBlock();

				//BB should be the immediately withnin L
				if (!myloop->contains(BB)) return;

				if (!inInnerLoop(BB)) {
					for (BasicBlock::iterator II = BB->begin(), E = BB->end(); II != E; ) {
						Instruction &I = *II++;

						//constant folding the instruction????....TO DO....
						
						if (myloop->hasLoopInvariantOperands(&I) && isSafePutAboveInst(I) && isSafeToExecuteUnconditionally(I)) {
							putAboveInst(I);
						}
					}
				}

				const std::vector<DomTreeNode*> &Children = N->getChildren();
				for (unsigned i = 0, e = Children.size(); i != e; ++i) {
					putAboveHandler(Children[i]);
				}	
			}

			bool isSafeToExecuteUnconditionally(Instruction &Inst) {
				if (isSafeToSpeculativelyExecute(&Inst))
					return true;
				
				if (Inst.getParent() == myloop->getHeader())
					return true;

				SmallVector<BasicBlock*, 8> ExitBlocks;
				myloop->getExitBlocks(ExitBlocks);

				for (unsigned i = 0, e = ExitBlocks.size(); i != e; ++i) {
					if (!DT->dominates(Inst.getParent(), ExitBlocks[i]))
						return false;
				}

				if (ExitBlocks.empty())
					return false;

				return true;
			}

			bool isSafePutAboveInst(Instruction  &I) {
				if (!isa<BinaryOperator>(I) && !isa<CastInst>(I) && !isa<SelectInst>(I) && !isa<GetElementPtrInst>(I) &&
						!isa<CmpInst>(I) && !isa<InsertElementInst>(I) && !isa<ExtractElementInst>(I) && !isa<ShuffleVectorInst>(I) &&
						!isa<ExtractValueInst>(I) && !isa<InsertValueInst>(I))
					return false;
				return isSafeToExecuteUnconditionally(I);
			}

			void putAboveInst(Instruction &I) {
				//DEBUG(dbgs() << "LICM hoisting to " << Preheader->getName() << ": " << I << "\n");
				I.moveBefore(preheader->getTerminator());
				isChanged = true;
			}

			// We don't modify the program, so we preserve all analyses
			virtual void getAnalysisUsage(AnalysisUsage &AU) const {
				//AU.setPreservesAll();
				AU.setPreservesCFG();
				AU.addRequired<LoopInfo>();
				//AU.addRequired<DominatorTreeWrapperPass>();
				AU.addRequired<DominatorTree>();
				AU.addRequiredID(LoopSimplifyID);
			}


			/*
			virtual bool runOnFunction(Function &F) {
				std::vector<Value *> domain;
				for (Function::arg_iterator arg = F.arg_begin(); arg != F.arg_end(); ++arg) {
					domain.push_back(arg);
				}
				for (inst_iterator ii = inst_begin(F), ie = inst_end(F); ii != ie; ++ii) {
					if (!ii->getName().empty()) {
						domain.push_back(&*ii);
					}
				}
				ValueMap<const BasicBlock *, idfaInfo *> *BBtoInfo = new ValueMap<const BasicBlock *, idfaInfo *>();		
				ValueMap<const Instruction *, idfaInfo *> *InstToInfo = new ValueMap<const Instruction *, idfaInfo *>();

				LICMAnalysis<llvm::Value *> *licmAnly = new LICMAnalysis<llvm::Value *>();
				licmAnly->analysis(domain, F, false, *BBtoInfo, *InstToInfo);

				Annotator<Value *> annot(*BBtoInfo, *InstToInfo, domain);
				F.print(errs(), &annot);

				BitVector *toErase = new BitVector(*((*BBtoInfo)[&(F.front())]->in)); 

				errs() << "flag1\n";
				delete BBtoInfo; 
				errs() << "flag2\n";
				delete InstToInfo;
				errs() << "flag3\n";

				errs() << "in of entry block\n";
				//licmAnly->BVprint(BBtoInfo[&(F.front())]->in);
				delete licmAnly;
				bool isChanged =  eraseLICM(domain, *toErase);
				return false;
			}
			*/
	};

	// LLVM uses the address of this static member to identify the pass, so the
	// initialization value is unimportant.
	char LICM::ID = 0;

	// Register this pass to be used by language front ends.
	// This allows this pass to be called using the command:
	//    clang -c -Xclang -load -Xclang ./FunctionInfo.so loop.c
	static void registerMyPass(const PassManagerBuilder &,
			PassManagerBase &PM) {
		PM.add(new LICM());
	}
	RegisterStandardPasses
		RegisterMyPass(PassManagerBuilder::EP_EarlyAsPossible,
				registerMyPass);

	// Register the pass name to allow it to be called with opt:
	//    clang -c -emit-llvm loop.c
	//    opt -load ./FunctionInfo.so -function-info loop.bc > /dev/null
	// See http://llvm.org/releases/3.4/docs/WritingAnLLVMPass.html#running-a-pass-with-opt for more info.
	RegisterPass<LICM> X("licm-pass", "licm-pass");

}
