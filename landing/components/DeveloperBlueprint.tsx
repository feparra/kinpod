"use client";

import { Terminal, Cpu, Download, ArrowRight, Check, Wrench, Shield, ExternalLink } from "lucide-react";
import { GithubIcon } from "./icons/GithubIcon";

export function DeveloperBlueprint() {
  const bomItems = [
    {
      part: "Waveshare ESP32-S3 Touch AMOLED 2.06\"",
      desc: "410×502 px curved AMOLED, ESP32-S3 (16MB Flash, 8MB Octal PSRAM), ES7210 mic, ES8311 speaker, AXP2101 PMU.",
      price: "~$28 USD",
      link: "https://www.waveshare.com/esp32-s3-touch-amoled-2.06.htm",
    },
    {
      part: "3D Printable Magnetic Enclosure (STL)",
      desc: "Minimalist two-piece snap-fit chassis with acoustic waveguide slits and magnetic harness channel.",
      price: "FREE (Open Source)",
      link: "https://github.com/feparra/kinpod/tree/master/chassis",
    },
    {
      part: "850mAh 3.7V LiPo Rechargeable Cell",
      desc: "Standard 603040 size lithium-polymer cell with JST 1.25mm connector directly matching the onboard PMU port.",
      price: "~$5 USD",
      link: "#",
    },
    {
      part: "Neodymium N52 Dual Magnets",
      desc: "15mm x 3mm strong magnetic pair for sandwiching through jackets, shirts, or technical lanyards.",
      price: "~$3 USD",
      link: "#",
    },
  ];

  return (
    <section id="blueprint" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Section Header */}
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-8 sm:pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#dc5000]">
              <Wrench className="h-3.5 w-3.5" />
              <span>THE HACKER&apos;S BILL OF MATERIALS</span>
            </div>
            <h2 className="mt-3 text-2xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
              BUILD OR FLASH YOUR OWN.
            </h2>
            <p className="mt-3 max-w-xl text-sm sm:text-base text-[#ffedd7]/80">
              No proprietary chips. Build a KinPod from standard off-the-shelf components for under $40, or flash existing development boards using our pre-compiled firmware.
            </p>
          </div>

          <div className="font-mono text-xs uppercase text-[#6c5f51]">
            <span>100% OFF-THE-SHELF HARDWARE</span>
          </div>
        </div>

        {/* Bill of Materials Grid */}
        <div className="mt-8 grid grid-cols-1 md:grid-cols-2 gap-4 sm:gap-6">
          {bomItems.map((item, idx) => (
            <div
              key={idx}
              className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-4 sm:p-5 flex flex-col justify-between transition-colors hover:border-[#6c5f51]"
            >
              <div>
                <div className="flex items-start justify-between gap-2">
                  <h3 className="font-mono text-xs sm:text-sm font-semibold uppercase text-[#ffedd7]">
                    {item.part}
                  </h3>
                  <span className="font-mono text-[11px] font-bold text-[#dc5000] shrink-0">
                    {item.price}
                  </span>
                </div>
                <p className="mt-2 text-xs text-[#ffedd7]/75 leading-relaxed">
                  {item.desc}
                </p>
              </div>

              {item.link !== "#" && (
                <a
                  href={item.link}
                  target="_blank"
                  rel="noopener noreferrer"
                  className="mt-4 inline-flex items-center gap-1 font-mono text-[11px] uppercase text-[#ffedd7]/80 hover:text-[#dc5000] transition-colors"
                >
                  <span>Component Source</span>
                  <ExternalLink className="h-3 w-3" />
                </a>
              )}
            </div>
          ))}
        </div>

        {/* Quick Flash Firmware Box */}
        <div className="mt-8 rounded-[16px] border border-[#40372e] bg-[#1a120c] p-6 sm:p-8">
          <div className="grid grid-cols-1 lg:grid-cols-12 gap-6 items-center">
            <div className="lg:col-span-7">
              <span className="font-mono text-[10px] uppercase tracking-widest text-[#dc5000]">
                ONE-COMMAND FLASHING (ESP-IDF 5.3+ / 6.0)
              </span>
              <h3 className="mt-2 text-xl sm:text-2xl font-medium uppercase text-[#ffedd7]">
                Connect via USB-C and Flash in 60 Seconds
              </h3>
              <p className="mt-2 text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed font-mono">
                The firmware comes with the FreeRTOS Mutex, QSPI AMOLED DMA driver, AXP2101 power rail setup, and the 6 official Kin characters pre-rendered in SPIFFS flash.
              </p>

              <div className="mt-4 rounded-md border border-[#40372e] bg-[#100904] p-3 font-mono text-xs text-[#ffedd7] overflow-x-auto">
                <span className="text-[#6c5f51] block text-[10px] uppercase">Terminal Commands</span>
                <div className="mt-1 flex gap-2">
                  <span className="text-[#dc5000] select-none">$</span>
                  <span className="text-emerald-400 font-semibold break-all">
                    git clone https://github.com/feparra/kinpod.git &amp;&amp; cd kinpod/firmware
                  </span>
                </div>
                <div className="mt-1 flex gap-2">
                  <span className="text-[#dc5000] select-none">$</span>
                  <span className="text-emerald-400 font-semibold break-all">
                    idf.py -p COM5 flash monitor
                  </span>
                </div>
              </div>
            </div>

            <div className="lg:col-span-5 flex flex-col gap-3">
              <a
                href="https://github.com/feparra/kinpod/tree/master/firmware"
                target="_blank"
                rel="noopener noreferrer"
                className="flex h-12 w-full items-center justify-center gap-2 rounded-[22.5px] bg-[#382416] border border-[#40372e] font-mono text-xs font-semibold uppercase tracking-wider text-[#ffedd7] hover:bg-[#523722] hover:border-[#dc5000] transition-all cursor-pointer"
              >
                <GithubIcon className="h-4 w-4" />
                <span>Browse Firmware Source (C)</span>
              </a>

              <a
                href="https://github.com/feparra/kinpod/blob/master/AGENTS.md"
                target="_blank"
                rel="noopener noreferrer"
                className="flex h-12 w-full items-center justify-center gap-2 rounded-[22.5px] bg-[#100904] border border-[#40372e] font-mono text-xs uppercase tracking-wider text-[#ffedd7]/80 hover:text-[#ffedd7] hover:border-[#6c5f51] transition-all cursor-pointer"
              >
                <Terminal className="h-4 w-4 text-[#dc5000]" />
                <span>Read AGENTS.md Protocol</span>
              </a>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}
