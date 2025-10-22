# backend/main.py
from fastapi import FastAPI
from fastapi.middleware.cors import CORSMiddleware
import os
import sys

# 1. 确保 Python 能找到 libcore.so
sys.path.append(os.path.dirname(os.path.abspath(__file__))) 
try:
    # 2. 导入 C++ 模块
    import libcore 
except ImportError as e:
    print(f"Error importing libcore.so: {e}")
    sys.exit(1)

app = FastAPI(title="C++ High-Performance Backend API")

# 3. 允许跨域请求 (重要: 否则前端无法调用)
origins = [
    "http://localhost:5173", # Vue/Vite 开发服务器的默认地址
    "http://127.0.0.1:5173",
]
app.add_middleware(
    CORSMiddleware,
    allow_origins=origins,
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

# 4. 根路由测试
@app.get("/")
def read_root():
    return {"message": "C++ Backend API is running."}

# 5. C++ 计算 API 路由
@app.get("/api/calculate")
def calculate_fibonacci(n: int = 10):
    """
    通过 Python 调用 C++ 核心函数计算斐波那契数列第 n 位。
    """
    if n > 93:
        # 避免 C++ long long 溢出
        return {"error": "Input N is too large for long long calculation."}
        
    # ✨ 核心步骤：调用 C++ 函数
    result = libcore.fibonacci(n)
    
    return {
        "n": n,
        "result": str(result), # 将大数字转为字符串避免 JSON 精度问题
        "source": "C++ Core"
    }