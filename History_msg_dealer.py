import os
import sys
from dashscope import Generation
import dashscope
dashscope.api_key ="sk-3cc6c772b820438a9ac2096449ddd0a0"
dashscope.base_http_api_url = 'https://dashscope.aliyuncs.com/api/v1'

#传入的信息
if len(sys.argv) < 2:
    print("错误：未提供临时文件路径", end="")
    sys.exit(1)
temp_file_path = sys.argv[1]

# 读取temped.txt中的内容
try:
    with open(temp_file_path, 'r', encoding='gbk') as f:
        history_msg = f.read()  # 从文件读取内容
except Exception as e:
    print(f"读取文件失败：{str(e)}", end="")
    sys.exit(1)

messages = [
    {"role": "system", "content": "You are a helpful assistant."},
    {"role": "user", "content":history_msg},
]
response = Generation.call(
    # 若没有配置环境变量，请用百炼API Key将下行替换为：api_key = "sk-xxx",
    api_key=os.getenv("DASHSCOPE_API_KEY"),
    model="qwen-turbo",
    messages=messages,
    result_format="message",
    # 深度思考
    enable_thinking=False,
)

if response.status_code == 200:
    # # 打印思考过程
    # print("=" * 20 + "思考过程" + "=" * 20)
    # print(response.output.choices[0].message.reasoning_content)
    print(response.output.choices[0].message.content)
else:
    print(f"HTTP返回码：{response.status_code}")
    print(f"错误码：{response.code}")
    print(f"错误信息：{response.message}")
