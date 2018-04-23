//Author: Wang Jiadong
//Matric Number: A0105703
//Email: E0267418@u.nus.edu

#include <cstdio>
#include <iostream>
#include <set>
#include <map>
#include <stack>

#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/Instruction.h"
#include "llvm/IRReader/IRReader.h"
#include "llvm/Support/SourceMgr.h"
#include "llvm/Support/raw_ostream.h"


#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

using namespace llvm;

// for each function
void generateCFG(BasicBlock*,std::map<BasicBlock*,std::set<Instruction*>>&);

// for each block
std::pair<std::set<Instruction*>, std::set<Instruction*>> checkLeakage(BasicBlock*,std::set<Instruction*>);

int main(int argc, char **argv)
{
    // Read the IR file.
    //nothing much 
    LLVMContext &Context = getGlobalContext();
    SMDiagnostic Err;
    Module *M = ParseIRFile(argv[1], Err, Context);
    if (M == nullptr)
    {
      fprintf(stderr, "error: failed to load LLVM IR file \"%s\"", argv[1]);
      return EXIT_FAILURE;
    }
    //importing completed
    //if importing failed, program finishes here.

    //FIND THE MAIN FUNCTION
    //WHICH IS THE GLOBAL ENTRY
    // 1.Extract Function main from Module M
    Function *F = M->getFunction("main");

    //secret Vars
    //a set of secrets, that contains the secret, or contaminated by the secret.
    //it is set up as global.
    std::set<Instruction*> secretVars;
    std::map<BasicBlock*,std::set<Instruction*>> analysisMap;
    //FOR EACH BASIC BLOCK INSIDE THE FUNCTION
    for (auto &BB: *F){
      std::set<Instruction*> emptySet;
      //INITIALIZE THE GLOBAL MAP
      BasicBlock *blockPointer = &BB;
      analysisMap[blockPointer] = emptySet;
      
      // std::out << BB.str().c_str() << std::endl;
      // BB.printAsOperand(err(), false);
    } 



    BasicBlock* BB = dyn_cast<BasicBlock>(F->begin()); 
    //Recursively search for secret vars.
    generateCFG(BB, analysisMap);


    std::cout << "With Registers: \n";
    int flag = false;
    for (auto& row : analysisMap){
      std::set<Instruction*> initializedVars = row.second;
      //std::string BBLabel = row.first;
      //errs() << BBLabel << ":\n";
      row.first->printAsOperand(errs(), false);
      for (Instruction* var : initializedVars){
        std::cout << "\t";
        var->dump();

        const TerminatorInst *TInst = row.first->getTerminator();
        int NSucc = TInst->getNumSuccessors();

        if (NSucc == 0) {
          if (strncmp(var->getName().str().c_str(),"sink",4) == 0) {
            flag = true;
          }
        }
      }
      std::cout << "\n";
    } 


    std::cout << "Without Registers: \n";
    for (auto& row : analysisMap){
        std::set<Instruction*> taints = row.second;
        BasicBlock *BB = row.first;
        BB->printAsOperand(errs(), false);
        std::cout << ":\t {";
        for (Instruction* taintInstruction : taints){
           std::string temp = taintInstruction -> getName().str().c_str();
           if(temp.size() == 0){
             continue;
           }
           if(taintInstruction != *taints.begin()){
            std::cout << ",";
           }
           std::cout << " ";

           std::cout << temp;

           // var->dump();
        }
        std::cout << "}\n";
    }

    return 0;
}

// Block-wise
void generateCFG(BasicBlock* BB, std::map<BasicBlock*,std::set<Instruction*>> & analysisMap)
{
  // printf("Label Name:%s\n", BB->getName().str().c_str());
  // Get the exit
  //Check the leakage inside the Basic Block
  //If found any, put inside the secret vars.
  std::pair<std::set<Instruction*>, std::set<Instruction*>> vars = checkLeakage(BB,analysisMap[BB]);
  std::set<Instruction*> newSecretVars = vars.first;
  std::set<Instruction*> killSecretVars = vars.second;
  analysisMap[BB].insert(newSecretVars.begin(), newSecretVars.end());
  analysisMap[BB].erase(killSecretVars.begin(), killSecretVars.end());
  // analysisMap[BB].insert(newSecretVars.begin(), newSecretVars.end());
  // Pass secretVars list to child BBs and check them
  // Iteratively


  //Get the last instruction, inside the block, and then get the entry of the successor's entry.
  const TerminatorInst *TInst = BB->getTerminator();
  int NSucc = TInst->getNumSuccessors();


  //for each of the successors
  //check again.
  for (int i = 0;  i < NSucc; ++i) {
    //the successor is a basic block.
    BasicBlock *Succ = TInst->getSuccessor(i); 
    analysisMap[Succ].insert(newSecretVars.begin(), newSecretVars.end());
    analysisMap[Succ].erase(killSecretVars.begin(), killSecretVars.end());
    //do the check.  
    generateCFG(Succ, analysisMap);
  }

  // // Last Basic Block, check if secret leaks to public
  // if (NSucc == 0){
  //   int flag = false;
  //   for (auto &S: newSecretVars)
  //     if (strncmp(S->getName().str().c_str(),"public",6) == 0) {
	 //      flag = true;
  //     }
  //   if (flag == true)
	 //    printf(ANSI_COLOR_RED "OMG, Secret leaks to the Public" ANSI_COLOR_RESET	"\n");
  //   else
	 //    printf(ANSI_COLOR_GREEN "Secret does not leak to the Public" ANSI_COLOR_RESET	"\n");
  // } 
}


// Block-wise analysis.
//For each Code Block, We check the output leakage
// As parameter, we pass in the sources.

//So basically, this function just get a list of variables that
//are contaminated by the secrets.
std::pair<std::set<Instruction*>, std::set<Instruction*>> checkLeakage(BasicBlock* BB, std::set<Instruction*> secretVars)
{
  //copy the secret vars array
  //but I dont understand, why have to copy?
  std::set<Instruction*> newSecretVars(secretVars);
  std::set<Instruction*> killSecretVars;
  // Loop through instructions in BB
  //I instructions, pass by reference
  for (auto &I: *BB)
  {

    //INITIALIZATION
    // Add secret variable to newSecretVars
    // when secret is initialized  
    if (strncmp(I.getName().str().c_str(),"source",6) == 0){
      // Print 
      //initialization
      // printf(ANSI_COLOR_BLUE "Secret found:%s" ANSI_COLOR_RESET "\n", I.getName().str().c_str());
      newSecretVars.insert(dyn_cast<Instruction>(&I));
    }



    // if I is store instruction
    if (isa<StoreInst>(I)){
      // Check store instructions
      // e.g. store i32 0, i32* %retval
      //operand 0: the value that is going to be assigned
      //operand 1: the variable
      Value* v = I.getOperand(0);
      Instruction* op1 = dyn_cast<Instruction>(v); 

      v = I.getOperand(1);
      Instruction* op2 = dyn_cast<Instruction>(v); 
      // If the FV is part of the secrets, insert the assigned variable
      //if the value expression is not NULL
      //if we can find it inside the secrets
      //then, put the variable into the secret set.
      if (op1 != nullptr && newSecretVars.find(op1) != newSecretVars.end()){
	       newSecretVars.insert(op2);	
      }else if(op1 != nullptr && newSecretVars.find(op1) == newSecretVars.end() && newSecretVars.find(op2) != newSecretVars.end()){
         newSecretVars.erase(op2);
         killSecretVars.insert(op2);
      }else{
        ;
      }

    }else{	
      // Check all other instructions
      bool flag = false;
      for (auto op = I.op_begin(); op != I.op_end(); op++) {
	      

        //parse the oprand as an instrcution type.
        Value* v = op->get();
	      Instruction* inst = dyn_cast<Instruction>(v);
        

        // for each assignment, if the FV contains secret, then the assigned also.
        if (inst != nullptr && newSecretVars.find(inst) != newSecretVars.end()){
	        newSecretVars.insert(dyn_cast<Instruction>(&I));
          flag = true;
        }
      }
      if(!flag){
        killSecretVars.insert(dyn_cast<Instruction>(&I));
        newSecretVars.erase(dyn_cast<Instruction>(&I));
      }
    }
  }
  return std::make_pair(newSecretVars,killSecretVars);
}
