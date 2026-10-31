#!/usr/bin/env bash
# Test ps2client against tests/fake_ps2link.py (Linux only).
#
# ps2client and ps2link both use UDP port 0x4712, so the fake runs in its own
# network namespace (10.0.0.2), linked by a veth pair to the client's
# (10.0.0.1). Both are unprivileged user namespaces; no root needed.
#
#   tests/run-tests.sh [path/to/ps2client]

set -u
HERE=$(cd "$(dirname "$0")" && pwd)
CLIENT=$(realpath "${1:-$HERE/../bin/ps2client}")

if [ -z "${PS2CLIENT_TEST_NS:-}" ]; then
    exec unshare -rn env PS2CLIENT_TEST_NS=1 "$0" "$CLIENT"
fi

ip link set lo up

WORK=$(mktemp -d)
trap 'stop_fake; rm -rf "$WORK"' EXIT
cd "$WORK"
FAKE_PID=

start_fake() {
    # The link goes away with the namespace of the previous fake.
    ip link del veth0 2>/dev/null
    ip link add veth0 type veth peer name veth1
    ip addr add 10.0.0.1/24 dev veth0
    ip link set veth0 up
    # shellcheck disable=SC2016 # $0 and $@ expand in the inner shell
    unshare -n sh -c '
        while ! ip link show veth1 >/dev/null 2>&1; do sleep 0.02; done
        ip link set lo up; ip addr add 10.0.0.2/24 dev veth1; ip link set veth1 up
        exec python3 -I "$0" --bind 10.0.0.2 "$@"' "$HERE/fake_ps2link.py" "$@" &
    FAKE_PID=$!
    # Move the link only once the child has its own namespace.
    while [ "$(readlink /proc/$FAKE_PID/ns/net)" = "$(readlink /proc/self/ns/net)" ]; do sleep 0.01; done
    ip link set veth1 netns "$FAKE_PID"
    # Wait until it accepts connections. A plain connect, so that no command
    # reaches the fake before the test does.
    for _ in $(seq 100); do
        (exec 3<>/dev/tcp/10.0.0.2/18193) 2>/dev/null && return 0
        sleep 0.05
    done
    echo "fake ps2link did not start" >&2
    exit 1
}

stop_fake() {
    [ -n "$FAKE_PID" ] && kill "$FAKE_PID" 2>/dev/null && wait "$FAKE_PID" 2>/dev/null
    FAKE_PID=
}

PASS=0 FAIL=0
# Monotonic milliseconds; the wall clock can be adjusted during a run.
now_ms() { awk '{ printf "%d", $1 * 1000 }' /proc/uptime; }
# check <name> <expected status> <command...>: runs the client, keeps
# stdout/stderr in $OUT/$ERR and its run time (ms) in $MS.
check() {
    local name=$1 want=$2 start; shift 2
    start=$(now_ms)
    "$CLIENT" "$@" > out.txt 2> err.txt
    local rc=$?
    MS=$(( $(now_ms) - start ))
    OUT=$(cat out.txt) ERR=$(cat err.txt)
    if [ "$rc" = "$want" ]; then
        PASS=$((PASS + 1)); echo "PASS $name (status $rc, ${MS} ms)"
    else
        FAIL=$((FAIL + 1)); echo "FAIL $name: status $rc, want $want"; echo "  stdout: $OUT"; echo "  stderr: $ERR"
    fi
}
expect() { # expect <name> <condition...>
    local name=$1; shift
    if "$@"; then PASS=$((PASS + 1)); echo "PASS $name"; else FAIL=$((FAIL + 1)); echo "FAIL $name"; echo "  stdout: $OUT"; echo "  stderr: $ERR"; fi
}

H=(-h 10.0.0.2)

# --- without ps2link
check "ping, connection refused" 69 -h 127.0.0.1 -t 1 ping
expect "ping fails fast when refused" test "$MS" -lt 500
check "execee, nothing listening" 69 "${H[@]}" -t 5 execee host:x.elf
expect "error on stderr, stdout empty" test -z "$OUT" -a -n "$ERR"

# --- usage errors
check "unknown command" 64 "${H[@]}" frobnicate
check "missing arguments" 64 "${H[@]}" dumpmem 0x100000
check "bad register type" 64 "${H[@]}" dumpreg eeregs x.bin
check "-e without marker" 64 "${H[@]}" -e
check "arguments too long" 64 "${H[@]}" execee host:x.elf "$(printf 'a%.0s' $(seq 300))"
PS2HOSTNAME=1.1.l.1 check "invalid PS2HOSTNAME" 69 ping
expect "invalid hostname has an error, not a crash" test -n "$ERR"

start_fake

check "ping" 0 "${H[@]}" -t 2 ping
expect "ping reports round trip" grep -q "answered in" out.txt
check "ping, no answer" 69 -h 10.0.0.99 -t 1 ping
expect "ping gives up after -t" test "$MS" -ge 990 -a "$MS" -lt 2500

# --- the marker ends the run with its status
printf 'print hello\nprint @@END@@ status=3\nsleep 5\nprint late\n' > status.elf
check "marker status" 3 "${H[@]}" -t 10 -e @@END@@ execee host:status.elf one two
expect "output before marker" grep -q "^hello$" out.txt
expect "arguments passed" grep -q "^fake: args one two$" out.txt
expect "exits at the marker" test "$MS" -lt 3000
expect "nothing after the marker" bash -c '! grep -q late out.txt'

printf 'print @@END@@\n' > nostatus.elf
check "marker without status" 0 "${H[@]}" -t 10 -e @@END@@ execee host:nostatus.elf

# --- console packets are not null-terminated
printf 'raw 1024 A\nraw 10 B\nraw 1500 C\nprint\nprint @@END@@ status=0\n' > raw.elf
check "console packets" 0 "${H[@]}" -t 10 -e @@END@@ execee host:raw.elf
expect "exact console bytes, packets over 1 KB too" test "$(grep -c '^A\{1024\}B\{10\}C\{1500\}$' out.txt)" = 1

# --- ps2link going away
printf 'print bye\nclose\n' > close.elf
check "connection closed" 74 "${H[@]}" -t 10 -e @@END@@ execee host:close.elf
expect "closed: fast" test "$MS" -lt 3000
expect "closed: console kept" grep -q "^bye$" out.txt
check "connection closed, no marker option" 74 "${H[@]}" -t 10 execee host:close.elf

# --- output is written as it comes in, so a killed client loses nothing
printf 'print early\nsleep 10\n' > early.elf
"$CLIENT" "${H[@]}" -t 20 execee host:early.elf > early.txt 2>&1 &
CPID=$!
sleep 1.5
kill -TERM $CPID; wait $CPID 2>/dev/null
OUT=$(cat early.txt) ERR=
expect "unbuffered output survives kill" grep -q "^early$" early.txt

# --- idle timeout
printf 'print quiet\nsleep 10\n' > quiet.elf
check "idle timeout before marker" 124 "${H[@]}" -t 1 -e @@END@@ execee host:quiet.elf
check "idle timeout without -e" 0 "${H[@]}" -t 1 execee host:quiet.elf
expect "idle timeout waits" test "$MS" -ge 990

# --- host: file serving with big reads
head -c 300000 /dev/urandom > blob.bin
printf 'readback blob.bin\nprint @@END@@ status=0\n' > readback.elf
check "big host: read" 0 "${H[@]}" -t 10 -e @@END@@ execee host:readback.elf
expect "readback size and crc" grep -q "^readback 300000 $(python3 -c 'import sys,zlib;print("%08x"%zlib.crc32(open(sys.argv[1],"rb").read()))' blob.bin)$" out.txt

# --- second client
"$CLIENT" "${H[@]}" -t 5 listen > /dev/null 2>&1 &
LISTENER=$!
sleep 0.5
check "console port in use" 75 "${H[@]}" -t 5 -e @@END@@ execee host:nostatus.elf
expect "port in use: clear message" grep -q "another ps2client" err.txt
kill $LISTENER; wait $LISTENER 2>/dev/null

# --- reset needs neither console nor request connection
check "reset" 0 "${H[@]}" reset

# --- ps2link not ready right after accepting
stop_fake
start_fake --ignore-dumpreg 1
check "handshake retries" 0 "${H[@]}" -t 10 -e @@END@@ execee host:nostatus.elf
expect "retried after 1 s" test "$MS" -ge 990
stop_fake

echo "passed $PASS, failed $FAIL"
[ "$FAIL" = 0 ]
