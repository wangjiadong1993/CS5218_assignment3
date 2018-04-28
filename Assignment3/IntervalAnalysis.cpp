//Name: Wang Jiadong
//Matric: A0105703
//Email: E0267418@u.nus.edu
#include <iostream>
#include <cstdio>
#include <set>
#include <map>
#include <stack>
#include <string>
#include <cmath>
#include <sstream>
#include <iostream>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IR/Constants.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/DataTypes.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

/**
 * This is a data structure for variable interval. variable is in [lower, upper];
 * Here we assume 1000 as INF_POS (positive infinity), and -1000 as INF_NEG(negative infinity)
 * No value shall be set outside this range.
 */
class varInterval {
private:
    //lower and upper boundaries
    int lower;
    int upper;
public:
    //artificial infinity values.
    const static int INF_POS = 1000;
    const static int INF_NEG = -1000;

    //constructors
    varInterval() {
        ;
    }

    varInterval(int lower, int upper) {
        if (lower > upper) {
            this->lower = INF_POS;
            this->upper = INF_NEG;
        }
        if (lower <= INF_NEG)
            this->lower = INF_NEG;
        else if (lower >= INF_POS)
            this->lower = INF_POS;
        else
            this->lower = lower;

        if (upper >= INF_POS)
            this->upper = INF_POS;
        else if (upper <= INF_NEG)
            this->upper = INF_NEG;
        else
            this->upper = upper;
    }

    //getter
    int getUpper() {
        return this->upper;
    }

    int getLower() {
        return this->lower;
    }

    static varInterval getIntersection(varInterval v1, varInterval v2) {
        if (v1.getLower() == varInterval::INF_POS || v2.getLower() == varInterval::INF_POS) {
            return varInterval(varInterval::INF_POS, varInterval::INF_NEG);
        } else if (v1.getLower() > v2.getUpper() || v2.getLower() > v1.getUpper()) {
            return varInterval(varInterval::INF_POS, varInterval::INF_NEG);
        } else {
            return varInterval(std::max(v1.getLower(), v2.getLower()), std::min(v1.getUpper(), v2.getUpper()));
        }
    }

    //setter
    void setLower(int lower) {
        if (lower <= INF_NEG)
            this->lower = INF_NEG;
        else if (lower >= INF_POS)
            this->lower = INF_POS;
        else
            this->lower = lower;

    }

    void setUpper(int upper) {
        if (upper >= INF_POS)
            this->upper = INF_POS;
        else if (upper <= INF_NEG)
            this->upper = INF_NEG;
        else
            this->upper = upper;
    }

    //check if varInterval A belongs to varInterval B.
    bool operator<=(varInterval v) {
        if (this->getLower() == INF_POS && this->getUpper() == INF_NEG) {
            return true;
        }
        if (v.getLower() == INF_POS && v.getUpper() == INF_NEG) {
            return false;
        }
        return this->getLower() >= v.getLower() && this->getUpper() <= v.getUpper();
    }

    bool isEmpty() {
        return this->getLower() == INF_POS && this->getUpper() == INF_NEG;
    }

    //Arithmetic Operator Support for varIntervals.
    static varInterval add(varInterval a, varInterval b) {
        if (a.isEmpty() || b.isEmpty()) {
            return varInterval(INF_POS, INF_NEG);
        }
        int lower = a.getLower() + b.getLower();
        int upper = a.getUpper() + b.getUpper();
        if (a.getLower() == INF_NEG || b.getLower() == INF_NEG) {
            lower = INF_NEG;
        }
        if (a.getUpper() == INF_POS || b.getUpper() == INF_POS) {
            upper = INF_POS;
        }
        return varInterval(lower, upper);
    }

    static varInterval mul(varInterval a, varInterval b) {
        if (a.isEmpty() || b.isEmpty()) {
            return varInterval(INF_POS, INF_NEG);
        }
        std::vector<int> temp;
        temp.push_back(a.getLower() * b.getLower());
        temp.push_back(a.getLower() * b.getUpper());
        temp.push_back(a.getUpper() * b.getLower());
        temp.push_back(a.getUpper() * b.getUpper());
        return varInterval(*(std::min_element(temp.begin(), temp.end())),
                           *(std::max_element(temp.begin(), temp.end())));
    }

    static varInterval div(varInterval a, varInterval b) {
        if (a.isEmpty() || b.isEmpty()) {
            return varInterval(INF_POS, INF_NEG);
        }
        std::vector<int> temp;
        if (b.getLower() == 0 && b.getUpper() == 0) {
            return varInterval(INF_POS, INF_NEG);
        }
        if (b.getLower() * b.getUpper() < 0) {
            temp.push_back(a.getLower());
            temp.push_back(a.getLower() / b.getUpper());
            temp.push_back(a.getUpper());
            temp.push_back(a.getUpper() / b.getUpper());
            temp.push_back(a.getLower() / b.getLower());
            temp.push_back(a.getLower() * -1);
            temp.push_back(a.getUpper() / b.getLower());
            temp.push_back(a.getUpper() * -1);
        } else if (b.getLower() == 0) {
            temp.push_back(a.getLower());
            temp.push_back(a.getLower() / b.getUpper());
            temp.push_back(a.getUpper());
            temp.push_back(a.getUpper() / b.getUpper());
        } else if (b.getUpper() == 0) {
            temp.push_back(a.getLower() / b.getLower());
            temp.push_back(a.getLower() * -1);
            temp.push_back(a.getUpper() / b.getLower());
            temp.push_back(a.getUpper() * -1);
        } else {
            temp.push_back(a.getLower() / b.getLower());
            temp.push_back(a.getLower() / b.getUpper());
            temp.push_back(a.getUpper() / b.getLower());
            temp.push_back(a.getUpper() / b.getUpper());
        }
        return varInterval(*(std::min_element(temp.begin(), temp.end())),
                           *(std::max_element(temp.begin(), temp.end())));
    }

    static varInterval rem(varInterval a, varInterval b) {
        if (a.isEmpty() || b.isEmpty()) {
            return varInterval(INF_POS, INF_NEG);
        }
        //only consider the positive part for a (0 inclusive)
        if (a.getUpper() * a.getLower() <= 0) {
            a.setLower(0);
            a.setUpper(std::max(std::abs(a.getLower()), std::abs(a.getUpper())));
        } else {
            a.setLower(std::min(std::abs(a.getLower()), std::abs(a.getUpper())));
            a.setUpper(std::max(std::abs(a.getLower()), std::abs(a.getUpper())));
        }
        //only consider the positive part for b (0 exclusive)
        if (b.getUpper() * b.getLower() <= 0) {
            b.setLower(1);
            b.setUpper(std::max(std::abs(b.getLower()), std::abs(b.getUpper())));
        } else {
            b.setLower(std::min(std::abs(b.getLower()), std::abs(b.getUpper())));
            b.setUpper(std::max(std::abs(b.getLower()), std::abs(b.getUpper())));
        }

        if (a.getUpper() == varInterval::INF_POS &&
            b.getUpper() == varInterval::INF_POS) {
            return varInterval(0, varInterval::INF_POS);
        } else if (a.getUpper() == varInterval::INF_POS) {
            return varInterval(0, b.getUpper() - 1);
        } else if (b.getUpper() == varInterval::INF_POS) {
            return varInterval(a.getUpper() < b.getLower() ? a.getLower() : 0,
                               std::max(std::abs(a.getLower()), std::abs(a.getUpper())));
        } else {
            if (a.getUpper() < b.getLower()) {
                return a;
            } else if (a.getUpper() < b.getUpper()) {
                return varInterval(0, a.getUpper());
            } else if (a.getLower() < b.getUpper()) {
                return varInterval(0, b.getUpper() - 1);
            } else {
                //can be more accurate
                return varInterval(0, b.getUpper() - 1);
            }

        }
    }

    static varInterval sub(varInterval a, varInterval b) {
        if (a.isEmpty() || b.isEmpty()) {
            return varInterval(INF_POS, INF_NEG);
        }
        int lower = a.getLower() - b.getUpper();
        int upper = a.getUpper() - b.getLower();
        if (a.getLower() == INF_NEG || b.getUpper() == INF_POS) {
            lower = INF_NEG;
        }
        if (a.getUpper() == INF_POS || b.getLower() == INF_NEG) {
            upper = INF_POS;
        }
        return varInterval(lower, upper);
    }

    //formatted printing
    void printIntervals() {
        std::cout << getIntervalString() << std::endl;
    }

    std::string getIntervalString() {
        if (this->isEmpty())
            return "EMPTY SET";
        return (this->getLower() == INF_NEG ? "INF_NEG" : std::to_string(this->getLower())) + "-" +
               (this->getUpper() == INF_POS ? "INF_POS" : std::to_string(
                       this->getUpper()));
    }
};

/**
 * Something used globally:
 *
 * AnalysisMap: Analysis Output, std::map<BasicBlock*, std::map<Instruction*, varInterval>>
 * for each instruction in each BasicBlock, there is a set of analysis output.
 *
 * InstructionMap: InstructionSearchMap, std::map<string, Instruction*>
 * from the instruction name, we can get the pointer to the instruction, where a variable is
 * initially declared.
 *
 */

/*
 * Get the instruction name from the instruction.
 */
std::string getInstructionString(Instruction &I) {
    std::string instructionStr;
    llvm::raw_string_ostream rso(instructionStr);
    I.print(rso);
    return instructionStr;
}

std::string getBasicBlockLabel(BasicBlock *BB) {
    std::string basicBlockStr;
    llvm::raw_string_ostream rso(basicBlockStr);
    rso << ">>>>Basic Block: ";
    BB->printAsOperand(rso, false);
    return basicBlockStr;
}

//debug flags
//to print more stuff.
static bool debug = false;
static bool pause = false;
/**
 * The whole concept of the program:
 * Similar to the idea of Assignment, we use an work list and iterate until the fix point.
 * For each Basic Block, the input is std::map<Instruction*, varInterval>, which means variable -> range pair.
 * For each Instruction inside the basic block, calculate the new range for the variable.
 * Till the end of the Basic Block, Which can be one of the three cases:
 * 1. Ret Statement, no successors
 * 2. Unconditional Br, one successor, no need for backward analysis.
 * 3. Conditional Br, two successors, need for backward analysis.
 * The most difficult part is the conditional Br. When conducting forward analysis till the conditional Br instruction.
 * We get the exit variable -> range pair for the block. But we need to conduct the backward analysis to get the entry set for each of the successors.
 * For unconditional Br, the exit value of the Basic Block is the entry value for the sole successor.
 * And, of course, union is still needed, to union the exit value with the existing exit value set.
 * If the union output is different from that of the last iteration, then the successors must be added into work list.
 */


/*
 *  for X <= Y, or X > Y, we can generalize them into:
 *  X - Y = [-INF, 0], X - Y = [1, INF].
 *
 *  ^   ^        ^     ^   ^        ^
 *  X   Y        R     X   Y        R
 */
void comp(varInterval &intervalX, varInterval &intervalY, varInterval &intervalR);



/**
 * Here are the analysis functions for each arithmetic operators.
 * The analysis for operators, are both forward and backward.
 * which means, we need to handle the range change for each variable due to the use of the operator / instruction, during
 * both forward analysis and backward analysis.
 */
/** analysis for alloca instruction, supports both forward and backward.*/
void analyzeAlloca(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                   std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for add instruction, supports both forward and backward.*/
void analyzeAdd(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for sub instruction, supports both forward and backward.*/
void analyzeSub(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for mul instruction, supports both forward and backward.*/
void analyzeMul(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for srem instruction, supports both forward and backward.*/
void analyzeSrem(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for store instruction, supports both forward and backward.*/
void analyzeStore(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                  std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for load instruction, supports both forward and backward.*/
void analyzeLoad(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward);

/** analysis for br instruction, supports both forward and backward.*/
void analyzeBr(BasicBlock *BB, Instruction &I, std::map<Instruction *, varInterval> &blockMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result);

/**
 * The analysis entry for each basic block.
 * The input are entry variables.
 * analysisMap is the place to place the union output.
 * result is the map to place the entry value set for successors.
 */

/*
 * analyze a block
 */
bool analyzeBlock(BasicBlock *BB, std::map<Instruction *, varInterval> &input,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, varInterval>> &analysisMap,
                  std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result);

bool analyzeBlockWithContext(BasicBlock *BB,
                             std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &result);

/**
 * Printing Function
 */
/*
 * print the output
 */
void printAnalysisMap(std::map<BasicBlock *, std::map<Instruction *, varInterval>> &analysisMap,
                      std::map<std::string, Instruction *> &instructionMap);


void printBasicBlockContext(std::map<BasicBlock *, std::vector<std::map<Instruction *, varInterval>>> context) {
    for (auto &p : context) {
        std::cout << getBasicBlockLabel(p.first) << std::endl;
        for (auto &pp : p.second) {
            std::cout << "Case:" << std::endl;
            for (auto &ppp : pp) {
                std::cout << getInstructionString(*(ppp.first)) << "  >>  " << ppp.second.getIntervalString()
                          << std::endl;
            }
        }
    }
}

void printcontextCombination(std::vector<std::map<Instruction *, varInterval>> &contextCombination) {
    for (auto &combination : contextCombination) {
        std::cout << "================Context=============" << std::endl;
        for (auto &combination_pair : combination) {
            std::cout << getInstructionString(*(combination_pair.first)) << "  >>  "
                      << combination_pair.second.getIntervalString()
                      << std::endl;
        }
    }
}

void printAnalysisMapWithContext(
        std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &contextAnalysisMap) {
    for (auto &pair : contextAnalysisMap) {
        std::cout << getBasicBlockLabel(pair.first) << std::endl;
        for (auto &context_variables : pair.second) {
            bool isNotEmpty = true;
            for (auto &context_pair : *context_variables.first){
                if(context_pair.second.isEmpty()){
                    isNotEmpty = false;
                    break;
                }
            }
            for (auto &variable_pair : context_variables.second){
                if(variable_pair.second.isEmpty()){
                    isNotEmpty = false;
                    break;
                }
            }
            if(!isNotEmpty){
                continue;
            }
            std::cout << "Context:  ====" << std::endl;
            for (auto &context_pair : *context_variables.first) {
                std::cout << getInstructionString(*context_pair.first) << ">>"
                          << context_pair.second.getIntervalString() << std::endl;
            }
            std::cout << "Variables: ====" << std::endl;
            for (auto &variable_pair : context_variables.second) {
                std::cout << getInstructionString(*variable_pair.first) << ">>"
                          << variable_pair.second.getIntervalString() << std::endl;
            }
        }
    }
}


//main function
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
    traversalStack.push(std::make_pair(entryBB, emptySet));

    while (!traversalStack.empty()) {
        std::map<BasicBlock *, std::map<Instruction *, varInterval>> result;
        auto pair = traversalStack.top();
        traversalStack.pop();
        if (debug) {
            std::cout << "==============BLOCK ANALYSIS " << getBasicBlockLabel(pair.first) << "============="
                      << std::endl;
            std::cout << ">>>>INPUT:" << std::endl;
            for (auto &p : pair.second) {
                std::cout << getInstructionString(*(p.first)) << "    " << std::endl;
                p.second.printIntervals();
            }
            std::cout << ">>>>PROCESSING:" << std::endl;
        }

        auto changed = analyzeBlock(pair.first, pair.second, instructionMap, analysisMap, result);


        if (changed) {
            for (auto &p : result) {
                traversalStack.push(p);
            }
        }

        if (debug) {
            std::cout << ">>>>OUTPUT:" << std::endl;
            for (auto &p : analysisMap[pair.first]) {
                std::cout << getInstructionString(*(p.first)) << "    " << std::endl;
                p.second.printIntervals();
            }
            std::cout << ">>>>SUCCESSORS:" << std::endl;
            for (auto &p : result) {
                std::cout << getBasicBlockLabel(p.first) << std::endl;
                for (auto &pp : p.second) {
                    std::cout << getInstructionString(*(pp.first)) << "    " << std::endl;
                    pp.second.printIntervals();
                }
            }
            std::cout << "================================================================" << std::endl;
        }


        //to pause for each iteration during debug.
        if (pause) {
            std::cin.get();
        }
    }

//    printAnalysisMap(analysisMap, instructionMap);

    /**
     * for each basic block, generate the context
     */
    std::map<BasicBlock *, std::vector<std::map<Instruction *, varInterval>>> context;

    for (auto &p : analysisMap) {
        BasicBlock *BB = p.first;
        std::map<Instruction *, varInterval> bbOutput = p.second;
        std::map<BasicBlock *, std::map<Instruction *, varInterval>> result;
        /**
         * for each basic block, find the br instruction
         */
        for (auto &I: *BB) {
            if (I.getOpcode() == Instruction::Br) {
                /**
                 * analyze the block backward
                 */
                analyzeBr(BB, I, bbOutput, instructionMap, result);
                /**
                 * if ret, or unconditional br, ignore
                 */
                if (result.size() <= 1)
                    break;
                /**
                 * for conditional br, record the result
                 */
                for (auto &pp : result) {
                    context[BB].push_back(pp.second);
                }
                /**
                 * for two conditions, remove the variables, that have idential varInterval in two conditions
                 */
                for (auto it_0 = context[BB][0].begin(); it_0 != context[BB][0].end();) {
                    bool broke = false;
                    for (auto it_1 = context[BB][1].begin(); it_1 != context[BB][1].end();) {
                        if (it_0->second.getIntervalString() == it_1->second.getIntervalString()) {
                            it_0 = context[BB][0].erase(it_0);
                            it_1 = context[BB][1].erase(it_1);
                            broke = true;
                            break;
                        }
                        ++it_1;
                    }
                    if (!broke)
                        ++it_0;
                }
                break;
            }
        }
    }
    /**
     * finish generating the context for each block
     */
//    std::map<BasicBlock*, std::vector<std::map<Instruction*, varInterval>>> context;
    printBasicBlockContext(context);

    /**
     * generates different context combinations
     */
    std::vector<std::map<Instruction *, varInterval>> contextCombination;

    /**
         * context: pair(BasicBlock*, std::vector<std::map<Instruction*, varInterval>>)
         * >>>>Basic Block: %2
         * Case:
         *  %N = alloca i32, align 4  >>  1-INF_POS
         *  %5 = load i32* %N, align 4  >>  1-INF_POS
         *Case:
         *  %N = alloca i32, align 4  >>  INF_NEG-0
         *  %5 = load i32* %N, align 4  >>  INF_NEG-0
         *>>>>Basic Block: %7
         *Case:
         *  %a = alloca i32, align 4  >>  1-6
         *  %8 = load i32* %a, align 4  >>  1-6
         *Case:
         *  %a = alloca i32, align 4  >>  -5-0
         *  %8 = load i32* %a, align 4  >>  -5-0
         */
    //for each context
    for (auto context_pair : context) {
        /**
         * context_pair: pair(BasicBlock*, std::vector<std::map<Instruction*, varInterval>>)
         * key:
         * Basic Block: %2
         * Values:
         *  %N = alloca i32, align 4  >>  1-INF_POS
         *  %5 = load i32* %N, align 4  >>  1-INF_POS
         *  ----
         *  %N = alloca i32, align 4  >>  INF_NEG-0
         *  %5 = load i32* %N, align 4  >>  INF_NEG-0
         */
        if (contextCombination.size() == 0) {
            contextCombination = context_pair.second;
            continue;
        }
        auto contextCombinationBackUp = contextCombination;
        contextCombination.clear();
        //for each case
        for (auto context_pair_map : context_pair.second) {
            /**
            * context_pair_map: std::map<Instruction*, varInterval>)
            *  %N = alloca i32, align 4  >>  INF_NEG-0
            *  %5 = load i32* %N, align 4  >>  INF_NEG-0
            */
            for (auto temp_map : contextCombinationBackUp) {
                /**
                 * temp_pair: std::map<Instruction *, varInterval>
                 */
                for (auto it = context_pair_map.begin(); it != context_pair_map.end(); ++it) {
                    if (temp_map.find(it->first) != temp_map.end()) {
                        temp_map.insert(std::make_pair(it->first,
                                                       varInterval::getIntersection(temp_map.find(it->first)->second,
                                                                                    it->second)));
                    } else {
                        temp_map.insert(*it);
                    }
                }
                contextCombination.push_back(temp_map);
            }
        }
    }
//    printcontextCombination(contextCombination);


    //CONSTRUCT All Data Structures
    std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> contextAnalysisMap;
    std::stack<std::pair<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>>> contextTraversalStack;
    std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> initialSet;
    for (auto &combination : contextCombination) {
        std::map<Instruction *, varInterval> emptySet;
        initialSet.insert(std::make_pair(&combination, combination));
    }
    contextTraversalStack.push(std::make_pair(entryBB, initialSet));

    while (!contextTraversalStack.empty()) {
        std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> result;
        auto pair = contextTraversalStack.top();
        contextTraversalStack.pop();

        auto changed = analyzeBlockWithContext(pair.first, pair.second, instructionMap, contextAnalysisMap, result);
        if (changed) {
            for (auto &p : result) {
                contextTraversalStack.push(p);
            }
        }
    }

    printAnalysisMapWithContext(contextAnalysisMap);
}


std::vector<varInterval> getOperandIntervals(Instruction &I,
                                             Value *op1,
                                             Value *op2,
                                             std::map<Instruction *, varInterval> context,
                                             std::map<Instruction *, varInterval> &variables,
                                             std::map<std::string, Instruction *> &instructionMap) {
    bool isInContext = context.find(&I) != context.end();
    varInterval interval_op1;
    varInterval interval_op2;
    if (isa<llvm::ConstantInt>(op2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        interval_op1 = (context.find(op1_instr) != context.end() ? context[op1_instr] : variables[op1_instr]);
        interval_op2 = varInterval(op2_val, op2_val);

    } else if (isa<llvm::ConstantInt>(op1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = varInterval(op1_val, op1_val);
        interval_op2 = (context.find(op2_instr) != context.end() ? context[op2_instr] : variables[op2_instr]);
    } else {
        auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        interval_op1 = context.find(op1_instr) != context.end() ? context[op1_instr] : variables[op1_instr];
        interval_op2 = context.find(op2_instr) != context.end() ? context[op2_instr] : variables[op2_instr];
    }
    std::vector<varInterval> results;
    results.push_back(interval_op1);
    results.push_back(interval_op2);
    return results;
}

bool unionAndCheckChanged(std::map<Instruction *, varInterval> &input,
                          std::map<Instruction *, varInterval> &analysisMap) {
    if (debug) {
        std::cout << ">>>>BEFORE UNION:" << std::endl;
        std::cout << ">>INPUT" << std::endl;
        for (auto &p : input) {
            std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
        }
        std::cout << ">>ANALYSIS MAP" << std::endl;
        for (auto &p : analysisMap) {
            std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
        }
    }
    bool changed = false;
    for (auto &p : input) {
        if (analysisMap.find(p.first) == analysisMap.end()) {
            analysisMap[p.first] = p.second;
            changed = true;
        } else if (analysisMap[p.first].isEmpty()) {
            if (!p.second.isEmpty()) {
                analysisMap[p.first] = p.second;
                changed = true;
            }
        } else if (!(p.second <= analysisMap[p.first])) {
            analysisMap[p.first].setLower(std::min(p.second.getLower(), analysisMap[p.first].getLower()));
            analysisMap[p.first].setUpper(std::max(p.second.getUpper(), analysisMap[p.first].getUpper()));
            changed = true;
        } else { ;
        }
    }
    if (debug) {
        std::cout << ">>>>AFTER UNION:" << std::endl;
        std::cout << ">>ANALYSIS MAP" << std::endl;
        for (auto &p : analysisMap) {
            std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
        }
    }

    return changed;
}


void analyzeAddWithContext(Instruction &I,
                           std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                           std::map<std::string, Instruction *> &instructionMap) {
    for (auto &pair : blockMap) {
        auto context = *pair.first;
        auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, pair.second,
                                          instructionMap);
        varInterval temp = varInterval::add(result[0], result[1]);
        if (pair.first->find(&I) != pair.first->end()) {
            if (varInterval::getIntersection(temp, context[&I]).isEmpty())
                for (auto &var : pair.second) var.second = varInterval(varInterval::INF_POS, varInterval::INF_NEG);
            else{
                pair.second[&I] = temp;
            }

        } else {
            pair.second[&I] = temp;
        }
    }
}

void analyzeSubWithContext(Instruction &I,
                           std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                           std::map<std::string, Instruction *> &instructionMap) {
    for (auto &pair : blockMap) {
        auto context = *pair.first;
        auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, pair.second,
                                          instructionMap);
        varInterval temp = varInterval::sub(result[0], result[1]);
        if (pair.first->find(&I) != pair.first->end()) {
            if (varInterval::getIntersection(temp, context[&I]).isEmpty())
                for (auto &var : pair.second) var.second = varInterval(varInterval::INF_POS, varInterval::INF_NEG);
            else{
                pair.second[&I] = temp;
            }
        } else {
            pair.second[&I] = temp;
        }
    }
}

void analyzeMulWithContext(Instruction &I,
                           std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                           std::map<std::string, Instruction *> &instructionMap) {
    for (auto &pair : blockMap) {
        auto context = *pair.first;
        auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, pair.second,
                                          instructionMap);
        varInterval temp = varInterval::mul(result[0], result[1]);
        if (pair.first->find(&I) != pair.first->end()) {
            if (varInterval::getIntersection(temp, context[&I]).isEmpty())
                for (auto &var : pair.second) var.second = varInterval(varInterval::INF_POS, varInterval::INF_NEG);
            else{
                pair.second[&I] = temp;
            }
        } else {
            pair.second[&I] = temp;
        }
    }
}

void analyzeSremWithContext(Instruction &I,
                            std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                            std::map<std::string, Instruction *> &instructionMap) {
    for (auto &pair : blockMap) {
        auto context = *pair.first;
        auto result = getOperandIntervals(I, I.getOperand(0), I.getOperand(1), *pair.first, pair.second,
                                          instructionMap);
        varInterval temp = varInterval::rem(result[0], result[1]);
        if (pair.first->find(&I) != pair.first->end()) {
            if (varInterval::getIntersection(temp, context[&I]).isEmpty())
                for (auto &var : pair.second) var.second = varInterval(varInterval::INF_POS, varInterval::INF_NEG);
            else{
                pair.second[&I] = temp;
            }
        } else {
            pair.second[&I] = temp;
        }
    }
}


void analyzeStoreWithContext(Instruction &I,
                             std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                             std::map<std::string, Instruction *> &instructionMap) {
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);
    varInterval temp;
    for (auto &pair : blockMap) {
        auto context = *pair.first;
        auto variables = pair.second;
        if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            temp = varInterval(op1_val, op1_val);
        } else {
            auto op1_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))];
            temp = context.find(op1_instr) != context.end() ? context[op1_instr] : variables[op1_instr];
        }
        auto op2_instr = instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))];
        if (context.find(op2_instr) != context.end()) {
            if (varInterval::getIntersection(temp, context[op2_instr]).isEmpty())
                for (auto &var : pair.second) var.second = varInterval(varInterval::INF_POS, varInterval::INF_NEG);
            else{
                pair.second[op2_instr] = temp;
            }
        } else {
            pair.second[op2_instr] = temp;
        }
    }
}

void analyzeLoadWithContext(Instruction &I,
                            std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                            std::map<std::string, Instruction *> &instructionMap) {
    for (auto &pair : blockMap) {
        auto context = *pair.first;
        auto variables = pair.second;
        varInterval temp;
        auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
        auto op_instr = instructionMap[getInstructionString(*op_instruction)];
        temp = context.find(op_instr) != context.end() ? context[op_instr] : variables[op_instr];

        if (context.find(&I) != context.end()) {
            if (varInterval::getIntersection(temp, context[&I]).isEmpty())
                for (auto &var : pair.second) var.second = varInterval(varInterval::INF_POS, varInterval::INF_NEG);
        } else {
            pair.second[&I] = varInterval(varInterval::INF_NEG, varInterval::INF_POS);
        }
    }
}

void analyzeAllocaWithContext(Instruction &I,
                              std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                              std::map<std::string, Instruction *> &instructionMap) {
    for (auto &pair : blockMap) {
        auto context = *pair.first;
        auto variables = pair.second;
        if (context.find(&I) != context.end()) { ;
        } else {
            pair.second[&I] = varInterval(varInterval::INF_NEG, varInterval::INF_POS);
        }
    }
}

void analyzeBrWithContext(Instruction &I,
                          std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &blockMap,
                          std::map<std::string, Instruction *> &instructionMap,
                          std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &result) {
    auto *branchInst = dyn_cast<BranchInst>(&I);
    auto *bb_op1 = branchInst->getSuccessor(0);
    if (!(branchInst->isConditional())) {
        result[bb_op1] = blockMap;
        return;
    }
    auto *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));
    result[bb_op1] = blockMap;
    result[bb_op2] = blockMap;
}


bool unionAndCheckChangedWithContext(
        std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &input,
        std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &analysisMap) {
    bool changed = false;
    for (auto &context : input) {
        /**
         * Each context
         */
        auto input_result = context.second;
        auto analysisMap_result = analysisMap[context.first];
        for (auto &p : input_result) {
            if (analysisMap_result.find(p.first) == analysisMap_result.end()) {
                analysisMap[context.first][p.first] = p.second;
                changed = true;
            } else if (analysisMap_result[p.first].isEmpty()) {
                if (!p.second.isEmpty()) {
                    analysisMap[context.first][p.first] = p.second;
                    changed = true;
                }
            } else if (!(p.second <= analysisMap_result[p.first])) {
                analysisMap[context.first][p.first].setLower(
                        std::min(p.second.getLower(), analysisMap_result[p.first].getLower()));
                analysisMap[context.first][p.first].setUpper(
                        std::max(p.second.getUpper(), analysisMap_result[p.first].getUpper()));
                changed = true;
            } else { ;
            }
        }
    }
    return changed;
}


bool analyzeBlockWithContext(BasicBlock *BB,
                             std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>> &input,
                             std::map<std::string, Instruction *> &instructionMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &contextAnalysisMap,
                             std::map<BasicBlock *, std::map<std::map<Instruction *, varInterval> *, std::map<Instruction *, varInterval>>> &result) {
    //with context, the algo will differ a bit
    for (auto &I: *BB) {
        switch (I.getOpcode()) {
            case Instruction::Add: {
                analyzeAddWithContext(I, input, instructionMap);
                break;
            }
            case Instruction::Sub: {
                analyzeSubWithContext(I, input, instructionMap);
                break;
            }
            case Instruction::Mul: {
                analyzeMulWithContext(I, input, instructionMap);
                break;
            }
            case Instruction::SRem: {
                analyzeSremWithContext(I, input, instructionMap);
                break;
            }
            case Instruction::Alloca: {
                analyzeAllocaWithContext(I, input, instructionMap);
                break;
            }
            case Instruction::Store: {
                analyzeStoreWithContext(I, input, instructionMap);
                break;
            }
            case Instruction::Load: {
                analyzeLoadWithContext(I, input, instructionMap);
                break;
            }
            case Instruction::ICmp: {
                break;
            }
            case Instruction::Br: {
                analyzeBrWithContext(I, input, instructionMap, result);
                return unionAndCheckChangedWithContext(input, contextAnalysisMap[BB]);
            }
            case Instruction::Ret: {
                return unionAndCheckChangedWithContext(input, contextAnalysisMap[BB]);
            }
            default: {
                std::cout << "Unknown: " << I.getOpcodeName() << std::endl;
                break;
            }
        }
    }
    return false;


}

bool analyzeBlock(BasicBlock *BB, std::map<Instruction *, varInterval> &input,
                  std::map<std::string, Instruction *> &instructionMap,
                  std::map<BasicBlock *, std::map<Instruction *, varInterval>> &analysisMap,
                  std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result) {
    for (auto &I: *BB) {
        std::string instructionStr = getInstructionString(I);
        instructionMap[instructionStr] = &I;
        switch (I.getOpcode()) {
            case Instruction::Add: {
                analyzeAdd(I, input, instructionMap, false);
                break;
            }
            case Instruction::Sub: {
                analyzeSub(I, input, instructionMap, false);
                break;
            }
            case Instruction::Mul: {
                analyzeMul(I, input, instructionMap, false);
                break;
            }
            case Instruction::SRem: {
                analyzeSrem(I, input, instructionMap, false);
                break;
            }
            case Instruction::Alloca: {
                analyzeAlloca(I, input, instructionMap, false);
                break;
            }
            case Instruction::Store: {
                analyzeStore(I, input, instructionMap, false);
                break;
            }
            case Instruction::Load: {
                analyzeLoad(I, input, instructionMap, false);
                break;
            }
            case Instruction::ICmp: {
                break;
            }
            case Instruction::Br: {
                analyzeBr(BB, I, input, instructionMap, result);
                return unionAndCheckChanged(input, analysisMap[BB]);
            }
            case Instruction::Ret: {
                return unionAndCheckChanged(input, analysisMap[BB]);
            }
            default: {
                std::cout << "Unknown: " << I.getOpcodeName() << std::endl;
                break;
            }
        }
    }
    return false;
}

//------OPERATOR FUNCTION IMPLEMENTATION
void analyzeAdd(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            varInterval interval_op1 = blockMap[instructionMap[op1_str]];
            blockMap[instructionMap[instructionName]] = varInterval::add(interval_op1, varInterval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            varInterval interval_op2 = blockMap[instructionMap[op2_str]];
            blockMap[instructionMap[instructionName]] = varInterval::add(interval_op2, varInterval(op1_val, op1_val));
        } else {
            varInterval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            varInterval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = varInterval::add(interval_op1, interval_op2);
        }
        if (debug) {
            std::cout << instructionName << " ===>" << blockMap[instructionMap[instructionName]].getIntervalString()
                      << std::endl;
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = varInterval::sub(instructionInterval, varInterval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = varInterval::sub(instructionInterval, varInterval(op1_val, op1_val));
        } else {
            //auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            int lowerInstruction = instructionInterval.getLower();// - op1_val;
            int upperInstruction = instructionInterval.getUpper();// - op1_val;
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            int lowerOp1 = op1Interval.getLower();
            int upperOp1 = op1Interval.getUpper();
            int lowerOp2 = op2Interval.getLower();
            int upperOp2 = op2Interval.getUpper();
            int newLowerOp1 = std::max(lowerOp1, lowerInstruction - upperOp2);
            int newUpperOp1 = std::min(upperOp1, upperInstruction - lowerOp2);
            int newLowerOp2 = std::max(lowerOp2, lowerInstruction - upperOp1);
            int newUpperOp2 = std::min(upperOp2, upperInstruction - lowerOp1);
            blockMap[instructionMap[op1String]] = varInterval(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = varInterval(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeSub(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            varInterval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            blockMap[instructionMap[instructionName]] = varInterval::sub(interval_op1, varInterval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            varInterval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = varInterval::sub(varInterval(op1_val, op1_val), interval_op2);
        } else {
            varInterval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            varInterval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = varInterval::sub(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = varInterval::add(instructionInterval, varInterval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = varInterval::sub(varInterval(op1_val, op1_val), instructionInterval);
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            varInterval op1Temp = varInterval::sub(instructionInterval, op2Interval);
            varInterval op2Temp = varInterval::sub(instructionInterval, op1Interval);
            int newLowerOp1 = std::max(op1Interval.getLower(), op1Temp.getLower());
            int newUpperOp1 = std::min(op1Interval.getUpper(), op1Temp.getUpper());
            int newLowerOp2 = std::max(op2Interval.getLower(), op2Temp.getLower());
            int newUpperOp2 = std::min(op2Interval.getUpper(), op2Temp.getUpper());
            blockMap[instructionMap[op1String]] = varInterval(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = varInterval(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeMul(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            varInterval interval_op1 = blockMap[instructionMap[op1_str]];
            blockMap[instructionMap[instructionName]] = varInterval::mul(interval_op1, varInterval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            varInterval interval_op2 = blockMap[instructionMap[op2_str]];
            blockMap[instructionMap[instructionName]] = varInterval::mul(interval_op2, varInterval(op1_val, op1_val));
        } else {
            varInterval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            varInterval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = varInterval::mul(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            blockMap[instructionMap[op1_str]] = varInterval::div(instructionInterval, varInterval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = varInterval::div(instructionInterval, varInterval(op1_val, op1_val));
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            auto op1Interval = blockMap[instructionMap[op1String]];
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
            auto op2Interval = blockMap[instructionMap[op2String]];
            varInterval op1Temp = varInterval::div(instructionInterval, op2Interval);
            varInterval op2Temp = varInterval::div(instructionInterval, op1Interval);
            int newLowerOp1 = std::max(op1Interval.getLower(), op1Temp.getLower());
            int newUpperOp1 = std::min(op2Interval.getUpper(), op1Temp.getUpper());
            int newLowerOp2 = std::max(op2Interval.getLower(), op2Temp.getLower());
            int newUpperOp2 = std::min(op2Interval.getUpper(), op2Temp.getUpper());
            blockMap[instructionMap[op1String]] = varInterval(newLowerOp1, newUpperOp1);
            blockMap[instructionMap[op2String]] = varInterval(newLowerOp2, newUpperOp2);
        }
    }
}

void analyzeSrem(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward) {
    std::string instructionName = getInstructionString(I);
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);

    if (!backward) {
        if (isa<llvm::ConstantInt>(op2)) {
            auto op2_val = dyn_cast<llvm::ConstantInt>(op2)->getZExtValue();
            varInterval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            blockMap[instructionMap[instructionName]] = varInterval::rem(interval_op1, varInterval(op2_val, op2_val));
        } else if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            varInterval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = varInterval::rem(varInterval(op1_val, op1_val), interval_op2);
        } else {
            varInterval interval_op1 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op1))]];
            varInterval interval_op2 = blockMap[instructionMap[getInstructionString(*dyn_cast<Instruction>(op2))]];
            blockMap[instructionMap[instructionName]] = varInterval::rem(interval_op1, interval_op2);
        }
    } else {
        auto instructionInterval = blockMap[instructionMap[instructionName]];
        if (isa<llvm::ConstantInt>(op2)) {
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
//            blockMap[instructionMap[op1_str]] = varInterval(varInterval::INF_NEG, varInterval::INF_POS);
        } else if (isa<llvm::ConstantInt>(op1)) {
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
//            blockMap[instructionMap[op2_str]] = varInterval(varInterval::INF_NEG, varInterval::INF_POS);
        } else {
            std::string op1String = getInstructionString(*dyn_cast<Instruction>(op1));
            std::string op2String = getInstructionString(*dyn_cast<Instruction>(op2));
//            blockMap[instructionMap[op1String]] = varInterval(varInterval::INF_NEG, varInterval::INF_POS);
//            blockMap[instructionMap[op2String]] = varInterval(varInterval::INF_NEG, varInterval::INF_POS);
        }
    }
}

void analyzeStore(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                  std::map<std::string, Instruction *> &instructionMap, bool backward) {
    Value *op1 = I.getOperand(0);
    Value *op2 = I.getOperand(1);
    if (!backward) {
        if (isa<llvm::ConstantInt>(op1)) {
            auto op1_val = dyn_cast<llvm::ConstantInt>(op1)->getZExtValue();
            std::string op2Str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2Str]] = varInterval(op1_val, op1_val);
        } else {
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op2_str]] = blockMap[instructionMap[op1_str]];
        }
    } else {
        if (!isa<llvm::ConstantInt>(op1)) {
            std::string op1_str = getInstructionString(*dyn_cast<Instruction>(op1));
            std::string op2_str = getInstructionString(*dyn_cast<Instruction>(op2));
            blockMap[instructionMap[op1_str]] = blockMap[instructionMap[op2_str]];
        }
    }
}

void analyzeLoad(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                 std::map<std::string, Instruction *> &instructionMap, bool backward) {
    if (!backward) {
        auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
        std::string str = getInstructionString(*op_instruction);
        blockMap[instructionMap[getInstructionString(I)]] = blockMap[instructionMap[str]];
    } else {
        auto *op_instruction = dyn_cast<Instruction>(I.getOperand(0));
        std::string str = getInstructionString(*op_instruction);
        if (blockMap[instructionMap[str]].isEmpty()) { ;
        } else if (blockMap[instructionMap[getInstructionString(I)]].isEmpty()) {
            blockMap[instructionMap[str]].setLower(varInterval::INF_POS);
            blockMap[instructionMap[str]].setUpper(varInterval::INF_NEG);
        } else {
            int lower = std::max(blockMap[instructionMap[str]].getLower(),
                                 blockMap[instructionMap[getInstructionString(I)]].getLower());
            int upper = std::min(blockMap[instructionMap[str]].getUpper(),
                                 blockMap[instructionMap[getInstructionString(I)]].getUpper());
            blockMap[instructionMap[str]].setLower(lower);
            blockMap[instructionMap[str]].setUpper(upper);
        }
    }
}

void analyzeAlloca(Instruction &I, std::map<Instruction *, varInterval> &blockMap,
                   std::map<std::string, Instruction *> &instructionMap, bool backward) {
    if (!backward) {
        blockMap[instructionMap[getInstructionString(I)]] = varInterval(varInterval::INF_NEG, varInterval::INF_POS);
    } else {
        return;
    }
}

//------COMPARISON FUNCTION IMPLEMENTATION
void comp(varInterval &intervalX, varInterval &intervalY, varInterval &intervalR) {
    varInterval xRange = varInterval::add(intervalY, intervalR);
    intervalX.setLower(std::max(xRange.getLower(), intervalX.getLower()));
    intervalX.setUpper(std::min(xRange.getUpper(), intervalX.getUpper()));
    if (intervalX.getLower() > intervalX.getUpper()) {
        intervalX.setLower(varInterval::INF_POS);
        intervalX.setUpper(varInterval::INF_NEG);
    }
    varInterval yMinusRange = varInterval::add(intervalR, intervalX);
    intervalY.setLower(std::max(yMinusRange.getUpper() * -1, intervalY.getLower()));
    intervalY.setUpper(std::min(yMinusRange.getLower() * -1, intervalY.getUpper()));
    if (intervalY.getLower() > intervalY.getUpper()) {
        intervalY.setLower(varInterval::INF_POS);
        intervalY.setUpper(varInterval::INF_NEG);
    }
}

void compare(Value *cmpOp1, Value *cmpOp2, varInterval &R, std::map<Instruction *, varInterval> &result,
             std::map<std::string, Instruction *> &instructionMap) {
    if (isa<llvm::ConstantInt>(cmpOp1)) {
        auto op1_val = dyn_cast<llvm::ConstantInt>(cmpOp1)->getZExtValue();
        varInterval X(op1_val, op1_val);
        comp(X, result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))]], R);
    } else if (isa<llvm::ConstantInt>(cmpOp2)) {
        auto op2_val = dyn_cast<llvm::ConstantInt>(cmpOp2)->getZExtValue();
        varInterval Y(op2_val, op2_val);
        comp(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))]], Y, R);
    } else {
        comp(result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp1))]],
             result[instructionMap[getInstructionString(*dyn_cast<Instruction>(cmpOp2))]], R);
    }
}

void analyzeBr(BasicBlock *BB, Instruction &I, std::map<Instruction *, varInterval> &blockMap,
               std::map<std::string, Instruction *> &instructionMap,
               std::map<BasicBlock *, std::map<Instruction *, varInterval>> &result) {
    auto *branchInst = dyn_cast<BranchInst>(&I);
    auto *bb_op1 = branchInst->getSuccessor(0);
    if (!(branchInst->isConditional())) {
        result[bb_op1] = blockMap;
        return;
    }
    //cmp instruction
    auto *cmpIns = dyn_cast<ICmpInst>(I.getOperand(0));
    //cmp variables
    Value *cmpOp1 = cmpIns->getOperand(0);
    Value *cmpOp2 = cmpIns->getOperand(1);
    //br blocks
    auto *bb_op2 = dyn_cast<BasicBlock>(branchInst->getSuccessor(1));

    std::map<Instruction *, varInterval> branch1 = blockMap;
    std::map<Instruction *, varInterval> branch2 = blockMap;

    varInterval Y(varInterval::INF_NEG, varInterval::INF_POS);
    switch (cmpIns->getSignedPredicate()) {
        case CmpInst::Predicate::ICMP_SGT : {
            varInterval R1(1, varInterval::INF_POS);
            varInterval R2(varInterval::INF_NEG, 0);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SGE : {
            varInterval R1(0, varInterval::INF_POS);
            varInterval R2(varInterval::INF_NEG, -1);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SLT : {
            varInterval R1(varInterval::INF_NEG, -1);
            varInterval R2(0, varInterval::INF_POS);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_SLE : {
            varInterval R1(varInterval::INF_NEG, 0);
            varInterval R2(1, varInterval::INF_POS);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_EQ : {
            varInterval R1(0, 0);
            varInterval R2(varInterval::INF_NEG, varInterval::INF_POS);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        case CmpInst::Predicate::ICMP_NE : {
            varInterval R1(varInterval::INF_NEG, varInterval::INF_POS);
            varInterval R2(0, 0);
            compare(cmpOp1, cmpOp2, R1, branch1, instructionMap);
            compare(cmpOp1, cmpOp2, R2, branch2, instructionMap);
            break;
        }
        default: {
            std::cout << "Alert: " << "Compare Type Unknown." << std::endl;
            break;
        }
    }
    if (debug) {
        std::cout << ">>>>AFTER COMPARE:" << std::endl;
        std::cout << ">> BRANCH1" << std::endl;
        for (auto &p : branch1) {
            std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
        }
        std::cout << ">> BRANCH2" << std::endl;
        for (auto &p : branch2) {
            std::cout << getInstructionString(*p.first) << " >> " << p.second.getIntervalString() << std::endl;
        }
    }
    auto it = BB->rend();
    bool conti = true;
    while (conti) {
        it--;
        if (it == BB->rbegin()) {
            conti = false;
        }
        switch (it->getOpcode()) {
            case Instruction::Add: {
                analyzeAdd(*it, branch1, instructionMap, true);
                analyzeAdd(*it, branch2, instructionMap, true);
            }
            case Instruction::Sub: {
                analyzeSub(*it, branch1, instructionMap, true);
                analyzeSub(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Mul: {
                analyzeMul(*it, branch1, instructionMap, true);
                analyzeMul(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::SRem: {
                analyzeSrem(*it, branch1, instructionMap, true);
                analyzeSrem(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Alloca: {
                analyzeAlloca(*it, branch1, instructionMap, true);
                analyzeAlloca(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Store: {
                analyzeStore(*it, branch1, instructionMap, true);
                analyzeStore(*it, branch2, instructionMap, true);
                break;
            }
            case Instruction::Load: {
                analyzeLoad(*it, branch1, instructionMap, true);
                analyzeLoad(*it, branch2, instructionMap, true);
                break;
            }
            default: {
                break;
            }
        }

    }
    for (auto it = BB->rend(); it != BB->rbegin(); it--) {

    }
    result[bb_op1] = branch1;
    result[bb_op2] = branch2;
}

void printAnalysisMap(std::map<BasicBlock *, std::map<Instruction *, varInterval>> &analysisMap,
                      std::map<std::string, Instruction *> &instructionMap) {
    std::cout << "==============Analysis Report==============" << std::endl;
    for (auto &m : analysisMap) {
        std::cout << getBasicBlockLabel(m.first) << std::endl;
        for (auto &_m : m.second) {
            std::cout << getInstructionString(*(_m.first)) << "  >>  " << _m.second.getIntervalString() << std::endl;
        }
        for (auto mm = m.second.begin(); mm != m.second.end(); mm++) {
            std::string temp_mm = (*mm).first->getName().str().c_str();
            if (temp_mm.size() == 0) {
                continue;
            }
            for (auto mm1 = mm; mm1 != m.second.end(); mm1++) {
                std::string temp_mm1 = (*mm1).first->getName().str().c_str();
                if (temp_mm1.size() == 0 || temp_mm == temp_mm1) {
                    continue;
                }
                int v1_range[2] = {(*mm).second.getLower(), (*mm).second.getUpper()};
                int v2_range[2] = {(*mm1).second.getLower(), (*mm1).second.getUpper()};
                if (v1_range[0] == varInterval::INF_NEG || v1_range[1] == varInterval::INF_POS) {
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : "
                              << "INF" << std::endl;
                } else if (v2_range[0] == varInterval::INF_NEG || v2_range[1] == varInterval::INF_POS) {
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : "
                              << "INF" << std::endl;
                } else {
                    int a = std::abs(v1_range[0] - v2_range[1]);
                    int b = std::abs(v1_range[1] - v2_range[0]);
                    std::cout << temp_mm << " <--> " << temp_mm1 << " : "
                              << (a > b ? (a >= varInterval::INF_POS ? "INF" : std::to_string(a)) : (b >=
                                                                                                     varInterval::INF_POS
                                                                                                     ? "INF"
                                                                                                     : std::to_string(
                                              b)))
                              << std::endl;
                }

            }
        }
        std::cout << "===========================================" << std::endl;
    }
}