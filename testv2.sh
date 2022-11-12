# use pass
cd Test

file_name=test
command="-csub"

# Test
echo "\n\033[32m> Test Case <\033[0m"

# opt
clang -S -emit-llvm $file_name.cpp -o IR/$file_name.ll
opt -load ../Build/LLVMObfuscator.so $command -S IR/$file_name.ll -o IR/${file_name}_cmd.ll

# generate
clang IR/$file_name.ll -o Bin/$file_name
# run
Bin/$file_name