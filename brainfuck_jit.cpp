//#define ASMJIT_STATIC

#ifdef _MSC_VER
#pragma warning(disable: 4996)
#endif

#include <stdio.h>
#include <time.h>
#include <vector>
#include "asmjit/x86.h"

using namespace asmjit;

#define MAX_STACK   1024
#define MAX_LOOP    64

class Brainfuck
{
private:
    const char *name;
    const char *prog;

    // JIT runtime
    JitRuntime rt;
    CodeHolder code;
    // Product
    void (*func)() = nullptr;

    // Loop
    struct Loop {
        Label start;
        Label end;
        Loop(x86::Compiler &cc)
            : start(cc.newLabel()),
            end(cc.newLabel()) {}
    };

public:
    Brainfuck(const char *name, const char *prog)
        : name(name), prog(prog) {}

    ~Brainfuck() {
        if (func != nullptr) rt.release(func);
    }

    void execute() {
        if (func != nullptr) {
            func();
            fflush(stdout);
        }
    }

    bool compile() {
        // Init env, compiler
        code.init(rt.environment());
        x86::Compiler cc(&code);

        // Add function signature -> void (*func)()
        cc.addFunc(FuncSignatureT<void>());

        // Invokable
        InvokeNode *invk;
        // Loop stack
        std::vector<Loop> loops;

        // Main stack                               // char stack[MAX_STACK]
        x86::Mem stack = cc.newStack(MAX_STACK, sizeof(void *));
        // Stack pointer
        x86::Gp sp = cc.newGpz();                   // size_t sp
        // Temp (u8)
        x86::Gp tmp = cc.newGpb();                  // char tmp

        // Stack index (&idx := stack[sp])
        x86::Mem idx(stack);
        idx.setIndex(sp);
        idx.setSize(sizeof(char));

        // Clear the stack (memset zero)
        cc.mov(sp, 0);                              // sp = 0
        Label loop = cc.newLabel();                 // loop:
        cc.bind(loop);                      

        cc.mov(idx, 0);                             // stack[i] = 0
        cc.inc(sp);                                 // sp++
        cc.cmp(sp, MAX_STACK);                      // if sp < MAX_STACK then
        cc.jb(loop);                                //     goto loop

        // Clear sp by xor
        cc.xor_(sp, sp);                            // sp = 0

        while (*prog) {
            switch (*prog++) {
                case '>':
                    cc.inc(sp);                     // sp++
                    break;
                case '<':
                    cc.dec(sp);                     // sp--
                    break;
                case '+':
                    cc.inc(idx);                    // stack[sp]++
                    break;
                case '-':
                    cc.dec(idx);                    // stack[sp]--
                    break;
                case '.':
                    cc.mov(tmp, idx);               // tmp = stack[sp]
                    cc.invoke(&invk, imm(&putchar),
                        FuncSignatureT<void, char>());
                    invk->setArg(0, tmp);           // putchar(tmp)
                    break;
                case ',':
                    cc.invoke(&invk, imm(&getchar),
                        FuncSignatureT<char>());
                    invk->setRet(0, tmp);           // tmp = getchar()
                    cc.mov(idx, tmp);               // stack[sp] = tmp
                    break;
                case '[':
                    // Check loop depth
                    if (loops.size() >= MAX_LOOP) {
                        printf("[%s] compile error: Too many loops!\n", name);
                        return false;
                    }

                    loops.push_back(Loop(cc));

                    cc.bind(loops.back().start);    // start:
                    cc.cmp(idx, 0);                 // if stack[sp] == 0 then
                    cc.je(loops.back().end);        //      goto end

                    break;
                case ']':
                    // Check unmatched loop
                    if (loops.size() <= 0) {
                        printf("[%s] compile error: Unmatched loop!\n", name);
                        return false;
                    }

                    cc.jmp(loops.back().start);     // goto start
                    cc.bind(loops.back().end);      // end:

                    loops.pop_back();
                    break;
            }
        }

        cc.endFunc();
        cc.finalize();

        // Generate code
        if (rt.add(&func, &code) != kErrorOk) {
            printf("[%s] compile error: Couldn't generate code!\n", name);
            return false;
        }

        return true;
    }
};

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Please enter file name!\n");
        return 0;
    }

    FILE *fp = fopen(argv[argc - 1], "rb");
    if (fp == nullptr) {
        printf("Couldn't open file `%s`!\n", argv[argc - 1]);
        return 0;
    }

    fseek(fp, 0, SEEK_END);
    size_t fileSize = ftell(fp);
    rewind(fp);

    char *code = new char[fileSize + 1];
    size_t bytesRead = fread(code, sizeof(char), fileSize, fp);
    code[bytesRead] = '\0';

    Brainfuck bf_main("main", code);
    if (bf_main.compile()) {
        bf_main.execute();
    }

    delete[] code;
    return 0;
}