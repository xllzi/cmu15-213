#!/bin/bash
#
# time-concurrency.sh — 测量 proxy 在不同线程数下的加速比和效率
#
# 前置条件: tiny server 已在 15213 端口运行
# 输出: concurrency.csv
# ============================================================================

TINY_PORT=15213
PROXY_PORT=15214
TOTAL_REQUESTS=500
CLIENT_CONCURRENCY=64
ROUNDS=3
THREADS_LIST=(1 2 4 8 16 32)

PROXY_SRC="proxy.c"
RESULT_FILE="concurrency.csv"

# ---- 辅助函数 -------------------------------------------------------------

# 确保服务端口空闲
wait_port_free() {
    local port=$1
    while lsof -i :"$port" &>/dev/null; do
        sleep 0.2
    done
}

# 等待 proxy 就绪（最多等 5 秒）
wait_proxy_ready() {
    for i in $(seq 1 50); do
        curl -s -o /dev/null --proxy http://localhost:"$PROXY_PORT" \
             http://localhost:"$TINY_PORT"/home.html 2>/dev/null && return 0
        sleep 0.1
    done
    echo "ERROR: proxy did not start on port $PROXY_PORT" >&2
    return 1
}

# 预热 + 正式测试，所有输出吃进 /dev/null
bench_run() {
    local start end
    start=$(date +%s%N)
    seq "$TOTAL_REQUESTS" | xargs -P "$CLIENT_CONCURRENCY" -I {} \
        curl -s -o /dev/null --proxy http://localhost:"$PROXY_PORT" \
             http://localhost:"$TINY_PORT"/home.html >/dev/null 2>&1
    end=$(date +%s%N)
    echo "scale=3; ($end - $start) / 1000000000" | bc
}

# ---- 开始 -----------------------------------------------------------------
echo "NTHREADS,Time_s,Speedup,Efficiency" > "$RESULT_FILE"
BASELINE_TIME=""

for p in "${THREADS_LIST[@]}"; do
    echo "=== NTHREADS=$p ==="

    # 1. 修改宏并编译
    sed -i "s/^#define NTHREADS .*/#define NTHREADS $p/" "$PROXY_SRC"
    if ! make -s; then
        echo "ERROR: compile failed for NTHREADS=$p" >&2
        continue
    fi

    # 2. 启动 proxy
    ./proxy "$PROXY_PORT" >/dev/null 2>&1 &
    PROXY_PID=$!

    # 3. 等 proxy 就绪
    if ! wait_proxy_ready; then
        kill "$PROXY_PID" 2>/dev/null
        wait_port_free "$PROXY_PORT"
        continue
    fi

    # 4. 预热
    seq 10 | xargs -P 10 -I {} curl -s -o /dev/null \
        --proxy http://localhost:"$PROXY_PORT" \
        http://localhost:"$TINY_PORT"/home.html >/dev/null 2>&1

    # 5. 正式测试
    sum=0
    for ((r = 1; r <= ROUNDS; r++)); do
        elapsed=$(bench_run)
        sum=$(echo "$sum + $elapsed" | bc)
        echo "  Round $r: ${elapsed}s"
    done
    avg=$(echo "scale=3; $sum / $ROUNDS" | bc)

    # 6. 停 proxy，等端口释放
    kill "$PROXY_PID" 2>/dev/null
    wait "$PROXY_PID" 2>/dev/null
    wait_port_free "$PROXY_PORT"

    # 7. 加速比 & 效率
    if [ "$p" -eq 1 ]; then
        BASELINE_TIME="$avg"
        speedup="1.00"
        efficiency="1.00"
    else
        speedup=$(echo "scale=2; $BASELINE_TIME / $avg" | bc)
        efficiency=$(echo "scale=2; $speedup / $p" | bc)
    fi

    echo "$p,${avg},${speedup},${efficiency}" >> "$RESULT_FILE"
    echo "Avg: ${avg}s  Speedup: ${speedup}x  Eff: ${efficiency}"
    echo
done

# ---- 恢复默认 ---------------------------------------------------------------
sed -i "s/^#define NTHREADS .*/#define NTHREADS 8/" "$PROXY_SRC"
make -s

echo "Done. Results in $RESULT_FILE:"
cat "$RESULT_FILE"
