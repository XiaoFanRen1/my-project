#!/bin/bash

# 获取脚本所在目录（用于在仓库内定位目标目录）
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

# 设定工具链 URL
TOOLCHAIN_URL="https://pkgman.jieliapp.com/s/linux-toolchain"
# 下载工具链到当前目录
echo "开始下载工具链文件..."
curl -o toolchain.tar.xz -L "$TOOLCHAIN_URL"
if [ $? -ne 0 ]; then
    echo "下载失败，请检查 URL 或网络连接。"
    exit 1
fi

# 确保目标目录存在
TARGET_DIR="/opt/jieli"
sudo mkdir -p "$TARGET_DIR"

# 根据文件后缀解压
FILE="toolchain.tar.xz"
case "$FILE" in
    *.tar.gz)
        echo "解压 tar.gz 文件..."
        sudo tar -xzf "$FILE" -C "$TARGET_DIR"
        ;;
    *.tgz)
        echo "解压 tgz 文件..."
        sudo tar -xzf "$FILE" -C "$TARGET_DIR"
        ;;
    *.tar.bz2)
        echo "解压 tar.bz2 文件..."
        sudo tar -xjf "$FILE" -C "$TARGET_DIR"
        ;;
    *.tar.xz)
        echo "解压 tar.xz 文件..."
        sudo tar -xJf "$FILE" -C "$TARGET_DIR"  # 使用 -J 选项解压 tar.xz 文件 [^1^] [^4^]
        ;;
    *.zip)
        echo "解压 zip 文件..."
        sudo unzip "$FILE" -d "$TARGET_DIR"
        ;;
    *)
        echo "不支持的文件格式：$FILE"
        exit 1
        ;;
esac

# 检查解压是否成功
if [ $? -eq 0 ]; then
    echo "工具链已成功解压到 $TARGET_DIR"
else
    echo "解压失败，请检查文件格式或完整性。"
    exit 1
fi

# 检查关键文件是否存在
if [ -f "$TARGET_DIR/common/bin/clang" ]; then
    echo "clang 位置确认：$TARGET_DIR/common/bin/clang"
else
    echo "警告：未找到 clang 文件，请检查工具链完整性。"
fi

# 可选：添加到环境变量
echo "是否将工具链路径添加到环境变量？(y/n)"
read -r response
if [ "$response" = "y" ]; then
    echo "将路径添加到 ~/.bashrc"
    echo "export PATH=$TARGET_DIR/common/bin:\$PATH" | sudo tee -a ~/.bashrc
    source ~/.bashrc
    echo "工具链已添加到环境变量。"
fi

# 下载并解压烧写工具到 cpu/wl82/tools 目录（始终取最新）
POSTBUILD_URL="https://pkgman.jieliapp.com/s/linux-postbuild"
POSTBUILD_FILE="postbuild.tar.xz"
POSTBUILD_DIR="$SCRIPT_DIR/cpu/wl82/tools"

echo "开始下载烧写工具文件（最新版本）..."
curl -o "$POSTBUILD_FILE" -L "$POSTBUILD_URL"
if [ $? -ne 0 ]; then
    echo "烧写工具下载失败，请检查 URL 或网络连接。"
    exit 1
fi

# 确保目标目录存在
mkdir -p "$POSTBUILD_DIR"

# 根据文件后缀解压到指定目录，兼容常见压缩格式
case "$POSTBUILD_FILE" in
    *.tar.gz)
        echo "解压烧写工具 tar.gz 文件..."
        tar -xzf "$POSTBUILD_FILE" -C "$POSTBUILD_DIR"
        ;;
    *.tgz)
        echo "解压烧写工具 tgz 文件..."
        tar -xzf "$POSTBUILD_FILE" -C "$POSTBUILD_DIR"
        ;;
    *.tar.bz2)
        echo "解压烧写工具 tar.bz2 文件..."
        tar -xjf "$POSTBUILD_FILE" -C "$POSTBUILD_DIR"
        ;;
    *.tar.xz)
        echo "解压烧写工具 tar.xz 文件..."
        tar -xJf "$POSTBUILD_FILE" -C "$POSTBUILD_DIR"
        ;;
    *.zip)
        echo "解压烧写工具 zip 文件..."
        unzip -o "$POSTBUILD_FILE" -d "$POSTBUILD_DIR"
        ;;
    *)
        echo "不支持的烧写工具文件格式：$POSTBUILD_FILE"
        exit 1
        ;;
esac

# 检查解压是否成功
if [ $? -eq 0 ]; then
    echo "烧写工具已成功解压到 $POSTBUILD_DIR"
    # 扁平化：若存在 jieli-linux-post-build-tools-* 顶层目录，移动其内容到 tools 根并移除该目录
    MATCHED_DIRS=$(find "$POSTBUILD_DIR" -maxdepth 1 -type d -name 'jieli-linux-post-build-tools-*')
    if [ -n "$MATCHED_DIRS" ]; then
        echo "检测到顶层目录 jieli-linux-post-build-tools-*，开始扁平化并安全合并..."
        while IFS= read -r dir; do
            [ -d "$dir" ] || continue
            # 遍历普通与隐藏条目
            for entry in "$dir"/* "$dir"/.[!.]* "$dir"/..?*; do
                [ -e "$entry" ] || continue
                name=$(basename "$entry")
                dest="$POSTBUILD_DIR/$name"
                # 若目标存在且类型冲突（目录 vs 文件），优先移除目标以避免 cp 报错
                if [ -e "$dest" ]; then
                    if [ -d "$dest" ] && [ ! -d "$entry" ]; then
                        rm -rf "$dest"
                    elif [ ! -d "$dest" ] && [ -d "$entry" ]; then
                        rm -f "$dest"
                    fi
                fi
                # 合并目录或复制文件
                if [ -d "$entry" ]; then
                    mkdir -p "$dest"
                    cp -a "$entry"/. "$dest"/
                else
                    cp -a "$entry" "$POSTBUILD_DIR"/
                fi
            done
            # 删除原顶层目录
            rm -rf "$dir"
        done <<< "$MATCHED_DIRS"
        echo "已完成扁平化：文件直接位于 $POSTBUILD_DIR"
    fi
else
    echo "烧写工具解压失败，请检查文件格式或完整性。"
    exit 1
fi

# 统一重命名并放置 packres 到 tools 根为 pack_res
PACKRES_PATH="$(find "$POSTBUILD_DIR" \( -type f -o -type d \) -name 'packres' | head -n 1)"
if [ -n "$PACKRES_PATH" ]; then
    echo "发现 packres：$PACKRES_PATH，开始重命名为 pack_res 并放到 $POSTBUILD_DIR"
    # 若目标已存在，先移除以避免冲突
    if [ -e "$POSTBUILD_DIR/pack_res" ]; then
        rm -rf "$POSTBUILD_DIR/pack_res"
    fi
    mv "$PACKRES_PATH" "$POSTBUILD_DIR/pack_res"
    # 若是文件，则确保可执行
    if [ -f "$POSTBUILD_DIR/pack_res" ]; then
        chmod +x "$POSTBUILD_DIR/pack_res" || true
    fi
    echo "已重命名：$POSTBUILD_DIR/pack_res"
else
    echo "未找到 packres，若包结构改变请确认工具名称。"
fi

# 清理下载的文件
echo "是否删除下载的工具链文件？(y/n)"
read -r response
if [ "$response" = "y" ]; then
    sudo rm -f "$FILE"
    echo "已删除下载的文件：$FILE"
fi

# 清理下载的烧写工具文件
echo "是否删除下载的烧写工具文件？(y/n)"
read -r response
if [ "$response" = "y" ]; then
    rm -f "$POSTBUILD_FILE"
    echo "已删除下载的文件：$POSTBUILD_FILE"
fi

echo "初始化完成！"


