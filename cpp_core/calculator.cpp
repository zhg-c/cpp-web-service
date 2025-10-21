#include <iostream>

// 模拟一个计算密集型的函数：计算斐波那契数列
// 实际中这里将是您的核心算法
long long fibonacci(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;

    long long a = 0;
    long long b = 1;
    long long result = 0;

    // 循环计算，模拟耗时操作
    for (int i = 2; i <= n; i++) {
        result = a + b;
        a = b;
        b = result;
    }
    return result;
}

// -------------------------------------------------------------------
// Pybind11 模块定义：将 C++ 函数暴露给 Python
// -------------------------------------------------------------------
#include <pybind11/pybind11.h>
namespace py = pybind11;

// 模块名称必须和最终生成的 .so 文件名匹配 (libcore)
PYBIND11_MODULE(libcore, m) {
    m.doc() = "Pybind11 C++ core for high-performance calculations"; // 模块文档
    
    // 暴露 fibonacci 函数给 Python
    m.def("fibonacci", &fibonacci, "A function that calculates the Nth Fibonacci number");
}