import type { Metadata, Viewport } from "next";
import { Inter, JetBrains_Mono } from "next/font/google";
import "./globals.css";

const inter = Inter({
  subsets: ["latin"],
  variable: "--font-sans",
  display: "swap",
});

const jetbrainsMono = JetBrains_Mono({
  subsets: ["latin"],
  variable: "--font-mono",
  display: "swap",
});

export const metadata: Metadata = {
  metadataBase: new URL("https://thekinpod.com"),
  title: "KinPod — Physical Wearable Body for AI Agents",
  description:
    "The world's first open-source wearable physical body designed for Autonomous AI Agents (OpenClaw, Claude Code, Gemini, Hermes, Ollama). 2.06\" Curved AMOLED, hardware Push-To-Talk mic, acoustic speaker, and tactile Human-In-The-Loop approval.",
  keywords: [
    "AI Wearable",
    "OpenClaw",
    "Claude Code",
    "KinPod",
    "Model Context Protocol",
    "MCP",
    "ESP32-S3 AMOLED",
    "Human in the loop",
    "AI Agent Embodiment",
  ],
  authors: [{ name: "KinPod Project", url: "https://thekinpod.com" }],
  openGraph: {
    title: "KinPod — Physical Wearable Body for AI Agents",
    description:
      "Give your autonomous AI agent physical embodiment. Curved 2.06\" AMOLED, Push-To-Talk audio, and tactile chest approval guardian.",
    url: "https://thekinpod.com",
    siteName: "KinPod",
    images: [
      {
        url: "/assets/marketing/hero_editorial_4k.jpg",
        width: 1200,
        height: 630,
        alt: "KinPod Wearable AI Companion",
      },
    ],
    locale: "en_US",
    type: "website",
  },
  twitter: {
    card: "summary_large_image",
    title: "KinPod — Physical Wearable Body for AI Agents",
    description:
      "Give your autonomous AI agent physical embodiment. Curved 2.06\" AMOLED, Push-To-Talk audio, and tactile chest approval guardian.",
    images: ["/assets/marketing/hero_editorial_4k.jpg"],
  },
  icons: {
    icon: "/assets/characters/crab.png",
  },
};

export const viewport: Viewport = {
  themeColor: "#100904",
  width: "device-width",
  initialScale: 1,
};

export default function RootLayout({
  children,
}: {
  children: React.ReactNode;
}) {
  return (
    <html lang="en" className={`${inter.variable} ${jetbrainsMono.variable} scroll-smooth`}>
      <body className="bg-[#100904] text-[#ffedd7] antialiased selection:bg-[#dc5000] selection:text-white">
        {children}
      </body>
    </html>
  );
}
