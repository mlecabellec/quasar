#!/bin/bash

# Configuration
PORT=8086
URL="http://localhost:$PORT"
BIN="./bin/sampleweb001"

echo "[Test] Starting sampleweb001 in background..."
$BIN &
PID=$!

# Ensure cleanup on exit
trap "kill $PID" EXIT

# Wait for server to start
echo "[Test] Waiting for server to initialize..."
MAX_RETRIES=10
COUNT=0
while ! curl -s $URL/api/v1/walk > /dev/null; do
    sleep 1
    ((COUNT++))
    if [ $COUNT -ge $MAX_RETRIES ]; then
        echo "[Error] Server failed to start."
        exit 1
    fi
done

echo "[Test] 1. Verifying tree structure (/api/v1/walk)..."
RES_WALK=$(curl -s "$URL/api/v1/walk?path=/Registry/Status")
echo $RES_WALK | grep -q "cmdStart" || { echo "[Fail] cmdStart not found in tree"; exit 1; }
echo "[Pass] Tree structure verified."

echo "[Test] 2. Verifying initial state machine state (/api/v1/meta)..."
RES_META=$(curl -s "$URL/api/v1/meta?path=/Registry/Status/systemState")
echo $RES_META | grep -q "STANDBY" || { echo "[Fail] Initial state is not STANDBY"; echo $RES_META; exit 1; }
echo "[Pass] Initial state verified."

echo "[Test] 3. Triggering transition STANDBY -> STARTING -> STARTED..."
curl -s -X POST "$URL/api/v1/set?path=/Registry/Status/cmdStart" \
     -H "Content-Type: application/json" \
     -d '{"value": true}' > /dev/null

# Wait for logic cycle and transition
sleep 1

RES_META_STARTED=$(curl -s "$URL/api/v1/meta?path=/Registry/Status/systemState")
echo $RES_META_STARTED | grep -q "STARTED" || { echo "[Fail] State did not transition to STARTED"; echo $RES_META_STARTED; exit 1; }
echo "[Pass] Transition to STARTED successful."

echo "[Test] 4. Triggering transition STARTED -> STOPPING -> STANDBY..."
curl -s -X POST "$URL/api/v1/set?path=/Registry/Status/cmdStop" \
     -H "Content-Type: application/json" \
     -d '{"value": true}' > /dev/null

# Wait for logic cycle
sleep 1

RES_META_FINAL=$(curl -s "$URL/api/v1/meta?path=/Registry/Status/systemState")
echo $RES_META_FINAL | grep -q "STANDBY" || { echo "[Fail] State did not return to STANDBY"; echo $RES_META_FINAL; exit 1; }
echo "[Pass] Return to STANDBY successful."

echo "[Test] All integration tests PASSED."
exit 0
