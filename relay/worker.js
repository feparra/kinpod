export default {
  async fetch(request, env, ctx) {
    const url = new URL(request.url);
    const corsHeaders = {
      "Access-Control-Allow-Origin": "*",
      "Access-Control-Allow-Methods": "GET, POST, OPTIONS",
      "Access-Control-Allow-Headers": "Content-Type, User-Agent",
    };

    if (request.method === "OPTIONS") {
      return new Response(null, { headers: corsHeaders });
    }

    // POST /api/pair -> Body: {"url": "wss://..."}
    if (request.method === "POST" && url.pathname === "/api/pair") {
      try {
        const data = await request.json();
        if (!data || !data.url) {
          return new Response(JSON.stringify({ error: "Missing 'url' field" }), {
            status: 400,
            headers: { "Content-Type": "application/json", ...corsHeaders },
          });
        }

        // Generate a random 4-digit PIN between 1000 and 9999
        const pin = Math.floor(1000 + Math.random() * 9000).toString();

        // Store in Cloudflare Global KV for 10 minutes (600s)
        if (env.KINPOD_KV) {
          await env.KINPOD_KV.put(pin, data.url, { expirationTtl: 600 });
        }

        return new Response(
          JSON.stringify({
            status: "ok",
            pin: pin,
            url: data.url,
            expires_in: 600,
          }),
          {
            status: 200,
            headers: { "Content-Type": "application/json", ...corsHeaders },
          }
        );
      } catch (err) {
        return new Response(JSON.stringify({ error: err.message }), {
          status: 500,
          headers: { "Content-Type": "application/json", ...corsHeaders },
        });
      }
    }

    // GET /api/pair/:pin
    if (request.method === "GET" && url.pathname.startsWith("/api/pair/")) {
      const pin = url.pathname.split("/")[3];
      if (!pin || pin.length !== 4) {
        return new Response(JSON.stringify({ error: "Invalid PIN format" }), {
          status: 400,
          headers: { "Content-Type": "application/json", ...corsHeaders },
        });
      }

      let targetUrl = null;
      if (env.KINPOD_KV) {
        targetUrl = await env.KINPOD_KV.get(pin);
      }

      if (targetUrl) {
        return new Response(
          JSON.stringify({
            status: "ok",
            pin: pin,
            url: targetUrl,
            expires_in: 600,
          }),
          {
            status: 200,
            headers: { "Content-Type": "application/json", ...corsHeaders },
          }
        );
      }

      return new Response(
        JSON.stringify({ detail: "PIN not found or expired", pin: pin }),
        {
          status: 404,
          headers: { "Content-Type": "application/json", ...corsHeaders },
        }
      );
    }

    return new Response(
      JSON.stringify({
        service: "KinPod Pairing Relay (Cloudflare KV)",
        version: "2.0.0",
        docs: {
          "POST /api/pair": "Register tunnel with {'url': 'wss://...'}",
          "GET /api/pair/:pin": "Lookup tunnel URL by 4-digit PIN",
        },
      }),
      {
        status: 200,
        headers: { "Content-Type": "application/json", ...corsHeaders },
      }
    );
  },
};
