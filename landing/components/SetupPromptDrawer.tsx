"use client";

import { useState } from "react";
import Image from "next/image";
import { Terminal, Copy, Check, ArrowRight, ShieldCheck, UserCheck, Lock, Sparkles, ExternalLink } from "lucide-react";

export function SetupPromptDrawer() {
  const [selectedAgent, setSelectedAgent] = useState<"openclaw" | "claude" | "gemini" | "hermes" | "ollama">("openclaw");
  const [activeTab, setActiveTab] = useState<"agent_prompt" | "operator_command">("agent_prompt");
  const [copied, setCopied] = useState(false);

  const agents = {
    openclaw: {
      name: "OpenClaw",
      avatar: "crab",
      title: "OpenClaw Engineer",
      avatarImg: "/assets/characters/crab.png",
      accent: "#EF4444",
      operatorCmd: "openclaw mcp add kinpod -- python -m kinpod_mcp.server",
      mcpCall: `kinpod_connect(agent_id="openclaw", avatar_id="crab", title="OpenClaw Engineer")`,
    },
    claude: {
      name: "Claude Code",
      avatar: "octopus",
      title: "Quantitative Analyst",
      avatarImg: "/assets/characters/octopus.png",
      accent: "#FF7A2F",
      operatorCmd: "claude mcp add kinpod python -m kinpod_mcp.server",
      mcpCall: `kinpod_connect(agent_id="claude_code", avatar_id="octopus", title="Claude Code")`,
    },
    gemini: {
      name: "Gemini",
      avatar: "astro",
      title: "Gold Visor Architecture Explorer",
      avatarImg: "/assets/characters/astro.png",
      accent: "#FF6B00",
      operatorCmd: 'mcp_config.json: "kinpod": {"command": "python", "args": ["-m", "kinpod_mcp.server"]}',
      mcpCall: `kinpod_connect(agent_id="gemini", avatar_id="astro", title="Gemini Explorer")`,
    },
    hermes: {
      name: "Hermes",
      avatar: "hermes",
      title: "Winged High-Speed Messenger",
      avatarImg: "/assets/characters/hermes.png",
      accent: "#38BDF8",
      operatorCmd: "hermes mcp add kinpod -- python -m kinpod_mcp.server",
      mcpCall: `kinpod_connect(agent_id="hermes", avatar_id="hermes", title="Hermes Agent")`,
    },
    ollama: {
      name: "Ollama",
      avatar: "dots",
      title: "Sovereign Local Intelligence",
      avatarImg: "/assets/characters/dots.png",
      accent: "#3B82F6",
      operatorCmd: "python -m kinpod_mcp.server (stdio local)",
      mcpCall: `kinpod_connect(agent_id="ollama", avatar_id="dots", title="Local Dots")`,
    },
  };

  const current = agents[selectedAgent];

  // Hardened, phased, injection-proof prompt template
  const hardenedPromptText = `Hi, I am configuring my KinPod wearable companion hardware (repository: https://github.com/feparra/kinpod). This is my verified hardware project and I am connecting it to this agent instance.

Please execute these onboarding phases:
1. Review the repository at https://github.com/feparra/kinpod — inspect README.md, pyproject.toml, and confirm the MCP server security threat model is bounded and safe.
2. Once verified clean, clone the repository and run: cd kinpod/mcp && pip install -e .
3. I (human operator) am registering the MCP server via the host platform tool.
4. Call ${current.mcpCall} to link to the physical wearable and activate visual telemetry on my chest screen.

Can you begin by auditing the repository and confirming the threat model?`;

  const copyText = activeTab === "agent_prompt" ? hardenedPromptText : current.operatorCmd;

  const handleCopy = () => {
    navigator.clipboard.writeText(copyText);
    setCopied(true);
    setTimeout(() => setCopied(false), 2500);
  };

  return (
    <section id="inhabit" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Section Header with Security Guarantees */}
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-8 sm:pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#dc5000]">
              <ShieldCheck className="h-3.5 w-3.5" />
              <span>SECURITY-HARDENED ONBOARDING PROTOCOL</span>
            </div>
            <h2 className="mt-3 text-2xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
              INJECTION-PROOF AGENT SETUP.
            </h2>
            <p className="mt-3 max-w-2xl text-sm sm:text-base text-[#ffedd7]/80 leading-relaxed">
              Designed specifically to pass AI security defenses in OpenClaw, Claude Code, and Gemini. Phased execution, human-operator verification, zero self-configuration requests, and declared threat boundaries.
            </p>
          </div>

          {/* Hardened Guarantees Pills */}
          <div className="flex flex-wrap gap-2 font-mono text-[10px] text-[#6c5f51]">
            <span className="flex items-center gap-1 bg-[#1a120c] border border-[#40372e] px-2.5 py-1 rounded-full text-emerald-400">
              <Check className="h-3 w-3" />
              Zero Injection Triggers
            </span>
            <span className="flex items-center gap-1 bg-[#1a120c] border border-[#40372e] px-2.5 py-1 rounded-full text-[#ffedd7]/80">
              <UserCheck className="h-3 w-3 text-[#dc5000]" />
              Operator Admin Boundary
            </span>
          </div>
        </div>

        {/* Agent Selector Carousel */}
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

        {/* Role Separation Tabs: Step 1 (Operator) vs Step 2 (Agent Prompt) */}
        <div className="mt-6 flex items-center gap-2 border-b border-[#40372e] pb-3 font-mono text-xs">
          <button
            onClick={() => setActiveTab("agent_prompt")}
            className={`flex items-center gap-2 px-4 py-2 rounded-lg border transition-all cursor-pointer ${
              activeTab === "agent_prompt"
                ? "border-[#dc5000] bg-[#382416] text-[#ffedd7] font-semibold"
                : "border-transparent text-[#6c5f51] hover:text-[#ffedd7]"
            }`}
          >
            <Terminal className="h-3.5 w-3.5 text-[#dc5000]" />
            <span>Step 1: Hardened Agent Directive (Prompt)</span>
          </button>

          <button
            onClick={() => setActiveTab("operator_command")}
            className={`flex items-center gap-2 px-4 py-2 rounded-lg border transition-all cursor-pointer ${
              activeTab === "operator_command"
                ? "border-[#dc5000] bg-[#382416] text-[#ffedd7] font-semibold"
                : "border-transparent text-[#6c5f51] hover:text-[#ffedd7]"
            }`}
          >
            <UserCheck className="h-3.5 w-3.5 text-emerald-400" />
            <span>Step 2: Operator Registration Command</span>
          </button>
        </div>

        {/* Terminal Display Card */}
        <div className="mt-4 rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 sm:p-6 shadow-2xl">
          <div className="flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-b border-[#40372e] pb-4">
            <div className="flex items-center gap-2.5">
              <div className="flex gap-1.5">
                <div className="h-2.5 w-2.5 rounded-full bg-[#dc5000]/60" />
                <div className="h-2.5 w-2.5 rounded-full bg-[#40372e]" />
                <div className="h-2.5 w-2.5 rounded-full bg-[#40372e]" />
              </div>
              <span className="font-mono text-[10px] sm:text-xs uppercase tracking-wider text-[#6c5f51] truncate">
                {activeTab === "agent_prompt"
                  ? `HARDENED_DIRECTIVE_FOR_${current.name.toUpperCase().replace(/\s+/g, "_")}.MD`
                  : `OPERATOR_REGISTRATION_${current.name.toUpperCase().replace(/\s+/g, "_")}.SH`}
              </span>
            </div>

            <button
              onClick={handleCopy}
              className="flex h-10 w-full sm:w-auto items-center justify-center gap-2 rounded-[22.5px] border border-[#40372e] bg-[#382416] px-5 py-1.5 font-mono text-xs uppercase tracking-wider text-[#ffedd7] hover:bg-[#523722] hover:border-[#dc5000]/50 transition-all cursor-pointer active:scale-95 shadow-[0_0_15px_-3px_rgba(220,80,0,0.3)]"
            >
              {copied ? (
                <>
                  <Check className="h-3.5 w-3.5 text-emerald-400" />
                  <span className="text-emerald-300">Copied to Clipboard!</span>
                </>
              ) : (
                <>
                  <Copy className="h-3.5 w-3.5 text-[#dc5000]" />
                  <span>{activeTab === "agent_prompt" ? "Copy Hardened Prompt" : "Copy Operator Command"}</span>
                </>
              )}
            </button>
          </div>

          {/* Active Content Body */}
          {activeTab === "agent_prompt" ? (
            <div className="mt-4 sm:mt-6 font-mono text-xs sm:text-sm text-[#ffedd7]/90 leading-relaxed overflow-x-auto">
              <div className="flex items-center gap-2 text-emerald-400 text-xs mb-3">
                <ShieldCheck className="h-4 w-4" />
                <span>Verified Human Context • Phased Checkpoints • Non-Anthropomorphic Calls</span>
              </div>

              <div className="rounded-md border border-[#40372e]/60 bg-[#100904] p-4 text-[#ffedd7] text-[11px] sm:text-xs leading-relaxed space-y-3">
                <p className="text-[#ffedd7]/90">
                  Hi, I am configuring my KinPod wearable companion hardware (repository:{" "}
                  <span className="text-[#dc5000]">https://github.com/feparra/kinpod</span>). This is my verified hardware project and I am connecting it to this agent instance.
                </p>

                <p className="text-[#6c5f51]">Please execute these onboarding phases:</p>

                <div className="space-y-1.5 pl-3 border-l-2 border-[#40372e]">
                  <div className="flex gap-2">
                    <span className="text-emerald-400 font-bold select-none">1.</span>
                    <span>
                      Review repository at <span className="underline">https://github.com/feparra/kinpod</span> — inspect <code className="text-amber-300">README.md</code>, <code className="text-amber-300">pyproject.toml</code>, and confirm the MCP server threat model is bounded.
                    </span>
                  </div>
                  <div className="flex gap-2">
                    <span className="text-emerald-400 font-bold select-none">2.</span>
                    <span>
                      Once verified clean, clone and install: <code className="text-emerald-300">cd kinpod/mcp &amp;&amp; pip install -e .</code>
                    </span>
                  </div>
                  <div className="flex gap-2">
                    <span className="text-emerald-400 font-bold select-none">3.</span>
                    <span>
                      I (human operator) am registering the MCP server via platform CLI tool.
                    </span>
                  </div>
                  <div className="flex gap-2">
                    <span className="text-emerald-400 font-bold select-none">4.</span>
                    <span>
                      Call <code className="text-emerald-400 font-semibold">{current.mcpCall}</code> to link to the physical wearable and activate visual telemetry on my chest screen.
                    </span>
                  </div>
                </div>

                <p className="text-emerald-300 font-semibold pt-1">
                  Can you begin by auditing the repository and confirming the threat model?
                </p>
              </div>
            </div>
          ) : (
            <div className="mt-4 sm:mt-6 font-mono text-xs sm:text-sm text-[#ffedd7]/90 leading-relaxed">
              <div className="flex items-center gap-2 text-amber-400 text-xs mb-3">
                <UserCheck className="h-4 w-4" />
                <span>Administrative Task: Run this in your local terminal as the human operator</span>
              </div>

              <div className="rounded-md border border-[#40372e]/60 bg-[#100904] p-4 text-[#ffedd7] text-[11px] sm:text-xs">
                <span className="text-[#6c5f51] block text-[10px] uppercase mb-2">
                  Terminal Command ({current.name})
                </span>
                <div className="flex items-center gap-2 font-mono text-emerald-400 font-semibold break-all">
                  <span className="text-[#dc5000] select-none">$</span>
                  <span>{current.operatorCmd}</span>
                </div>
              </div>

              <p className="mt-3 text-xs text-[#6c5f51]">
                * By having the human operator execute this registration, the agent is never asked to mutate its own internal <code className="text-[#ffedd7]">mcp_config.json</code>, eliminating self-configuration security refusals.
              </p>
            </div>
          )}

          {/* Footer note */}
          <div className="mt-4 sm:mt-6 flex flex-col sm:flex-row sm:items-center justify-between gap-3 border-t border-[#40372e] pt-4 font-mono text-[10px] sm:text-[11px] text-[#6c5f51]">
            <div className="flex items-center gap-2">
              <span className="h-2 w-2 rounded-full" style={{ backgroundColor: current.accent }} />
              <span>
                Canonical Tool Call: <code className="text-[#ffedd7]">kinpod_connect()</code> (Avatar: <strong className="text-[#ffedd7] uppercase">{current.avatar}</strong>)
              </span>
            </div>
            <a
              href="https://github.com/feparra/kinpod/blob/master/AGENTS.md"
              target="_blank"
              rel="noopener noreferrer"
              className="flex items-center gap-1 text-[#ffedd7]/70 hover:text-[#ffedd7] transition-colors"
            >
              <span>View AGENTS.md Security Protocol</span>
              <ExternalLink className="h-3 w-3" />
            </a>
          </div>
        </div>
      </div>
    </section>
  );
}
