import { NextResponse } from "next/server";
import Stripe from "stripe";

export async function POST(req: Request) {
  try {
    const origin = req.headers.get("origin") || "https://thekinpod.com";

    if (!process.env.STRIPE_SECRET_KEY) {
      // Demo / Preview mode fallback when STRIPE_SECRET_KEY is not configured
      return NextResponse.json({
        demo: true,
        message:
          "Stripe Secret Key not yet set in environment variables. KinPod Founder Edition order recorded in demo mode.",
        url: `${origin}/?checkout=demo_success`,
      });
    }

    const stripe = new Stripe(process.env.STRIPE_SECRET_KEY);

    const session = await stripe.checkout.sessions.create({
      line_items: [
        {
          price_data: {
            currency: "usd",
            product_data: {
              name: "KinPod Wearable AI Companion — Founder Edition",
              description:
                "Physical chest wearable body with 2.06\" Curved AMOLED, Push-To-Talk mic, acoustic speaker, CST9220 touchscreen, and magnetic chest harness.",
              images: [`${origin}/assets/marketing/hero_editorial_4k.jpg`],
            },
            unit_amount: 12000, // $120.00 USD
          },
          quantity: 1,
        },
      ],
      mode: "payment",
      shipping_address_collection: {
        allowed_countries: [
          "US",
          "CA",
          "GB",
          "DE",
          "FR",
          "ES",
          "IT",
          "JP",
          "AU",
          "MX",
          "BR",
          "AR",
          "CL",
        ],
      },
      success_url: `${origin}/?session_id={CHECKOUT_SESSION_ID}&status=success`,
      cancel_url: `${origin}/?canceled=true`,
    } as any);

    return NextResponse.json({ url: session.url });
  } catch (err: any) {
    console.error("Stripe Checkout Error:", err);
    return NextResponse.json(
      { error: err.message || "Failed to initialize checkout" },
      { status: 500 }
    );
  }
}
