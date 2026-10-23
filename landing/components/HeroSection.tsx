"use client";

import { useState } from "react";
import Image from "next/image";
import { ArrowRight, Copy, Check, Sparkles, Volume2, Mic, ShieldAlert, Cpu } from "lucide-react";

export function HeroSection({
  onAcquire,
  onCopyPrompt,
  copiedPrompt,
}: {
  onAcquire: () => void;
  onCopyPrompt: () => void;
  copiedPrompt: boolean;
}) {
  const [activeHotspot, setActiveHotspot] = useState<number | null>(null);

  const hotspots = [
    {
      id: 1,
      title: "2.06\" Curved AMOLED",
      desc: "410×502 px, 60 FPS, 100% ink blacks with AXP2101 power-saving auto-dim.",
      top: "28%",
      left: "48%",
      icon: Cpu,
    },
    {
      id: 2,
      title: "Tactile Push-To-Talk",
      desc: "Lateral hardware switch triggering dual ES7210 microphone capture.",
      top: "42%",
      left: "82%",
      icon: Mic,
    },
    {
      id: 3,
      title: "Front Acoustic Speaker",
      desc: "ES8311 codec + NS4150 high-output amplifier behind CNC acoustic slots.",
      top: "76%",
      left: "52%",
      icon: Volume2,
    },
    {
      id: 4,
      title: "CST9220 HITL Touchscreen",
      desc: "Physical chest approval before agent file deletions or financial calls.",
      top: "54%",
      left: "32%",
      icon: ShieldAlert,
    },
  ];

  return (
    <section className="relative overflow-hidden pt-12 pb-24 md:pt-20 md:pb-32">
      {/* Background Radial Glow Void */}
      <div className="pointer-events-none absolute inset-0 -z-10 bg-[radial-gradient(ellipse_80%_60%_at_50%_-10%,rgba(220,80,0,0.12),transparent_70%)]" />

      <div className="mx-auto max-w-7xl px-6">
        {/* Top Declarative Badge */}
        <div className="flex flex-col items-center text-center">
          <div className="inline-flex items-center gap-2 rounded-[9999px] border border-[#40372e] bg-[#1a120c] px-4 py-1.5 font-mono text-[11px] uppercase tracking-wider text-[#ffedd7]">
            <span className="h-1.5 w-1.5 rounded-full bg-[#dc5000] animate-pulse" />
            <span>FOUNDER EDITION BATCH 01</span>
            <span className="text-[#6c5f51]">•</span>
            <span className="text-[#ffedd7]/70">OPEN SOURCE HARDWARE</span>
          </div>

          {/* Primary Museum Uppercase Heading */}
          <h1 className="mt-8 max-w-5xl text-4xl sm:text-6xl md:text-7xl lg:text-8xl font-medium tracking-tight text-[#ffedd7] uppercase leading-[0.92]">
            GIVE YOUR AI AGENT <br className="hidden sm:inline" />
            <span className="text-transparent bg-clip-text bg-gradient-to-b from-[#ffedd7] via-[#ffedd7] to-[#6c5f51]">
              A PHYSICAL BODY.
            </span>
          </h1>

          {/* Mixed-Case Conversational Narrative Copy */}
          <p className="mt-6 max-w-2xl text-lg sm:text-xl text-[#ffedd7]/80 leading-relaxed font-normal">
            KinPod is an open-source chest companion for <span className="text-[#ffedd7] font-medium">OpenClaw</span>, <span className="text-[#ffedd7] font-medium">Claude Code</span>, <span className="text-[#ffedd7] font-medium">Gemini</span>, and <span className="text-[#ffedd7] font-medium">Hermes</span>. It features a curved 2.06&quot; AMOLED face, Push-To-Talk ears, acoustic voice, and a physical touchscreen guardian that halts risky autonomous actions until you tap approve on your chest.
          </p>

          {/* Action Row */}
          <div className="mt-8 flex flex-col sm:flex-row items-center gap-4">
            <button
              onClick={onAcquire}
              className="group flex h-14 w-full sm:w-auto items-center justify-center gap-3 rounded-[36px] bg-[#382416] px-8 font-mono text-sm font-semibold uppercase tracking-wider text-[#ffedd7] border border-[#40372e] hover:bg-[#523722] hover:border-[#dc5000]/60 transition-all active:scale-95 shadow-[0_0_30px_-5px_rgba(220,80,0,0.3)] cursor-pointer"
            >
              <span>Acquire KinPod — $120</span>
              <ArrowRight className="h-4 w-4 text-[#dc5000] transition-transform group-hover:translate-x-1" />
            </button>

            <button
              onClick={onCopyPrompt}
              className="flex h-14 w-full sm:w-auto items-center justify-center gap-2.5 rounded-[22.5px] border border-[#40372e] bg-[#1a120c]/60 px-6 font-mono text-xs uppercase tracking-wider text-[#ffedd7] hover:bg-[#1a120c] hover:border-[#6c5f51] transition-all active:scale-95 cursor-pointer"
            >
              {copiedPrompt ? (
                <>
                  <Check className="h-4 w-4 text-emerald-400" />
                  <span className="text-emerald-300">Prompt Copied!</span>
                </>
              ) : (
                <>
                  <Copy className="h-4 w-4 text-[#dc5000]" />
                  <span>Copy Agent Setup Prompt</span>
                </>
              )}
            </button>
          </div>
        </div>

        {/* 4K Hero Floating Render with Interactive Hotspots */}
        <div className="relative mt-16 sm:mt-24 mx-auto max-w-4xl">
          <div className="relative aspect-[16/10] w-full overflow-hidden rounded-[12px] border border-[#40372e] bg-[#1a120c]">
            <Image
              src="/assets/marketing/hero_editorial_4k.jpg"
              alt="KinPod 4K Floating Hardware Render"
              fill
              priority
              className="object-cover transition-transform duration-700 hover:scale-[1.02]"
            />

            {/* Subtle Gradient Vignette */}
            <div className="pointer-events-none absolute inset-0 bg-gradient-to-t from-[#100904] via-transparent to-transparent opacity-60" />

            {/* Hotspots */}
            {hotspots.map((spot) => {
              const Icon = spot.icon;
              const isActive = activeHotspot === spot.id;
              return (
                <div
                  key={spot.id}
                  style={{ top: spot.top, left: spot.left }}
                  className="absolute -translate-x-1/2 -translate-y-1/2 z-20"
                >
                  <button
                    onClick={() => setActiveHotspot(isActive ? null : spot.id)}
                    className={`relative flex h-8 w-8 items-center justify-center rounded-full border transition-all duration-300 cursor-pointer ${
                      isActive
                        ? "border-[#dc5000] bg-[#dc5000] text-white scale-110 shadow-[0_0_20px_#dc5000]"
                        : "border-[#ffedd7]/60 bg-[#100904]/80 text-[#ffedd7] hover:border-[#dc5000] hover:scale-105"
                    }`}
                    title={spot.title}
                  >
                    <Icon className="h-3.5 w-3.5" />
                    <span className="absolute -inset-1.5 -z-10 rounded-full bg-[#dc5000]/20 animate-ping" />
                  </button>

                  {/* Hotspot Popup Card */}
                  {isActive && (
                    <div className="absolute left-1/2 top-10 -translate-x-1/2 z-30 w-64 rounded-[12px] border border-[#40372e] bg-[#100904]/95 p-4 backdrop-blur-md shadow-2xl">
                      <div className="flex items-center gap-2">
                        <Icon className="h-4 w-4 text-[#dc5000]" />
                        <h4 className="font-mono text-xs uppercase tracking-wider font-semibold text-[#ffedd7]">
                          {spot.title}
                        </h4>
                      </div>
                      <p className="mt-1.5 text-xs text-[#ffedd7]/80 leading-normal">
                        {spot.desc}
                      </p>
                      <button
                        onClick={() => setActiveHotspot(null)}
                        className="mt-2 text-[10px] font-mono uppercase text-[#6c5f51] hover:text-[#ffedd7]"
                      >
                        Dismiss ×
                      </button>
                    </div>
                  )}
                </div>
              );
            })}
          </div>

          <div className="mt-3 flex items-center justify-between px-2 font-mono text-[10px] uppercase tracking-widest text-[#6c5f51]">
            <span>FIG 01. SUSPENDED HARDWARE SILHOUETTE</span>
            <span>TAP HOTSPOTS TO INSPECT SUBSYSTEMS</span>
          </div>
        </div>

        {/* Telemetry Stat Strip */}
        <div className="mt-16 grid grid-cols-2 gap-4 border-t border-[#40372e] pt-8 sm:grid-cols-4">
          <div className="flex flex-col">
            <span className="font-mono text-[11px] uppercase tracking-widest text-[#6c5f51]">
              DISPLAY
            </span>
            <span className="mt-1 font-mono text-lg font-medium text-[#ffedd7]">
              2.06&quot; AMOLED
            </span>
            <span className="text-xs text-[#ffedd7]/60">
              410×502 px • 60 FPS • Ink Black
            </span>
          </div>

          <div className="flex flex-col">
            <span className="font-mono text-[11px] uppercase tracking-widest text-[#6c5f51]">
              ACOUSTICS
            </span>
            <span className="mt-1 font-mono text-lg font-medium text-[#ffedd7]">
              ES7210 + ES8311
            </span>
            <span className="text-xs text-[#ffedd7]/60">
              Hardware PTT • NS4150 Amp
            </span>
          </div>

          <div className="flex flex-col">
            <span className="font-mono text-[11px] uppercase tracking-widest text-[#6c5f51]">
              GUARDIAN
            </span>
            <span className="mt-1 font-mono text-lg font-medium text-[#ffedd7]">
              HITL Touch
            </span>
            <span className="text-xs text-[#ffedd7]/60">
              Tactile physical chest approval
            </span>
          </div>

          <div className="flex flex-col">
            <span className="font-mono text-[11px] uppercase tracking-widest text-[#6c5f51]">
              PROTOCOL
            </span>
            <span className="mt-1 font-mono text-lg font-medium text-[#ffedd7]">
              Native MCP
            </span>
            <span className="text-xs text-[#ffedd7]/60">
              Zero-Cable Roaming Tunnel
            </span>
          </div>
        </div>
      </div>
    </section>
  );
}
