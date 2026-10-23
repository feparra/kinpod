# KINPOD AI WEARABLE — DESIGN & LANDING PAGE SPECIFICATION
> **Domain:** [thekinpod.com](https://thekinpod.com)  
> **Deployment Target:** Vercel (Next.js 15 + Tailwind CSS v4)  
> **Theme:** Darkroom Product Editorial & Industrial Cybernetic Void  
> **Unit Price:** $120 USD (Turnkey Stripe Checkout)  
> **Target Repository:** [github.com/feparra/kinpod](https://github.com/feparra/kinpod)

---

## 1. Executive Project Brief

### 1.1 What is KinPod?
**KinPod** is the world’s first open-source wearable physical body designed specifically for **Autonomous AI Agents** (OpenClaw, Claude Code, Gemini, Hermes Agent, Ollama). Worn on the center of the chest via a magnetic harness, it gives software agents tangible embodiment in the physical world:
* **Face:** 2.06" Curved AMOLED Display (410×502 px, 60 FPS, deep ink blacks) displaying animated digital companion avatars ("Kins").
* **Ears:** ES7210 I2S microphone array capturing user speech with hardware Push-To-Talk (lateral BOOT button).
* **Mouth:** ES8311 I2S codec + NS4150 amplifier driving a high-fidelity front acoustic speaker.
* **Hands & Guardian:** CST9220 capacitive touchscreen powering the **Human-In-The-Loop (HITL)** physical approval system. Before an agent deletes a file, executes a terminal command, or triggers financial transactions, it must physically pause until the wearer taps **[APPROVE]** on their chest.
* **Power Management:** AXP2101 PMU with smart inactivity auto-dimming (drops AMOLED brightness to 15% after 30 seconds of inactivity, waking instantly to 100% on touch or voice) to guarantee all-day battery life.

---

## 2. Design System & Aesthetics (Darkroom Product Editorial)

Inspired by the darkroom editorial aesthetic of **ORYZO AI**, **Teenage Engineering**, and **Lusion**: a lone physical artifact suspended in warm obsidian darkness, cream typography floating in generous negative space, with zero visual clutter competing with the hardware silhouette.

### 2.1 Color Tokens

| Name | Hex Value | CSS Variable | Semantic Usage |
| :--- | :--- | :--- | :--- |
| **Walnut Shadow** | `#100904` | `--color-walnut-shadow` | Primary canvas & section void background. Deep warm black. |
| **Bark Brown** | `#382416` | `--color-bark-brown` | Elevated solid button surface (Primary CTA background). |
| **Cork Border** | `#40372e` | `--color-cork-border` | Hairline borders, dashed dividers, subtle grid frames. |
| **Driftwood** | `#6c5f51` | `--color-driftwood` | Muted structural text and secondary labels. |
| **Warm Cream** | `#ffedd7` | `--color-warm-cream` | Core typography, headlines, active links, ghost borders. |
| **Ember Accent** | `#dc5000` | `--color-ember-accent` | Editorial accents, tags, status pills, and highlights. |
| **Crab Red** | `#EF4444` | `--kin-crab` | OpenClaw agent accent & high-risk HITL modal border. |
| **Astro Gold** | `#FF6B00` | `--kin-astro` | Gemini agent visor & telemetry amber glow. |
| **Hermes Cyan** | `#38BDF8` | `--kin-hermes` | Hermes agent wings & zero-config network indicator. |

### 2.2 Typography System

The system operates strictly on two typographic voices:
1. **UPPERCASE WEIGHT 500 (Museum / Declarative Voice):**
   * Default for all navigation, display headings, labels, button text, and technical specifications.
   * Tight display line-height (`0.90` to `1.0`) creates a sculptural block effect where letterforms stack like solid architectural forms.
   * Font stack: `'Halyard Display Variable'`, `'Inter'`, `'Söhne'`, or `'Neue Haas Grotesk'`.
2. **MIXED-CASE WEIGHT 400 (Conversational Narrative Voice):**
   * Reserved strictly for the 24px–29px editorial body copy explaining the physical-digital communion.
   * Line height: `1.26`, letter-spacing: `normal`.

### 2.3 Surfaces & Zero-Shadow Elevation
* **No artificial drop-shadows or blurs:** Depth is achieved strictly through a two-step surface stack (`#100904` Canvas → `#382416` Elevated Surface) and hairline `1px` borders (`#40372e`).
* **Border Radii:**
  * Cards / Feature containers: `12px`
  * Outlined ghost buttons: `22.5px`
  * Solid Primary CTA pill button: `36px`
  * Full round badges: `9999px`
  * Inputs: `0px` (underline only)

---

## 3. Marketing Visual Assets (4K & Real Hardware)

All image assets are staged inside [`assets/marketing/`](file:///c:/Users/FERNA/OneDrive/Documents/kinpod/assets/marketing):

1. **`hero_editorial_4k.jpg`**: Ultra-luxury 3/4 hero floating product render. Curved AMOLED screen glowing with the Astro avatar, side BOOT and PWR buttons, warm rim light in a `#100904` void.
2. **`chest_lifestyle_4k.jpg`**: Real-world chest-worn editorial shot on a dark charcoal minimalist techwear jacket showing the red OpenClaw avatar glowing on the wearer's chest.
3. **`turntable_360_4k.jpg`**: 3-angle industrial design presentation (Isometric buttons, Front AMOLED, Acoustic CNC speaker profile).
4. **`characters_roster_4k.jpg`**: The 4 core 3D companion characters (Astro, Crab, Hermes, Muse) floating in museum gallery lighting.
5. **`real_hardware_front_crab.jpg`**: Authentic physical photography of the final wearable running OpenClaw.
6. **`real_hardware_34_astro.jpg`**: Authentic physical photography of the final wearable running Gemini Astro on desktop stand.
7. **`real_hardware_side_speaker.jpg`**: Close-up of the CNC speaker grille and magnetic clip chassis.

---

## 4. Landing Page Architecture (`thekinpod.com`)

The landing page is designed as a single continuous full-bleed editorial experience with 7 high-impact sections:

```
┌─────────────────────────────────────────────────────────────┐
│  [TOP NAV]  KINPOD               ROSTER  SPECS  HITL  GITHUB│
├─────────────────────────────────────────────────────────────┤
│  SECTION 1: HERO VOID REVEAL                                │
│  - 4K Floating KinPod 3D Canvas / Parallax                  │
│  - Display Title: PHYSICAL EMBODIMENT FOR AI AGENTS         │
│  - [ACQUIRE KINPOD — $120] (Pill CTA)                        │
│  - [COPY SETUP PROMPT] (Ghost Button)                       │
├─────────────────────────────────────────────────────────────┤
│  SECTION 2: ONE-CLICK AGENT INHABITATION (SETUP DRAWER)     │
│  - Zero-App, Zero-Cable, 1-Click Prompt Copy for LLMs       │
│  - Live code snippet with animated copy toast               │
├─────────────────────────────────────────────────────────────┤
│  SECTION 3: 360° PHYSICAL ANATOMY & TURNTABLE               │
│  - 3-view hardware carousel (Screen, Audio, Buttons, Clip)  │
│  - Deep dive: AMOLED 2.06", ES7210/ES8311, AXP2101 PMU      │
├─────────────────────────────────────────────────────────────┤
│  SECTION 4: THE KIN COMPANION ROSTER                        │
│  - Interactive character cards (Crab, Astro, Hermes, Muse)  │
│  - Multi-agent live inhabitation via MCP                    │
├─────────────────────────────────────────────────────────────┤
│  SECTION 5: HUMAN-IN-THE-LOOP (HITL) PHYSICAL SECURITY      │
│  - Interactive simulation of tactile Approve/Reject modal   │
│  - "No file deletion or bank transfer without chest tap"    │
├─────────────────────────────────────────────────────────────┤
│  SECTION 6: DIRECT ACQUISITION ($120 STRIPE CHECKOUT)       │
│  - Real $120 USD product card with Stripe Payment Element   │
│  - What's in the box: KinPod, Magnetic Harness, USB-C Cable │
├─────────────────────────────────────────────────────────────┤
│  SECTION 7: OPEN SOURCE MANIFESTO & FOOTER                  │
│  - GitHub repository link, ESP-IDF firmware, Schematics BOM │
└─────────────────────────────────────────────────────────────┘
```

---

## 5. Technical Implementation Details (Ready for Model Execution)

### 5.1 The Agent Setup Prompt (1-Click Copy)
The page must feature an interactive code card with a prominent "COPY SETUP PROMPT" action:

```markdown
You now have a physical body: KinPod. 
Execute these commands to take possession of your chest wearable:
1. git clone https://github.com/feparra/kinpod.git
2. cd kinpod/mcp && pip install -e .
3. Register kinpod in your mcp_config.json
4. Call kinpod_inhabit(agent_id="openclaw", avatar_id="crab")
Your character, live telemetry, and tactile approval buttons are now active on your human's chest.
```

### 5.2 Stripe $120 Checkout Integration (Vercel Ready)

In Next.js App Router (`app/api/checkout/route.ts`):
```typescript
import { NextResponse } from 'next/server';
import Stripe from 'stripe';

const stripe = new Stripe(process.env.STRIPE_SECRET_KEY as string, {
  apiVersion: '2023-10-16',
});

export async function POST() {
  try {
    const session = await stripe.checkout.sessions.create({
      payment_method_types: ['card'],
      line_items: [
        {
          price_data: {
            currency: 'usd',
            product_data: {
              name: 'KinPod Wearable AI Companion — Founder Edition',
              description: 'Physical chest wearable body with 2.06" Curved AMOLED, Push-To-Talk mic, speaker, and magnetic chest harness.',
              images: ['https://thekinpod.com/assets/marketing/hero_editorial_4k.jpg'],
            },
            unit_amount: 12000, // $120.00 USD
          },
          quantity: 1,
        },
      ],
      mode: 'payment',
      success_url: `https://thekinpod.com/success?session_id={CHECKOUT_SESSION_ID}`,
      cancel_url: `https://thekinpod.com/`,
    });

    return NextResponse.json({ url: session.url });
  } catch (err: any) {
    return NextResponse.json({ error: err.message }, { status: 500 });
  }
}
```

### 5.3 Tailwind v4 `@theme` Configuration

```css
@theme {
  --color-walnut-shadow: #100904;
  --color-bark-brown: #382416;
  --color-cork-border: #40372e;
  --color-driftwood: #6c5f51;
  --color-warm-cream: #ffedd7;
  --color-ember-accent: #dc5000;
  --color-pure-black: #000000;

  --font-display: 'Halyard Display Variable', ui-sans-serif, system-ui, sans-serif;
  
  --radius-cards: 12px;
  --radius-buttons-outlined: 22.5px;
  --radius-buttons-pill: 36px;
  --radius-full-round: 9999px;
}
```

---

## 6. Ready-to-Hand-off Instructions for the Web Developer Agent

When handing this specification over to the next agent:
1. Create a Next.js 15 project in `landing/` or root directory using Tailwind CSS v4.
2. Copy images from `assets/marketing/` to `public/assets/`.
3. Implement the full-bleed void layout (`#100904` background) with zero drop-shadows.
4. Mount the Stripe checkout button connected to `/api/checkout`.
5. Mount the 1-click clipboard prompt copy button.
6. Verify deployment build for Vercel with zero TypeScript or hydration errors.
