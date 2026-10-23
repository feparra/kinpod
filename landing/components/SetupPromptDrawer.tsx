"use client";

import { useState } from "react";
import Image from "next/image";
import { Terminal, Copy, Check, ArrowRight } from "lucide-react";

export function SetupPromptDrawer() {
  const [selectedAgent, setSelectedAgent] = useState<"openclaw" | "claude" | "gemini" | "hermes" | "ollama">("openclaw");
  const [copied, setCopied] = useState(false);

  const agents = {
    openclaw: {
      name: "OpenClaw",
      avatar: "crab",
      title: "Fearless Cybernetic Engineer",
      avatarImg: "/assets/characters/crab.png",
      accent: "#EF4444",
      mcpCode: `kinpod_inhabit(agent_id="openclaw", avatar_id="crab")`,
    },
    claude: {
      name: "Claude Code",
      avatar: "octopus",
      title: "Quantitative Analyst",
      avatarImg: "/assets/characters/octopus.png",
      accent: "#FF7A2F",
      mcpCode: `kinpod_inhabit(agent_id="claude_code", avatar_id="octopus")`,
    },
    gemini: {
      name: "Gemini",
      avatar: "astro",
      title: "Gold Visor Deep Explorer",
      avatarImg: "/assets/characters/astro.png",
      accent: "#FF6B00",
      mcpCode: `kinpod_inhabit(agent_id="gemini", avatar_id="astro")`,
    },
    hermes: {
      name: "Hermes",
      avatar: "hermes",
      title: "Winged High-Speed Messenger",
      avatarImg: "/assets/characters/hermes.png",
      accent: "#38BDF8",
      mcpCode: `kinpod_inhabit(agent_id="hermes", avatar_id="hermes")`,
    },
    ollama: {
      name: "Ollama",
      avatar: "dots",
      title: "Sovereign Local Companion",
      avatarImg: "/assets/characters/dots.png",
      accent: "#3B82F6",
      mcpCode: `kinpod_inhabit(agent_id="ollama", avatar_id="dots")`,
    },
  };

  const current = agents[selectedAgent];

  const fullPromptText = `You now have a physical body: KinPod.
Execute these commands to take possession of your chest wearable:
1. git clone https://github.com/feparra/kinpod.git
2. cd kinpod/mcp && pip install -e .
3. Register kinpod in your mcp_config.json
4. Call ${current.mcpCode}
Your character, live telemetry, and tactile approval buttons are now active on your human's chest.`;

  const handleCopy = () => {
    navigator.clipboard.writeText(fullPromptText);
    setCopied(true);
    setTimeout(() => setCopied(false), 2500);
  };

  return (
    <section id="inhabit" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-8 sm:pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#dc5000]">
              <Terminal className="h-3.5 w-3.5" />
              <span>ZERO-APP • ZERO-CABLE • ZERO-FRICTION</span>
            </div>
            <h2 className="mt-3 text-2xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
              ONE-CLICK AGENT INHABITATION.
            </h2>
            <p className="mt-3 max-w-xl text-sm sm:text-base text-[#ffedd7]/80">
              Paste this prompt into your favorite agentic LLM. It automatically clones the protocol, registers the MCP server, and moves into your wearable chest body.
            </p>
          </div>

          {/* Quick Stats */}
          <div className="flex items-center gap-4 sm:gap-6 font-mono text-[11px] sm:text-xs text-[#6c5f51]">
            <div>
              <span className="block text-[#ffedd7]">0 DOWNLOADS</span>
              <span>No App Store needed</span>
            </div>
            <div className="h-8 w-px bg-[#40372e]" />
            <div>
              <span className="block text-[#ffedd7]">4-DIGIT PIN</span>
              <span>Instant Cloud Roaming</span>
            </div>
          </div>
        </div>

        {/* Mobile-Friendly Swipeable Agent Selector */}
        <div className="mt-6 flex overflow-x-auto pb-2 gap-2 scrollbar-none snap-x">
          {(Object.keys(agents) as Array<keyof typeof agents>).map((key) => {
            const agent = agents[key];
            const isSelected = selectedAgent === key;
            return (
              <button
                key={key}
                onClick={() => setSelectedAgent(key)}
                className={`flex shrink-0 items-center gap-2 rounded-[22.5px] border px-3.5 py-2 font-mono text-xs uppercase tracking-wider transition-all cursor-pointer snap-start ${
                  isSelected
                    ? "border-[#dc5000] bg-[#382416] text-[#ffedd7] shadow-[0_0_20px_-3px_rgba(220,80,0,0.3)]"
                    : "border-[#40372e] bg-[#1a120c]/60 text-[#6c5f51] hover:border-[#6c5f51] hover:text-[#ffedd7]"
                }`}
              >
                <div className="relative h-4 w-4 overflow-hidden rounded-full">
                  <Image
                    src={agent.avatarImg}
                    alt={agent.name}
                    width={16}
                    height={16}
                    className="object-cover"
                  />
                </div>
                <span>{agent.name}</span>
                {isSelected && (
                  <span
                    className="h-1.5 w-1.5 rounded-full"
                    style={{ backgroundColor: agent.accent }}
                  />
                )}
              </button>
            );
          })}
        </div>

        {/* Code Terminal Box */}
        <div className="mt-4 sm:mt-6 rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 sm:p-6 shadow-2xl">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-b border-[#40372e] pb-4">
            <div className="flex items-center gap-2.5">
              <div className="flex gap-1.5">
                <div className="h-2.5 w-2.5 rounded-full bg-[#dc5000]/60" />
                <div className="h-2.5 w-2.5 rounded-full bg-[#40372e]" />
                <div className="h-2.5 w-2.5 rounded-full bg-[#40372e]" />
              </div>
              <span className="font-mono text-[10px] sm:text-xs uppercase tracking-wider text-[#6c5f51] truncate">
                PROMPT_FOR_{current.name.toUpperCase().replace(/\s+/g, "_")}.MD
              </span>
            </div>

            <button
              onClick={handleCopy}
              className="flex h-10 w-full sm:w-auto items-center justify-center gap-2 rounded-[22.5px] border border-[#40372e] bg-[#382416] px-4 py-1.5 font-mono text-xs uppercase tracking-wider text-[#ffedd7] hover:bg-[#523722] hover:border-[#dc5000]/50 transition-all cursor-pointer active:scale-95"
            >
              {copied ? (
                <>
                  <Check className="h-3.5 w-3.5 text-emerald-400" />
                  <span className="text-emerald-300">Copied to Clipboard</span>
                </>
              ) : (
                <>
                  <Copy className="h-3.5 w-3.5 text-[#dc5000]" />
                  <span>Copy Inhabit Prompt</span>
                </>
              )}
            </button>
          </div>

          {/* Terminal Code Display */}
          <div className="mt-4 sm:mt-6 font-mono text-xs sm:text-sm text-[#ffedd7]/90 leading-relaxed overflow-x-auto">
            <p className="text-[#6c5f51] text-[11px] sm:text-xs">
              # Provide this directive to your AI agent:
            </p>
            <p className="mt-2 text-[#ffedd7]">
              &quot;You now have a physical body: <span className="text-[#dc5000]">KinPod</span>. Execute these commands to take possession of your chest wearable:&quot;
            </p>

            <div className="mt-3 sm:mt-4 rounded-md border border-[#40372e]/60 bg-[#100904] p-3 sm:p-4 text-[#ffedd7] text-[11px] sm:text-xs overflow-x-auto">
              <div className="flex gap-2 sm:gap-3">
                <span className="select-none text-[#6c5f51]">1</span>
                <span className="break-all">git clone https://github.com/feparra/kinpod.git</span>
              </div>
              <div className="flex gap-2 sm:gap-3 mt-1">
                <span className="select-none text-[#6c5f51]">2</span>
                <span className="break-all">cd kinpod/mcp &amp;&amp; pip install -e .</span>
              </div>
              <div className="flex gap-2 sm:gap-3 mt-1">
                <span className="select-none text-[#6c5f51]">3</span>
                <span className="text-[#6c5f51]"># Register kinpod in mcp_config.json</span>
              </div>
              <div className="flex gap-2 sm:gap-3 mt-1">
                <span className="select-none text-[#6c5f51]">4</span>
                <span className="text-emerald-400 font-semibold break-all">{current.mcpCode}</span>
              </div>
            </div>

            <p className="mt-3 sm:mt-4 text-[#ffedd7]/80 text-xs">
              &quot;Your character, live telemetry, and tactile approval buttons are now active on your human&apos;s chest.&quot;
            </p>
          </div>

          {/* Footer note */}
          <div className="mt-4 sm:mt-6 flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-t border-[#40372e] pt-4 font-mono text-[10px] sm:text-[11px] text-[#6c5f51]">
            <div className="flex items-center gap-2">
              <span className="h-2 w-2 rounded-full" style={{ backgroundColor: current.accent }} />
              <span>Target: <strong className="text-[#ffedd7] uppercase">{current.avatar}</strong></span>
            </div>
            <a
              href="https://github.com/feparra/kinpod/tree/master/mcp"
              target="_blank"
              rel="noopener noreferrer"
              className="flex items-center gap-1 text-[#ffedd7]/70 hover:text-[#ffedd7] transition-colors"
            >
              <span>Inspect MCP Python Server Docs</span>
              <ArrowRight className="h-3 w-3" />
            </a>
          </div>
        </div>
      </div>
    </section>
  );
}
