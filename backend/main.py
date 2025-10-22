# backend/main.py
from fastapi import FastAPI,File,UploadFile
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

# 4. C++ 驱动的图片处理 API 路由
@app.post("/api/optimize")
async def optimize_image(image_file: UploadFile = File(...)):
    """
    接收图片文件，通过 Python 调用 C++ 核心函数生成优化/占位符图片。
    """
    try:
        # 1. 从上传的文件中异步读取原始字节数据
        original_bytes = await image_file.read()
        
        # 2. ✨ 核心步骤：调用 C++ 函数
        # 注意：libcore.generate_placeholder 期望 bytes 或 list[int]
        # Pybind11 自动将 Python bytes 转换为 std::vector<unsigned char>
        processed_data = libcore.generate_placeholder(original_bytes)
        
        # 3. 将 C++ 返回的字节数据解码为字符串进行验证（正式环境应返回图片）
        # 只有在测试阶段我们才解码为字符串
        response_message = processed_data.decode('utf-8')

        return {
            "filename": image_file.filename,
            "original_size": len(original_bytes),
            "status": "success",
            "c_plus_plus_log": response_message # 返回 C++ 核心的反馈信息
        }

    except ImportError as e:
        return {"error": f"C++ Core (libcore.so) import failed: {e}"}
    except Exception as e:
        # 如果 C++ 内部出错或文件读取出错
        return {"error": f"Processing failed: {e}"}