# make pass
if [ ! -d "Build" ]; then
    mkdir Build
fi
cd ./Build
cmake ../Transforms
make

# use pass
cd ../Test

# compile without obfuscator
clang -S -emit-llvm TestProgram.cpp -o IR/TestProgram_orig.ll
clang IR/TestProgram_orig.ll -o Bin/TestProgram_orig
echo "\n\033[32m> Test case on Original Binary <\033[0m"
./Bin/TestProgram_orig flag{s1mpl3_11vm_d3m0}


# demo pass
echo "\n\033[32m> Test case on Demo Pass <\033[0m"
# opt
opt -load ../Build/LLVMObfuscator.so -hlw -S IR/TestProgram.ll -o IR/TestProgram_hlw.ll
# generate
clang IR/TestProgram_hlw.ll -o Bin/TestProgram_hlw
# run
Bin/TestProgram_hlw flag{s1mpl3_11vm_d3m0}


# split basic block pass 基本块分割
echo "\n\033[32m> Test case on Split Basic Block Pass <\033[0m"
# opt
opt -load ../Build/LLVMObfuscator.so -split -split_num 3 -S IR/TestProgram_orig.ll -o IR/TestProgram_split.ll
# generate
clang IR/TestProgram_split.ll -o Bin/TestProgram_split
# run
Bin/TestProgram_split flag{s1mpl3_11vm_d3m0}

# flattening pass 控制流平坦化
echo "\n\033[32m> Test case on Flattening Pass <\033[0m"
# opt
opt -lowerswitch -S IR/TestProgram_orig.ll -o IR/TestProgram_lowerswitch.ll
opt -load ../Build/LLVMObfuscator.so -fla -split_num 3 -S IR/TestProgram_lowerswitch.ll -o IR/TestProgram_fla.ll
# generate
clang IR/TestProgram_fla.ll -o Bin/TestProgram_fla
# run
Bin/TestProgram_fla flag{s1mpl3_11vm_d3m0}

# BogusControlFlow pass 虚假控制流
echo "\n\033[32m> Test case on BogusControlFlow Pass <\033[0m"
# opt
opt -load ../Build/LLVMObfuscator.so -fla -split_num 3 -S IR/TestProgram.ll -o IR/TestProgram_bcf.ll
# opt -load ../Build/LLVMObfuscator.so -fla -split_num 3 -bcf_loop 2 -S IR/TestProgram.ll -o IR/TestProgram_bcf.ll
# generate
clang IR/TestProgram_bcf.ll -o Bin/TestProgram_bcf
# run
Bin/TestProgram_bcf flag{s1mpl3_11vm_d3m0}

# Substitution pass 指令替代
echo "\n\033[32m> Test case on Substitution Pass <\033[0m"
# opt
opt -load ../Build/LLVMObfuscator.so -sub -sub_loop 1 -S IR/TestProgram.ll -o IR/TestProgram_sub.ll
# generate
clang IR/TestProgram_sub.ll -o Bin/TestProgram_sub
# run
Bin/TestProgram_sub flag{s1mpl3_11vm_d3m0}

# RandomControlFlow pass 随机控制流
echo "\n\033[32m> Test case on RandomControlFlow Pass <\033[0m"
# opt
# opt -load ../Build/LLVMObfuscator.so -rcf -rcf_loop 2 -S IR/TestProgram.ll -o IR/TestProgram_rcf.ll
opt -load ../Build/LLVMObfuscator.so -rcf -S IR/TestProgram.ll -o IR/TestProgram_rcf.ll
# generate
llc -filetype=obj -mattr=+rdrnd IR/TestProgram_rcf.ll -o Bin/TestProgram_rcf.o
clang Bin/TestProgram_rcf.o -o Bin/TestProgram_rcf
# run
Bin/TestProgram_rcf flag{s1mpl3_11vm_d3m0}

# ConstantSubstitution pass 常量替代
echo "\n\033[32m> Test case on ConstantSubstitution Pass <\033[0m"
# opt
# opt -load ../Build/LLVMObfuscator.so -csub -csub_loop 2 -S IR/TestProgram.ll -o IR/TestProgram_csub.ll
opt -load ../Build/LLVMObfuscator.so -csub -S IR/TestProgram.ll -o IR/TestProgram_csub.ll
# generate
clang IR/TestProgram_csub.ll -o Bin/TestProgram_csub
# run
./Bin/TestProgram_csub flag{s1mpl3_11vm_d3m0}
