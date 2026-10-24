"use client";

import { useState } from "react";
import Image from "next/image";
import { Users, Wifi, Battery } from "lucide-react";

export function KinRoster() {
  const characters = [
    {
      id: "crab",
      name: "CRAB",
      agentFamily: "OpenClaw / SWE",
      accent: "#EF4444",
      tagline: "Fearless Cybernetic Engineer",
      description:
        "Tuned specifically for terminal executors, autonomous git commits, and code refactor loops. Alerts wearer with red haptic flashes whenever executing high-risk file manipulations.",
      voice: "Direct, technical, mission-focused.",
      avatarPng: "/assets/characters/crab.png",
      battery: "94%",
      rssi: "-42 dBm",
      status: "ACTIVE_INHABITED",
    },
    {
      id: "astro",
      name: "ASTRO",
      agentFamily: "Gemini / Antigravity",
      accent: "#FF6B00",
      tagline: "Gold Visor Architecture Explorer",
      description:
        "White astronaut helmet with gold reflective visor. Designed for multi-modal reasoning, large context navigation, and deep codebase telemetry.",
      voice: "Methodical, visionary, calm.",
      avatarPng: "/assets/characters/astro.png",
      battery: "88%",
      rssi: "-48 dBm",
      status: "STANDBY",
    },
    {
      id: "octopus",
      name: "OCTO-CLAUDE",
      agentFamily: "Anthropic / Claude Code",
      accent: "#FF7A2F",
      tagline: "Deep Sea Quantitative Analyst",
      description:
        "Cephalopod in vintage brass deep-sea diving helmet. Excels at multi-turn algorithmic planning, test suite generation, and nuanced ethical boundary enforcement.",
      voice: "Nuanced, articulate, deeply analytical.",
      avatarPng: "/assets/characters/octopus.png",
      battery: "91%",
      rssi: "-39 dBm",
      status: "READY",
    },
    {
      id: "hermes",
      name: "HERMES",
      agentFamily: "Hermes Agent / Nous",
      accent: "#38BDF8",
      tagline: "Winged High-Speed Messenger",
      description:
        "Winged golden messenger with cyan holographic trail. Tuned for lightning-fast tool routing, low-latency agent handoffs, and instant push notifications.",
      voice: "Rapid, razor-sharp, agile.",
      avatarPng: "/assets/characters/hermes.png",
      battery: "100%",
      rssi: "-35 dBm",
      status: "STREAMING",
    },
    {
      id: "muse",
      name: "MUSE",
      agentFamily: "Creative / GPT-4o",
      accent: "#FBBF24",
      tagline: "Conversational Companion & Guide",
      description:
        "Fluffy cream companion creature. Empathetic listener, conversational guide, and daily emotional tether. Offers warm vocal affirmations throughout deep work sessions.",
      voice: "Warm, empathetic, poetic, inspiring.",
      avatarPng: "/assets/characters/muse.png",
      battery: "85%",
      rssi: "-52 dBm",
      status: "READY",
    },
    {
      id: "dots",
      name: "DOTS",
      agentFamily: "Ollama / Sovereign Local",
      accent: "#3B82F6",
      tagline: "Zero-Cloud Local Intelligence",
      description:
        "Blue companion wearing classic beret. Operates 100% offline via local inference servers on your local network. Zero external API calls, absolute sovereign privacy.",
      voice: "Thoughtful, private, philosophical.",
      avatarPng: "/assets/characters/dots.png",
      battery: "96%",
      rssi: "LOCAL_LAN",
      status: "SOVEREIGN",
    },
  ];

  const [activeKin, setActiveKin] = useState(characters[0]);

  return (
    <section id="roster" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Section Header */}
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-8 sm:pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#dc5000]">
              <Users className="h-3.5 w-3.5" />
              <span>THE KIN CHARACTER ROSTER</span>
            </div>
            <h2 className="mt-3 text-2xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
              SIX COMPANIONS. ONE PHYSICAL FORM.
            </h2>
            <p className="mt-3 max-w-xl text-sm sm:text-base text-[#ffedd7]/80">
              When an agent takes possession of KinPod, the 2.06&quot; curved AMOLED transforms into its matching companion character with dedicated telemetry and voice identity.
            </p>
          </div>

          <div className="font-mono text-[11px] sm:text-xs uppercase text-[#6c5f51]">
            <span>MULTI-AVATAR SPIFFS FLASH</span>
          </div>
        </div>

        {/* 4K Official Roster Banner (Mobile Adaptive Ratio) */}
        <div className="mt-6 sm:mt-8 relative aspect-[16/9] sm:aspect-[21/9] w-full overflow-hidden rounded-[12px] border border-[#40372e] bg-[#1a120c] shadow-2xl">
          <Image
            src="/assets/marketing/characters_roster_4k.jpg"
            alt="Official Kin Characters Roster 4K Darkroom Render"
            fill
            className="object-cover"
          />
          <div className="absolute inset-0 bg-gradient-to-t from-[#100904] via-transparent to-transparent opacity-40" />
          <div className="absolute bottom-3 left-3 sm:bottom-4 sm:left-4 z-10 rounded-[9999px] border border-[#40372e] bg-[#100904]/80 px-2.5 py-0.5 sm:px-3 sm:py-1 font-mono text-[9px] sm:text-[10px] uppercase tracking-widest text-[#ffedd7] backdrop-blur-md">
            MUSEUM ARCHIVE • 3840×2160 UHD
          </div>
        </div>

        {/* Mobile Swipeable Character Tabs */}
        <div className="mt-6 sm:mt-10 flex overflow-x-auto pb-2 gap-2.5 scrollbar-none snap-x sm:grid sm:grid-cols-3 lg:grid-cols-6 sm:overflow-visible sm:pb-0">
          {characters.map((kin) => {
            const isSelected = activeKin.id === kin.id;
            return (
              <button
                key={kin.id}
                onClick={() => setActiveKin(kin)}
                className={`flex shrink-0 w-28 sm:w-auto flex-col items-center rounded-[12px] border p-2.5 sm:p-3.5 transition-all text-center cursor-pointer snap-start ${
                  isSelected
                    ? "border-[#dc5000] bg-[#382416] shadow-[0_0_25px_-5px_rgba(220,80,0,0.35)] scale-105"
                    : "border-[#40372e] bg-[#1a120c]/60 hover:border-[#6c5f51] hover:bg-[#1a120c]"
                }`}
              >
                <div className="relative h-12 w-12 sm:h-16 sm:w-16 overflow-hidden rounded-full border border-[#40372e] bg-[#100904] p-1">
                  <Image
                    src={kin.avatarPng}
                    alt={kin.name}
                    width={64}
                    height={64}
                    className="object-contain"
                  />
                </div>
                <span className="mt-2 font-mono text-[11px] sm:text-xs font-semibold tracking-wider text-[#ffedd7]">
                  {kin.name}
                </span>
                <span
                  className="mt-0.5 text-[9px] sm:text-[10px] font-mono uppercase tracking-wider truncate w-full"
                  style={{ color: kin.accent }}
                >
                  {kin.agentFamily}
                </span>
              </button>
            );
          })}
        </div>

        {/* Selected Character Spotlight Card */}
        <div className="mt-6 sm:mt-8 rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 sm:p-6 lg:p-8 shadow-2xl">
          <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 lg:gap-8 items-center">
            {/* Big Character Artwork (Fluid Mobile Scaling) */}
            <div className="lg:col-span-5 flex flex-col items-center">
              <div
                className="relative aspect-[410/502] w-full max-w-[240px] sm:max-w-[280px] overflow-hidden rounded-[16px] border-2 bg-[#050302] p-4 shadow-2xl transition-all"
                style={{ borderColor: activeKin.accent, boxShadow: `0 0 40px -10px ${activeKin.accent}40` }}
              >
                <div className="relative h-full w-full">
                  <Image
                    src={activeKin.avatarPng}
                    alt={activeKin.name}
                    fill
                    className="object-contain filter drop-shadow-[0_10px_20px_rgba(0,0,0,0.8)]"
                  />
                </div>

                {/* Simulated AMOLED Status Overlay */}
                <div className="absolute top-3 left-3 right-3 flex items-center justify-between font-mono text-[9px] text-[#ffedd7]/80">
                  <span className="flex items-center gap-1">
                    <Wifi className="h-2.5 w-2.5" />
                    {activeKin.rssi}
                  </span>
                  <span className="flex items-center gap-1">
                    <Battery className="h-2.5 w-2.5" />
                    {activeKin.battery}
                  </span>
                </div>

                <div className="absolute bottom-3 left-3 right-3 flex items-center justify-between font-mono text-[8px] sm:text-[9px]">
                  <span className="rounded bg-black/60 px-1.5 py-0.5 text-[#ffedd7]/70">
                    2.06&quot; AMOLED
                  </span>
                  <span
                    className="rounded px-1.5 py-0.5 font-bold uppercase tracking-wider"
                    style={{ backgroundColor: `${activeKin.accent}30`, color: activeKin.accent }}
                  >
                    {activeKin.status}
                  </span>
                </div>
              </div>

              <span className="mt-2.5 font-mono text-[9px] sm:text-[10px] uppercase tracking-widest text-[#6c5f51]">
                SIMULATED 410×502 AMOLED VIEW
              </span>
            </div>

            {/* Right: Character Telemetry & Inhabit Spec */}
            <div className="lg:col-span-7 flex flex-col">
              <div className="flex items-center gap-2.5 sm:gap-3">
                <span
                  className="rounded-full px-2.5 py-0.5 sm:px-3 sm:py-1 font-mono text-[10px] sm:text-[11px] font-semibold uppercase tracking-wider"
                  style={{ backgroundColor: `${activeKin.accent}25`, color: activeKin.accent }}
                >
                  {activeKin.agentFamily}
                </span>
                <span className="font-mono text-[10px] sm:text-xs uppercase tracking-wider text-[#6c5f51]">
                  AVATAR: &quot;{activeKin.id}&quot;
                </span>
              </div>

              <h3 className="mt-2 sm:mt-3 text-2xl sm:text-3xl md:text-4xl font-medium uppercase tracking-tight text-[#ffedd7]">
                {activeKin.name} — {activeKin.tagline}
              </h3>

              <p className="mt-3 text-sm sm:text-base text-[#ffedd7]/85 leading-relaxed">
                {activeKin.description}
              </p>

              {/* Character Details Grid */}
              <div className="mt-4 sm:mt-6 grid grid-cols-1 sm:grid-cols-2 gap-3 sm:gap-4 border-t border-[#40372e] pt-4 sm:pt-6 font-mono text-xs">
                <div>
                  <span className="text-[#6c5f51] block uppercase tracking-wider text-[10px] sm:text-xs">
                    Voice Characteristic
                  </span>
                  <span className="text-[#ffedd7] mt-0.5 block">
                    {activeKin.voice}
                  </span>
                </div>
                <div>
                  <span className="text-[#6c5f51] block uppercase tracking-wider text-[10px] sm:text-xs">
                    Screen Accent Token
                  </span>
                  <span className="mt-0.5 flex items-center gap-2 text-[#ffedd7]">
                    <span
                      className="h-2.5 w-2.5 rounded-full"
                      style={{ backgroundColor: activeKin.accent }}
                    />
                    {activeKin.accent}
                  </span>
                </div>
              </div>

              {/* MCP Inhabit Directive */}
              <div className="mt-4 sm:mt-6 rounded-md border border-[#40372e] bg-[#100904] p-2.5 sm:p-3 font-mono text-xs overflow-x-auto">
                <span className="text-[#6c5f51] block text-[9px] sm:text-[10px] uppercase tracking-wider">
                  Direct Telemetry Connection Call
                </span>
                <code className="text-emerald-400 mt-1 block font-semibold text-[11px] sm:text-xs break-all">
                  kinpod_connect(agent_id=&quot;{activeKin.id}&quot;, avatar_id=&quot;{activeKin.id}&quot;)
                </code>
              </div>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}
