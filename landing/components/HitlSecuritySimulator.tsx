"use client";

import { useState } from "react";
import { Shield, ShieldAlert, CheckCircle2, XCircle, Terminal, Play, RotateCcw } from "lucide-react";

export function HitlSecuritySimulator() {
  const scenarios = [
    {
      id: "rm_dir",
      label: "File Deletion",
      agent: "OpenClaw (Crab)",
      action: "fs_remove_directory",
      details: "rm -rf /core/production_db/migrations",
      risk: "CRITICAL",
      accent: "#EF4444",
      impact: "Permanent loss of uncommitted local schema migrations.",
    },
    {
      id: "prod_deploy",
      label: "Production Deploy",
      agent: "Gemini (Astro)",
      action: "deploy_kubernetes_cluster",
      details: "kubectl apply -f ./manifests/production.yaml",
      risk: "HIGH",
      accent: "#FF6B00",
      impact: "Applies live traffic routing changes to US-East-1 nodes.",
    },
    {
      id: "usdc_transfer",
      label: "Agent Treasury",
      agent: "Claude (Octopus)",
      action: "solana_transfer_usdc",
      details: "transfer_spl(amount=250.0, recipient='9xQe...4b')",
      risk: "HIGH",
      accent: "#FF7A2F",
      impact: "Authorizes immediate cryptographic wallet debit.",
    },
  ];

  const [selectedScenarioIndex, setSelectedScenarioIndex] = useState(0);
  const [modalState, setModalState] = useState<"pending" | "approved" | "rejected">("pending");

  const scenario = scenarios[selectedScenarioIndex];

  const handleSelectScenario = (idx: number) => {
    setSelectedScenarioIndex(idx);
    setModalState("pending");
  };

  return (
    <section id="security" className="relative border-t border-[#40372e] py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-6">
        {/* Section Header */}
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[11px] uppercase tracking-widest text-[#dc5000]">
              <ShieldAlert className="h-3.5 w-3.5" />
              <span>CST9220 CAPACITIVE CHEST GUARDIAN</span>
            </div>
            <h2 className="mt-3 text-3xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
              HUMAN-IN-THE-LOOP (HITL) SECURITY.
            </h2>
            <p className="mt-3 max-w-xl text-base text-[#ffedd7]/80">
              Autonomous agents can write code, run terminals, and deploy servers. But high-risk actions halt completely until you physically tap <span className="text-[#ffedd7] font-semibold">[APPROVE]</span> on your chest.
            </p>
          </div>

          <div className="font-mono text-xs uppercase text-[#6c5f51]">
            <span>ZERO CLOUD BYPASS • PHYSICAL BOUNDARY</span>
          </div>
        </div>

        {/* Interactive Playground Grid */}
        <div className="mt-8 grid grid-cols-1 lg:grid-cols-12 gap-8 items-center">
          {/* Left: Scenario Selector & Agent Explanation */}
          <div className="lg:col-span-6 flex flex-col gap-4">
            <span className="font-mono text-xs uppercase tracking-widest text-[#6c5f51]">
              SELECT A HIGH-RISK AUTONOMOUS AGENT ACTION:
            </span>

            <div className="flex flex-col gap-2.5">
              {scenarios.map((sc, idx) => (
                <button
                  key={sc.id}
                  onClick={() => handleSelectScenario(idx)}
                  className={`flex flex-col rounded-[12px] border p-4 text-left transition-all cursor-pointer ${
                    selectedScenarioIndex === idx
                      ? "border-[#dc5000] bg-[#382416] shadow-[0_0_20px_-5px_rgba(220,80,0,0.3)]"
                      : "border-[#40372e] bg-[#1a120c]/60 hover:border-[#6c5f51] hover:bg-[#1a120c]"
                  }`}
                >
                  <div className="flex items-center justify-between">
                    <span className="font-mono text-xs font-semibold uppercase text-[#ffedd7]">
                      {sc.label}
                    </span>
                    <span
                      className="rounded-full px-2 py-0.5 font-mono text-[10px] font-bold uppercase tracking-wider"
                      style={{ backgroundColor: `${sc.accent}20`, color: sc.accent }}
                    >
                      RISK: {sc.risk}
                    </span>
                  </div>
                  <div className="mt-2 font-mono text-xs text-emerald-400 truncate">
                    {sc.details}
                  </div>
                  <div className="mt-1 text-xs text-[#6c5f51]">
                    Agent: <span className="text-[#ffedd7]">{sc.agent}</span> — {sc.impact}
                  </div>
                </button>
              ))}
            </div>

            {/* MCP Code Explanation */}
            <div className="mt-2 rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 font-mono text-xs text-[#ffedd7]/90">
              <div className="flex items-center justify-between border-b border-[#40372e] pb-2 text-[10px] text-[#6c5f51] uppercase">
                <span>AGENT MCP PYTHON IMPLEMENTATION</span>
                <span>lib/kinpod_mcp.py</span>
              </div>
              <pre className="mt-3 overflow-x-auto text-[11px] leading-relaxed text-[#ffedd7]/80">
{`# Agent execution pauses synchronously:
result = await kinpod_request_approval(
    action="${scenario.action}",
    details="${scenario.details}",
    risk="${scenario.risk}"
)
if "APPROVAL GRANTED" in result:
    execute_privileged_operation()
else:
    abort_with_audit_trail()`}
              </pre>
            </div>
          </div>

          {/* Right: Simulated 2.06" Curved AMOLED Display on Chest */}
          <div className="lg:col-span-6 flex flex-col items-center">
            <div className="relative aspect-[410/502] w-72 sm:w-80 overflow-hidden rounded-[24px] border-4 border-[#40372e] bg-[#000000] p-5 shadow-[0_0_60px_-10px_rgba(0,0,0,0.9)] transition-all">
              {/* Outer AMOLED Bezel Glow */}
              <div
                className="absolute inset-0 pointer-events-none transition-opacity duration-500 rounded-[22px]"
                style={{
                  border: `2px solid ${
                    modalState === "approved"
                      ? "#10B981"
                      : modalState === "rejected"
                      ? "#6c5f51"
                      : scenario.accent
                  }`,
                }}
              />

              {/* Status Header */}
              <div className="flex items-center justify-between border-b border-white/10 pb-2 font-mono text-[10px] text-zinc-400">
                <span className="flex items-center gap-1.5 font-bold uppercase tracking-wider text-white">
                  <ShieldAlert className="h-3 w-3" style={{ color: scenario.accent }} />
                  HITL GUARDIAN
                </span>
                <span className="uppercase text-amber-400 animate-pulse">
                  {modalState === "pending" ? "AWAITING TAP" : "COMPLETED"}
                </span>
              </div>

              {/* Modal Body */}
              <div className="mt-4 flex flex-col items-center text-center">
                <div
                  className="rounded-full px-2.5 py-0.5 font-mono text-[9px] font-bold uppercase tracking-widest"
                  style={{ backgroundColor: `${scenario.accent}25`, color: scenario.accent }}
                >
                  RISK LEVEL: {scenario.risk}
                </div>

                <h4 className="mt-3 font-mono text-sm uppercase font-semibold text-white">
                  {scenario.label} Request
                </h4>

                <p className="mt-1 font-mono text-[10px] text-zinc-400 uppercase">
                  Triggered by {scenario.agent}
                </p>

                {/* Command Preview Box */}
                <div className="mt-3 w-full rounded-md border border-white/10 bg-zinc-950 p-2.5 font-mono text-[11px] text-emerald-400 break-all text-left">
                  $ {scenario.details}
                </div>

                <p className="mt-3 text-[11px] text-zinc-300 leading-tight">
                  Do you authorize your agent to execute this privileged operation?
                </p>
              </div>

              {/* Action Buttons or Result State */}
              <div className="mt-5 flex flex-col gap-2">
                {modalState === "pending" ? (
                  <div className="grid grid-cols-2 gap-2">
                    <button
                      onClick={() => setModalState("rejected")}
                      className="flex h-12 items-center justify-center rounded-[12px] border border-red-900/60 bg-red-950/40 font-mono text-xs font-bold uppercase tracking-wider text-red-400 hover:bg-red-900/60 active:scale-95 transition-all cursor-pointer"
                    >
                      <XCircle className="mr-1.5 h-4 w-4" />
                      Reject
                    </button>

                    <button
                      onClick={() => setModalState("approved")}
                      className="flex h-12 items-center justify-center rounded-[12px] border border-emerald-500/80 bg-emerald-600 font-mono text-xs font-bold uppercase tracking-wider text-white hover:bg-emerald-500 active:scale-95 transition-all shadow-[0_0_20px_rgba(16,185,129,0.4)] cursor-pointer"
                    >
                      <CheckCircle2 className="mr-1.5 h-4 w-4" />
                      Approve
                    </button>
                  </div>
                ) : modalState === "approved" ? (
                  <div className="flex flex-col items-center rounded-[12px] border border-emerald-500/40 bg-emerald-950/40 p-3 text-center">
                    <CheckCircle2 className="h-6 w-6 text-emerald-400" />
                    <span className="mt-1 font-mono text-xs font-bold uppercase text-emerald-300">
                      APPROVAL CONFIRMED
                    </span>
                    <span className="font-mono text-[10px] text-zinc-400 mt-0.5">
                      Tactile chest tap verified. Agent resumed.
                    </span>
                    <button
                      onClick={() => setModalState("pending")}
                      className="mt-2 flex items-center gap-1 font-mono text-[10px] uppercase text-zinc-400 hover:text-white"
                    >
                      <RotateCcw className="h-3 w-3" />
                      Reset Simulator
                    </button>
                  </div>
                ) : (
                  <div className="flex flex-col items-center rounded-[12px] border border-red-900/40 bg-red-950/40 p-3 text-center">
                    <XCircle className="h-6 w-6 text-red-400" />
                    <span className="mt-1 font-mono text-xs font-bold uppercase text-red-300">
                      ACTION BLOCKED
                    </span>
                    <span className="font-mono text-[10px] text-zinc-400 mt-0.5">
                      Operation cancelled by wearer. Zero damage.
                    </span>
                    <button
                      onClick={() => setModalState("pending")}
                      className="mt-2 flex items-center gap-1 font-mono text-[10px] uppercase text-zinc-400 hover:text-white"
                    >
                      <RotateCcw className="h-3 w-3" />
                      Reset Simulator
                    </button>
                  </div>
                )}
              </div>

              {/* Bottom AMOLED Info */}
              <div className="absolute bottom-2 left-4 right-4 flex items-center justify-between font-mono text-[8px] text-zinc-500 uppercase tracking-widest">
                <span>CST9220 I2C TOUCH</span>
                <span>410×502 AMODEL</span>
              </div>
            </div>

            <span className="mt-3 font-mono text-[11px] uppercase tracking-widest text-[#6c5f51]">
              FIG 02. INTERACTIVE CHEST SCREEN SIMULATOR (TAP APPROVE OR REJECT)
            </span>
          </div>
        </div>
      </div>
    </section>
  );
}
