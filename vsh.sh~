clang -O0 -emit-llvm -c $1.c -o $1.bc
opt -mem2reg $1.bc -o $1-simp.bc
#opt -load ./dce-pass.so -dce-pass $1-simp.bc > /dev/null
#opt -load ./dce-pass.so -dce-pass $1-simp.bc -o $1-opt.bc
opt -load ./licm-pass.so -licm-pass $1-simp.bc -o $1-opt.bc
