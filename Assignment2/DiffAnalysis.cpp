#include <iostream>
#include <cstdio>
#include <set>
#include <map>
#include <stack>
#include <string>


#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/IR/Constants.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class varInterval {
private:
    std::vector<std::pair<int *, int *>> intervals;
public:
    void setVal(int val) {
        this->intervals.clear();
        this->intervals.push_back(std::make_pair(new int(val), new int(val)));
    }

    void setNull() {
        this->intervals.clear();
        this->intervals.push_back(std::make_pair((int *) NULL, (int *) NULL));
    }

    void setLessThan(int val) {
        this->intervals.clear();
        this->intervals.push_back(std::make_pair((int *) NULL, new int(val)));
    }

    void setLargerThan(int val) {
        this->intervals.clear();
        this->intervals.push_back(std::make_pair(new int(val), (int *) NULL));
    }

    void setInterval(int val1, int val2) {
        this->intervals.clear();
        this->intervals.push_back(std::make_pair(new int(val1), new int(val2)));
    }

    std::vector<std::pair<int *, int *>> getIntervals() {
        return this->intervals;
    }

    void setIntervals(varInterval intervals_extra) {
        this->intervals.clear();
        this->setIntervals(intervals_extra.getIntervals());
    }

    void setIntervals(std::vector<std::pair<int *, int *>> pairs) {
        this->intervals.clear();
        for (auto p : pairs) {
            if (p.first == NULL && p.second == NULL) {
                this->intervals.push_back(std::make_pair((int *) NULL, (int *) NULL));
            } else if (p.first == NULL) {
                this->intervals.push_back(std::make_pair((int *) NULL, new int(*(p.second))));
            } else if (p.second == NULL) {
                this->intervals.push_back(std::make_pair(new int(*(p.first)), (int *) NULL));
            } else {
                this->intervals.push_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            }
        }
    }

    void printIntervals() {
        for (auto &p : this->intervals) {
            if (p.first == NULL) {
                std::cout << "NULL";
            } else {
                std::cout << *(p.first);
            }
            std::cout << "  -  ";
            if (p.second == NULL) {
                std::cout << "NULL";
            } else {
                std::cout << *(p.second);
            }
            std::cout << std::endl;
        }
    }
};

//std::set<Instruction*> analyzeDifference(BasicBlock*,std::set<Instruction*>);
//std::set<Instruction*> union_sets(std::set<Instruction*>, std::set<Instruction*>);
void analyzeAdd(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap);

void analyzeSub(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap);

void analyzeStore(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                  std::map<std::string, Instruction *> &instructionMap);

void analyzeLoad(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                 std::map<std::string, Instruction *> &instructionMap);

std::map<BasicBlock *, std::map<Instruction *, varInterval> *>* analyzeBr(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
          std::map<std::string, Instruction *> &instructionMap);

std::map<BasicBlock *, std::map<Instruction *, varInterval> *>*
analyzeBlock(BasicBlock *BB, std::map<Instruction *, varInterval> &analysisMap,
             std::map<std::string, Instruction *> &instructionMap);

void sge(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded);

void sgt(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded);

void sle(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded);

void slt(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded);

void eq(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
        std::vector<std::pair<int *, int *>> &excluded);

void ne(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
        std::vector<std::pair<int *, int *>> &excluded);

std::string getInstructionString(Instruction &I);

void sortAndMerge(std::vector<std::pair<int *, int *>> &outcome);


int main(int argc, char **argv) {
    //Preparation
    LLVMContext &Context = getGlobalContext();
    SMDiagnostic Err;
    Module *M = ParseIRFile(argv[1], Err, Context);
    if (M == nullptr) {
        fprintf(stderr, "error: failed to load LLVM IR file \"%s\"", argv[1]);
        return EXIT_FAILURE;
    }
    Function *F = M->getFunction("main");



    //CONSTRUCT A MAP
    std::map<BasicBlock *, std::map<Instruction *, varInterval> *> analysisMap;

    for (auto &BB: *F) {
        std::map<Instruction *, varInterval> emptyMap;
        BasicBlock *blockPointer = &BB;
        analysisMap[blockPointer] = &emptyMap;
    }

//    std::stack<std::pair<BasicBlock *, std::set<Instruction *>>> traversalStack;
    BasicBlock *entryBB = &F->getEntryBlock();
//    std::set<Instruction *> emptySet;
//    std::pair<BasicBlock *, std::set<Instruction *> > analysisNode = std::make_pair(entryBB, emptySet);
//    traversalStack.push(analysisNode);

    std::map<std::string, Instruction *> instructionMap;
    analyzeBlock(entryBB, *analysisMap[entryBB], instructionMap);

//    while (!traversalStack.empty()) {
//    std::pair<BasicBlock *, std::set<Instruction *>> analysisNode = traversalStack.top();
//    traversalStack.pop();
//    BasicBlock *BB = analysisNode.first;
//    std::set<Instruction *> initializedVars = analysisNode.second;
//    std::map<Instruction *, varInterval> emptyMap;

//    std::cout << "start printing" << std::endl;
//    for (auto it = instructionMap.cbegin(); it != instructionMap.cend(); ++it) {
//        std::cout << "print:" << std::endl;
//        std::cout << it->first << "\n";
//        it->second->dump();
//    }
//    for (auto it = emptyMap.cbegin(); it != emptyMap.cend(); ++it) {
//        it->first->dump();
//        varInterval interval = it->second;
//        interval.printIntervals();
//    }
//    }
}
//void analyzeProgram(std::map<BasicBlock *, std::map<Instruction *, varInterval> *> analysisMap, std::map<std::string, Instruction *> instructionMap, BasicBlock* entryBlock){
//
//}

std::map<BasicBlock *, std::map<Instruction *, varInterval> *>*
analyzeBlock(BasicBlock *BB, std::map<Instruction *, varInterval> &analysisMap,
             std::map<std::string, Instruction *> &instructionMap) {
    std::map<BasicBlock *, std::map<Instruction *, varInterval> *> *result;
    for (auto &I: *BB) {
        std::string instructionStr = getInstructionString(I);
        instructionMap[instructionStr] = &I;

        switch (I.getOpcode()) {
            case Instruction::Add: {
                analyzeAdd(I, analysisMap, instructionMap);
            }
            case Instruction::Sub: {
                analyzeSub(I, analysisMap, instructionMap);
                break;
            }
            case Instruction::Mul: {
                // analyzeMul(I, emptyMap);
                break;
            }
            case Instruction::SRem: {
                // analyzeSrem(I, emptyMap);
                break;
            }
            case Instruction::Alloca: {
                std::cout << "Alloca" << std::endl;
                varInterval tempInterval;
                tempInterval.setNull();
                analysisMap[&I] = tempInterval;
                break;
            }
            case Instruction::Store: {
                analyzeStore(I, analysisMap, instructionMap);
                break;
            }
            case Instruction::Load: {
                analyzeLoad(I, analysisMap, instructionMap);
                break;
            }
            case Instruction::ICmp: {
                std::cout << "ICmp" << std::endl;
                dyn_cast<Instruction>(I.getOperand(0))->dump();
                break;
            }
            case Instruction::Br: {
                result = analyzeBr(I, analysisMap, instructionMap);
                return result;
            }
            default: {
                std::cout << "Unknown" << std::endl;
                break;
            }
        }
    }
    return result;
}

//arithmetic operators

void analyzeAdd(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "Add" << std::endl;
    //get op1
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (isa<llvm::ConstantInt>(op2)) {
        Value *op_temp = op2;
        op2 = op1;
        op1 = op_temp;
    }

    if (isa<llvm::ConstantInt>(op1)) {
        llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(op1);
        int op1_val = CI->getZExtValue();
        std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        varInterval interval_op2 = analysisMap[instructionMap[op2_str]];


        varInterval interval_ins;
        interval_ins.setIntervals(interval_op2);

        for (auto &p : interval_ins.getIntervals()) {
            if (p.first != NULL) {
                *(p.first) = *(p.first) + op1_val;
            }
            if (p.second != NULL) {
                *(p.second) = *(p.second) + op1_val;
            }
        }
        std::string instructionString = getInstructionString(I);
        analysisMap[instructionMap[instructionString]] = interval_ins;
        //both variable
    } else {
        std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        varInterval interval_op2 = analysisMap[instructionMap[op2_str]];
        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];

        std::vector<std::pair<int *, int *>> outcome;
        for (auto &p1 : interval_op1.getIntervals()) {
            for (auto &p2 : interval_op2.getIntervals()) {
                int *left = NULL;
                int *right = NULL;
                if (p1.first != NULL && p2.first != NULL) {
                    left = new int(*(p1.first) + *(p2.first));
                }
                if (p1.second != NULL || p2.second != NULL) {
                    right = new int(*(p1.second) + *(p2.second));
                }
                outcome.push_back(std::make_pair(left, right));
            }
        }
        sortAndMerge(outcome);
        varInterval interval;
        interval.setIntervals(outcome);
        std::string instructionString = getInstructionString(I);
        analysisMap[instructionMap[instructionString]] = interval;
    }
}


void analyzeSub(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "Sub" << std::endl;
    //get op1
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    std::vector<std::pair<int *, int *>> op1_vector;
    std::vector<std::pair<int *, int *>> op2_vector;
    if (isa<llvm::ConstantInt>(op2)) {
        llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(op2);
        int op2_val = CI->getZExtValue();

        int *left = new int(op2_val);
        int *right = new int(op2_val);

        op2_vector.push_back(std::make_pair(left, right));

        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];
        op1_vector = interval_op1.getIntervals();
    } else if (isa<llvm::ConstantInt>(op1)) {
        llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(op1);
        int op1_val = CI->getZExtValue();

        int *left = new int(op1_val);
        int *right = new int(op1_val);

        op1_vector.push_back(std::make_pair(left, right));

        std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        varInterval interval_op2 = analysisMap[instructionMap[op2_str]];
        op2_vector = interval_op2.getIntervals();
        //both variable
    } else {
        std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        varInterval interval_op2 = analysisMap[instructionMap[op2_str]];
        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];
        op1_vector = interval_op1.getIntervals();
        op2_vector = interval_op2.getIntervals();
    }


    std::vector<std::pair<int *, int *>> outcome;

    for (auto &p1 : op1_vector) {
        for (auto &p2 : op2_vector) {
            int *left = NULL;
            int *right = NULL;
            if (p1.first != NULL && p2.second != NULL) {
                left = new int(*(p1.first) - *(p2.second));
            }
            if (p1.second != NULL && p2.first != NULL) {
                right = new int(*(p1.second) - *(p2.first));
            }
            outcome.push_back(std::make_pair(left, right));
        }
    }
    sortAndMerge(outcome);
    varInterval interval;
    interval.setIntervals(outcome);
    std::string instructionString = getInstructionString(I);
    analysisMap[instructionMap[instructionString]] = interval;
}


void analyzeMul(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "Sub" << std::endl;
    //get op1
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    std::vector<std::pair<int *, int *>> op1_vector;
    std::vector<std::pair<int *, int *>> op2_vector;
    if (isa<llvm::ConstantInt>(op2)) {
        llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(op2);
        int op2_val = CI->getZExtValue();

        int *left = new int(op2_val);
        int *right = new int(op2_val);

        op2_vector.push_back(std::make_pair(left, right));

        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];
        op1_vector = interval_op1.getIntervals();
    } else if (isa<llvm::ConstantInt>(op1)) {
        llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(op1);
        int op1_val = CI->getZExtValue();

        int *left = new int(op1_val);
        int *right = new int(op1_val);

        op1_vector.push_back(std::make_pair(left, right));

        std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        varInterval interval_op2 = analysisMap[instructionMap[op2_str]];
        op2_vector = interval_op2.getIntervals();
        //both variable
    } else {
        std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        varInterval interval_op2 = analysisMap[instructionMap[op2_str]];
        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];
        op1_vector = interval_op1.getIntervals();
        op2_vector = interval_op2.getIntervals();
    }


    std::vector<std::pair<int *, int *>> outcome;

    for (auto &p1 : op1_vector) {
        for (auto &p2 : op2_vector) {
            int *left = NULL;
            int *right = NULL;
            if (p1.first == NULL && p2.first == NULL) {

            } else if (p1.first == NULL && p2.second == NULL) {

            } else if (p1.first == NULL && *(p2.first) < 0) {

            }
            outcome.push_back(std::make_pair(left, right));
        }
    }
    sortAndMerge(outcome);
    varInterval interval;
    interval.setIntervals(outcome);
    std::string instructionString = getInstructionString(I);
    analysisMap[instructionMap[instructionString]] = interval;
}


void analyzeStore(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
                  std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "Store" << std::endl;
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);
    if (isa<llvm::ConstantInt>(op1)) {
        llvm::ConstantInt *CI = dyn_cast<llvm::ConstantInt>(op1);
        int op1_val = CI->getZExtValue();

        std::cout << op1_val << std::endl;

        varInterval tempInterval;
        tempInterval.setVal(op1_val);
        Instruction *op2_instruction = dyn_cast<Instruction>(op2);
        std::string op2Str = getInstructionString(*op2_instruction);
        emptyMap[instructionMap[op2Str]] = tempInterval;
    } else {
        dyn_cast<Instruction>(op1)->dump();
        varInterval tempInterval;
        Instruction *op1_instruction = dyn_cast<Instruction>(op1);
        Instruction *op2_instruction = dyn_cast<Instruction>(op2);
        std::string op1_str = getInstructionString(*op1_instruction);
        tempInterval.setIntervals(emptyMap[instructionMap[op1_str]]);
        std::string op2_str = getInstructionString(*op2_instruction);
        emptyMap[instructionMap[op2_str]] = tempInterval;
    }
    dyn_cast<Instruction>(op2)->dump();
}

void analyzeLoad(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
                 std::map<std::string, Instruction *> &instructionMap) {
    //print
    std::cout << "Load" << std::endl;
    dyn_cast<Instruction>(I.getOperand(0))->dump();
    //processing
    varInterval tempInterval;
    Instruction *op_instruction = dyn_cast<Instruction>(I.getOperand(0));

    std::string str = getInstructionString(*op_instruction);
    tempInterval.setIntervals(emptyMap[instructionMap[str]]);

    emptyMap[&I] = tempInterval;
}

std::map<BasicBlock *, std::map<Instruction *, varInterval> *>*
analyzeBr(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
          std::map<std::string, Instruction *> &instructionMap) {
    //br instruction
    BranchInst *branchInst = dyn_cast<BranchInst>(&I);
    //cmp instruction
    ICmpInst *cmpIns = dyn_cast<ICmpInst>(I.getOperand(0));
    //cmp variables
    Value *cmpOp1 = cmpIns->getOperand(0);
    Value *cmpOp2 = cmpIns->getOperand(1);
    //cmp type
    auto p = cmpIns->getSignedPredicate();
    //br blocks
    BasicBlock *bb_op1 = dyn_cast<BasicBlock>(branchInst->getSuccessor(0));
    BasicBlock *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));

    std::vector<std::pair<int *, int *>> selected;
    std::vector<std::pair<int *, int *>> excluded;
    //printing
    std::cout << "Br" << std::endl;
    std::cout << branchInst->isConditional() << std::endl;
    cmpIns->dump();
    std::cout << p << std::endl;
    bb_op1->dump();
    bb_op2->dump();

    Instruction *cmpVarInst;
    if (p == 38) {
        //sgt
        if (isa<llvm::ConstantInt>(cmpOp1)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))];
            slt(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue(), selected, excluded);
        } else if (isa<llvm::ConstantInt>(cmpOp2)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))];
            sgt(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue(), selected, excluded);
        } else { ;
        }
    } else if (p == 39) {
        //sge
        if (isa<llvm::ConstantInt>(cmpOp1)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))];
            sle(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue(), selected, excluded);
        } else if (isa<llvm::ConstantInt>(cmpOp2)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))];
            sge(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue(), selected, excluded);
        } else { ;
        }
    } else if (p == 40) {
        //slt
        if (isa<llvm::ConstantInt>(cmpOp1)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))];
            sgt(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue(), selected, excluded);
        } else if (isa<llvm::ConstantInt>(cmpOp2)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))];
            slt(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue(), selected, excluded);
        } else { ;
        }
    } else if (p == 41) {
        //sle
        if (isa<llvm::ConstantInt>(cmpOp1)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))];
            sge(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue(), selected, excluded);
        } else if (isa<llvm::ConstantInt>(cmpOp2)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))];
            sle(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue(), selected, excluded);
        } else { ;
        }
    } else if (p == 32) {
        //eq
        if (isa<llvm::ConstantInt>(cmpOp1)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))];
            eq(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue(), selected, excluded);
        } else if (isa<llvm::ConstantInt>(cmpOp2)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))];
            eq(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue(), selected, excluded);
        } else { ;
        }
    } else if (p == 33) {
        //ne
        if (isa<llvm::ConstantInt>(cmpOp1)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))];
            ne(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue(), selected, excluded);
        } else if (isa<llvm::ConstantInt>(cmpOp2)) {
            cmpVarInst = instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))];
            ne(emptyMap[cmpVarInst], dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue(), selected, excluded);
        } else { ;
        }
    } else {

    }
    std::map<BasicBlock *, std::map<Instruction *, varInterval> *> *result = new std::map<BasicBlock *, std::map<Instruction *, varInterval> *>();
    std::map<Instruction *, varInterval> result_op1;
    std::map<Instruction *, varInterval> result_op2;
    for (auto const &x : emptyMap) {
        varInterval v1;
        v1.setIntervals(x.second);
        result_op1[x.first] = v1;
        varInterval v2;
        v2.setIntervals(x.second);
        result_op2[x.first] = v2;
    }
    varInterval v1;
    v1.setIntervals(selected);
    result_op1[cmpVarInst] = v1;
    varInterval v2;
    v2.setIntervals(excluded);
    result_op2[cmpVarInst] = v2;
    (*result)[bb_op1] = &result_op1;
    (*result)[bb_op2] = &result_op2;
    return result;
}

//comparison

void sge(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded) {
    for (auto &p : interval.getIntervals()) {
        if (p.first == NULL && p.second == NULL) {
            selected.push_back(std::make_pair(new int(a), (int *)NULL));
            excluded.push_back(std::make_pair((int *)NULL, new int(a - 1)));
        } else if (p.first == NULL) {
            if (*(p.second) < a) {
                excluded.push_back(std::make_pair((int *)NULL, new int(*(p.second))));
            } else {
                excluded.push_back(std::make_pair((int *)NULL, new int(a - 1)));
                selected.push_back(std::make_pair(new int(a), new int(*(p.second))));
            }
        } else if (p.second == NULL) {
            if (*(p.first) < a) {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                selected.push_back(std::make_pair(new int(a), (int *)NULL));
            } else {
                selected.push_back(std::make_pair(new int(*(p.first)), (int *)NULL));
            }
        } else {
            if (*(p.first) >= a) {
                selected.push_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else if (*(p.second) <= a) {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                selected.push_back(std::make_pair(new int(a), new int(*(p.second))));
            }
        }
    }
}

void sgt(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded) {
    for (auto &p : interval.getIntervals()) {
        if (p.first == NULL && p.second == NULL) {
            selected.push_back(std::make_pair(new int(a + 1), (int *)NULL));
            excluded.push_back(std::make_pair((int *)NULL, new int(a)));
        } else if (p.first == NULL) {
            if (*(p.second) <= a) {
                excluded.push_back(std::make_pair((int *)NULL, new int(*(p.second))));
            } else {
                excluded.push_back(std::make_pair((int *)NULL, new int(a)));
                selected.push_back(std::make_pair(new int(a + 1), new int(*(p.second))));
            }
        } else if (p.second == NULL) {
            if (*(p.first) <= a) {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(a)));
                selected.push_back(std::make_pair(new int(a + 1), (int *)NULL));
            } else {
                selected.push_back(std::make_pair(new int(*(p.first)), (int *)NULL));
            }
        } else {
            if (*(p.first) >= a + 1) {
                selected.push_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else if (*(p.second) <= a) {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(a + 1)));
                selected.push_back(std::make_pair(new int(a + 1), new int(*(p.second))));
            }
        }
    }
}

void sle(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded) {
    sgt(interval, a, excluded, selected);
}

void slt(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded) {
    sge(interval, a, excluded, selected);
}

void eq(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
        std::vector<std::pair<int *, int *>> &excluded) {
    for (auto &p : interval.getIntervals()) {
        if (p.first == NULL && p.second == NULL) {
            selected.push_back(std::make_pair(new int(a), new int(a)));
            excluded.push_back(std::make_pair((int *)NULL, new int(a - 1)));
            excluded.push_back(std::make_pair(new int(a + 1), (int *)NULL));
        } else if (p.first == NULL) {
            if (*(p.second) < a) {
                excluded.push_back(std::make_pair((int *)NULL, new int(*(p.second))));
            } else if (*(p.second) == a) {
                excluded.push_back(std::make_pair((int *)NULL, new int(a - 1)));
                selected.push_back(std::make_pair(new int(a), new int(a)));
            } else {
                excluded.push_back(std::make_pair((int *)NULL, new int(a - 1)));
                excluded.push_back(std::make_pair(new int(a + 1), new int(*(p.second))));
                selected.push_back(std::make_pair(new int(a), new int(a)));
            }
        } else if (p.second == NULL) {
            if (*(p.first) < a) {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                excluded.push_back(std::make_pair(new int(a + 1), (int *)NULL));
                selected.push_back(std::make_pair(new int(a), new int(a)));
            } else if (*(p.first) == a) {
                excluded.push_back(std::make_pair(new int(a + 1), (int *)NULL));
                selected.push_back(std::make_pair(new int(a), new int(a)));
            } else {
                excluded.push_back(std::make_pair(new int(*(p.first)), (int *)NULL));
            }
        } else {
            if (*(p.first) >= a + 1) {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else if (*(p.second) < a) {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else {
                excluded.push_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                excluded.push_back(std::make_pair(new int(a + 1), new int(*(p.second))));
                selected.push_back(std::make_pair(new int(a), new int(a)));
            }
        }
    }
}

void ne(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
        std::vector<std::pair<int *, int *>> &excluded) {
    sge(interval, a, excluded, selected);
}

//utility

std::string getInstructionString(Instruction &I) {
    std::string instructionStr;
    llvm::raw_string_ostream rso(instructionStr);
    I.print(rso);
    return instructionStr;
}

void sortAndMerge(std::vector<std::pair<int *, int *>> &outcome) {
    std::sort(outcome.begin(), outcome.end(),
              [](const std::pair<int *, int *> &lhs, const std::pair<int *, int *> &rhs) {
                  if (lhs.first == NULL && rhs.first == NULL) {
                      if (lhs.second == NULL || rhs.second == NULL) {
                          return lhs.second != NULL;
                      } else {
                          return *(lhs.first) < *(rhs.first);
                      }
                  } else if (lhs.first == NULL || rhs.first == NULL) {
                      return lhs.first == NULL;
                  } else {
                      return *(lhs.first) < *(rhs.first);
                  }
              });

    for (int i = 1; i < outcome.size(); i++) {
        if (outcome[i].first == NULL) {
            outcome[i - 1].second = outcome[i].second;
            outcome.erase(outcome.begin() + i);
            i--;
        } else if (outcome[i].second == NULL) {
            if (outcome[i - 1].second == NULL) {
                outcome.erase(outcome.begin() + i);
                i--;
            } else if (outcome[i - 1].first == NULL) {
                if (*(outcome[i].first) <= *(outcome[i - 1].second)) {
                    outcome[i - 1].second = outcome[i].second;
                    outcome.erase(outcome.begin() + i);
                    i--;
                }
            } else {
                if (*(outcome[i].first) <= *(outcome[i - 1].second)) {
                    if (*(outcome[i - 1].second) < *(outcome[i].second)) {
                        outcome[i - 1].second = outcome[i].second;
                    }
                    outcome.erase(outcome.begin() + i);
                    i--;
                }
            }
        } else {
            if (outcome[i - 1].second == NULL) {
                outcome.erase(outcome.begin() + i);
                i--;
            } else {
                if (*(outcome[i - 1].second) >= *(outcome[i].first)) {
                    outcome[i - 1].second = NULL;
                    outcome.erase(outcome.begin() + i);
                    i--;
                }
            }
        }
    }
}