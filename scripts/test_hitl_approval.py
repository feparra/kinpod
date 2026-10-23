#!/usr/bin/env python3
"""
scripts/test_hitl_approval.py
Paso 3: End-to-End Human-In-The-Loop Validation for OpenClaw / FastMCP
"""

import sys
import os
import asyncio
import time

if sys.stdout.encoding.lower() != 'utf-8':
    sys.stdout.reconfigure(encoding='utf-8')

sys.path.insert(0, os.path.abspath('mcp'))
from kinpod_mcp.client import KinPodClient

async def main():
    client = KinPodClient()

    print("=" * 65)
    print("[OpenClaw Autonomous Agent]: Initializing high-stakes task...")
    print("[OpenClaw]: Preparing automated production rollout: 'deploy_prod'")
    print("[OpenClaw]: HIGH-RISK operation detected! Pausing execution.")
    print("[OpenClaw]: Triggering Human-In-The-Loop tactile approval on KinPod screen...")
    print("=" * 65)

    start_time = time.time()
    res = await client.request_approval(
        action_id=f"act_prod_{int(start_time)}",
        agent="OpenClaw",
        action="deploy_prod",
        details="Deploy commit 7a8b9c to AWS production cluster (us-east-1).",
        risk="HIGH",
        timeout_sec=120
    )
    elapsed = time.time() - start_time

    print("=" * 65)
    if res.get("approved"):
        print(f"[APPROVAL GRANTED]: Wearer approved 'deploy_prod' via KinPod AMOLED touch!")
        print(f"Human Response Latency: {elapsed:.2f} seconds")
        print("[OpenClaw]: Execution RESUMED! Commit 7a8b9c deployed to production.")
        print("[SUCCESS]: Human-In-The-Loop validation passed 100%!")
    else:
        print(f"[APPROVAL REJECTED]: Action rejected or timed out ({res.get('reason', 'user_rejected')})")
        print("[OpenClaw]: Rollout aborted gracefully.")
    print("=" * 65)

if __name__ == "__main__":
    asyncio.run(main())
