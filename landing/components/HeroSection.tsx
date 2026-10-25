"use client";

import { useState } from "react";
import Image from "next/image";
import { ArrowRight, Copy, Check, Volume2, Mic, ShieldAlert, Cpu, Info } from "lucide-react";

export function HeroSection({
  onAcquire,
  onCopyPrompt,
  copiedPrompt,
}: {
  onAcquire: () => void;
  onCopyPrompt: () => void;
  copiedPrompt: boolean;
}) {
  const [activeHotspot, setActiveHotspot] = useState<number | null>(1);

  const hotspots = [
    {
      id: 1,
      title: "2.06\" Curved AMOLED",
      desc: "410×502 px, 60 FPS, 100% ink blacks with AXP2101 power-saving auto-dim.",
      top: "30%",
      left: "48%",
      icon: Cpu,
    },
    {
      id: 2,
      title: "Tactile Push-To-Talk",
      desc: "Lateral hardware switch triggering dual ES7210 microphone capture.",
      top: "42%",
      left: "78%",
      icon: Mic,
    },
    {
      id: 3,
      title: "Front Acoustic Speaker",
      desc: "ES8311 codec + NS4150 high-output amplifier behind CNC acoustic slots.",
      top: "74%",
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

  const currentSpot = hotspots.find((h) => h.id === activeHotspot) || hotspots[0];
  const CurrentIcon = currentSpot.icon;

  return (
    <section className="relative overflow-hidden pt-8 pb-16 sm:pt-16 sm:pb-24 md:pt-20 md:pb-32">
      {/* Background Radial Glow Void */}
      <div className="pointer-events-none absolute inset-0 -z-10 bg-[radial-gradient(ellipse_80%_60%_at_50%_-10%,rgba(220,80,0,0.12),transparent_70%)]" />

      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Top Declarative Badge */}
        <div className="flex flex-col items-center text-center">
          <div className="inline-flex items-center gap-2 rounded-[9999px] border border-[#40372e] bg-[#1a120c] px-3 py-1 font-mono text-[10px] sm:text-[11px] uppercase tracking-wider text-[#ffedd7]">
            <span className="h-1.5 w-1.5 rounded-full bg-[#dc5000] animate-pulse" />
            <span>FOUNDER BATCH 01</span>
            <span className="text-[#6c5f51]">•</span>
            <span className="text-[#ffedd7]/70">OPEN SOURCE HARDWARE</span>
          </div>

          {/* Primary Museum Uppercase Heading (Mobile-First Fluid Scale) */}
          <h1 className="mt-6 sm:mt-8 max-w-5xl text-3xl sm:text-5xl md:text-7xl lg:text-8xl font-medium tracking-tight text-[#ffedd7] uppercase leading-[0.94] sm:leading-[0.92]">
            GIVE YOUR AI AGENT <br className="hidden sm:inline" />
            <span className="text-transparent bg-clip-text bg-gradient-to-b from-[#ffedd7] via-[#ffedd7] to-[#6c5f51]">
              A PHYSICAL BODY.
            </span>
          </h1>

          {/* Mixed-Case Conversational Narrative Copy */}
          <p className="mt-4 sm:mt-6 max-w-2xl text-base sm:text-lg md:text-xl text-[#ffedd7]/80 leading-relaxed font-normal">
            KinPod is an open-source chest companion for <span className="text-[#ffedd7] font-medium">OpenClaw</span>, <span className="text-[#ffedd7] font-medium">Claude Code</span>, <span className="text-[#ffedd7] font-medium">Gemini</span>, and <span className="text-[#ffedd7] font-medium">Hermes</span>. 2.06&quot; curved AMOLED face, Push-To-Talk audio, and a physical touchscreen guardian that halts risky actions until you tap approve on your chest.
          </p>

          {/* Action Row - Mobile First Full Width Buttons */}
          <div className="mt-6 sm:mt-8 flex flex-col sm:flex-row items-stretch sm:items-center gap-3 sm:gap-4 w-full sm:w-auto">
            <a
              href="/docs"
              className="group flex h-13 sm:h-14 w-full sm:w-auto items-center justify-center gap-3 rounded-[36px] bg-[#382416] px-6 sm:px-8 font-mono text-xs sm:text-sm font-semibold uppercase tracking-wider text-[#ffedd7] border border-[#40372e] hover:bg-[#523722] hover:border-[#dc5000]/60 transition-all active:scale-95 shadow-[0_0_30px_-5px_rgba(220,80,0,0.3)] cursor-pointer"
            >
              <span>Explore Documentation</span>
              <ArrowRight className="h-4 w-4 text-[#dc5000] transition-transform group-hover:translate-x-1" />
            </a>

            <button
              onClick={onCopyPrompt}
              className="flex h-13 sm:h-14 w-full sm:w-auto items-center justify-center gap-2.5 rounded-[22.5px] border border-[#40372e] bg-[#1a120c]/60 px-5 sm:px-6 font-mono text-xs uppercase tracking-wider text-[#ffedd7] hover:bg-[#1a120c] hover:border-[#6c5f51] transition-all active:scale-95 cursor-pointer"
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

        {/* 4K Hero Floating Render with Mobile-Optimized Hotspots */}
        <div className="relative mt-12 sm:mt-20 mx-auto max-w-4xl">
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

            {/* Hotspots Buttons */}
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
                    className={`relative flex h-8 w-8 sm:h-9 sm:w-9 items-center justify-center rounded-full border transition-all duration-300 cursor-pointer ${
                      isActive
                        ? "border-[#dc5000] bg-[#dc5000] text-white scale-110 shadow-[0_0_20px_#dc5000]"
                        : "border-[#ffedd7]/60 bg-[#100904]/80 text-[#ffedd7] hover:border-[#dc5000] hover:scale-105"
                    }`}
                    aria-label={spot.title}
                  >
                    <Icon className="h-3.5 w-3.5 sm:h-4 sm:w-4" />
                    <span className="absolute -inset-1.5 -z-10 rounded-full bg-[#dc5000]/20 animate-ping" />
                  </button>
                </div>
              );
            })}
          </div>

          {/* Active Subsystem Card (Mobile-First Safe Area - No Overflow Clipping) */}
          {activeHotspot && (
            <div className="mt-3 rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 shadow-xl flex items-start gap-3 sm:hidden animate-fade-in">
              <div className="flex h-8 w-8 shrink-0 items-center justify-center rounded-full border border-[#dc5000] bg-[#382416] text-[#dc5000]">
                <CurrentIcon className="h-4 w-4" />
              </div>
              <div className="flex-1">
                <div className="flex items-center justify-between">
                  <h4 className="font-mono text-xs uppercase tracking-wider font-semibold text-[#ffedd7]">
                    {currentSpot.title}
                  </h4>
                  <button
                    onClick={() => setActiveHotspot(null)}
                    className="font-mono text-[10px] text-[#6c5f51] hover:text-[#ffedd7]"
                  >
                    ×
                  </button>
                </div>
                <p className="mt-1 text-xs text-[#ffedd7]/80 leading-normal">
                  {currentSpot.desc}
                </p>
              </div>
            </div>
          )}

          <div className="mt-3 flex items-center justify-between px-1 font-mono text-[9px] sm:text-[10px] uppercase tracking-widest text-[#6c5f51]">
            <span>FIG 01. SUSPENDED HARDWARE SILHOUETTE</span>
            <span>TAP HOTSPOTS TO INSPECT</span>
          </div>
        </div>

        {/* Telemetry Stat Strip - Mobile 2x2 Grid */}
        <div className="mt-12 sm:mt-16 grid grid-cols-2 gap-4 border-t border-[#40372e] pt-6 sm:pt-8 sm:grid-cols-4">
          <div className="flex flex-col">
            <span className="font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#6c5f51]">
              DISPLAY
            </span>
            <span className="mt-1 font-mono text-base sm:text-lg font-medium text-[#ffedd7]">
              2.06&quot; AMOLED
            </span>
            <span className="text-[11px] sm:text-xs text-[#ffedd7]/60">
              410×502 px • 60 FPS
            </span>
          </div>

          <div className="flex flex-col">
            <span className="font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#6c5f51]">
              ACOUSTICS
            </span>
            <span className="mt-1 font-mono text-base sm:text-lg font-medium text-[#ffedd7]">
              Dual ES7210
            </span>
            <span className="text-[11px] sm:text-xs text-[#ffedd7]/60">
              Hardware PTT • NS4150 Amp
            </span>
          </div>

          <div className="flex flex-col">
            <span className="font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#6c5f51]">
              GUARDIAN
            </span>
            <span className="mt-1 font-mono text-base sm:text-lg font-medium text-[#ffedd7]">
              HITL Touch
            </span>
            <span className="text-[11px] sm:text-xs text-[#ffedd7]/60">
              Physical chest approval
            </span>
          </div>

          <div className="flex flex-col">
            <span className="font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#6c5f51]">
              PROTOCOL
            </span>
            <span className="mt-1 font-mono text-base sm:text-lg font-medium text-[#ffedd7]">
              Native MCP
            </span>
            <span className="text-[11px] sm:text-xs text-[#ffedd7]/60">
              Zero-Cable Roaming
            </span>
          </div>
        </div>
      </div>
    </section>
  );
}
