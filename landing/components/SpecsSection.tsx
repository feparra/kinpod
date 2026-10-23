"use client";

import { Cpu, Eye, Volume2, Shield, Battery, Radio, Box } from "lucide-react";

export function SpecsSection() {
  const specCategories = [
    {
      category: "COMPUTE & MEMORY",
      icon: Cpu,
      specs: [
        { name: "Processor SoC", value: "Espressif ESP32-S3 Dual-Core Xtensa LX7 @ 240 MHz" },
        { name: "Flash Storage", value: "16 MB High-Speed Quad-SPI Flash (SPIFFS assets)" },
        { name: "PSRAM", value: "8 MB Octal-SPI Ultra-High-Speed Memory" },
        { name: "Hardware Security", value: "AES-128/256, RSA-3072, SHA-256, Secure Boot" },
      ],
    },
    {
      category: "VISUAL & DISPLAY",
      icon: Eye,
      specs: [
        { name: "Panel Technology", value: "2.06\" Curved AMOLED (Continuous ink blacks)" },
        { name: "Resolution", value: "410 × 502 pixels (332 PPI Retina Density)" },
        { name: "Refresh Rate", value: "60 FPS Hardware-Accelerated Animation" },
        { name: "Color Gamut", value: "100% DCI-P3 wide color gamut, infinite contrast" },
      ],
    },
    {
      category: "ACOUSTIC CODEC & PTT",
      icon: Volume2,
      specs: [
        { name: "Microphone Array", value: "Dual ES7210 I2S ADC Array (Hardware PTT)" },
        { name: "Voice Trigger", value: "Lateral Momentary Push-To-Talk Switch (KEY3)" },
        { name: "Audio DAC Codec", value: "Everest ES8311 Low-Power I2S Codec" },
        { name: "Speaker Amplifier", value: "NS4150 High-Efficiency 3W Class-D Mono" },
      ],
    },
    {
      category: "TOUCH & HUMAN-IN-THE-LOOP",
      icon: Shield,
      specs: [
        { name: "Touch Controller", value: "CST9220 Capacitive Multi-Touch IC" },
        { name: "Touch Surface", value: "Anti-fingerprint oleophobic curved glass" },
        { name: "Approval Latency", value: "< 50ms hardware interrupt dispatch" },
        { name: "Security Boundary", value: "Strict physical chest tap verification" },
      ],
    },
    {
      category: "POWER & RUNTIME",
      icon: Battery,
      specs: [
        { name: "Power Management", value: "AXP2101 Integrated High-Precision PMU" },
        { name: "Battery Cell", value: "850 mAh Rechargeable LiPo Battery" },
        { name: "Smart Auto-Dim", value: "15% brightness @ 30s idle, 100% instant wake" },
        { name: "Charging Interface", value: "USB Type-C 5V / 1.0A fast charge" },
      ],
    },
    {
      category: "ROAMING & ENCLOSURE",
      icon: Box,
      specs: [
        { name: "Wireless Connectivity", value: "Wi-Fi 802.11 b/g/n (2.4GHz) + BLE 5.0" },
        { name: "Outdoor Roaming", value: "Zero-Config Cloudflare Tunnel (Cellular Hotspot)" },
        { name: "Dimensions & Weight", value: "58 mm × 46 mm × 14 mm • 48 grams" },
        { name: "Mounting System", value: "Dual Neodymium N52 Magnetic Chest Plate" },
      ],
    },
  ];

  return (
    <section id="specs" className="relative border-t border-[#40372e] py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-6">
        {/* Section Header */}
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[11px] uppercase tracking-widest text-[#dc5000]">
              <Cpu className="h-3.5 w-3.5" />
              <span>MUSEUM ARCHIVAL RECORD</span>
            </div>
            <h2 className="mt-3 text-3xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
              TECHNICAL SPECIFICATIONS.
            </h2>
            <p className="mt-3 max-w-xl text-base text-[#ffedd7]/80">
              Built on verified production components. Open-source firmware, schematics, and pinout definitions freely accessible.
            </p>
          </div>

          <div className="font-mono text-xs uppercase text-[#6c5f51]">
            <span>ENGINEERED FOR LONGEVITY & HACKABILITY</span>
          </div>
        </div>

        {/* Categories Grid */}
        <div className="mt-12 grid grid-cols-1 md:grid-cols-2 lg:grid-cols-3 gap-6">
          {specCategories.map((cat, idx) => {
            const Icon = cat.icon;
            return (
              <div
                key={idx}
                className="rounded-[12px] border border-[#40372e] bg-[#1a120c] p-6 transition-all hover:border-[#6c5f51]"
              >
                <div className="flex items-center gap-2.5 border-b border-[#40372e] pb-4">
                  <div className="flex h-7 w-7 items-center justify-center rounded-md border border-[#40372e] bg-[#100904] text-[#dc5000]">
                    <Icon className="h-3.5 w-3.5" />
                  </div>
                  <h3 className="font-mono text-xs uppercase tracking-wider font-semibold text-[#ffedd7]">
                    {cat.category}
                  </h3>
                </div>

                <dl className="mt-4 flex flex-col gap-3 font-mono text-xs">
                  {cat.specs.map((item, sIdx) => (
                    <div key={sIdx} className="flex flex-col pb-2 border-b border-[#40372e]/40 last:border-0 last:pb-0">
                      <dt className="text-[10px] uppercase text-[#6c5f51] tracking-wider">
                        {item.name}
                      </dt>
                      <dd className="mt-0.5 text-[#ffedd7] font-medium leading-normal">
                        {item.value}
                      </dd>
                    </div>
                  ))}
                </dl>
              </div>
            );
          })}
        </div>
      </div>
    </section>
  );
}
