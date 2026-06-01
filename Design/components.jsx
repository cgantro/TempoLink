/* components.jsx — shared primitives, ornaments, waveform, chrome
   Exports to window for cross-file use. */
const { useState, useEffect, useRef, useMemo } = React;

/* ---------------- Celestial / instrument glyphs (simple iconography) ---------------- */
const Glyph = {
  crescent: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <path d="M16.5 3.2A9 9 0 1 0 20.8 16 7 7 0 0 1 16.5 3.2Z" fill="currentColor"/>
    </svg>
  ),
  moon: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <circle cx="12" cy="12" r="9" stroke="currentColor" strokeWidth="1.4"/>
      <path d="M12 3a9 9 0 0 1 0 18 6.4 6.4 0 0 1 0-18Z" fill="currentColor" opacity=".5"/>
    </svg>
  ),
  star: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <path d="M12 2v20M2 12h20M5 5l14 14M19 5 5 19" stroke="currentColor" strokeWidth="1.1"/>
    </svg>
  ),
  asterisk: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <path d="M12 3v18M4.5 7.5l15 9M19.5 7.5l-15 9" stroke="currentColor" strokeWidth="1.3" strokeLinecap="round"/>
    </svg>
  ),
  lobby: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <circle cx="12" cy="9" r="6.2" stroke="currentColor" strokeWidth="1.5"/>
      <path d="M5 21c1.4-3.4 4-5 7-5s5.6 1.6 7 5" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
    </svg>
  ),
  room: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <path d="M3 12c2-5 4-5 4.5 0S9 19 9.5 12 11 5 12 12s2 7 2.5 0 1.5-5 2 0 1.5 5 2 0" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round" strokeLinejoin="round"/>
    </svg>
  ),
  audio: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <rect x="9" y="2.5" width="6" height="12" rx="3" stroke="currentColor" strokeWidth="1.5"/>
      <path d="M5 11a7 7 0 0 0 14 0M12 18v3" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/>
    </svg>
  ),
  network: (p) => (
    <svg viewBox="0 0 24 24" fill="none" {...p}>
      <circle cx="12" cy="12" r="2.4" fill="currentColor"/>
      <circle cx="12" cy="12" r="6" stroke="currentColor" strokeWidth="1.3" opacity=".7"/>
      <circle cx="12" cy="12" r="9.5" stroke="currentColor" strokeWidth="1.1" opacity=".4"/>
    </svg>
  ),
  guitar: (p) => (<svg viewBox="0 0 24 24" {...p}><path d="M15 3l3 3-2 2 1 1-5 5a4 4 0 1 1-2-2l5-5 1 1 2-2-3-3z" stroke="currentColor" strokeWidth="1.4" fill="none" strokeLinejoin="round"/></svg>),
  keys: (p) => (<svg viewBox="0 0 24 24" {...p}><rect x="3" y="5" width="18" height="14" rx="1.5" stroke="currentColor" strokeWidth="1.4" fill="none"/><path d="M7 5v9M11 5v9M15 5v9M19 5v9" stroke="currentColor" strokeWidth="1.1"/></svg>),
  drums: (p) => (<svg viewBox="0 0 24 24" {...p}><ellipse cx="12" cy="9" rx="8" ry="3.5" stroke="currentColor" strokeWidth="1.4" fill="none"/><path d="M4 9v5c0 1.9 3.6 3.5 8 3.5s8-1.6 8-3.5V9" stroke="currentColor" strokeWidth="1.4" fill="none"/></svg>),
  bass: (p) => (<svg viewBox="0 0 24 24" {...p}><path d="M16 3l4 4-3 1 1 2-6 6a3.5 3.5 0 1 1-2-2l6-6 2 1 1-3z" stroke="currentColor" strokeWidth="1.4" fill="none" strokeLinejoin="round"/></svg>),
  vocal: (p) => (<svg viewBox="0 0 24 24" {...p}><rect x="9.5" y="3" width="5" height="10" rx="2.5" stroke="currentColor" strokeWidth="1.4" fill="none"/><path d="M6 11a6 6 0 0 0 12 0M12 17v4M9 21h6" stroke="currentColor" strokeWidth="1.4" strokeLinecap="round" fill="none"/></svg>),
  copy: (p) => (<svg viewBox="0 0 24 24" {...p}><rect x="8" y="8" width="12" height="12" rx="2" stroke="currentColor" strokeWidth="1.5" fill="none"/><path d="M16 8V6a2 2 0 0 0-2-2H6a2 2 0 0 0-2 2v8a2 2 0 0 0 2 2h2" stroke="currentColor" strokeWidth="1.5" fill="none"/></svg>),
  settings: (p) => (<svg viewBox="0 0 24 24" fill="none" {...p}><circle cx="12" cy="12" r="3.2" stroke="currentColor" strokeWidth="1.5"/><path d="M12 2.4v2.3M12 19.3v2.3M21.6 12h-2.3M4.7 12H2.4M18.7 5.3l-1.6 1.6M6.9 17.1l-1.6 1.6M18.7 18.7l-1.6-1.6M6.9 6.9 5.3 5.3" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/></svg>),
  chat: (p) => (<svg viewBox="0 0 24 24" fill="none" {...p}><path d="M4 5.5h16v10H9.5L5 19.5V5.5z" stroke="currentColor" strokeWidth="1.5" strokeLinejoin="round"/><path d="M8 9.5h8M8 12.5h5" stroke="currentColor" strokeWidth="1.4" strokeLinecap="round"/></svg>),
  send: (p) => (<svg viewBox="0 0 24 24" fill="none" {...p}><path d="M3.5 11 21 3.5 13 21l-2.6-7.4L3.5 11z" stroke="currentColor" strokeWidth="1.5" strokeLinejoin="round"/></svg>),
  close: (p) => (<svg viewBox="0 0 24 24" fill="none" {...p}><path d="M6 6 18 18M18 6 6 18" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round"/></svg>),
  mail: (p) => (<svg viewBox="0 0 24 24" fill="none" {...p}><rect x="3" y="5" width="18" height="14" rx="2" stroke="currentColor" strokeWidth="1.5"/><path d="m4 7 8 6 8-6" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/></svg>),
  lock: (p) => (<svg viewBox="0 0 24 24" fill="none" {...p}><rect x="5" y="10.5" width="14" height="9.5" rx="2" stroke="currentColor" strokeWidth="1.5"/><path d="M8 10.5V8a4 4 0 0 1 8 0v2.5" stroke="currentColor" strokeWidth="1.5"/></svg>),
  google: (p) => (<svg viewBox="0 0 24 24" {...p}><path fill="currentColor" d="M12 11v2.6h3.7c-.16 1-.65 1.8-1.6 2.4l2.5 1.9c1.5-1.4 2.3-3.4 2.3-5.9 0-.6-.05-1.1-.15-1.6H12z"/><path fill="currentColor" d="M6.6 14.3 4.3 16.1C5.7 18.6 8.6 20.5 12 20.5c2.4 0 4.5-.8 6-2.2l-2.5-1.9c-.7.5-1.6.8-3.5.8-2.3 0-4.2-1.5-4.9-3.6z"/><path fill="currentColor" opacity=".55" d="M4.3 7.9C3.7 9.1 3.4 10.5 3.4 12s.3 2.9.9 4.1l2.3-1.8c-.2-.5-.3-1.1-.3-1.7s.1-1.2.3-1.7L4.3 7.9z"/><path fill="currentColor" d="M12 6.7c1.3 0 2.5.5 3.4 1.3l2.2-2.2C16.5 4.5 14.4 3.5 12 3.5 8.6 3.5 5.7 5.4 4.3 7.9l2.3 1.8C7.8 8.2 9.7 6.7 12 6.7z"/></svg>),
  apple: (p) => (<svg viewBox="0 0 24 24" {...p}><path fill="currentColor" d="M17.05 12.5c-.02-2.2 1.8-3.26 1.88-3.3-1.03-1.5-2.62-1.7-3.18-1.73-1.35-.14-2.64.8-3.32.8-.69 0-1.74-.78-2.86-.76-1.47.02-2.83.85-3.59 2.17-1.53 2.65-.39 6.57 1.1 8.72.73 1.05 1.6 2.23 2.73 2.19 1.1-.04 1.51-.71 2.84-.71 1.32 0 1.7.71 2.86.69 1.18-.02 1.93-1.07 2.65-2.13.84-1.22 1.18-2.4 1.2-2.46-.03-.01-2.3-.88-2.32-3.48zM14.88 6.1c.6-.73 1.01-1.75.9-2.76-.87.03-1.92.58-2.55 1.31-.56.64-1.05 1.68-.92 2.67.97.07 1.96-.49 2.57-1.22z"/></svg>),
  kakao: (p) => (<svg viewBox="0 0 24 24" {...p}><path fill="currentColor" d="M12 4C7 4 3 7.1 3 10.9c0 2.45 1.7 4.6 4.27 5.82-.19.65-.68 2.36-.78 2.73-.12.46.17.45.36.33.15-.1 2.36-1.6 3.32-2.26.6.08 1.21.13 1.83.13 5 0 9-3.1 9-6.92S17 4 12 4z"/></svg>),
  check: (p) => (<svg viewBox="0 0 24 24" {...p}><path d="M4 12.5 9 17.5 20 6.5" stroke="currentColor" strokeWidth="1.8" fill="none" strokeLinecap="round" strokeLinejoin="round"/></svg>),
  plus: (p) => (<svg viewBox="0 0 24 24" {...p}><path d="M12 5v14M5 12h14" stroke="currentColor" strokeWidth="1.7" strokeLinecap="round"/></svg>),
  search: (p) => (<svg viewBox="0 0 24 24" {...p}><circle cx="11" cy="11" r="7" stroke="currentColor" strokeWidth="1.5" fill="none"/><path d="m20 20-3.5-3.5" stroke="currentColor" strokeWidth="1.6" strokeLinecap="round"/></svg>),
  mute: (p) => (<svg viewBox="0 0 24 24" {...p}><path d="M11 5 6 9H3v6h3l5 4V5z" stroke="currentColor" strokeWidth="1.5" fill="none" strokeLinejoin="round"/><path d="m16 9 5 6M21 9l-5 6" stroke="currentColor" strokeWidth="1.5" strokeLinecap="round"/></svg>),
  spk: (p) => (<svg viewBox="0 0 24 24" {...p}><path d="M11 5 6 9H3v6h3l5 4V5z" stroke="currentColor" strokeWidth="1.5" fill="none" strokeLinejoin="round"/><path d="M15 9a4 4 0 0 1 0 6M18 6.5a8 8 0 0 1 0 11" stroke="currentColor" strokeWidth="1.5" fill="none" strokeLinecap="round"/></svg>),
  leave: (p) => (<svg viewBox="0 0 24 24" {...p}><path d="M14 4h4a2 2 0 0 1 2 2v12a2 2 0 0 1-2 2h-4M10 8l-4 4 4 4M6 12h11" stroke="currentColor" strokeWidth="1.5" fill="none" strokeLinecap="round" strokeLinejoin="round"/></svg>),
};
const InstIcon = { '기타': Glyph.guitar, '베이스': Glyph.bass, '건반': Glyph.keys, '드럼': Glyph.drums, '보컬': Glyph.vocal };

/* ---------------- Waveform (CSS animated bars) ---------------- */
function Waveform({ n = 28, active = true, seed = 1, className = '', style }) {
  const bars = useMemo(() => {
    const a = [];
    let s = seed * 9301 + 49297;
    const rnd = () => { s = (s * 9301 + 49297) % 233280; return s / 233280; };
    for (let i = 0; i < n; i++) {
      const base = 0.25 + Math.abs(Math.sin(i * 0.55 + seed)) * 0.7;
      a.push({ dur: 0.55 + rnd() * 0.9, delay: -rnd() * 1.2, peak: base });
    }
    return a;
  }, [n, seed]);
  return (
    <div className={`wave ${active ? '' : 'static'} ${className}`} style={style}>
      {bars.map((b, i) => (
        <span key={i} className="bar" style={{
          height: `${Math.max(10, Math.round(b.peak * 100))}%`,
          animationDuration: `${b.dur}s`, animationDelay: `${b.delay}s`,
          opacity: active ? 1 : 0.5,
        }} />
      ))}
    </div>
  );
}

/* ---------------- Animated level meter ---------------- */
function Meter({ vert = false, base = 0.5, active = true }) {
  const [v, setV] = useState(base);
  useEffect(() => {
    if (!active) { setV(0.04); return; }
    let raf, t = Math.random() * 10;
    const tick = () => {
      t += 0.08;
      const x = base + Math.sin(t * 2.3) * 0.18 + Math.sin(t * 7.1) * 0.1 + (Math.random() - 0.5) * 0.12;
      setV(Math.max(0.04, Math.min(1, x)));
      raf = requestAnimationFrame(tick);
    };
    raf = requestAnimationFrame(tick);
    return () => cancelAnimationFrame(raf);
  }, [base, active]);
  return (
    <div className={`meter ${vert ? 'vert' : ''}`} style={vert ? { width: 8, height: '100%' } : { height: 6, width: '100%' }}>
      <div className="fill" style={vert ? { height: `${v * 100}%` } : { width: `${v * 100}%` }} />
    </div>
  );
}

/* ---------------- Avatar ---------------- */
function Avatar({ name, size = 40, you = false, inst }) {
  const Icon = inst && InstIcon[inst];
  return (
    <div className={`ava ${you ? 'you' : ''}`} style={{ width: size, height: size, fontSize: size * 0.4 }}>
      {Icon ? <Icon style={{ width: size * 0.5, height: size * 0.5 }} /> : (name || '?').slice(0, 1)}
    </div>
  );
}

/* ---------------- RTT pill (instrument readout) ---------------- */
function RttPill({ ms, big = false }) {
  const tone = ms <= 15 ? 'var(--good)' : ms <= 30 ? 'var(--gold)' : 'var(--warn)';
  return (
    <span className="readout" style={{ color: tone, fontSize: big ? 22 : 12, fontWeight: 700, letterSpacing: big ? 0 : '.02em' }}>
      {ms}<span style={{ fontSize: big ? 12 : 9, opacity: .7, marginLeft: 2 }}>ms</span>
    </span>
  );
}

/* ---------------- macOS chrome + rail ---------------- */
function MacChrome({ title, children, rttText, screen, onNav, showRail = true }) {
  const items = [
    { id: 'lobby', label: '로비', icon: Glyph.lobby },
    { id: 'room', label: '합주룸', icon: Glyph.room },
    { id: 'audio', label: '오디오', icon: Glyph.audio },
    { id: 'network', label: '연결', icon: Glyph.network },
    { id: 'settings', label: '설정', icon: Glyph.settings },
  ];
  return (
    <div className="mac-window">
      <div className="titlebar">
        <div className="traffic"><i className="r" /><i className="y" /><i className="g" /></div>
        <div className="title"><Glyph.crescent className="crescent" style={{ width: 16, height: 16 }} /> TEMPOLINK</div>
        <div className="tb-right">
          <span className="tb-dot" />
          <span>{rttText || '직접 연결 · 안정'}</span>
        </div>
      </div>
      <div className="win-body">
        {showRail && (
          <nav className="rail">
            <Glyph.crescent className="brand-moon" />
            {items.map(it => (
              <div key={it.id} className={`rail-item ${screen === it.id ? 'active' : ''}`} onClick={() => onNav(it.id)}>
                <it.icon /><span className="rl">{it.label}</span>
              </div>
            ))}
            <div className="rail-spacer" />
            <div className="rail-status">안정적 직접 연결</div>
            <div className="rail-item" onClick={() => onNav('start')} title="세션 종료">
              <Glyph.leave /><span className="rl">종료</span>
            </div>
          </nav>
        )}
        {children}
      </div>
    </div>
  );
}

Object.assign(window, { Glyph, InstIcon, Waveform, Meter, Avatar, RttPill, MacChrome });
