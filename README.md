My Brainf*ck compiler using [Asmjit](https://github.com/asmjit/asmjit).

### Getting started

- Install [xmake](https://xmake.io/).
- Clone the repo
```
$ git clone https://github.com/nomi-san/brainfuck_jit.git
```

- Build
```
$ cd brainfuck_jit
$ git submodule update --init --recursive
$ xmake
```

- Test
```
$ ./brainfuck_jit hello.bf
```

### Benchmark

```
> mandelbrot.bf
```

Implementation | Time
:---|:---
brainfuck (C interpreter) | 9.3s
[bff](https://github.com/apankrat/bff) | 6.5s
[bf2c](https://github.com/pablojorge/brainfuck/blob/master/haskell/bf2c.hs) | 1.5s
brainfuck_jit (our JIT) | 2.0s
