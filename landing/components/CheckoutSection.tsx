"use client";

import { useState } from "react";
import Image from "next/image";
import { Check, ShieldCheck, Truck, ArrowRight, Sparkles, Loader2, CreditCard } from "lucide-react";

export function CheckoutSection() {
  const [loading, setLoading] = useState(false);
  const [toastMessage, setToastMessage] = useState<string | null>(null);

  const handleCheckout = async () => {
    try {
      setLoading(true);
      const res = await fetch("/api/checkout", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
      });

      const data = await res.json();
      if (data.url) {
        if (data.demo) {
          setToastMessage(data.message || "Order registered in demo mode!");
          setTimeout(() => setToastMessage(null), 5000);
        } else {
          window.location.href = data.url;
        }
      } else if (data.error) {
        setToastMessage(`Error: ${data.error}`);
        setTimeout(() => setToastMessage(null), 5000);
      }
    } catch (err: any) {
      setToastMessage("Failed to initiate checkout. Please check connection.");
      setTimeout(() => setToastMessage(null), 5000);
    } finally {
      setLoading(false);
    }
  };

  const boxContents = [
    "1× KinPod Wearable Unit (2.06\" AMOLED, ESP32-S3, ES7210/ES8311, AXP2101 PMU)",
    "1× Neodymium N52 Dual-Plate Magnetic Chest Harness",
    "1× Braided 1.2m High-Speed USB Type-C Cable",
    "1× Founder Edition Physical Field Guide & Sticker Sheet",
    "Pre-flashed with ESP-IDF Firmware & 6 Official Kin Characters",
  ];

  return (
    <section id="acquire" className="relative border-t border-[#40372e] py-16 sm:py-24 bg-[#100904]">
      <div className="mx-auto max-w-7xl px-4 sm:px-6">
        {/* Toast Alert */}
        {toastMessage && (
          <div className="fixed bottom-4 left-4 right-4 sm:left-auto sm:right-6 sm:bottom-6 z-50 max-w-md rounded-[12px] border border-[#dc5000] bg-[#1a120c] p-4 font-mono text-xs text-[#ffedd7] shadow-2xl backdrop-blur-md animate-fade-in">
            <div className="flex items-center gap-2 text-[#dc5000] font-semibold">
              <Sparkles className="h-4 w-4 shrink-0" />
              <span>KINPOD CHECKOUT NOTICE</span>
            </div>
            <p className="mt-1 text-[#ffedd7]/80">{toastMessage}</p>
          </div>
        )}

        <div className="mx-auto max-w-4xl rounded-[16px] border border-[#40372e] bg-[#1a120c] p-4 sm:p-8 md:p-12 shadow-2xl">
          <div className="grid grid-cols-1 md:grid-cols-12 gap-6 sm:gap-8 items-center">
            {/* Left: Product Information */}
            <div className="md:col-span-7 flex flex-col">
              <div className="inline-flex items-center gap-2 rounded-[9999px] border border-[#40372e] bg-[#100904] px-3 py-1 font-mono text-[9px] sm:text-[10px] uppercase tracking-widest text-[#dc5000] w-fit">
                <span>FOUNDER EDITION • BATCH 01</span>
              </div>

              <h2 className="mt-3 sm:mt-4 text-2xl sm:text-3xl md:text-4xl font-medium uppercase tracking-tight text-[#ffedd7]">
                ACQUIRE KINPOD.
              </h2>

              <p className="mt-2 text-xs sm:text-sm text-[#ffedd7]/80 leading-relaxed">
                Complete turnkey wearable body ready to inhabit out of the box. Compatible with any autonomous agent running on macOS, Linux, or Windows.
              </p>

              {/* What's in the box */}
              <div className="mt-4 sm:mt-6 border-t border-[#40372e] pt-4 sm:pt-6">
                <span className="font-mono text-[11px] sm:text-xs uppercase tracking-widest text-[#6c5f51]">
                  WHAT&apos;S IN THE BOX:
                </span>
                <ul className="mt-2.5 sm:mt-3 flex flex-col gap-2">
                  {boxContents.map((item, idx) => (
                    <li key={idx} className="flex items-start gap-2 text-[11px] sm:text-xs text-[#ffedd7]/90 font-mono">
                      <Check className="h-3.5 w-3.5 text-[#dc5000] shrink-0 mt-0.5" />
                      <span>{item}</span>
                    </li>
                  ))}
                </ul>
              </div>

              {/* Guarantees */}
              <div className="mt-4 sm:mt-6 flex flex-wrap gap-3 sm:gap-4 font-mono text-[10px] sm:text-[11px] text-[#6c5f51]">
                <span className="flex items-center gap-1.5">
                  <Truck className="h-3.5 w-3.5 text-[#ffedd7]" />
                  Free Worldwide Tracked Shipping
                </span>
                <span className="flex items-center gap-1.5">
                  <ShieldCheck className="h-3.5 w-3.5 text-[#ffedd7]" />
                  1-Year Full Hardware Warranty
                </span>
              </div>
            </div>

            {/* Right: Price & CTA Card */}
            <div className="md:col-span-5 flex flex-col rounded-[12px] border border-[#40372e] bg-[#100904] p-4 sm:p-6 text-center">
              <div className="relative aspect-video w-full overflow-hidden rounded-md border border-[#40372e] bg-[#1a120c] mb-3 sm:mb-4">
                <Image
                  src="/assets/marketing/hero_editorial_4k.jpg"
                  alt="KinPod Founder Edition Box Set"
                  fill
                  className="object-cover"
                />
              </div>

              <span className="font-mono text-[10px] sm:text-[11px] uppercase tracking-widest text-[#6c5f51]">
                UNIT PRICE (TURNKEY)
              </span>

              <div className="mt-1 sm:mt-2 flex items-baseline justify-center gap-2">
                <span className="text-3xl sm:text-4xl md:text-5xl font-mono font-medium text-[#ffedd7]">
                  $120
                </span>
                <span className="font-mono text-xs sm:text-sm text-[#6c5f51]">USD</span>
              </div>

              <p className="mt-1 font-mono text-[10px] sm:text-[11px] text-emerald-400">
                Batch 01 Shipping November 2026
              </p>

              <button
                onClick={handleCheckout}
                disabled={loading}
                className="mt-4 sm:mt-6 flex h-13 sm:h-14 w-full items-center justify-center gap-2.5 sm:gap-3 rounded-[36px] bg-[#382416] font-mono text-xs sm:text-sm font-semibold uppercase tracking-wider text-[#ffedd7] border border-[#40372e] hover:bg-[#523722] hover:border-[#dc5000]/60 transition-all active:scale-95 shadow-[0_0_25px_-5px_rgba(220,80,0,0.3)] disabled:opacity-50 cursor-pointer"
              >
                {loading ? (
                  <>
                    <Loader2 className="h-4 w-4 animate-spin text-[#dc5000]" />
                    <span>Connecting Stripe...</span>
                  </>
                ) : (
                  <>
                    <CreditCard className="h-4 w-4 text-[#dc5000]" />
                    <span>Instant Stripe Checkout</span>
                    <ArrowRight className="h-4 w-4" />
                  </>
                )}
              </button>

              <span className="mt-2.5 sm:mt-3 font-mono text-[9px] sm:text-[10px] text-[#6c5f51] uppercase tracking-wider">
                STRIPE ENCRYPTED • ZERO SUBSCRIPTIONS
              </span>
            </div>
          </div>
        </div>
      </div>
    </section>
  );
}
