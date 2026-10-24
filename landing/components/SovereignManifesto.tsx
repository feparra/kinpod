"use client";

import { Shield, Lock, Unlock, X, Check, Terminal, Cpu, Zap, Radio } from "lucide-react";

export function SovereignManifesto() {
  const comparisons = [
    {
      feature: "Ownership & Licensing",
      closedAI: "Proprietary locked black box. Voided warranty if opened.",
      kinpod: "100% Open Source (MIT). ESP-IDF C firmware, schematics, and 3D STL files in your hands.",
    },
    {
      feature: "Subscriptions & Telemetry",
      closedAI: "Forced $24/month cloud subscription. Voice audio permanently logged to private corporate servers.",
      kinpod: "Zero subscriptions forever. Direct WebSocket binary streaming to your localhost or private VPS.",
    },
    {
      feature: "Safety & Physical Boundary",
      closedAI: "Autonomous actions execute invisibly in the cloud with zero physical verification.",
      kinpod: "Human-In-The-Loop (HITL) sternum killswitch. Risky terminal commands halt until you physically tap your chest.",
    },
    {
      feature: "Agent Compatibility",
      closedAI: "Walled garden. Only works with whatever slow proprietary assistant the company ships.",
      kinpod: "Native Model Context Protocol (MCP). Inhabited directly by OpenClaw, Claude Code, Gemini, Hermes, or local Ollama.",
    },
    {
      feature: "Hackability & Hardware",
      closedAI: "Glued batteries, sealed chassis, locked bootloader.",
      kinpod: "Waveshare ESP32-S3 Dual-Core Xtensa @ 240MHz, 16MB Flash, 8MB Octal PSRAM, standard I2S audio codecs.",
    },
  ];

  return (
    <section id="manifesto" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Section Header */}
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-8 sm:pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#dc5000]">
              <Shield className="h-3.5 w-3.5" />
              <span>THE SOVEREIGN EMBODIMENT MANIFESTO</span>
            </div>
            <h2 className="mt-3 text-2xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-tight">
              WHY WE REJECT PROPRIETARY AI PINS.
            </h2>
            <p className="mt-3 max-w-2xl text-sm sm:text-base text-[#ffedd7]/80 leading-relaxed">
              Big tech tried to sell $700 locked gadgets with $24/month subscriptions to replace our phones. They failed because users don&apos;t want closed surveillance necklaces. KinPod is the open-source antithesis: an embodiment body for the autonomous agents you already own and control.
            </p>
          </div>

          <div className="font-mono text-xs uppercase text-[#6c5f51]">
            <span>SOVEREIGN HARDWARE • ZERO CLOUD LOCK-IN</span>
          </div>
        </div>

        {/* The Kin Node Geometry Banner */}
        <div className="mt-8 rounded-[16px] border border-[#40372e] bg-[#1a120c] p-6 sm:p-8 shadow-2xl">
          <div className="grid grid-cols-1 lg:grid-cols-12 gap-8 items-center">
            {/* Left: The Architectural Philosophy */}
            <div className="lg:col-span-7 flex flex-col">
              <span className="font-mono text-xs uppercase tracking-widest text-[#dc5000]">
                THE KIN NODE ARCHITECTURE
              </span>
              <h3 className="mt-2 text-xl sm:text-2xl font-medium uppercase text-[#ffedd7]">
                Mind in the Cloud. Bridge in Real-Time. Body on your Chest.
              </h3>
              <p className="mt-3 text-sm text-[#ffedd7]/80 leading-relaxed font-mono">
                The KinPod insignia (<code className="text-[#dc5000]">logo splash-01.png</code>) isn&apos;t decorative branding. It is an architectural schematic of human-machine symbiosis:
              </p>
              <div className="mt-4 flex flex-col gap-2.5 font-mono text-xs">
                <div className="flex items-center gap-3 text-[#ffedd7]">
                  <span className="flex h-6 w-6 items-center justify-center rounded bg-[#382416] text-[#dc5000] font-bold">1</span>
                  <span><strong>Upper Pixel:</strong> The Agent Mind (Claude, OpenClaw, Gemini, Ollama).</span>
                </div>
                <div className="flex items-center gap-3 text-[#ffedd7]">
                  <span className="flex h-6 w-6 items-center justify-center rounded bg-[#382416] text-[#dc5000] font-bold">2</span>
                  <span><strong>Central Bridge:</strong> Bi-directional binary WebSockets (audio + telemetry).</span>
                </div>
                <div className="flex items-center gap-3 text-[#ffedd7]">
                  <span className="flex h-6 w-6 items-center justify-center rounded bg-[#382416] text-[#dc5000] font-bold">3</span>
                  <span><strong>Lower Pixel:</strong> The Physical KinPod body with eyes, ears, and sternum killswitch.</span>
                </div>
              </div>
            </div>

            {/* Right: ASCII Architecture Visual */}
            <div className="lg:col-span-5 rounded-lg border border-[#40372e] bg-[#100904] p-4 font-mono text-xs text-[#ffedd7]">
              <pre className="text-[11px] leading-tight text-[#ffedd7]/90 select-none overflow-x-auto">
{`   ╭────────────────────────────╮
   │                            │
   │           ███              │ ◄── THE MIND (Agent / LLM)
   │       ███████████          │ ◄── THE SYNAPSE (MCP / WS)
   │       ███                  │ ◄── THE BODY (KinPod AMOLED)
   │                            │
   ╰────────────────────────────╯
         K  I  N  P  O  D`}
              </pre>
              <div className="mt-3 border-t border-[#40372e] pt-2 text-[10px] text-[#6c5f51]">
                TRUE-BLACK AMOLED DISPLAY (0 mA INK BLACKS)
              </div>
            </div>
          </div>
        </div>

        {/* Comparison Table: Closed AI Wearables vs KinPod */}
        <div className="mt-12 rounded-[16px] border border-[#40372e] bg-[#1a120c] overflow-hidden shadow-2xl">
          <div className="grid grid-cols-1 md:grid-cols-12 border-b border-[#40372e] bg-[#100904] p-4 sm:p-5 font-mono text-xs uppercase">
            <div className="md:col-span-3 text-[#6c5f51] font-semibold">DIMENSION</div>
            <div className="md:col-span-4 text-red-400 font-semibold flex items-center gap-1.5 mt-2 md:mt-0">
              <Lock className="h-3.5 w-3.5" />
              PROPRIETARY AI PINS (HUMANE / RABBIT)
            </div>
            <div className="md:col-span-5 text-emerald-400 font-semibold flex items-center gap-1.5 mt-2 md:mt-0">
              <Unlock className="h-3.5 w-3.5" />
              KINPOD OPEN EMBODIMENT
            </div>
          </div>

          <div className="divide-y divide-[#40372e]/60">
            {comparisons.map((item, idx) => (
              <div key={idx} className="grid grid-cols-1 md:grid-cols-12 p-4 sm:p-5 gap-3 md:gap-4 text-xs font-mono">
                <div className="md:col-span-3 font-semibold uppercase text-[#ffedd7] flex items-center">
                  {item.feature}
                </div>
                <div className="md:col-span-4 text-red-300/80 flex items-start gap-2 bg-red-950/20 p-2.5 rounded-md border border-red-900/30">
                  <X className="h-4 w-4 text-red-400 shrink-0 mt-0.5" />
                  <span>{item.closedAI}</span>
                </div>
                <div className="md:col-span-5 text-emerald-200/90 flex items-start gap-2 bg-emerald-950/20 p-2.5 rounded-md border border-emerald-900/30">
                  <Check className="h-4 w-4 text-emerald-400 shrink-0 mt-0.5" />
                  <span>{item.kinpod}</span>
                </div>
              </div>
            ))}
          </div>
        </div>
      </div>
    </section>
  );
}
