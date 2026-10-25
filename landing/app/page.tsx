"use client";

import { Navbar } from "@/components/Navbar";
import { KinPodLiveConsole } from "@/components/KinPodLiveConsole";
import { SetupPromptDrawer } from "@/components/SetupPromptDrawer";
import { SovereignManifesto } from "@/components/SovereignManifesto";
import { HitlSecuritySimulator } from "@/components/HitlSecuritySimulator";
import { AnatomyTurntable } from "@/components/AnatomyTurntable";
import { KinRoster } from "@/components/KinRoster";
import { DeveloperBlueprint } from "@/components/DeveloperBlueprint";
import { SpecsSection } from "@/components/SpecsSection";
import { DocsCalloutSection } from "@/components/DocsCalloutSection";
import { FooterManifesto } from "@/components/FooterManifesto";

export default function Home() {
  return (
    <div className="min-h-screen bg-[#100904] text-[#ffedd7] flex flex-col font-sans selection:bg-[#dc5000] selection:text-white">
      {/* Top Sticky Navigation */}
      <Navbar />

      {/* Main Experience Flow */}
      <main className="flex-1">
        {/* Section 1: Live Interactive Hardware Console (Digital Twin) */}
        <section id="console" className="relative pt-6 sm:pt-10">
          <KinPodLiveConsole />
        </section>

        {/* Section 2: One-Click Agent Inhabitation Setup */}
        <SetupPromptDrawer />

        {/* Section 3: The Sovereign Embodiment Manifesto (Open Source vs Closed AI Pins) */}
        <SovereignManifesto />

        {/* Section 4: Human-In-The-Loop Physical Security (The Sternum Killswitch) */}
        <HitlSecuritySimulator />

        {/* Section 5: 360° Physical Anatomy & Authentic Hardware Photography */}
        <AnatomyTurntable />

        {/* Section 6: The Kin Companion Roster */}
        <KinRoster />

        {/* Section 7: The Hacker's Blueprint & Bill of Materials */}
        <DeveloperBlueprint />

        {/* Section 8: Technical Engineering Specifications */}
        <SpecsSection />

        {/* Section 9: Project Documentation Callout & Architecture Guide */}
        <DocsCalloutSection />
      </main>

      {/* Open Source Manifesto & Repository Index */}
      <FooterManifesto />
    </div>
  );
}
