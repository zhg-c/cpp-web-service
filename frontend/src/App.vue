<script setup lang="ts">
import { ref } from 'vue';
import axios from 'axios';

// --- 状态管理 ---
const nInput = ref(20); // 用户输入的 N 值
const result = ref<string | null>(null); // 计算结果
const errorMessage = ref<string | null>(null); // 错误信息
const isLoading = ref(false); // 加载状态

// --- API 地址 ---
// 注意：前端运行在 5173，后端运行在 8000，必须使用完整的 URL
const API_URL = 'http://localhost:8000/api/calculate';

// --- 核心函数：调用 FastAPI (它会调用 C++) ---
const runCalculation = async () => {
  if (isLoading.value) return;
  isLoading.value = true;
  result.value = null;
  errorMessage.value = null;

  try {
    // 发送 GET 请求给 FastAPI，并附带参数 n
    const response = await axios.get(API_URL, {
      params: { n: nInput.value }
    });
    
    // 从 API 响应中提取 C++ 计算的结果
    result.value = response.data.result;
    
  } catch (error) {
    console.error('API Error:', error);
    // @ts-ignore
    errorMessage.value = `计算失败 (${error.message})。请确保后端服务在 http://localhost:8000 运行。`;
  } finally {
    isLoading.value = false;
  }
};
</script>

<template>
  <div style="max-width: 600px; margin: 50px auto; padding: 20px; border: 1px solid #ccc; border-radius: 8px; font-family: sans-serif;">
    <h2 style="color: #2c3e50;">C++ 后台高性能计算演示</h2>
    <p style="color: #666;">通过 Web 界面调用您的 C++ 核心算法（计算斐波那契数列）。</p>

    <div style="margin: 20px 0; padding: 15px; border: 1px dashed #42b983; border-radius: 6px;">
      <label for="n-input" style="font-weight: bold; color: #35495e;">输入 N (计算斐波那契数列第 N 位):</label>
      <input 
        id="n-input" 
        type="number" 
        v-model.number="nInput" 
        min="1" 
        max="93" 
        style="padding: 8px; margin-left: 10px; border: 1px solid #ccc; border-radius: 4px; width: 100px;"
      />
    </div>

    <button 
      @click="runCalculation" 
      :disabled="isLoading"
      style="padding: 10px 20px; background-color: #42b983; color: white; border: none; border-radius: 4px; cursor: pointer; transition: background-color 0.3s;"
    >
      {{ isLoading ? '正在调用 C++ 核心...' : '🚀 运行 C++ 计算' }}
    </button>

    <div v-if="result !== null" style="margin-top: 20px; padding: 15px; background-color: #e6ffe6; border: 1px solid #42b983; border-radius: 4px; font-size: 1.1em; word-wrap: break-word;">
      <strong>✅ C++ 计算结果:</strong> <span style="font-family: monospace;">{{ result }}</span>
    </div>

    <div v-if="errorMessage" style="margin-top: 20px; padding: 15px; background-color: #ffe6e6; border: 1px solid red; border-radius: 4px; color: red;">
      <strong>❌ 错误信息:</strong> {{ errorMessage }}
    </div>
  </div>
</template>

<style>
/* 可以在这里添加一些基本样式，或者使用你的 UnoCSS/Tailwind */
</style>