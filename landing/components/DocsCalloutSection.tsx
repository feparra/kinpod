"use client";

import Link from "next/link";
import { BookOpen, Cpu, Shield, Terminal, ArrowRight, Code, Layers } from "lucide-react";
import { GithubIcon } from "./icons/GithubIcon";

export function DocsCalloutSection() {
  const docCards = [
    {
      title: "MCP Protocol & Agent Tools",
      desc: "Complete Model Context Protocol reference for OpenClaw, Claude Code, and Gemini. Tool bindings for speech, listening, and display telemetry.",
      icon: Terminal,
      badge: "PROTOCOLS",
      href: "/docs#mcp",
    },
    {
      title: "Hardware Pinout & Schematics",
      desc: "Exact GPIO mapping for the Waveshare ESP32-S3 Touch AMOLED 2.06\". I2C0 bus, QSPI display, dual ES7210 mics, and AXP2101 PMU configuration.",
      icon: Cpu,
      badge: "HARDWARE",
      href: "/docs#hardware",
    },
    {
      title: "HITL Physical Sternum Killswitch",
      desc: "How capacitive touchscreen interrupts create a physical hardware boundary, pausing autonomous agents until you touch the chest screen.",
      icon: Shield,
      badge: "SECURITY",
      href: "/docs#hitl",
    },
  ];

  return (
    <section id="docs-preview" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        <div className="mx-auto max-w-5xl rounded-[16px] border border-[#40372e] bg-[#1a120c] p-6 sm:p-10 md:p-12 shadow-2xl">
          {/* Header */}
          <div className="flex flex-col items-start max-w-3xl">
            <div className="inline-flex items-center gap-2 rounded-[9999px] border border-[#40372e] bg-[#100904] px-3 py-1 font-mono text-[9px] sm:text-[10px] uppercase tracking-widest text-[#dc5000]">
              <BookOpen className="h-3.5 w-3.5" />
              <span>OPEN ARCHITECTURE • PROJECT DOCUMENTATION</span>
            </div>

            <h2 className="mt-4 text-2xl sm:text-3xl md:text-4xl font-medium uppercase tracking-tight text-[#ffedd7]">
              EXPLORE KINPOD DOCUMENTATION.
            </h2>

            <p className="mt-2 text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-sans">
              Everything you need to understand, build, flash, and connect autonomous AI agents to your KinPod wearable body. Transparent, hackable, and zero proprietary lock-in.
            </p>
          </div>

          {/* 3 Interactive Cards */}
          <div className="mt-8 grid grid-cols-1 md:grid-cols-3 gap-4 sm:gap-6">
            {docCards.map((card, idx) => {
              const Icon = card.icon;
              return (
                <Link
                  key={idx}
                  href={card.href}
                  className="group flex flex-col justify-between rounded-[12px] border border-[#40372e] bg-[#100904]/80 p-5 hover:border-[#dc5000]/60 hover:bg-[#100904] transition-all"
                >
                  <div>
                    <div className="flex items-center justify-between mb-3">
                      <div className="p-2 rounded-md bg-[#1a120c] border border-[#40372e] text-[#dc5000] group-hover:scale-105 transition-transform">
                        <Icon className="h-4 w-4" />
                      </div>
                      <span className="font-mono text-[9px] uppercase tracking-widest text-[#6c5f51]">
                        {card.badge}
                      </span>
                    </div>

                    <h3 className="font-mono text-sm font-semibold text-[#ffedd7] group-hover:text-[#dc5000] transition-colors">
                      {card.title}
                    </h3>

                    <p className="mt-2 text-[11px] sm:text-xs text-[#ffedd7]/70 leading-relaxed font-sans">
                      {card.desc}
                    </p>
                  </div>

                  <div className="mt-4 pt-3 border-t border-[#40372e]/60 flex items-center justify-between font-mono text-[10px] text-[#dc5000]">
                    <span>Read section</span>
                    <ArrowRight className="h-3 w-3 transition-transform group-hover:translate-x-1" />
                  </div>
                </Link>
              );
            })}
          </div>

          {/* Action Row */}
          <div className="mt-8 sm:mt-10 pt-6 sm:pt-8 border-t border-[#40372e] flex flex-col sm:flex-row items-center justify-between gap-4">
            <div className="flex items-center gap-2 font-mono text-[11px] text-[#6c5f51]">
              <Layers className="h-4 w-4 text-[#dc5000]" />
              <span>ESP-IDF C Firmware • Python MCP Hub • Cloudflare Roaming</span>
            </div>

            <div className="flex items-center gap-3 w-full sm:w-auto">
              <a
                href="https://github.com/feparra/kinpod"
                target="_blank"
                rel="noopener noreferrer"
                className="flex-1 sm:flex-initial flex h-11 items-center justify-center gap-2 rounded-[22.5px] border border-[#40372e] bg-[#100904] px-5 font-mono text-xs uppercase tracking-wider text-[#ffedd7] hover:border-[#6c5f51] hover:bg-[#1a120c] transition-all"
              >
                <GithubIcon className="h-4 w-4" />
                <span>GitHub</span>
              </a>

              <Link
                href="/docs"
                className="flex-1 sm:flex-initial flex h-11 items-center justify-center gap-2 rounded-[36px] bg-[#382416] px-6 font-mono text-xs font-semibold uppercase tracking-wider text-[#ffedd7] border border-[#40372e] hover:bg-[#523722] hover:border-[#dc5000]/60 transition-all shadow-[0_0_20px_-5px_rgba(220,80,0,0.3)]"
              >
                <BookOpen className="h-3.5 w-3.5 text-[#dc5000]" />
                <span>View Full Documentation</span>
                <ArrowRight className="h-3.5 w-3.5" />
              </Link>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}
