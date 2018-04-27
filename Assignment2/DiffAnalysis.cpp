#include <iostream>
#include <cstdio>
#include <set>
#include <map>
#include <stack>
#include <string>
#include <cmath>

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
    //in order to use NULL to describe -inf & +inf,
    //int* is used instead of int
    std::vector<std::pair<int *, int *>> intervals;
public:
    void setVal(int val) {
        this->intervals.clear();
        this->intervals.emplace_back(std::make_pair(new int(val), new int(val)));
    }

    void setNull() {
        this->intervals.clear();
        this->intervals.emplace_back(std::make_pair((int *) nullptr, (int *) nullptr));
    }

    void setLessThan(int val) {
        this->intervals.clear();
        this->intervals.emplace_back(std::make_pair((int *) nullptr, new int(val)));
    }

    void setLargerThan(int val) {
        this->intervals.clear();
        this->intervals.emplace_back(std::make_pair(new int(val), (int *) nullptr));
    }

    void setInterval(int val1, int val2) {
        this->intervals.clear();
        this->intervals.emplace_back(std::make_pair(new int(val1), new int(val2)));
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
                this->intervals.emplace_back(std::make_pair((int *) nullptr, (int *) nullptr));
            } else if (p.first == NULL) {
                this->intervals.emplace_back(std::make_pair((int *) nullptr, new int(*(p.second))));
            } else if (p.second == NULL) {
                this->intervals.emplace_back(std::make_pair(new int(*(p.first)), (int *) nullptr));
            } else {
                this->intervals.emplace_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
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

    bool operator==(varInterval interval) {
        auto extIntervals = interval.getIntervals();
        if (this->intervals.size() != extIntervals.size()) {
            return false;
        }
        for (int i = 0; i < this->intervals.size(); i++) {
            if (this->intervals[i].first == nullptr) {
                if (extIntervals[i].first != nullptr) return false;
            } else {
                if (extIntervals[i].first == nullptr || *(this->intervals[i].first) != *(extIntervals[i].first)) {
                    return false;
                }
            }
            if (this->intervals[i].second == nullptr) {
                if (extIntervals[i].second != nullptr)
                    return false;
            } else {
                if (extIntervals[i].second == nullptr || *(this->intervals[i].second) != *(extIntervals[i].second)) {
                    return false;
                }
            }
        }
        return true;
    }
};

//std::set<Instruction*> analyzeDifference(BasicBlock*,std::set<Instruction*>);
//std::set<Instruction*> union_sets(std::set<Instruction*>, std::set<Instruction*>);
void analyzeAdd(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap);

void analyzeSub(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap);

void analyzeMul(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap);

void analyzeSrem(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                 std::map<std::string, Instruction *> &instructionMap);

void analyzeStore(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
                  std::map<std::string, Instruction *> &instructionMap);

void analyzeLoad(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
                 std::map<std::string, Instruction *> &instructionMap);

void analyzeBr(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result);

void analyzeBlock(BasicBlock *BB, std::map<Instruction *, varInterval> &analysisMap,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result);

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

void printAnalysisMap(std::map<BasicBlock *, std::map<Instruction *, varInterval>> &analysisMap,
                      std::map<std::string, Instruction *> &instructionMap);

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

    BasicBlock *entryBB = &F->getEntryBlock();

    //CONSTRUCT All Data Structures
    std::map<BasicBlock *, std::map<Instruction *, varInterval>> analysisMap;
    std::map<std::string, Instruction *> instructionMap;

    std::stack<std::pair<BasicBlock *, std::map<Instruction *, varInterval>>> traversalStack;

    std::map<Instruction *, varInterval> emptySet;
    auto analysisNode = std::make_pair(entryBB, emptySet);
    traversalStack.push(analysisNode);


    // Recursive
    while (!traversalStack.empty()) {
        std::map<BasicBlock *, std::map<Instruction *, varInterval>> result;
        auto pair = traversalStack.top();
        traversalStack.pop();
        analyzeBlock(pair.first, pair.second, instructionMap, result);
        bool flag = false;
        for (auto &p : pair.second) {
            if (! p.second == analysisMap[pair.first][p.first]) { ;

            }
        }

        for (auto &p : result) {
            traversalStack.push(std::make_pair(p.first, p.second));
        }
    }

    //printing
    printAnalysisMap(analysisMap, instructionMap);
}

void analyzeBlock(BasicBlock *BB, std::map<Instruction *, varInterval> &analysisMap,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result) {
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
                analyzeMul(I, analysisMap, instructionMap);
                break;
            }
            case Instruction::SRem: {
                analyzeSrem(I, analysisMap, instructionMap);
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
                std::cout << "Info: " << "Load Completed." << std::endl;
                break;
            }
            case Instruction::ICmp: {
                std::cout << "ICmp" << std::endl;
                dyn_cast<Instruction>(I.getOperand(0))->dump();
                break;
            }
            case Instruction::Br: {
                analyzeBr(I, analysisMap, instructionMap, result);
                std::cout << "INFO: " << "Print Inside BR" << std::endl;
                std::cout << "INFO: " << "Print Inside BR END." << std::endl;
                return;
            }
            default: {
                std::cout << "Unknown" << std::endl;
                break;
            }
        }
    }
    return;
}

//arithmetic operators

void analyzeAdd(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "Add" << std::endl;
    //get op1
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (isa<llvm::ConstantInt>(op2)) {
        std::cout << "If op2 constant, swap" << std::endl;
        Value *op_temp = op2;
        op2 = op1;
        op1 = op_temp;
    }
    if (isa<llvm::ConstantInt>(op1)) {
        std::cout << "If op1 constant, Process" << std::endl;
        auto *CI = dyn_cast<llvm::ConstantInt>(op1);
        auto op1_val = CI->getZExtValue();
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
        std::cout << "Info: If Both not const." << std::endl;
        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        std::cout << "Info: " << op1_str << std::endl;
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];
        std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
        std::cout << "Info: " << op2_str << std::endl;
        varInterval interval_op2 = analysisMap[instructionMap[op2_str]];


        std::vector<std::pair<int *, int *>> outcome;
        std::cout << "Info: " << "Before Combining..." << std::endl;
        for (auto &p1 : interval_op1.getIntervals()) {
            for (auto &p2 : interval_op2.getIntervals()) {
                auto *left = (int *) nullptr;
                auto *right = (int *) nullptr;
                if (p1.first != NULL && p2.first != NULL) {
                    left = new int(*(p1.first) + *(p2.first));
                }
                if (p1.second != NULL && p2.second != NULL) {
                    right = new int(*(p1.second) + *(p2.second));
                }
                outcome.emplace_back(std::make_pair(left, right));
            }
        }
        std::cout << "Info: " << "Complete Combining..." << std::endl;
        sortAndMerge(outcome);
        std::cout << "Info: " << "Complete Sorting & Merging..." << std::endl;
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
        auto *CI = dyn_cast<llvm::ConstantInt>(op2);
        auto op2_val = CI->getZExtValue();

        auto *left = new int(op2_val);
        auto *right = new int(op2_val);

        op2_vector.emplace_back(std::make_pair(left, right));

        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];
        op1_vector = interval_op1.getIntervals();
    } else if (isa<llvm::ConstantInt>(op1)) {
        auto *CI = dyn_cast<llvm::ConstantInt>(op1);
        auto op1_val = CI->getZExtValue();

        auto *left = new int(op1_val);
        auto *right = new int(op1_val);

        op1_vector.emplace_back(std::make_pair(left, right));

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
            int *left = nullptr;
            int *right = nullptr;
            if (p1.first != nullptr && p2.second != nullptr) {
                left = new int(*(p1.first) - *(p2.second));
            }
            if (p1.second != nullptr && p2.first != nullptr) {
                right = new int(*(p1.second) - *(p2.first));
            }
            outcome.emplace_back(std::make_pair(left, right));
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
        auto *CI = dyn_cast<llvm::ConstantInt>(op2);
        auto op2_val = CI->getZExtValue();

        auto *left = new int(op2_val);
        auto *right = new int(op2_val);

        op2_vector.emplace_back(std::make_pair(left, right));

        std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
        varInterval interval_op1 = analysisMap[instructionMap[op1_str]];
        op1_vector = interval_op1.getIntervals();
    } else if (isa<llvm::ConstantInt>(op1)) {
        auto *CI = dyn_cast<llvm::ConstantInt>(op1);
        auto op1_val = CI->getZExtValue();

        auto *left = new int(op1_val);
        auto *right = new int(op1_val);

        op1_vector.emplace_back(std::make_pair(left, right));

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
            int *left = nullptr;
            int *right = nullptr;
            if (p1.first == nullptr && p2.first == nullptr) {

            } else if (p1.first == nullptr && p2.second == nullptr) {

            } else if (p1.first == nullptr && *(p2.first) < 0) {

            }
            outcome.emplace_back(std::make_pair(left, right));
        }
    }
    sortAndMerge(outcome);
    varInterval interval;
    interval.setIntervals(outcome);
    std::string instructionString = getInstructionString(I);
    analysisMap[instructionMap[instructionString]] = interval;
}


void analyzeSrem(Instruction &I, std::map<Instruction *, varInterval> &analysisMap,
                 std::map<std::string, Instruction *> &instructionMap) {

}

void analyzeStore(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
                  std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "Store" << std::endl;
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);
    if (isa<llvm::ConstantInt>(op1)) {
        auto *CI = dyn_cast<llvm::ConstantInt>(op1);
        auto op1_val = CI->getZExtValue();

        std::cout << op1_val << std::endl;

        varInterval tempInterval;
        tempInterval.setVal((int) op1_val);
        auto *op2_instruction = dyn_cast<Instruction>(op2);
        std::string op2Str = getInstructionString(*op2_instruction);
        emptyMap[instructionMap[op2Str]] = tempInterval;
    } else {
        dyn_cast<Instruction>(op1)->dump();
        varInterval tempInterval;
        auto *op1_instruction = dyn_cast<Instruction>(op1);
        auto *op2_instruction = dyn_cast<Instruction>(op2);
        std::string op1_str = getInstructionString(*op1_instruction);
        tempInterval.setIntervals(emptyMap[instructionMap[op1_str]]);
        std::string op2_str = getInstructionString(*op2_instruction);
        emptyMap[instructionMap[op2_str]] = tempInterval;
    }
    dyn_cast<Instruction>(op2)->dump();
}

void analyzeLoad(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
                 std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "Load" << std::endl;
    auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
    std::string str = getInstructionString(*op_instruction);
    emptyMap[&I].setIntervals(emptyMap[instructionMap[str]]);
}

void analyzeBr(Instruction &I, std::map<Instruction *, varInterval> &emptyMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result) {
    std::cout << "Br" << std::endl;
    //br instruction
    auto *branchInst = dyn_cast<BranchInst>(&I);
    auto *bb_op1 = branchInst->getSuccessor(0);
    if (!(branchInst->isConditional())) {
        std::map<Instruction *, varInterval> result_op1;
        for (auto const &x : emptyMap) {
            varInterval v1;
            v1.setIntervals(x.second);
            result_op1[x.first] = v1;
        }
        result[bb_op1] = result_op1;
        return;
    }
    //cmp instruction
    auto *cmpIns = dyn_cast<ICmpInst>(I.getOperand(0));
    std::cout << "info: ";
    cmpIns -> dump();
    //cmp variables
    Value *cmpOp1 = cmpIns->getOperand(0);
    Value *cmpOp2 = cmpIns->getOperand(1);
    //cmp type
    auto p = cmpIns->getSignedPredicate();
    //br blocks
    auto *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));

    std::vector<std::pair<int *, int *>> selected;
    std::vector<std::pair<int *, int *>> excluded;
    //printing
    cmpIns->dump();
    std::cout << p << std::endl;
    bb_op1->dump();
    bb_op2->dump();

    Instruction *cmpVarInst = nullptr;
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
        std::cout << "Alert: " << "Compare Type Unknown." << std::endl;
    }
    std::cout << "INFO: " << "Start Populating..." << std::endl;
    std::map<Instruction *, varInterval> result_op1;
    std::map<Instruction *, varInterval> result_op2;
    for (auto const &x : emptyMap) {
        varInterval v1;
        v1.setIntervals(x.second);
        result_op1[x.first] = v1;
        std::cout << "DEBUG: " << &result_op1[x.first] << std::endl;
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
    std::cout << "Debug: " << "BB OP1 " << bb_op1 << std::endl;
    std::cout << "Debug: " << "BB OP2 " << bb_op2 << std::endl;
    result[bb_op1] = result_op1;
    result[bb_op2] = result_op2;
    return;
}

//comparison

void sge(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded) {
    for (auto &p : interval.getIntervals()) {
        if (p.first == NULL && p.second == NULL) {
            selected.emplace_back(std::make_pair(new int(a), (int *) nullptr));
            excluded.emplace_back(std::make_pair((int *) nullptr, new int(a - 1)));
        } else if (p.first == NULL) {
            if (*(p.second) < a) {
                excluded.emplace_back(std::make_pair((int *) nullptr, new int(*(p.second))));
            } else {
                excluded.emplace_back(std::make_pair((int *) nullptr, new int(a - 1)));
                selected.emplace_back(std::make_pair(new int(a), new int(*(p.second))));
            }
        } else if (p.second == NULL) {
            if (*(p.first) < a) {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                selected.emplace_back(std::make_pair(new int(a), (int *) nullptr));
            } else {
                selected.emplace_back(std::make_pair(new int(*(p.first)), (int *) nullptr));
            }
        } else {
            if (*(p.first) >= a) {
                selected.emplace_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else if (*(p.second) <= a) {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                selected.emplace_back(std::make_pair(new int(a), new int(*(p.second))));
            }
        }
    }
}

void sgt(varInterval &interval, int a, std::vector<std::pair<int *, int *>> &selected,
         std::vector<std::pair<int *, int *>> &excluded) {
    for (auto &p : interval.getIntervals()) {
        if (p.first == NULL && p.second == NULL) {
            selected.emplace_back(std::make_pair(new int(a + 1), (int *) nullptr));
            excluded.emplace_back(std::make_pair((int *) nullptr, new int(a)));
        } else if (p.first == NULL) {
            if (*(p.second) <= a) {
                excluded.emplace_back(std::make_pair((int *) nullptr, new int(*(p.second))));
            } else {
                excluded.emplace_back(std::make_pair((int *) nullptr, new int(a)));
                selected.emplace_back(std::make_pair(new int(a + 1), new int(*(p.second))));
            }
        } else if (p.second == NULL) {
            if (*(p.first) <= a) {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(a)));
                selected.emplace_back(std::make_pair(new int(a + 1), (int *) nullptr));
            } else {
                selected.emplace_back(std::make_pair(new int(*(p.first)), (int *) nullptr));
            }
        } else {
            if (*(p.first) >= a + 1) {
                selected.emplace_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else if (*(p.second) <= a) {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(a + 1)));
                selected.emplace_back(std::make_pair(new int(a + 1), new int(*(p.second))));
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
            selected.emplace_back(std::make_pair(new int(a), new int(a)));
            excluded.emplace_back(std::make_pair((int *) nullptr, new int(a - 1)));
            excluded.emplace_back(std::make_pair(new int(a + 1), (int *) nullptr));
        } else if (p.first == NULL) {
            if (*(p.second) < a) {
                excluded.emplace_back(std::make_pair((int *) nullptr, new int(*(p.second))));
            } else if (*(p.second) == a) {
                excluded.emplace_back(std::make_pair((int *) nullptr, new int(a - 1)));
                selected.emplace_back(std::make_pair(new int(a), new int(a)));
            } else {
                excluded.emplace_back(std::make_pair((int *) nullptr, new int(a - 1)));
                excluded.emplace_back(std::make_pair(new int(a + 1), new int(*(p.second))));
                selected.emplace_back(std::make_pair(new int(a), new int(a)));
            }
        } else if (p.second == NULL) {
            if (*(p.first) < a) {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                excluded.emplace_back(std::make_pair(new int(a + 1), (int *) nullptr));
                selected.emplace_back(std::make_pair(new int(a), new int(a)));
            } else if (*(p.first) == a) {
                excluded.emplace_back(std::make_pair(new int(a + 1), (int *) nullptr));
                selected.emplace_back(std::make_pair(new int(a), new int(a)));
            } else {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), (int *) nullptr));
            }
        } else {
            if (*(p.first) >= a + 1) {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else if (*(p.second) < a) {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(*(p.second))));
            } else {
                excluded.emplace_back(std::make_pair(new int(*(p.first)), new int(a - 1)));
                excluded.emplace_back(std::make_pair(new int(a + 1), new int(*(p.second))));
                selected.emplace_back(std::make_pair(new int(a), new int(a)));
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

void printAnalysisMap(std::map<BasicBlock *, std::map<Instruction *, varInterval>> &analysisMap,
                      std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "===========================================" << std::endl;
    std::cout << "==============Analysis Report==============" << std::endl;
    for (auto &m : analysisMap) {
        m.first->dump();
        for (auto &_m : m.second) {
            std::cout << "-------------------------------------------" << std::endl;
            std::cout << getInstructionString(*(_m.first)) << std::endl;
            _m.second.printIntervals();
        }
        for (auto &mm: m.second) {
            std::string temp_mm = mm.first->getName().str().c_str();
            if (temp_mm.size() == 0) {
                continue;
            }
            for (auto &&mm1: m.second) {
                std::string temp_mm1 = mm1.first->getName().str().c_str();
                if (temp_mm1.size() == 0 || temp_mm == temp_mm1) {
                    continue;
                }
                auto v1 = mm.second.getIntervals();
                int *v1_range[2] = {v1[0].first, v1[v1.size() - 1].second};
                auto v2 = mm1.second.getIntervals();
                int *v2_range[2] = {v2[0].first, v2[v2.size() - 1].second};
                if (v1_range[0] == nullptr || v1_range[1] == nullptr || v2_range[0] == nullptr ||
                    v2_range[1] == nullptr) {
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : " << "Inf" << std::endl;
                } else {
                    long a = std::abs((long) (*v1_range[0] - *v2_range[1]));
                    long b = std::abs((long) (*v1_range[1] - *v2_range[0]));
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : " << (a > b ? a : b) << std::endl;
                }

            }
        }
        std::cout << "===========================================" << std::endl;
    }
}