const path = require('path');
const fs = require('fs');
const puppeteer = require('puppeteer-core');

// Detecta un Chrome/Chromium disponible en el sistema
function chromePath() {
  const candidates = [
    '/usr/bin/google-chrome-stable',
    '/usr/bin/google-chrome',
    '/usr/bin/chromium',
    '/usr/bin/chromium-browser',
    process.env.CHROME_PATH,
  ].filter(Boolean);
  for (const c of candidates) if (fs.existsSync(c)) return c;
  throw new Error('No se encontró Chrome/Chromium (define CHROME_PATH).');
}

(async () => {
  const url = process.argv[2];
  const out = path.resolve(process.argv[3] || 'presentacion.pdf');
  if (!url) throw new Error('Uso: node topdf.js <url> [salida.pdf]');

  const browser = await puppeteer.launch({
    executablePath: chromePath(),
    headless: 'new',
    args: ['--no-sandbox', '--disable-gpu', '--font-render-hinting=none'],
  });

  try {
    const page = await browser.newPage();
    await page.setViewport({ width: 1280, height: 800 });
    await page.goto(url, { waitUntil: 'networkidle0', timeout: 60000 });

    // Espera a que reveal.js termine de inicializar (modo ?print-pdf)
    await page.waitForFunction(
      () => window.Reveal && Reveal.isReady && Reveal.isReady(),
      { timeout: 30000 }
    );
    await new Promise(r => setTimeout(r, 1500));

    const n = await page.evaluate(() => document.querySelectorAll('.pdf-page').length);
    if (!n) throw new Error('No se detectaron diapositivas (.pdf-page) en modo impresión.');

    await page.pdf({
      path: out,
      width: '1280px',   // tamaño de cada diapositiva en modo ?print-pdf
      height: '794px',
      printBackground: true,
      margin: { top: '0', right: '0', bottom: '0', left: '0' },
      pageRanges: `1-${n}`, // descarta la página en blanco final que agrega Chrome
    });

    console.log(`Diapositivas: ${n}`);
    console.log(`PDF: ${out} (${fs.statSync(out).size} bytes)`);
  } finally {
    await browser.close();
  }
})().catch(err => {
  console.error('ERROR:', err.message);
  process.exit(1);
});
