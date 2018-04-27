# CS5218_assignment3
May the force be with the code!
### export llvm file 
export llvm_bin_path="xxxx";\
${llvm_bin_path}clang -emit-llvm -S -o example2.ll example2.c\
### build cpp file
export llvm_bin_path="xxxx";\
${llvm_bin_path}clang++ -o IntervalAnalysis IntervalAnalysis.cpp \\\
\`${llvm_bin_path}llvm-config --cxxflags\` \\\
\`${llvm_bin_path}llvm-config --ldflags\` \\\
\`${llvm_bin_path}llvm-config --libs\` \\\
-lpthread -lncurses -ldl
### run
./IntervalAnalysis example2.ll
