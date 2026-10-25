"use client";

import { useState, useEffect } from "react";
import Link from "next/link";
import Image from "next/image";
import {
  BookOpen,
  Terminal,
  Cpu,
  Shield,
  Layers,
  Code,
  Copy,
  Check,
  ArrowLeft,
  ExternalLink,
  ChevronRight,
  Radio,
  Volume2,
  Mic,
  Fingerprint,
  Zap,
  Wrench,
  Sparkles,
  Server,
  Download,
  AlertTriangle,
} from "lucide-react";
import { GithubIcon } from "@/components/icons/GithubIcon";

export default function DocsPage() {
  const [activeSection, setActiveSection] = useState("overview");
  const [copiedCode, setCopiedCode] = useState<string | null>(null);

  const handleCopy = (code: string, id: string) => {
    navigator.clipboard.writeText(code);
    setCopiedCode(id);
    setTimeout(() => setCopiedCode(null), 2500);
  };

  const navItems = [
    { id: "overview", label: "01. Architecture Overview", icon: Layers },
    { id: "hardware", label: "02. Hardware & Pinout", icon: Cpu },
    { id: "mcp", label: "03. MCP Protocol & Tools", icon: Terminal },
    { id: "hitl", label: "04. HITL Sternum Guardian", icon: Shield },
    { id: "kinbridge", label: "05. KinBridge & WebSocket", icon: Server },
    { id: "firmware", label: "06. Firmware & Flashing", icon: Code },
    { id: "bom", label: "07. Hacker BOM & 3D Chassis", icon: Wrench },
  ];

  useEffect(() => {
    const handleScroll = () => {
      const sections = navItems.map((item) => document.getElementById(item.id));
      const scrollPosition = window.scrollY + 200;

      for (let i = sections.length - 1; i >= 0; i--) {
        const section = sections[i];
        if (section && section.offsetTop <= scrollPosition) {
          setActiveSection(navItems[i].id);
          break;
        }
      }
    };

    window.addEventListener("scroll", handleScroll);
    return () => window.removeEventListener("scroll", handleScroll);
  }, []);

  return (
    <div className="min-h-screen bg-[#100904] text-[#ffedd7] flex flex-col font-sans selection:bg-[#dc5000] selection:text-white">
      {/* Top Navigation */}
      <header className="sticky top-0 z-50 w-full border-b border-[#40372e] bg-[#100904]/90 backdrop-blur-md">
        <div className="mx-auto flex max-w-7xl items-center justify-between px-4 sm:px-6 py-3.5 sm:py-4">
          <div className="flex items-center gap-4">
            <Link href="/" className="flex items-center gap-3 group">
              <div className="relative h-8 w-8 overflow-hidden rounded-md border border-[#40372e] bg-[#1a120c] p-1 transition-transform group-hover:scale-105">
                <Image
                  src="/assets/characters/crab.png"
                  alt="KinPod Crab Logo"
                  width={32}
                  height={32}
                  className="object-contain"
                />
              </div>
              <div className="flex flex-col">
                <div className="flex items-center gap-2">
                  <span className="font-mono text-sm sm:text-base font-semibold tracking-wider text-[#ffedd7]">
                    KINPOD<span className="text-[#dc5000]">_</span>
                  </span>
                  <span className="rounded bg-[#382416] px-1.5 py-0.5 font-mono text-[9px] uppercase tracking-wider text-[#dc5000] border border-[#dc5000]/40">
                    DOCS
                  </span>
                </div>
                <span className="font-mono text-[8px] sm:text-[9px] uppercase tracking-widest text-[#6c5f51]">
                  TECHNICAL SPECIFICATION
                </span>
              </div>
            </Link>
          </div>

          <div className="flex items-center gap-3 sm:gap-4 font-mono text-xs">
            <Link
              href="/"
              className="flex items-center gap-1.5 text-[#ffedd7]/70 hover:text-[#ffedd7] transition-colors"
            >
              <ArrowLeft className="h-3.5 w-3.5" />
              <span className="hidden sm:inline">Back to Emulator</span>
            </Link>

            <a
              href="https://github.com/feparra/kinpod"
              target="_blank"
              rel="noopener noreferrer"
              className="flex items-center gap-1.5 rounded-[22.5px] border border-[#40372e] bg-[#1a120c] px-3.5 py-1.5 text-[#ffedd7] hover:border-[#dc5000] transition-colors"
            >
              <GithubIcon className="h-3.5 w-3.5" />
              <span>GitHub</span>
            </a>
          </div>
        </div>
      </header>

      {/* Docs Layout */}
      <div className="mx-auto max-w-7xl w-full px-4 sm:px-6 py-8 sm:py-12 flex-1 flex flex-col md:flex-row gap-8 lg:gap-12">
        {/* Sticky Sidebar */}
        <aside className="w-full md:w-64 lg:w-72 shrink-0">
          <div className="sticky top-24 flex flex-col gap-1 rounded-[14px] border border-[#40372e] bg-[#1a120c] p-3 shadow-lg">
            <div className="px-3 py-2 font-mono text-[10px] uppercase tracking-widest text-[#6c5f51] border-b border-[#40372e]/60 mb-1">
              Table of Contents
            </div>
            {navItems.map((item) => {
              const Icon = item.icon;
              const isActive = activeSection === item.id;
              return (
                <a
                  key={item.id}
                  href={`#${item.id}`}
                  onClick={() => setActiveSection(item.id)}
                  className={`flex items-center gap-2.5 rounded-[8px] px-3 py-2 font-mono text-xs transition-all ${
                    isActive
                      ? "bg-[#382416] text-[#ffedd7] border border-[#dc5000]/60 font-medium"
                      : "text-[#ffedd7]/70 hover:bg-[#100904] hover:text-[#ffedd7]"
                  }`}
                >
                  <Icon className={`h-3.5 w-3.5 shrink-0 ${isActive ? "text-[#dc5000]" : "text-[#6c5f51]"}`} />
                  <span className="truncate">{item.label}</span>
                </a>
              );
            })}

            <div className="mt-4 pt-3 border-t border-[#40372e]/60 flex flex-col gap-2">
              <a
                href="https://github.com/feparra/kinpod/blob/master/AGENTS.md"
                target="_blank"
                rel="noopener noreferrer"
                className="flex items-center justify-between px-3 py-1.5 font-mono text-[11px] text-[#6c5f51] hover:text-[#dc5000] transition-colors"
              >
                <span>AGENTS.md Protocol</span>
                <ExternalLink className="h-3 w-3" />
              </a>
              <a
                href="https://github.com/feparra/kinpod/blob/master/kinpod%20arch.md"
                target="_blank"
                rel="noopener noreferrer"
                className="flex items-center justify-between px-3 py-1.5 font-mono text-[11px] text-[#6c5f51] hover:text-[#dc5000] transition-colors"
              >
                <span>Architecture Whitepaper</span>
                <ExternalLink className="h-3 w-3" />
              </a>
            </div>
          </div>
        </aside>

        {/* Documentation Content Area */}
        <main className="flex-1 min-w-0 flex flex-col gap-16 lg:gap-24 pb-24">
          {/* Header Title Section */}
          <div className="border-b border-[#40372e] pb-8">
            <div className="inline-flex items-center gap-2 rounded-[9999px] border border-[#40372e] bg-[#1a120c] px-3 py-1 font-mono text-[9px] sm:text-[10px] uppercase tracking-widest text-[#dc5000]">
              <Radio className="h-3.5 w-3.5" />
              <span>THE OPEN-SOURCE EMBODIMENT STACK</span>
            </div>
            <h1 className="mt-4 text-3xl sm:text-4xl lg:text-5xl font-medium uppercase tracking-tight text-[#ffedd7]">
              KINPOD SYSTEM ARCHITECTURE &amp; DEVELOPER GUIDE
            </h1>
            <p className="mt-3 text-sm sm:text-base text-[#ffedd7]/80 leading-relaxed font-sans max-w-3xl">
              KinPod is an open-source wearable physical body designed for autonomous AI agents (OpenClaw, Claude Code, Gemini, Hermes). Worn on the chest via a magnetic harness, it provides physical eyes, ears, voice, and an unforgeable hardware approval boundary.
            </p>
          </div>

          {/* Section 1: Overview & Architecture */}
          <section id="overview" className="scroll-mt-24 flex flex-col gap-6">
            <div className="flex items-center gap-3">
              <span className="flex h-7 w-7 items-center justify-center rounded-md bg-[#382416] border border-[#dc5000]/50 font-mono text-xs text-[#dc5000] font-bold">
                01
              </span>
              <h2 className="text-xl sm:text-2xl font-mono uppercase tracking-tight text-[#ffedd7]">
                Architecture Overview
              </h2>
            </div>

            <p className="text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              The KinPod ecosystem operates across three decoupled layers:
            </p>

            {/* 3-Tier Diagram Card */}
            <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-5 sm:p-6 font-mono text-xs">
              <div className="text-[10px] uppercase tracking-widest text-[#6c5f51] mb-3">
                SYSTEM TOPOLOGY: THREE-TIER PIPELINE
              </div>
              <div className="bg-[#100904] border border-[#40372e] rounded-md p-4 text-[11px] leading-relaxed text-[#ffedd7]/90 overflow-x-auto whitespace-pre">
{`┌────────────────────────────────┐       WebSocket Stream (16kHz PCM)      ┌───────────────────────────────┐
│     KinPod Physical Body       │ ◄─────────────────────────────────────► │       KinBridge Hub           │
│ (ESP32-S3 Touch AMOLED 2.06") │    JSON Telemetry & Approvals           │   (FastAPI / Python 3.11+)    │
│  - CST9220 Capacitive Screen   │                                         │  - Google STT / Whisper STT   │
│  - ES7210 PTT Microphone       │                                         │  - Edge-TTS Voice Synthesis   │
│  - ES8311 + NS4150 Speaker     │                                         │  - Cloudflare Roaming Tunnel  │
└────────────────────────────────┘                                         └───────────────┬───────────────┘
                                                                                           │
                                                                   Model Context Protocol  │  (HTTP / SSE / stdio)
                                                                   REST chatCompletions    ▼
                                                                           ┌───────────────────────────────┐
                                                                           │   Autonomous Agent Host       │
                                                                           │ (OpenClaw / Claude / Gemini) │
                                                                           │  - Tool Calls & Reasoning     │
                                                                           │  - HITL Sternum Approval      │
                                                                           └───────────────────────────────┘`}
              </div>
            </div>

            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4">
              <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4">
                <h3 className="font-mono text-xs uppercase font-semibold text-[#dc5000] flex items-center gap-1.5">
                  <Zap className="h-3.5 w-3.5" />
                  Real-Time Duplex Audio
                </h3>
                <p className="mt-2 text-xs text-[#ffedd7]/75 font-sans leading-relaxed">
                  Pressing the physical BOOT button (GPIO 0) engages the ES7210 microphone, streaming binary 16kHz PCM frames over WebSocket. KinBridge transcribes with zero-latency streaming and replies via neural Edge-TTS through the chest speaker.
                </p>
              </div>

              <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4">
                <h3 className="font-mono text-xs uppercase font-semibold text-[#dc5000] flex items-center gap-1.5">
                  <Fingerprint className="h-3.5 w-3.5" />
                  The Tribe of Kin
                </h3>
                <p className="mt-2 text-xs text-[#ffedd7]/75 font-sans leading-relaxed">
                  Support for 6 distinct agent avatars: <strong>Crab</strong> (OpenClaw), <strong>Astro</strong> (Gemini), <strong>Octo</strong> (Claude Code), <strong>Hermes</strong> (Nous Research), <strong>Muse</strong> (GPT-4o), and <strong>Dots</strong> (Local Ollama).
                </p>
              </div>
            </div>
          </section>

          {/* Section 2: Hardware Specifications & Pinout */}
          <section id="hardware" className="scroll-mt-24 flex flex-col gap-6">
            <div className="flex items-center gap-3">
              <span className="flex h-7 w-7 items-center justify-center rounded-md bg-[#382416] border border-[#dc5000]/50 font-mono text-xs text-[#dc5000] font-bold">
                02
              </span>
              <h2 className="text-xl sm:text-2xl font-mono uppercase tracking-tight text-[#ffedd7]">
                Hardware Specifications &amp; Pinouts
              </h2>
            </div>

            <p className="text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              KinPod is built on top of the <strong>Waveshare ESP32-S3 Touch AMOLED 2.06&quot;</strong> development board, featuring an ESP32-S3R8 dual-core Xtensa LX7 SoC with 32MB Flash and 8MB Octal PSRAM.
            </p>

            {/* Pinout Table */}
            <div className="overflow-x-auto rounded-[12px] border border-[#40372e] bg-[#1a120c]">
              <table className="w-full text-left font-mono text-xs">
                <thead className="border-b border-[#40372e] bg-[#100904] text-[10px] uppercase text-[#6c5f51]">
                  <tr>
                    <th className="px-4 py-3">Peripheral Subsystem</th>
                    <th className="px-4 py-3">Pins / GPIO Bus</th>
                    <th className="px-4 py-3">Technical Description</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-[#40372e]/60 text-[#ffedd7]/85">
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">Shared I2C0 Bus</td>
                    <td className="px-4 py-3">SCL=14, SDA=15</td>
                    <td className="px-4 py-3">400kHz shared peripheral bus managed with FreeRTOS mutex.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">PMU AXP2101</td>
                    <td className="px-4 py-3">I2C `0x34`</td>
                    <td className="px-4 py-3">Critical ALDO2 power rail set to 3.3V for AMOLED display. CC 500mA charging.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">AMOLED 2.06&quot;</td>
                    <td className="px-4 py-3">CS=12, SCLK=38, D0-D3=4-7, RST=39</td>
                    <td className="px-4 py-3">410×502 px high-density curved display, CO5300 QSPI controller.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">Capacitive Touch</td>
                    <td className="px-4 py-3">INT=11, RST=40 (I2C `0x5A`)</td>
                    <td className="px-4 py-3">CST9220 multi-touch controller. Powers the sternum HITL approval boundary.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">I2S0 Audio Codec</td>
                    <td className="px-4 py-3">MCLK=42, BCLK=9, LRCK=45, DOUT=8, DIN=10</td>
                    <td className="px-4 py-3">16 kHz 16-bit Duplex: ES7210 microphone ADC + ES8311 speaker DAC.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">Power Amplifier</td>
                    <td className="px-4 py-3">GPIO 46</td>
                    <td className="px-4 py-3">NS4150 Class-D 3W audio amplifier (Active-High enable).</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">PTT Button</td>
                    <td className="px-4 py-3">GPIO 0 (BOOT)</td>
                    <td className="px-4 py-3">Push-To-Talk tactile active-low button with 300ms short-tap rejection.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">Haptic Feedback</td>
                    <td className="px-4 py-3">GPIO 18</td>
                    <td className="px-4 py-3">Miniature ERM tactile vibration motor for physical touch feedback.</td>
                  </tr>
                </tbody>
              </table>
            </div>

            <div className="rounded-[12px] border border-amber-600/30 bg-amber-950/20 p-4 flex gap-3 text-xs text-amber-200/90 font-mono">
              <AlertTriangle className="h-4 w-4 shrink-0 text-amber-400 mt-0.5" />
              <div>
                <strong className="text-amber-300">PMU AXP2101 Power Rail Requirement:</strong> The AMOLED display is powered via the ALDO2 power rail. If ALDO2 is not enabled at 3.3V during power-on initialization, the CO5300 display controller will not respond to QSPI commands.
              </div>
            </div>
          </section>

          {/* Section 3: Model Context Protocol (MCP) */}
          <section id="mcp" className="scroll-mt-24 flex flex-col gap-6">
            <div className="flex items-center gap-3">
              <span className="flex h-7 w-7 items-center justify-center rounded-md bg-[#382416] border border-[#dc5000]/50 font-mono text-xs text-[#dc5000] font-bold">
                03
              </span>
              <h2 className="text-xl sm:text-2xl font-mono uppercase tracking-tight text-[#ffedd7]">
                Model Context Protocol (MCP)
              </h2>
            </div>

            <p className="text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              KinPod exposes an official <strong>Model Context Protocol (MCP)</strong> server conforming to the open Anthropic specification. This enables OpenClaw, Claude Code, Cursor, Windsurf, and Antigravity to treat the physical wearable as a standard peripheral toolset.
            </p>

            {/* MCP Config Snippet */}
            <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 font-mono text-xs">
              <div className="flex items-center justify-between pb-2 mb-2 border-b border-[#40372e]/60">
                <span className="text-[#6c5f51] text-[10px] uppercase tracking-wider">
                  mcp_config.json — AGENT REGISTRATION
                </span>
                <button
                  onClick={() =>
                    handleCopy(
                      JSON.stringify(
                        {
                          mcpServers: {
                            kinpod: {
                              command: "python",
                              args: ["-m", "kinpod_mcp.server"],
                            },
                          },
                        },
                        null,
                        2
                      ),
                      "mcp_cfg"
                    )
                  }
                  className="flex items-center gap-1 text-[#dc5000] hover:text-[#ffedd7] transition-colors"
                >
                  {copiedCode === "mcp_cfg" ? <Check className="h-3.5 w-3.5 text-emerald-400" /> : <Copy className="h-3.5 w-3.5" />}
                  <span className="text-[10px]">{copiedCode === "mcp_cfg" ? "Copied" : "Copy"}</span>
                </button>
              </div>
              <pre className="text-[#ffedd7]/90 overflow-x-auto p-2 bg-[#100904] rounded-md">
{`{
  "mcpServers": {
    "kinpod": {
      "command": "python",
      "args": ["-m", "kinpod_mcp.server"]
    }
  }
}`}
              </pre>
            </div>

            {/* MCP Tools Table */}
            <h3 className="font-mono text-sm font-semibold uppercase tracking-wider text-[#ffedd7] mt-2">
              EXPOSED MCP TOOLS REFERENCE
            </h3>
            <div className="overflow-x-auto rounded-[12px] border border-[#40372e] bg-[#1a120c]">
              <table className="w-full text-left font-mono text-xs">
                <thead className="border-b border-[#40372e] bg-[#100904] text-[10px] uppercase text-[#6c5f51]">
                  <tr>
                    <th className="px-4 py-3">Tool Name</th>
                    <th className="px-4 py-3">Parameters</th>
                    <th className="px-4 py-3">Description</th>
                    <th className="px-4 py-3">Scope</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-[#40372e]/60 text-[#ffedd7]/85">
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">`kinpod_status`</td>
                    <td className="px-4 py-3 text-[#6c5f51]">None</td>
                    <td className="px-4 py-3">Returns battery voltage (mV), Wi-Fi RSSI, link state, and active visual theme.</td>
                    <td className="px-4 py-3 text-emerald-400">Read-Only</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">`kinpod_connect`</td>
                    <td className="px-4 py-3 text-[#ffedd7]/70">agent_id, avatar_id, title</td>
                    <td className="px-4 py-3">Inhabits the wearable, switching the AMOLED display to your agent&apos;s avatar.</td>
                    <td className="px-4 py-3 text-sky-400">Telemetry</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">`kinpod_speak`</td>
                    <td className="px-4 py-3 text-[#ffedd7]/70">text, voice</td>
                    <td className="px-4 py-3">Synthesizes audio via neural TTS and streams speech through chest speaker.</td>
                    <td className="px-4 py-3 text-sky-400">Audio Out</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">`kinpod_listen`</td>
                    <td className="px-4 py-3 text-[#ffedd7]/70">timeout_sec</td>
                    <td className="px-4 py-3">Blocks and waits for wearer to press BOOT button, speaks, and returns transcribed text.</td>
                    <td className="px-4 py-3 text-amber-400">Audio In</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">`kinpod_get_last_speech`</td>
                    <td className="px-4 py-3 text-[#6c5f51]">None</td>
                    <td className="px-4 py-3">Returns the most recent transcribed speech without blocking execution.</td>
                    <td className="px-4 py-3 text-emerald-400">Read-Only</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">`kinpod_request_approval`</td>
                    <td className="px-4 py-3 text-[#ffedd7]/70">action, details, risk, timeout_sec</td>
                    <td className="px-4 py-3"><strong>HITL Guardian:</strong> Displays modal on chest; pauses agent until human taps screen.</td>
                    <td className="px-4 py-3 text-rose-400 font-bold">Security</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">`kinpod_expose_tunnel`</td>
                    <td className="px-4 py-3 text-[#ffedd7]/70">port</td>
                    <td className="px-4 py-3">Spawns a Cloudflare tunnel for zero-configuration cellular hotspot roaming.</td>
                    <td className="px-4 py-3 text-sky-400">Network</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </section>

          {/* Section 4: HITL Sternum Guardian */}
          <section id="hitl" className="scroll-mt-24 flex flex-col gap-6">
            <div className="flex items-center gap-3">
              <span className="flex h-7 w-7 items-center justify-center rounded-md bg-[#382416] border border-[#dc5000]/50 font-mono text-xs text-[#dc5000] font-bold">
                04
              </span>
              <h2 className="text-xl sm:text-2xl font-mono uppercase tracking-tight text-[#ffedd7]">
                Human-In-The-Loop (HITL) Guardian
              </h2>
            </div>

            <p className="text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              As AI agents become autonomous developers capable of executing shell commands, deleting databases, or conducting financial transactions, software confirmation prompts inside chat interfaces are insufficient. KinPod enforces a <strong>physical sternum killswitch</strong>.
            </p>

            <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 font-mono text-xs">
              <div className="flex items-center justify-between pb-2 mb-2 border-b border-[#40372e]/60">
                <span className="text-[#6c5f51] text-[10px] uppercase tracking-wider">
                  agent_script.py — HARDWARE APPROVAL ENFORCEMENT
                </span>
                <button
                  onClick={() =>
                    handleCopy(
                      `# Synchronously pause for physical human confirmation on chest screen
result = await kinpod_request_approval(
    action="deploy_prod",
    details="Deploying commit 7a8b9c to production server.",
    risk="HIGH",
    timeout_sec=60
)
if "APPROVAL GRANTED" in result:
    print("Wearer tapped APPROVE on chest screen. Proceeding.")
    run_deploy()
else:
    print("Action REJECTED or timed out. Aborting gracefully.")
    abort_deploy()`,
                      "hitl_code"
                    )
                  }
                  className="flex items-center gap-1 text-[#dc5000] hover:text-[#ffedd7] transition-colors"
                >
                  {copiedCode === "hitl_code" ? <Check className="h-3.5 w-3.5 text-emerald-400" /> : <Copy className="h-3.5 w-3.5" />}
                  <span className="text-[10px]">{copiedCode === "hitl_code" ? "Copied" : "Copy"}</span>
                </button>
              </div>
              <pre className="text-[#ffedd7]/90 overflow-x-auto p-2 bg-[#100904] rounded-md">
{`# Synchronously pause for physical human confirmation on chest screen
result = await kinpod_request_approval(
    action="deploy_prod",
    details="Deploying commit 7a8b9c to production server.",
    risk="HIGH",
    timeout_sec=60
)
if "APPROVAL GRANTED" in result:
    print("Wearer tapped APPROVE on chest screen. Proceeding.")
    run_deploy()
else:
    print("Action REJECTED or timed out. Aborting gracefully.")
    abort_deploy()`}
              </pre>
            </div>
          </section>

          {/* Section 5: KinBridge & WebSocket Protocol */}
          <section id="kinbridge" className="scroll-mt-24 flex flex-col gap-6">
            <div className="flex items-center gap-3">
              <span className="flex h-7 w-7 items-center justify-center rounded-md bg-[#382416] border border-[#dc5000]/50 font-mono text-xs text-[#dc5000] font-bold">
                05
              </span>
              <h2 className="text-xl sm:text-2xl font-mono uppercase tracking-tight text-[#ffedd7]">
                KinBridge &amp; WebSocket Protocol
              </h2>
            </div>

            <p className="text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              <strong>KinBridge</strong> is a high-performance Python FastAPI daemon that handles the bidirectional WebSocket connection with the ESP32-S3 wearable, local Speech-To-Text (STT), Neural TTS synthesis, and routing to OpenClaw Gateway or Gemini.
            </p>

            <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 font-mono text-xs">
              <div className="flex items-center justify-between pb-2 mb-2 border-b border-[#40372e]/60">
                <span className="text-[#6c5f51] text-[10px] uppercase tracking-wider">
                  TERMINAL — START KINBRIDGE ON PORT 8005
                </span>
                <button
                  onClick={() =>
                    handleCopy(
                      `cd kinpod/server
pip install -r requirements.txt
export KIN_LANG="en"
export OPENCLAW_TOKEN="your_gateway_token"
python -m uvicorn kin_bridge:app --host 0.0.0.0 --port 8005`,
                      "start_bridge"
                    )
                  }
                  className="flex items-center gap-1 text-[#dc5000] hover:text-[#ffedd7] transition-colors"
                >
                  {copiedCode === "start_bridge" ? <Check className="h-3.5 w-3.5 text-emerald-400" /> : <Copy className="h-3.5 w-3.5" />}
                  <span className="text-[10px]">{copiedCode === "start_bridge" ? "Copied" : "Copy"}</span>
                </button>
              </div>
              <pre className="text-[#ffedd7]/90 overflow-x-auto p-2 bg-[#100904] rounded-md">
{`cd kinpod/server
pip install -r requirements.txt

# Configure language & OpenClaw Gateway token
export KIN_LANG="en"
export OPENCLAW_TOKEN="your_gateway_token"

# Start hub listening on port 8005
python -m uvicorn kin_bridge:app --host 0.0.0.0 --port 8005`}
              </pre>
            </div>

            <h3 className="font-mono text-sm font-semibold uppercase tracking-wider text-[#ffedd7] mt-2">
              WEBSOCKET EVENT SCHEMA (/ws/kinpod)
            </h3>
            <div className="grid grid-cols-1 sm:grid-cols-2 gap-4 font-mono text-xs">
              <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4">
                <div className="text-[10px] uppercase text-[#dc5000] mb-2 font-semibold">1. Audio Inbound (PTT)</div>
                <p className="text-[#ffedd7]/70 text-[11px] font-sans mb-3">
                  Wearer presses BOOT (GPIO 0). Wearable sends text `ptt_start`, streams binary 16kHz PCM chunks, and finishes with `ptt_stop`.
                </p>
                <pre className="bg-[#100904] p-2.5 rounded border border-[#40372e] text-[10px] text-[#ffedd7]/90">
{`// 1. Text control frame
{"event": "ptt_start", "kin": "crab"}

// 2. Binary frames (OpCode 0x02)
[16kHz 16-bit Mono PCM chunks]

// 3. Text control frame
{"event": "ptt_stop"}`}
                </pre>
              </div>

              <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4">
                <div className="text-[10px] uppercase text-[#dc5000] mb-2 font-semibold">2. Audio Outbound (TTS)</div>
                <p className="text-[#ffedd7]/70 text-[11px] font-sans mb-3">
                  KinBridge generates TTS response, sends text `reply`, streams binary 2048-byte PCM audio chunks, and signals `tts_end`.
                </p>
                <pre className="bg-[#100904] p-2.5 rounded border border-[#40372e] text-[10px] text-[#ffedd7]/90">
{`// 1. Text control frame
{"event": "reply", "kin": "crab", "text": "..."}

// 2. Binary audio stream
[2048-byte chunks every 30ms]

// 3. Completion event
{"event": "tts_end"}`}
                </pre>
              </div>
            </div>
          </section>

          {/* Section 6: Firmware & Flashing Guide */}
          <section id="firmware" className="scroll-mt-24 flex flex-col gap-6">
            <div className="flex items-center gap-3">
              <span className="flex h-7 w-7 items-center justify-center rounded-md bg-[#382416] border border-[#dc5000]/50 font-mono text-xs text-[#dc5000] font-bold">
                06
              </span>
              <h2 className="text-xl sm:text-2xl font-mono uppercase tracking-tight text-[#ffedd7]">
                Firmware &amp; Flashing Guide
              </h2>
            </div>

            <p className="text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              KinPod runs native C firmware built on <strong>ESP-IDF v5.4 / v6.0</strong> with FreeRTOS. It utilizes a 32MB custom partition table dividing flash into dual OTA banks and a 12MB SPIFFS partition for character animations.
            </p>

            <div className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 font-mono text-xs">
              <div className="flex items-center justify-between pb-2 mb-2 border-b border-[#40372e]/60">
                <span className="text-[#6c5f51] text-[10px] uppercase tracking-wider">
                  ESP-IDF COMPILATION &amp; FLASH
                </span>
                <button
                  onClick={() =>
                    handleCopy(
                      `cd kinpod/firmware
idf.py set-target esp32s3
idf.py build
idf.py -p COM5 flash monitor`,
                      "idf_flash"
                    )
                  }
                  className="flex items-center gap-1 text-[#dc5000] hover:text-[#ffedd7] transition-colors"
                >
                  {copiedCode === "idf_flash" ? <Check className="h-3.5 w-3.5 text-emerald-400" /> : <Copy className="h-3.5 w-3.5" />}
                  <span className="text-[10px]">{copiedCode === "idf_flash" ? "Copied" : "Copy"}</span>
                </button>
              </div>
              <pre className="text-[#ffedd7]/90 overflow-x-auto p-2 bg-[#100904] rounded-md">
{`# 1. Enter firmware directory
cd kinpod/firmware

# 2. Build binaries with ESP-IDF
idf.py set-target esp32s3
idf.py build

# 3. Flash to board on serial COM port (e.g. COM5 or /dev/ttyUSB0)
idf.py -p COM5 flash monitor`}
              </pre>
            </div>

            {/* Serial Configuration Channel */}
            <h3 className="font-mono text-sm font-semibold uppercase tracking-wider text-[#ffedd7] mt-2">
              USB SERIAL CONFIGURATION COMMANDS
            </h3>
            <p className="text-xs text-[#ffedd7]/75 font-sans">
              The firmware exposes a live interactive serial command channel at 115200 baud over the native USB CDC port:
            </p>
            <div className="overflow-x-auto rounded-[12px] border border-[#40372e] bg-[#1a120c]">
              <table className="w-full text-left font-mono text-xs">
                <thead className="border-b border-[#40372e] bg-[#100904] text-[10px] uppercase text-[#6c5f51]">
                  <tr>
                    <th className="px-4 py-2.5">Serial Command</th>
                    <th className="px-4 py-2.5">Arguments</th>
                    <th className="px-4 py-2.5">Description</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-[#40372e]/60 text-[#ffedd7]/85">
                  <tr>
                    <td className="px-4 py-2.5 text-[#dc5000]">`SET_WS &lt;url&gt;`</td>
                    <td className="px-4 py-2.5">wss://agent.thekinpod.com/ws/kinpod</td>
                    <td className="px-4 py-2.5">Stores WebSocket URL in NVS and immediately reconnects.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-2.5 text-[#dc5000]">`SET_WIFI &lt;ssid&gt; &lt;pass&gt;`</td>
                    <td className="px-4 py-2.5">MyNetwork MyPassword</td>
                    <td className="px-4 py-2.5">Stores Wi-Fi credentials in encrypted NVS partition.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-2.5 text-[#dc5000]">`GET_CONFIG`</td>
                    <td className="px-4 py-2.5">None</td>
                    <td className="px-4 py-2.5">Returns currently configured SSID and WebSocket URL.</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-2.5 text-[#dc5000]">`PAIR`</td>
                    <td className="px-4 py-2.5">None</td>
                    <td className="px-4 py-2.5">Displays the 4-digit pairing keypad on the AMOLED display.</td>
                  </tr>
                </tbody>
              </table>
            </div>
          </section>

          {/* Section 7: Hacker BOM & 3D Chassis */}
          <section id="bom" className="scroll-mt-24 flex flex-col gap-6">
            <div className="flex items-center gap-3">
              <span className="flex h-7 w-7 items-center justify-center rounded-md bg-[#382416] border border-[#dc5000]/50 font-mono text-xs text-[#dc5000] font-bold">
                07
              </span>
              <h2 className="text-xl sm:text-2xl font-mono uppercase tracking-tight text-[#ffedd7]">
                Hacker Bill of Materials (BOM) &amp; 3D Mount
              </h2>
            </div>

            <p className="text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              KinPod is 100% buildable using commercially available off-the-shelf components. Zero custom silicon, zero proprietary connectors.
            </p>

            <div className="overflow-x-auto rounded-[12px] border border-[#40372e] bg-[#1a120c]">
              <table className="w-full text-left font-mono text-xs">
                <thead className="border-b border-[#40372e] bg-[#100904] text-[10px] uppercase text-[#6c5f51]">
                  <tr>
                    <th className="px-4 py-3">Component</th>
                    <th className="px-4 py-3">Part Specification</th>
                    <th className="px-4 py-3">Vendor / Source</th>
                  </tr>
                </thead>
                <tbody className="divide-y divide-[#40372e]/60 text-[#ffedd7]/85">
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">Core MCU &amp; AMOLED</td>
                    <td className="px-4 py-3">Waveshare ESP32-S3 Touch AMOLED 2.06&quot;</td>
                    <td className="px-4 py-3">Waveshare / AliExpress / Amazon (~$34 USD)</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">Battery Cell</td>
                    <td className="px-4 py-3">3.7V 500mAh–1000mAh LiPo Cell (JST 1.25mm)</td>
                    <td className="px-4 py-3">Adafruit / MakerFocus (~$6 USD)</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">Chest Magnets</td>
                    <td className="px-4 py-3">Neodymium N52 Disc Magnets (10mm × 2mm, 4×)</td>
                    <td className="px-4 py-3">K&amp;J Magnetics / Amazon (~$5 USD)</td>
                  </tr>
                  <tr>
                    <td className="px-4 py-3 font-semibold text-[#dc5000]">3D Printable Chassis</td>
                    <td className="px-4 py-3">PETG / Tough Resin, 0.16mm layer height</td>
                    <td className="px-4 py-3">
                      <a href="https://github.com/feparra/kinpod/tree/master/hardware" target="_blank" rel="noopener noreferrer" className="text-[#dc5000] hover:underline flex items-center gap-1">
                        <span>hardware/cad/</span>
                        <ExternalLink className="h-3 w-3" />
                      </a>
                    </td>
                  </tr>
                </tbody>
              </table>
            </div>

            <div className="mt-4 p-6 rounded-[12px] border border-[#40372e] bg-[#1a120c] flex flex-col sm:flex-row items-center justify-between gap-4">
              <div>
                <h4 className="font-mono text-sm font-semibold uppercase text-[#ffedd7]">
                  Have improvements or questions?
                </h4>
                <p className="mt-1 text-xs text-[#ffedd7]/70 font-sans">
                  KinPod is community-driven. File an issue, submit a PR, or join the discussion on GitHub.
                </p>
              </div>

              <a
                href="https://github.com/feparra/kinpod"
                target="_blank"
                rel="noopener noreferrer"
                className="flex h-11 items-center justify-center gap-2 rounded-[36px] bg-[#382416] px-6 font-mono text-xs font-semibold uppercase tracking-wider text-[#ffedd7] border border-[#40372e] hover:bg-[#523722] hover:border-[#dc5000] transition-all shrink-0"
              >
                <GithubIcon className="h-4 w-4" />
                <span>Contribute on GitHub</span>
              </a>
            </div>
          </section>
        </main>
      </div>
    </div>
  );
}
