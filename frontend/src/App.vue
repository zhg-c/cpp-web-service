<script setup lang="ts">
import { ref } from 'vue';
import axios from 'axios';

// --- 状态管理 ---
const imageFile = ref<File | null>(null); 
const quality = ref(80); // 默认质量80
const optimizedImageSrc = ref<string | null>(null); // 优化后的图片URL
const originalSize = ref<number | null>(null); 
const optimizedSize = ref<number | null>(null); 
const compressionRatio = ref<string | null>(null); 
const errorMessage = ref<string | null>(null); 
const isLoading = ref(false); 

// --- API 地址 ---
// !!! 请将这里的 IP 地址替换为你的 Kali 虚拟机 IP !!!
const API_URL = 'http://localhost:8000/api/optimize'; 

// --- 文件选择处理器 ---
const handleFileUpload = (event: Event) => {
  const target = event.target as HTMLInputElement;
  if (target.files && target.files.length > 0) {
    imageFile.value = target.files[0]!; // 使用非空断言
    optimizedImageSrc.value = null;
    errorMessage.value = null;
    originalSize.value = imageFile.value.size;
    optimizedSize.value = null;
    compressionRatio.value = null;
  }
};

// --- 核心函数：调用 FastAPI (文件上传) ---
const runOptimization = async () => {
  if (isLoading.value || !imageFile.value) return;
  
  isLoading.value = true;
  optimizedImageSrc.value = null;
  errorMessage.value = null;

  const formData = new FormData();
  // 附加文件
  formData.append('image_file', imageFile.value);
  // 附加质量参数 (非文件字段)
  formData.append('quality', String(quality.value)); 

  try {
    // 发送 POST 请求，期待一个 Blob (图片数据) 返回
    const response = await axios.post(API_URL, formData, {
      responseType: 'blob', // 关键：告诉 axios 期待二进制数据
      headers: {
        'Content-Type': 'multipart/form-data',
      },
    });
    
    // 1. 从 HTTP 头中读取返回的统计数据
    originalSize.value = Number(response.headers['x-original-size']);
    optimizedSize.value = Number(response.headers['x-optimized-size']);
    compressionRatio.value = response.headers['x-compression-ratio'] as string;

    // 2. 将返回的二进制 Blob 数据转换为浏览器可识别的 URL
    const blob = new Blob([response.data], { type: 'image/jpeg' });
    optimizedImageSrc.value = URL.createObjectURL(blob);
    
  } catch (error) {
    console.error('API Error:', error);
    // ✨ 关键修复：使用类型保护
    if (axios.isAxiosError(error)) { // 检查 error 是否为 AxiosError
        // 现在 TypeScript 知道 error 具有 response 和 message 属性
        errorMessage.value = `优化失败。错误: ${error.message}`;

        if (error.response && error.response.data) {
            // 解析后端返回的 JSON 错误详情
            const errorBlob = new Blob([error.response.data], { type: 'application/json' });
            const reader = new FileReader();
            
            reader.onload = function() {
                try {
                    const errorData = JSON.parse(reader.result as string);
                    // 假设后端 HTTPException 返回 { "detail": "..." }
                    errorMessage.value += ` 错误详情: ${errorData.detail}`;
                } catch {
                    errorMessage.value += ` (无法解析错误详情)`;
                }
            };
            reader.readAsText(errorBlob);
        }
    } else {
        // 处理非 Axios 导致的未知错误
        errorMessage.value = `优化失败。发生未知错误: ${String(error)}`;
    }
  } finally {
    isLoading.value = false;
  }
};

// 格式化文件大小
const formatBytes = (bytes: number | null) => {
    if (bytes === null) return 'N/A';
    if (bytes === 0) return '0 Bytes';
    const k = 1024;
    const sizes = ['Bytes', 'KB', 'MB', 'GB'];
    const i = Math.floor(Math.log(bytes) / Math.log(k));
    return parseFloat((bytes / Math.pow(k, i)).toFixed(2)) + ' ' + sizes[i];
};
</script>

<template>
  <div style="max-width: 800px; margin: 50px auto; padding: 30px; border: 1px solid #ccc; border-radius: 10px; font-family: sans-serif; box-shadow: 0 4px 12px rgba(0,0,0,0.1);">
    <h2 style="color: #007bff; border-bottom: 2px solid #007bff; padding-bottom: 10px;">
      🚀 AccelCompress: C++ 极速 JPEG 优化器
    </h2>
    <p style="color: #666; margin-bottom: 25px;">
      利用 libjpeg-turbo C++ 核心实现高性能图片质量压缩。
    </p>

    <div style="margin: 20px 0; padding: 20px; border: 1px dashed #28a745; border-radius: 8px; background-color: #f8f9fa;">
      <label for="file-input" style="font-weight: bold; color: #35495e; display: block; margin-bottom: 10px;">
        选择 JPEG 图片文件
      </label>
      <input 
        id="file-input" 
        type="file" 
        @change="handleFileUpload"
        accept="image/jpeg"
        style="padding: 10px; border: 1px solid #ccc; border-radius: 4px; display: block; width: 100%; box-sizing: border-box;"
      />
      <p v-if="imageFile" style="margin-top: 10px; font-size: 0.9em; color: #28a745;">
        已选择: <strong>{{ imageFile.name }}</strong> | 原始大小: {{ formatBytes(imageFile.size) }}
      </p>

      <div style="margin-top: 20px;">
        <label style="font-weight: bold; display: block; margin-bottom: 5px;">
          目标质量 ({{ quality }})
        </label>
        <input 
          type="range" 
          v-model="quality" 
          min="1" 
          max="100" 
          step="1"
          style="width: 100%;"
        />
      </div>
    </div>

    <button 
      @click="runOptimization" 
      :disabled="isLoading || !imageFile"
      style="padding: 12px 25px; background-color: #007bff; color: white; border: none; border-radius: 6px; cursor: pointer; transition: background-color 0.3s; font-size: 1.1em;"
    >
      {{ isLoading ? '正在 C++ 核心中处理...' : '✨ 运行 C++ 优化' }}
    </button>

    <div v-if="optimizedImageSrc" style="margin-top: 30px; padding: 20px; background-color: #e9f7ef; border: 1px solid #28a745; border-radius: 8px;">
      <h3 style="color: #28a745;">✅ 优化成功！</h3>
      
      <p>原始大小: <strong>{{ formatBytes(originalSize) }}</strong></p>
      <p>优化后大小: <strong>{{ formatBytes(optimizedSize) }}</strong></p>
      <p>压缩率: <strong style="color: #dc3545;">{{ compressionRatio }}</strong></p>

      <h4 style="margin-top: 15px;">优化结果预览：</h4>
      <img :src="optimizedImageSrc" alt="Optimized Image Preview" style="max-width: 100%; height: auto; border: 1px solid #ccc; margin-top: 10px;"/>
    </div>

    <div v-if="errorMessage" style="margin-top: 30px; padding: 20px; background-color: #f8d7da; border: 1px solid #dc3545; border-radius: 8px; color: #dc3545;">
      <strong>❌ 错误信息:</strong> {{ errorMessage }}
    </div>
  </div>
</template>

<style>
/* 基础样式 (可选) */
body { margin: 0; background-color: #f4f4f4; }
</style>