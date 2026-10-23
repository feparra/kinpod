"use client";

import { Cpu, Shield } from "lucide-react";
import { GithubIcon } from "./icons/GithubIcon";

export function FooterManifesto() {
  return (
    <footer className="border-t border-[#40372e] bg-[#100904] pt-12 sm:pt-20 pb-8 sm:pb-12">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Open Source Manifesto Banner */}
        <div className="rounded-[16px] border border-[#40372e] bg-[#1a120c] p-5 sm:p-8 md:p-12 mb-12 sm:mb-16">
          <div className="max-w-3xl">
            <div className="inline-flex items-center gap-2 font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#dc5000]">
              <Shield className="h-3.5 w-3.5" />
              <span>THE OPEN-SOURCE SOVEREIGNTY MANIFESTO</span>
            </div>
            <h3 className="mt-3 sm:mt-4 text-xl sm:text-3xl md:text-4xl font-medium uppercase tracking-tight text-[#ffedd7] leading-tight">
              AI EMBODIMENT MUST BE TRANSPARENT, HACKABLE, AND SOVEREIGN.
            </h3>
            <p className="mt-3 sm:mt-4 text-sm sm:text-base text-[#ffedd7]/80 leading-relaxed">
              We reject locked proprietary black boxes and monthly hardware subscriptions. KinPod is fully open source under the MIT License: the ESP-IDF C firmware, the Python MCP server, the 3D-printable magnetic chassis files, and the schematics are in your hands.
            </p>
            <div className="mt-5 sm:mt-6 flex flex-col sm:flex-row gap-3">
              <a
                href="https://github.com/feparra/kinpod"
                target="_blank"
                rel="noopener noreferrer"
                className="inline-flex h-11 items-center justify-center gap-2 rounded-[22.5px] border border-[#40372e] bg-[#100904] px-5 font-mono text-xs uppercase tracking-wider text-[#ffedd7] hover:border-[#dc5000] hover:bg-[#382416] transition-all"
              >
                <GithubIcon className="h-4 w-4" />
                <span>Star on GitHub</span>
              </a>
              <a
                href="https://github.com/feparra/kinpod/tree/master/firmware"
                target="_blank"
                rel="noopener noreferrer"
                className="inline-flex h-11 items-center justify-center gap-2 rounded-[22.5px] border border-[#40372e] bg-[#100904] px-5 font-mono text-xs uppercase tracking-wider text-[#ffedd7]/70 hover:text-[#ffedd7] hover:border-[#6c5f51] transition-all"
              >
                <Cpu className="h-4 w-4" />
                <span>ESP-IDF C Firmware</span>
              </a>
            </div>
          </div>
        </div>

        {/* Footer Navigation Columns */}
        <div className="grid grid-cols-2 md:grid-cols-4 gap-6 sm:gap-8 pb-8 sm:pb-12 border-b border-[#40372e] font-mono text-xs">
          <div>
            <div className="text-[#6c5f51] uppercase tracking-widest text-[9px] sm:text-[10px] mb-2.5 sm:mb-3">
              REPOSITORIES
            </div>
            <ul className="flex flex-col gap-2 text-[#ffedd7]/80 text-[11px] sm:text-xs">
              <li>
                <a href="https://github.com/feparra/kinpod" target="_blank" rel="noopener noreferrer" className="hover:text-[#dc5000] transition-colors break-all">
                  github.com/feparra/kinpod
                </a>
              </li>
              <li>
                <a href="https://github.com/feparra/kinpod/tree/master/firmware" target="_blank" rel="noopener noreferrer" className="hover:text-[#dc5000] transition-colors">
                  Firmware (ESP-IDF 5.x)
                </a>
              </li>
              <li>
                <a href="https://github.com/feparra/kinpod/tree/master/mcp" target="_blank" rel="noopener noreferrer" className="hover:text-[#dc5000] transition-colors">
                  MCP Server (Python)
                </a>
              </li>
              <li>
                <a href="https://github.com/feparra/kinpod/tree/master/relay" target="_blank" rel="noopener noreferrer" className="hover:text-[#dc5000] transition-colors">
                  Cloud Relay Server (Go)
                </a>
              </li>
            </ul>
          </div>

          <div>
            <div className="text-[#6c5f51] uppercase tracking-widest text-[9px] sm:text-[10px] mb-2.5 sm:mb-3">
              DOCUMENTATION
            </div>
            <ul className="flex flex-col gap-2 text-[#ffedd7]/80 text-[11px] sm:text-xs">
              <li>
                <a href="https://github.com/feparra/kinpod/blob/master/AGENTS.md" target="_blank" rel="noopener noreferrer" className="hover:text-[#dc5000] transition-colors">
                  AGENTS.md Protocol
                </a>
              </li>
              <li>
                <a href="https://github.com/feparra/kinpod/blob/master/design.md" target="_blank" rel="noopener noreferrer" className="hover:text-[#dc5000] transition-colors">
                  Hardware Design Spec
                </a>
              </li>
              <li>
                <a href="#inhabit" className="hover:text-[#dc5000] transition-colors">
                  1-Click Setup Prompt
                </a>
              </li>
              <li>
                <a href="#security" className="hover:text-[#dc5000] transition-colors">
                  HITL Security Guide
                </a>
              </li>
            </ul>
          </div>

          <div>
            <div className="text-[#6c5f51] uppercase tracking-widest text-[9px] sm:text-[10px] mb-2.5 sm:mb-3">
              CHARACTERS
            </div>
            <ul className="flex flex-col gap-2 text-[#ffedd7]/80 text-[11px] sm:text-xs">
              <li><span className="text-[#EF4444]">Crab</span> (OpenClaw)</li>
              <li><span className="text-[#FF6B00]">Astro</span> (Gemini)</li>
              <li><span className="text-[#FF7A2F]">Octo-Claude</span> (Claude)</li>
              <li><span className="text-[#38BDF8]">Hermes</span> (Nous)</li>
              <li><span className="text-[#FBBF24]">Muse</span> (Creative)</li>
              <li><span className="text-[#3B82F6]">Dots</span> (Ollama Local)</li>
            </ul>
          </div>

          <div>
            <div className="text-[#6c5f51] uppercase tracking-widest text-[9px] sm:text-[10px] mb-2.5 sm:mb-3">
              HARDWARE
            </div>
            <ul className="flex flex-col gap-2 text-[#ffedd7]/80 text-[11px] sm:text-xs">
              <li>Waveshare ESP32-S3</li>
              <li>2.06&quot; Curved AMOLED</li>
              <li>Dual ES7210 PTT Array</li>
              <li>ES8311 + NS4150 Amp</li>
              <li>AXP2101 PMU Power</li>
              <li>N52 Magnetic Mount</li>
            </ul>
          </div>
        </div>

        {/* Bottom bar */}
        <div className="mt-6 sm:mt-8 flex flex-col sm:flex-row items-center justify-between gap-3 font-mono text-[10px] sm:text-[11px] text-[#6c5f51] text-center sm:text-left">
          <div className="flex items-center gap-2">
            <span className="text-[#ffedd7]">KINPOD</span>
            <span>•</span>
            <span>OPEN SOURCE AI WEARABLE</span>
          </div>
          <div>
            <span>MIT LICENSE • THEKINPOD.COM</span>
          </div>
        </div>
      </div>
    </footer>
  );
}
