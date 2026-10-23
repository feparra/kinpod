"use client";

import { useState } from "react";
import { Navbar } from "@/components/Navbar";
import { HeroSection } from "@/components/HeroSection";
import { SetupPromptDrawer } from "@/components/SetupPromptDrawer";
import { AnatomyTurntable } from "@/components/AnatomyTurntable";
import { KinRoster } from "@/components/KinRoster";
import { HitlSecuritySimulator } from "@/components/HitlSecuritySimulator";
import { SpecsSection } from "@/components/SpecsSection";
import { CheckoutSection } from "@/components/CheckoutSection";
import { FooterManifesto } from "@/components/FooterManifesto";

export default function Home() {
  const [copiedPrompt, setCopiedPrompt] = useState(false);

  const defaultPrompt = `You now have a physical body: KinPod. 
Execute these commands to take possession of your chest wearable:
1. git clone https://github.com/feparra/kinpod.git
2. cd kinpod/mcp && pip install -e .
3. Register kinpod in your mcp_config.json
4. Call kinpod_inhabit(agent_id="openclaw", avatar_id="crab")
Your character, live telemetry, and tactile approval buttons are now active on your human's chest.`;

  const handleCopyPrompt = () => {
    navigator.clipboard.writeText(defaultPrompt);
    setCopiedPrompt(true);
    setTimeout(() => setCopiedPrompt(false), 2500);
  };

  const scrollToAcquire = () => {
    const el = document.getElementById("acquire");
    if (el) {
      el.scrollIntoView({ behavior: "smooth" });
    }
  };

  return (
    <div className="min-h-screen bg-[#100904] text-[#ffedd7] flex flex-col font-sans">
      {/* Top Sticky Navigation */}
      <Navbar onAcquire={scrollToAcquire} />

      {/* Main Editorial Flow */}
      <main className="flex-1">
        {/* Section 1: Hero Void Reveal */}
        <HeroSection
          onAcquire={scrollToAcquire}
          onCopyPrompt={handleCopyPrompt}
          copiedPrompt={copiedPrompt}
        />

        {/* Section 2: One-Click Agent Inhabitation Setup */}
        <SetupPromptDrawer />

        {/* Section 3: 360° Physical Anatomy & Turntable */}
        <AnatomyTurntable />

        {/* Section 4: The Kin Companion Roster */}
        <KinRoster />

        {/* Section 5: Human-In-The-Loop Physical Security */}
        <HitlSecuritySimulator />

        {/* Section 6: Technical Specifications */}
        <SpecsSection />

        {/* Section 7: Direct Acquisition ($120 Stripe Checkout) */}
        <CheckoutSection />
      </main>

      {/* Open Source Manifesto & Footer */}
      <FooterManifesto />
    </div>
  );
}
