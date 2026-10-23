"use client";

import { useState } from "react";
import Image from "next/image";
import { Cpu, Mic, Volume2, Shield, BatteryCharging, Magnet, Eye } from "lucide-react";

export function AnatomyTurntable() {
  const views = [
    {
      id: "real_crab",
      title: "Real Wearable (Crab)",
      category: "PHYSICAL PHOTOGRAPHY",
      image: "/assets/marketing/real_hardware_front_crab.jpg",
      description: "Authentic hardware running the OpenClaw red crab avatar on 2.06\" AMOLED.",
    },
    {
      id: "real_astro",
      title: "Desktop Stand (Astro)",
      category: "DESKTOP DOCK",
      image: "/assets/marketing/real_hardware_34_astro.jpg",
      description: "Mounted on custom 3D angled magnetic stand running Gemini Astro telemetry.",
    },
    {
      id: "speaker_close",
      title: "CNC Acoustic Profile",
      category: "ACOUSTIC CHAMBER",
      image: "/assets/marketing/real_hardware_side_speaker.jpg",
      description: "Close-up of CNC front speaker slots, lateral BOOT button, and magnetic clip.",
    },
    {
      id: "lifestyle",
      title: "Chest Techwear Shot",
      category: "LIFESTYLE DEPLOYMENT",
      image: "/assets/marketing/chest_lifestyle_4k.jpg",
      description: "Worn on center chest over minimalist techwear jacket via neodymium magnets.",
    },
    {
      id: "turntable",
      title: "360° Industrial Turntable",
      category: "3-VIEW CAD / RENDER",
      image: "/assets/marketing/turntable_360_4k.jpg",
      description: "Isometric buttons, front 2.06\" curved panel, and acoustic chassis silhouette.",
    },
  ];

  const [activeViewIndex, setActiveViewIndex] = useState(0);
  const currentView = views[activeViewIndex];

  const subsystems = [
    {
      icon: Cpu,
      title: "2.06\" Curved AMOLED Face",
      detail: "410×502 px resolution with 60 FPS refresh rate. Deep ink blacks ensure avatars float seamlessly in obsidian space.",
    },
    {
      icon: Mic,
      title: "ES7210 PTT Mic Array",
      detail: "Push-To-Talk triggered by lateral BOOT button (KEY3 GPIO18). Streams 16-bit PCM voice to your agent.",
    },
    {
      icon: Volume2,
      title: "ES8311 + NS4150 Speaker",
      detail: "High-output Class-D amplifier driving a front acoustic transducer through CNC precision-machined sound slits.",
    },
    {
      icon: Shield,
      title: "CST9220 Touchscreen Guardian",
      detail: "Capacitive touch controller powering the physical Human-In-The-Loop approval modal on your chest.",
    },
    {
      icon: BatteryCharging,
      title: "AXP2101 PMU & Auto-Dim",
      detail: "Smart throttle drops brightness to 15% after 30s idle, snapping to 100% on touch or voice for all-day runtime.",
    },
    {
      icon: Magnet,
      title: "Neodymium Magnetic Mount",
      detail: "High-strength N52 magnet array sandwiches through any shirt or hoodie. Zero sagging, instant unclip.",
    },
  ];

  return (
    <section id="anatomy" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Section Header */}
        <div className="flex flex-col md:flex-row md:items-end justify-between gap-6 pb-8 sm:pb-12 border-b border-[#40372e]">
          <div>
            <div className="inline-flex items-center gap-2 font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#dc5000]">
              <Eye className="h-3.5 w-3.5" />
              <span>PHYSICAL INDUSTRIAL DESIGN</span>
            </div>
            <h2 className="mt-3 text-2xl sm:text-4xl md:text-5xl font-medium uppercase tracking-tight text-[#ffedd7] leading-none">
              360° PHYSICAL ANATOMY.
            </h2>
            <p className="mt-3 max-w-xl text-sm sm:text-base text-[#ffedd7]/80">
              Not a render concept. Real custom hardware engineered around the Waveshare ESP32-S3 Touch AMOLED 2.06&quot; platform.
            </p>
          </div>

          <div className="font-mono text-[11px] sm:text-xs uppercase tracking-wider text-[#6c5f51]">
            <span>ESP32-S3 • 16MB FLASH • 8MB PSRAM</span>
          </div>
        </div>

        {/* Interactive View Switcher */}
        <div className="mt-6 sm:mt-8 grid grid-cols-1 lg:grid-cols-12 gap-6 lg:gap-8 items-start">
          {/* Main Visual Display */}
          <div className="lg:col-span-8 flex flex-col">
            <div className="relative aspect-[16/10] w-full overflow-hidden rounded-[12px] border border-[#40372e] bg-[#1a120c] shadow-2xl">
              <Image
                src={currentView.image}
                alt={currentView.title}
                fill
                className="object-cover transition-all duration-500"
              />
              <div className="absolute top-3 left-3 sm:top-4 sm:left-4 z-10 rounded-[9999px] border border-[#40372e] bg-[#100904]/80 px-2.5 py-0.5 sm:px-3 sm:py-1 font-mono text-[9px] sm:text-[10px] uppercase tracking-widest text-[#ffedd7] backdrop-blur-md">
                {currentView.category}
              </div>
            </div>

            {/* Mobile-Friendly Swipeable View Selector */}
            <div className="mt-3 sm:mt-4 flex overflow-x-auto pb-2 gap-2 scrollbar-none snap-x sm:grid sm:grid-cols-5 sm:overflow-visible sm:pb-0">
              {views.map((v, idx) => (
                <button
                  key={v.id}
                  onClick={() => setActiveViewIndex(idx)}
                  className={`flex shrink-0 w-36 sm:w-auto flex-col items-start rounded-md border p-2 text-left transition-all cursor-pointer snap-start ${
                    activeViewIndex === idx
                      ? "border-[#dc5000] bg-[#382416] text-[#ffedd7]"
                      : "border-[#40372e] bg-[#1a120c]/40 text-[#6c5f51] hover:border-[#6c5f51] hover:text-[#ffedd7]"
                  }`}
                >
                  <span className="font-mono text-[8px] sm:text-[9px] uppercase tracking-wider">
                    VIEW 0{idx + 1}
                  </span>
                  <span className="font-mono text-[11px] sm:text-xs font-semibold truncate w-full">
                    {v.title}
                  </span>
                </button>
              ))}
            </div>

            <p className="mt-2 sm:mt-3 text-xs sm:text-sm text-[#ffedd7]/80 italic">
              &quot;{currentView.description}&quot;
            </p>
          </div>

          {/* Subsystems Specification List */}
          <div className="lg:col-span-4 flex flex-col gap-2.5 sm:gap-3">
            <div className="font-mono text-[11px] uppercase tracking-widest text-[#6c5f51] mb-1">
              ENGINEERED SUBSYSTEMS
            </div>
            {subsystems.map((sub, idx) => {
              const Icon = sub.icon;
              return (
                <div
                  key={idx}
                  className="rounded-[12px] border border-[#40372e] bg-[#1a120c]/80 p-3.5 sm:p-4 transition-colors hover:border-[#6c5f51]"
                >
                  <div className="flex items-center gap-2.5">
                    <div className="flex h-6 w-6 sm:h-7 sm:w-7 items-center justify-center rounded-md border border-[#40372e] bg-[#100904] text-[#dc5000]">
                      <Icon className="h-3 w-3 sm:h-3.5 sm:w-3.5" />
                    </div>
                    <h3 className="font-mono text-[11px] sm:text-xs uppercase tracking-wider font-semibold text-[#ffedd7]">
                      {sub.title}
                    </h3>
                  </div>
                  <p className="mt-1.5 text-xs text-[#ffedd7]/75 leading-relaxed">
                    {sub.detail}
                  </p>
                </div>
              );
            })}
          </div>
        </div>
      </div>
    </section>
  );
}
