## 使用说明

1. **首次设置（联网环境）**:
   ```bash
   ./scripts/system_insight_offline_prepare.sh
   ```
   这个脚本会自动下载并打包所有依赖（包括 protobuf 和 grpc 的子模块）。

2. **离线构建**:
   确保所有归档文件都在 `third_party/offline/archives/` 目录中，然后按照项目 README 进行 Docker 构建。

## 文件完整性

下载后，建议验证文件完整性：

```bash
# 检查文件大小
ls -lh third_party/offline/archives/

# 验证 tar.gz 文件
for f in third_party/offline/archives/*.tar.gz; do
  echo "Checking $f"
  tar -tzf "$f" > /dev/null && echo "✓ OK" || echo "✗ Corrupted"
done

# 验证 zip 文件
for f in third_party/offline/archives/*.zip; do
  echo "Checking $f"
  unzip -t "$f" > /dev/null && echo "✓ OK" || echo "✗ Corrupted"
done
```

## 注意事项

- ⚠️ `grpc-v1.62.0-with-deps.tar.gz` 文件非常大（~1.3 GB），下载和打包需要较长时间
- ⚠️ `protobuf-v25.3-with-deps.tar.gz` 也较大（~52 MB），但小于 GitHub 的 100MB 限制
- 这些文件仅用于离线构建，如果网络环境允许，CMake 的 FetchContent 可以自动下载较小的依赖
- 确保下载的文件名与上述列表完全一致，否则安装脚本可能无法识别