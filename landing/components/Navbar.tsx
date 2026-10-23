"use client";

import { useState } from "react";
import Image from "next/image";
import { Terminal, Shield, Cpu, Users, ArrowRight, Menu, X } from "lucide-react";
import { GithubIcon } from "./icons/GithubIcon";

export function Navbar({ onAcquire }: { onAcquire: () => void }) {
  const [mobileMenuOpen, setMobileMenuOpen] = useState(false);

  return (
    <header className="sticky top-0 z-50 w-full border-b border-[#40372e] bg-[#100904]/90 backdrop-blur-md">
      <div className="mx-auto flex max-w-7xl items-center justify-between px-6 py-4">
        {/* Brand */}
        <a href="#" className="flex items-center gap-3 group">
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
            <span className="font-mono text-base font-semibold tracking-wider text-[#ffedd7]">
              KINPOD<span className="text-[#dc5000]">_</span>
            </span>
            <span className="font-mono text-[9px] uppercase tracking-widest text-[#6c5f51]">
              AI WEARABLE BODY
            </span>
          </div>
        </a>

        {/* Desktop Navigation */}
        <nav className="hidden items-center gap-8 md:flex">
          <a
            href="#inhabit"
            className="flex items-center gap-1.5 font-mono text-xs uppercase tracking-wider text-[#ffedd7]/80 hover:text-[#ffedd7] transition-colors"
          >
            <Terminal className="h-3.5 w-3.5 text-[#dc5000]" />
            Inhabit
          </a>
          <a
            href="#anatomy"
            className="flex items-center gap-1.5 font-mono text-xs uppercase tracking-wider text-[#ffedd7]/80 hover:text-[#ffedd7] transition-colors"
          >
            <Cpu className="h-3.5 w-3.5 text-[#6c5f51]" />
            Anatomy
          </a>
          <a
            href="#roster"
            className="flex items-center gap-1.5 font-mono text-xs uppercase tracking-wider text-[#ffedd7]/80 hover:text-[#ffedd7] transition-colors"
          >
            <Users className="h-3.5 w-3.5 text-[#6c5f51]" />
            Kin Roster
          </a>
          <a
            href="#security"
            className="flex items-center gap-1.5 font-mono text-xs uppercase tracking-wider text-[#ffedd7]/80 hover:text-[#ffedd7] transition-colors"
          >
            <Shield className="h-3.5 w-3.5 text-[#6c5f51]" />
            HITL Guardian
          </a>
          <a
            href="#specs"
            className="font-mono text-xs uppercase tracking-wider text-[#ffedd7]/80 hover:text-[#ffedd7] transition-colors"
          >
            Specs
          </a>
          <a
            href="https://github.com/feparra/kinpod"
            target="_blank"
            rel="noopener noreferrer"
            className="flex items-center gap-1.5 font-mono text-xs uppercase tracking-wider text-[#6c5f51] hover:text-[#ffedd7] transition-colors"
          >
            <GithubIcon className="h-4 w-4" />
            GitHub
          </a>
        </nav>

        {/* CTA Button */}
        <div className="hidden items-center gap-3 md:flex">
          <span className="font-mono text-[11px] text-[#6c5f51] uppercase tracking-wider">
            FOUNDER BATCH 01
          </span>
          <button
            onClick={onAcquire}
            className="group flex items-center gap-2 rounded-[36px] bg-[#382416] px-5 py-2 text-xs font-semibold uppercase tracking-wider text-[#ffedd7] border border-[#40372e] hover:bg-[#523722] hover:border-[#dc5000]/50 transition-all active:scale-95 cursor-pointer"
          >
            <span>Acquire — $120</span>
            <ArrowRight className="h-3.5 w-3.5 text-[#dc5000] transition-transform group-hover:translate-x-0.5" />
          </button>
        </div>

        {/* Mobile menu button */}
        <button
          onClick={() => setMobileMenuOpen(!mobileMenuOpen)}
          className="p-2 text-[#ffedd7] md:hidden"
          aria-label="Toggle menu"
        >
          {mobileMenuOpen ? <X className="h-6 w-6" /> : <Menu className="h-6 w-6" />}
        </button>
      </div>

      {/* Mobile dropdown */}
      {mobileMenuOpen && (
        <div className="border-b border-[#40372e] bg-[#100904] px-6 py-4 md:hidden">
          <div className="flex flex-col gap-4">
            <a
              href="#inhabit"
              onClick={() => setMobileMenuOpen(false)}
              className="font-mono text-sm uppercase text-[#ffedd7]"
            >
              01. Inhabit
            </a>
            <a
              href="#anatomy"
              onClick={() => setMobileMenuOpen(false)}
              className="font-mono text-sm uppercase text-[#ffedd7]"
            >
              02. Anatomy
            </a>
            <a
              href="#roster"
              onClick={() => setMobileMenuOpen(false)}
              className="font-mono text-sm uppercase text-[#ffedd7]"
            >
              03. Kin Roster
            </a>
            <a
              href="#security"
              onClick={() => setMobileMenuOpen(false)}
              className="font-mono text-sm uppercase text-[#ffedd7]"
            >
              04. HITL Guardian
            </a>
            <a
              href="#specs"
              onClick={() => setMobileMenuOpen(false)}
              className="font-mono text-sm uppercase text-[#ffedd7]"
            >
              05. Specs
            </a>
            <a
              href="https://github.com/feparra/kinpod"
              target="_blank"
              rel="noopener noreferrer"
              className="flex items-center gap-2 font-mono text-sm uppercase text-[#6c5f51]"
            >
              <GithubIcon className="h-4 w-4" />
              GitHub Repository
            </a>
            <button
              onClick={() => {
                setMobileMenuOpen(false);
                onAcquire();
              }}
              className="mt-2 w-full rounded-[36px] bg-[#382416] py-3 text-center font-mono text-xs font-semibold uppercase tracking-wider text-[#ffedd7] border border-[#40372e]"
            >
              Acquire KinPod — $120 USD
            </button>
          </div>
        </div>
      )}
    </header>
  );
}
