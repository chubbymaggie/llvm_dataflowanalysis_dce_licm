clang -O0 -emit-llvm -c $1.c -o $1.bc
opt -mem2reg $1.bc -o $1-simp.bc
#opt -load ./dce-pass.so -dce-pass $1-simp.bc -o $1-opt.bc
opt -load ./licm-pass.so -licm-pass $1-simp.bc -o $1-opt.bc

#opt -load ./dce-pass.so -dce-pass $1-simp.bc -o $1-opt1.bc
#opt -load ./licm-pass.so -licm-pass $1-opt1.bc -o $1-opt.bc
llvm-dis $1-opt.bc
clang $1-simp.bc -O0 -o $1-simp
./$1-simp
clang $1-opt.bc -O0 -o $1-opt
./$1-opt

llc $1-simp.bc -o $1-simp.s
gcc $1-simp.s -O0 -o $1-simp.native
./$1-simp.native

llc $1-opt.bc -o $1-opt.s
gcc $1-opt.s -O0 -o $1-opt.native
./$1-opt.native

