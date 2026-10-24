"use client";

import { useState, useEffect, useRef } from "react";
import Image from "next/image";
import { Mic, Volume2, ShieldAlert, Wifi, Battery, Play, Terminal, CheckCircle2, XCircle, Sparkles, RefreshCw, Layers } from "lucide-react";

export function KinPodLiveConsole() {
  const [activeAvatar, setActiveAvatar] = useState<"crab" | "astro" | "octopus" | "hermes" | "muse" | "dots">("crab");
  const [deviceState, setDeviceState] = useState<"IDLE" | "LISTENING" | "THINKING" | "SPEAKING" | "HITL_ALERT">("IDLE");
  const [isPttPressed, setIsPttPressed] = useState(false);
  const [vuLevel, setVuLevel] = useState<number[]>([15, 30, 20, 45, 25]);
  const [subtitle, setSubtitle] = useState("KinPod Online. Ready for agent inhabitation.");
  const [terminalLogs, setTerminalLogs] = useState<string[]>([
    "[SYSTEM] ESP32-S3 boot verified. FreeRTOS Mutex initialized.",
    "[AXP2101] ALDO2 rail @ 3.3V — 2.06\" AMOLED active (410x502).",
    "[WS_BRIDGE] Connected: wss://agent.thekinpod.com/ws/kinpod",
    "[MCP] Agent registered: openclaw (Claw / Crab avatar loaded)",
  ]);

  const pttIntervalRef = useRef<NodeJS.Timeout | null>(null);

  const avatars = {
    crab: {
      name: "CRAB",
      agent: "OpenClaw",
      accent: "#EF4444",
      image: "/assets/characters/crab.png",
      tagline: "Fearless Cybernetic Engineer",
      defaultQuote: "Ready to inspect code diffs and commit changes.",
    },
    astro: {
      name: "ASTRO",
      agent: "Gemini",
      accent: "#FF6B00",
      image: "/assets/characters/astro.png",
      tagline: "Gold Visor Space Explorer",
      defaultQuote: "Multimodal live telemetry stream established.",
    },
    octopus: {
      name: "OCTO-CLAUDE",
      agent: "Claude Code",
      accent: "#FF7A2F",
      image: "/assets/characters/octopus.png",
      tagline: "Quantitative Analyst",
      defaultQuote: "Algorithmic synthesis and HITL boundary active.",
    },
    hermes: {
      name: "HERMES",
      agent: "Hermes Agent",
      accent: "#38BDF8",
      image: "/assets/characters/hermes.png",
      tagline: "Winged High-Speed Messenger",
      defaultQuote: "Zero-latency tool routing dispatched.",
    },
    muse: {
      name: "MUSE",
      agent: "GPT-4o Creative",
      accent: "#FBBF24",
      image: "/assets/characters/muse.png",
      tagline: "Conversational Companion",
      defaultQuote: "Warm acoustic presence active on your chest.",
    },
    dots: {
      name: "DOTS",
      agent: "Ollama Local",
      accent: "#3B82F6",
      image: "/assets/characters/dots.png",
      tagline: "Sovereign Offline Intelligence",
      defaultQuote: "Operating 100% locally. Zero telemetry.",
    },
  };

  const current = avatars[activeAvatar];

  const addLog = (msg: string) => {
    setTerminalLogs((prev) => [...prev.slice(-7), msg]);
  };

  // Keyboard shortcut: Spacebar holds PTT
  useEffect(() => {
    const handleKeyDown = (e: KeyboardEvent) => {
      if (e.code === "Space" && e.target === document.body && !isPttPressed) {
        e.preventDefault();
        startPtt();
      }
    };
    const handleKeyUp = (e: KeyboardEvent) => {
      if (e.code === "Space" && e.target === document.body && isPttPressed) {
        e.preventDefault();
        stopPtt();
      }
    };
    window.addEventListener("keydown", handleKeyDown);
    window.addEventListener("keyup", handleKeyUp);
    return () => {
      window.removeEventListener("keydown", handleKeyDown);
      window.removeEventListener("keyup", handleKeyUp);
    };
  }, [isPttPressed]);

  const startPtt = () => {
    setIsPttPressed(true);
    setDeviceState("LISTENING");
    setSubtitle("🎙️ Listening... (Hardware PTT GPIO18 Active)");
    addLog("[I2S_MIC] ES7210 PTT triggered. Streaming 16kHz 16-bit PCM...");

    pttIntervalRef.current = setInterval(() => {
      setVuLevel([
        Math.floor(Math.random() * 80) + 15,
        Math.floor(Math.random() * 95) + 20,
        Math.floor(Math.random() * 70) + 10,
        Math.floor(Math.random() * 90) + 25,
        Math.floor(Math.random() * 60) + 15,
      ]);
    }, 120);
  };

  const stopPtt = () => {
    setIsPttPressed(false);
    if (pttIntervalRef.current) clearInterval(pttIntervalRef.current);
    setVuLevel([15, 20, 15, 25, 15]);
    setDeviceState("THINKING");
    setSubtitle("⚡ Agent processing speech token stream...");
    addLog("[NLP_ROUTER] Audio payload dispatched to " + current.agent);

    setTimeout(() => {
      setDeviceState("SPEAKING");
      setSubtitle(`"${current.defaultQuote}"`);
      addLog(`[ES8311] PA Amp active (GPIO46). Synthesizing voice response.`);

      setTimeout(() => {
        setDeviceState("IDLE");
        setSubtitle("KinPod standby. 2.06\" AMOLED auto-dim primed.");
        addLog("[PMU] Inactivity timer reset. AMOLED running at nominal draw.");
      }, 3500);
    }, 1500);
  };

  const switchAvatar = (id: keyof typeof avatars) => {
    setActiveAvatar(id);
    addLog(`[MCP] Tool call: kinpod_inhabit(avatar="${id}", agent="${avatars[id].agent}")`);
    setSubtitle(`Inhabited by ${avatars[id].name}. Physical body claimed.`);
    setDeviceState("IDLE");
  };

  const triggerHitlApproval = () => {
    setDeviceState("HITL_ALERT");
    setSubtitle("⚠️ CRITICAL: Autonomous Agent Request Paused.");
    addLog("[HITL_SECURITY] Interrupt fired: CST9220 waiting for wearer chest tap.");
  };

  const resolveHitl = (approved: boolean) => {
    if (approved) {
      addLog("[HITL_SECURITY] APPROVAL GRANTED by wearer chest touch. Operation executed.");
      setSubtitle("✅ Approved. Agent resumed execution.");
    } else {
      addLog("[HITL_SECURITY] ACTION REJECTED by wearer. Agent execution aborted.");
      setSubtitle("❌ Rejected. Zero unauthorized mutations.");
    }
    setDeviceState("IDLE");
  };

  return (
    <div className="relative mx-auto max-w-7xl px-4 sm:px-6 pt-4 pb-16">
      {/* Console Headline */}
      <div className="mb-8 flex flex-col md:flex-row md:items-end justify-between gap-4 border-b border-[#40372e] pb-6">
        <div>
          <div className="inline-flex items-center gap-2 rounded-full border border-[#dc5000]/40 bg-[#382416]/50 px-3 py-1 font-mono text-[10px] uppercase tracking-wider text-[#ffedd7]">
            <span className="h-2 w-2 rounded-full bg-emerald-400 animate-pulse" />
            <span>KINPOD HARDWARE EMULATOR v6.0</span>
            <span className="text-[#6c5f51]">•</span>
            <span className="text-emerald-400">DIGITAL TWIN LIVE</span>
          </div>
          <h2 className="mt-3 text-2xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
            THE AGENT EMBODIMENT CONSOLE.
          </h2>
          <p className="mt-2 text-sm sm:text-base text-[#ffedd7]/80 max-w-2xl">
            Test the live hardware without flashing. Hold PTT to speak, switch agent inhabitants via MCP, or trigger a physical chest approval interrupt.
          </p>
        </div>

        {/* Quick Shortcut Tip */}
        <div className="hidden lg:flex items-center gap-2 font-mono text-xs text-[#6c5f51] bg-[#1a120c] border border-[#40372e] px-4 py-2 rounded-lg">
          <span className="rounded bg-[#382416] px-2 py-0.5 text-[#ffedd7]">SPACEBAR</span>
          <span>Hold to Push-To-Talk</span>
        </div>
      </div>

      {/* Main Console Grid */}
      <div className="grid grid-cols-1 lg:grid-cols-12 gap-8 items-start">
        {/* Left Column: Authentic 3D Device Silhouette */}
        <div className="lg:col-span-5 flex flex-col items-center">
          {/* Physical Wearable Device Chassis */}
          <div className="relative w-full max-w-[320px] rounded-[38px] border-4 border-[#332a24] bg-gradient-to-b from-[#1c1612] via-[#0d0a08] to-[#14100d] p-4 shadow-[0_25px_60px_-15px_rgba(0,0,0,0.95)]">
            {/* Top Lanyard Loop */}
            <div className="absolute -top-3 left-1/2 -translate-x-1/2 h-3.5 w-14 rounded-t-lg border-2 border-b-0 border-[#40372e] bg-[#1a120c]" />

            {/* Lateral PTT Physical Button (KEY3 / GPIO18) */}
            <button
              onMouseDown={startPtt}
              onMouseUp={stopPtt}
              onTouchStart={startPtt}
              onTouchEnd={stopPtt}
              className={`absolute -right-3 top-28 h-20 w-3 rounded-r-md border border-l-0 transition-all cursor-pointer ${
                isPttPressed
                  ? "bg-[#ff5500] shadow-[0_0_20px_#ff6b00] translate-x-0.5"
                  : "bg-gradient-to-r from-[#2a221b] to-[#dc5000] hover:brightness-125"
              }`}
              title="Push-To-Talk (KEY3)"
            />

            {/* 2.06" Curved AMOLED Display (410x502) */}
            <div
              className="relative aspect-[410/502] w-full overflow-hidden rounded-[24px] border-2 bg-[#000000] p-4 flex flex-col justify-between transition-all duration-300 shadow-inner"
              style={{
                borderColor: deviceState === "HITL_ALERT" ? "#EF4444" : current.accent,
                boxShadow: `0 0 30px -5px ${current.accent}30`,
              }}
            >
              {/* AMOLED Status Bar */}
              <div className="flex items-center justify-between font-mono text-[9px] text-[#ffedd7]/70 border-b border-white/10 pb-1.5 z-10">
                <div className="flex items-center gap-1.5">
                  <span className="h-1.5 w-1.5 rounded-full bg-emerald-400" />
                  <span className="font-semibold text-white uppercase">{current.agent}</span>
                </div>
                <div className="flex items-center gap-2">
                  <span className="flex items-center gap-1">
                    <Wifi className="h-2.5 w-2.5 text-emerald-400" />
                    -41dBm
                  </span>
                  <span className="flex items-center gap-1">
                    <Battery className="h-2.5 w-2.5 text-amber-400" />
                    94%
                  </span>
                </div>
              </div>

              {/* Screen Content: Avatar or HITL Approval Modal */}
              {deviceState === "HITL_ALERT" ? (
                /* HITL Tactile Approval Screen */
                <div className="flex flex-col items-center justify-center text-center my-auto py-2 animate-fade-in">
                  <div className="rounded-full bg-red-500/20 px-2 py-0.5 font-mono text-[8px] font-bold text-red-400 uppercase tracking-wider">
                    CRITICAL ACTION
                  </div>
                  <h4 className="mt-1 font-mono text-xs font-bold uppercase text-white">
                    Approval Required
                  </h4>
                  <div className="mt-2 w-full rounded bg-zinc-950 p-2 font-mono text-[10px] text-emerald-400 break-all text-left border border-white/10">
                    $ rm -rf /production-db
                  </div>
                  <p className="mt-2 text-[10px] text-zinc-300">
                    Tap chest screen to authorize agent execution:
                  </p>

                  <div className="mt-3 grid grid-cols-2 gap-2 w-full">
                    <button
                      onClick={() => resolveHitl(false)}
                      className="flex h-10 items-center justify-center rounded-lg bg-red-950/80 border border-red-500/60 font-mono text-[10px] font-bold text-red-300 active:scale-95 cursor-pointer"
                    >
                      <XCircle className="h-3.5 w-3.5 mr-1" />
                      REJECT
                    </button>
                    <button
                      onClick={() => resolveHitl(true)}
                      className="flex h-10 items-center justify-center rounded-lg bg-emerald-600 border border-emerald-400 font-mono text-[10px] font-bold text-white active:scale-95 cursor-pointer shadow-[0_0_15px_rgba(16,185,129,0.5)]"
                    >
                      <CheckCircle2 className="h-3.5 w-3.5 mr-1" />
                      APPROVE
                    </button>
                  </div>
                </div>
              ) : (
                /* Normal Avatar Face View */
                <div className="flex flex-col items-center justify-center my-auto relative">
                  {/* Glowing Aura */}
                  <div
                    className={`absolute h-36 w-36 rounded-full blur-2xl transition-all duration-500 ${
                      deviceState === "SPEAKING"
                        ? "scale-125 opacity-70"
                        : deviceState === "LISTENING"
                        ? "scale-110 opacity-60"
                        : "scale-90 opacity-30"
                    }`}
                    style={{ backgroundColor: current.accent }}
                  />

                  {/* Character Artwork */}
                  <div className="relative h-32 w-32 z-10 transition-transform duration-300">
                    <Image
                      src={current.image}
                      alt={current.name}
                      fill
                      className={`object-contain filter drop-shadow-[0_8px_16px_rgba(0,0,0,0.8)] ${
                        deviceState === "SPEAKING" ? "animate-pulse" : ""
                      }`}
                    />
                  </div>

                  {/* State Pill */}
                  <div
                    className="mt-2 rounded-full px-2.5 py-0.5 font-mono text-[9px] font-bold uppercase tracking-wider z-10"
                    style={{ backgroundColor: `${current.accent}30`, color: current.accent }}
                  >
                    ● {deviceState}
                  </div>

                  {/* VU Level Indicator (Active when listening/speaking) */}
                  {(deviceState === "LISTENING" || deviceState === "SPEAKING") && (
                    <div className="mt-2 flex items-center gap-1 z-10">
                      {vuLevel.map((lvl, idx) => (
                        <div
                          key={idx}
                          className="w-1 rounded-full transition-all duration-75"
                          style={{
                            height: `${lvl * 0.25}px`,
                            backgroundColor: current.accent,
                          }}
                        />
                      ))}
                    </div>
                  )}
                </div>
              )}

              {/* Subtitle / Telemetry Box */}
              <div className="rounded-lg bg-black/80 border border-white/10 p-2 font-mono text-[9px] text-[#ffedd7]/90 leading-tight min-h-[46px] flex items-center z-10">
                <span className="truncate">{subtitle}</span>
              </div>
            </div>

            {/* Bottom Acoustic CNC Slits */}
            <div className="mt-3 flex justify-center gap-1.5 opacity-60">
              <div className="h-1 w-3 rounded-full bg-[#40372e]" />
              <div className="h-1 w-6 rounded-full bg-[#40372e]" />
              <div className="h-1 w-3 rounded-full bg-[#40372e]" />
            </div>
          </div>

          <span className="mt-3 font-mono text-[10px] uppercase tracking-widest text-[#6c5f51]">
            TAP OR HOLD PTT SWITCH ON RIGHT BEZEL
          </span>
        </div>

        {/* Right Column: Interactive Deck & Event Stream */}
        <div className="lg:col-span-7 flex flex-col gap-6">
          {/* Deck Section 1: Inhabit an Agent */}
          <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-5 shadow-xl">
            <div className="flex items-center justify-between border-b border-[#40372e] pb-3">
              <div className="flex items-center gap-2">
                <Layers className="h-4 w-4 text-[#dc5000]" />
                <h3 className="font-mono text-xs uppercase tracking-wider font-semibold text-[#ffedd7]">
                  01. INHABIT PHYSICAL BODY VIA MCP
                </h3>
              </div>
              <span className="font-mono text-[10px] text-[#6c5f51] uppercase">
                INSTANT AVATAR MORPH
              </span>
            </div>

            <div className="mt-4 grid grid-cols-3 sm:grid-cols-6 gap-2">
              {(Object.keys(avatars) as Array<keyof typeof avatars>).map((key) => {
                const av = avatars[key];
                const isSelected = activeAvatar === key;
                return (
                  <button
                    key={key}
                    onClick={() => switchAvatar(key)}
                    className={`flex flex-col items-center rounded-lg border p-2 text-center transition-all cursor-pointer ${
                      isSelected
                        ? "border-[#dc5000] bg-[#382416] scale-105 shadow-[0_0_15px_-3px_rgba(220,80,0,0.4)]"
                        : "border-[#40372e] bg-[#100904] text-[#6c5f51] hover:text-[#ffedd7] hover:border-[#6c5f51]"
                    }`}
                  >
                    <div className="relative h-8 w-8">
                      <Image src={av.image} alt={av.name} fill className="object-contain" />
                    </div>
                    <span className="mt-1 font-mono text-[10px] font-semibold text-[#ffedd7]">
                      {av.name}
                    </span>
                    <span className="text-[8px] font-mono uppercase" style={{ color: av.accent }}>
                      {av.agent.split(" ")[0]}
                    </span>
                  </button>
                );
              })}
            </div>
          </div>

          {/* Deck Section 2: Actions & Simulation Triggers */}
          <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-5 shadow-xl">
            <div className="flex items-center justify-between border-b border-[#40372e] pb-3">
              <div className="flex items-center gap-2">
                <Terminal className="h-4 w-4 text-[#dc5000]" />
                <h3 className="font-mono text-xs uppercase tracking-wider font-semibold text-[#ffedd7]">
                  02. TEST PHYSICAL SIGNALS & SECURITY
                </h3>
              </div>
              <span className="font-mono text-[10px] text-[#6c5f51] uppercase">
                HARDWARE INTERRUPTS
              </span>
            </div>

            <div className="mt-4 grid grid-cols-1 sm:grid-cols-3 gap-3">
              {/* Push To Talk Button */}
              <button
                onMouseDown={startPtt}
                onMouseUp={stopPtt}
                onTouchStart={startPtt}
                onTouchEnd={stopPtt}
                className={`flex h-12 items-center justify-center gap-2 rounded-lg border font-mono text-xs uppercase font-semibold transition-all cursor-pointer ${
                  isPttPressed
                    ? "bg-[#ff5500] text-white border-[#ff5500] shadow-[0_0_20px_#ff6b00]"
                    : "bg-[#382416] border-[#40372e] text-[#ffedd7] hover:bg-[#523722]"
                }`}
              >
                <Mic className="h-4 w-4 text-[#dc5000]" />
                <span>{isPttPressed ? "LISTENING..." : "HOLD PTT TO TALK"}</span>
              </button>

              {/* Speak Test */}
              <button
                onClick={() => {
                  setDeviceState("SPEAKING");
                  setSubtitle(`"${current.defaultQuote}"`);
                  addLog(`[MCP] Tool call: kinpod_speak(text="${current.defaultQuote}")`);
                  setTimeout(() => setDeviceState("IDLE"), 3000);
                }}
                className="flex h-12 items-center justify-center gap-2 rounded-lg border border-[#40372e] bg-[#100904] font-mono text-xs uppercase font-semibold text-[#ffedd7] hover:border-[#6c5f51] hover:bg-[#1c140e] transition-all cursor-pointer"
              >
                <Volume2 className="h-4 w-4 text-amber-400" />
                <span>SPEAK TEST</span>
              </button>

              {/* HITL Killswitch Test */}
              <button
                onClick={triggerHitlApproval}
                className="flex h-12 items-center justify-center gap-2 rounded-lg border border-red-900/60 bg-red-950/40 font-mono text-xs uppercase font-semibold text-red-300 hover:bg-red-900/60 transition-all cursor-pointer shadow-[0_0_15px_rgba(239,68,68,0.2)]"
              >
                <ShieldAlert className="h-4 w-4 text-red-400" />
                <span>FIRE HITL ALERT</span>
              </button>
            </div>
          </div>

          {/* Deck Section 3: Live Telemetry Terminal Stream */}
          <div className="rounded-[12px] border border-[#40372e] bg-[#0d0906] p-4 font-mono text-xs shadow-2xl">
            <div className="flex items-center justify-between border-b border-[#40372e] pb-2 text-[10px] text-[#6c5f51] uppercase">
              <span className="flex items-center gap-1.5 text-emerald-400">
                <span className="h-2 w-2 rounded-full bg-emerald-400 animate-ping" />
                LIVE PROTOCOL LOG STREAM (ESP32-S3 ◄► MCP)
              </span>
              <span>115200 BAUD</span>
            </div>

            <div className="mt-3 flex flex-col gap-1.5 font-mono text-[11px] text-[#ffedd7]/80 h-36 overflow-y-auto">
              {terminalLogs.map((log, idx) => (
                <div key={idx} className="flex gap-2 leading-relaxed">
                  <span className="select-none text-[#6c5f51]">&gt;</span>
                  <span
                    className={
                      log.includes("HITL")
                        ? "text-red-400 font-semibold"
                        : log.includes("MCP")
                        ? "text-emerald-400"
                        : log.includes("I2S")
                        ? "text-amber-400"
                        : "text-[#ffedd7]/90"
                    }
                  >
                    {log}
                  </span>
                </div>
              ))}
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}
