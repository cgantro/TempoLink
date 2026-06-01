/* screens-room.jsx — Ensemble Room: left ME panel · center participant list · right chat */
const { useState: useStateR, useEffect: useEffectR, useRef: useRefR } = React;

/* ---- metronome beat dots ---- */
function Metronome({ bpm }) {
  const [beat, setBeat] = useStateR(0);
  useEffectR(() => {
    const iv = setInterval(() => setBeat(b => (b + 1) % 4), 60000 / bpm);
    return () => clearInterval(iv);
  }, [bpm]);
  return (
    <div className="row gap16 center">
      <div className="col" style={{ alignItems: 'center' }}>
        <span className="display" style={{ fontSize: 28, lineHeight: 1 }}>{bpm}</span>
        <span className="smallcaps" style={{ fontSize: 8, letterSpacing: '.2em' }}>BPM</span>
      </div>
      <div className="row gap8">
        {[0, 1, 2, 3].map(i => (
          <span key={i} style={{
            width: i === 0 ? 12 : 10, height: i === 0 ? 12 : 10, borderRadius: '50%',
            background: beat === i ? (i === 0 ? 'var(--gold)' : 'var(--steel)') : 'transparent',
            border: `1.5px solid ${i === 0 ? 'var(--gold)' : 'var(--steel)'}`,
            boxShadow: beat === i ? `0 0 10px ${i === 0 ? 'var(--gold)' : 'var(--steel)'}` : 'none',
            transition: 'all .08s',
          }} />
        ))}
      </div>
    </div>
  );
}

function SessionTimer() {
  const [s, setS] = useStateR(2 * 60 + 47);
  useEffectR(() => { const iv = setInterval(() => setS(x => x + 1), 1000); return () => clearInterval(iv); }, []);
  const mm = String(Math.floor(s / 60)).padStart(2, '0'), ss = String(s % 60).padStart(2, '0');
  return <span className="readout" style={{ fontSize: 15, color: 'var(--ink)' }}>{mm}:{ss}</span>;
}

/* ---- connection quality phrase (no jargon) ---- */
function connWord(rtt) { return rtt <= 15 ? '원활' : rtt <= 30 ? '양호' : '다소 지연'; }

/* =============== LEFT: ME panel =============== */
const QUALITY = {
  '고압축': '대역폭 절약 · 약간의 음질 손실',
  '저압축': '권장 · 음질과 지연의 균형',
  '무손실': '최고 음질 · 안정적인 회선에서',
};
function MePanel({ me, room }) {
  const [muted, setMuted] = useStateR(false);
  const [mon, setMon] = useStateR(true);
  const [q, setQ] = useStateR(me.quality || '저압축');
  return (
    <aside style={{ width: 286, flex: '0 0 286px', borderRight: '1px solid var(--line)', background: 'var(--bg-deep)', padding: 22, overflowY: 'auto' }} className="scroll col gap20">
      <div>
        <div className="eyebrow">내 채널</div>
        <div className="row gap14 center" style={{ marginTop: 14 }}>
          <Avatar name={me.name} inst={me.inst} you size={54} />
          <div className="col" style={{ gap: 3 }}>
            <span className="display" style={{ fontSize: 22, whiteSpace: 'nowrap' }}>{me.name}</span>
            <span className="smallcaps" style={{ fontSize: 9.5, letterSpacing: '.14em', color: 'var(--ink-faint)' }}>{me.inst}</span>
          </div>
        </div>
      </div>

      <div className="col gap10">
        <div className="row between center">
          <span className="smallcaps" style={{ fontSize: 9.5 }}>내 입력</span>
          <span className="row gap6 center" style={{ fontSize: 12, color: 'var(--good)' }}><span className="badge idle" style={{ padding: 0, border: 0 }}><span className="dot" style={{ background: 'var(--good)' }} /></span>{connWord(me.rtt)}</span>
        </div>
        <div style={{ height: 8 }}><Meter active={!muted} base={0.58} /></div>
        <div className="row gap8" style={{ marginTop: 2 }}>
          <button className={`btn btn-sm ${muted ? '' : 'btn-ghost'}`} style={{ flex: 1, justifyContent: 'center', ...(muted ? { background: 'var(--bad)', borderColor: 'var(--bad)', color: '#fff' } : {}) }} onClick={() => setMuted(m => !m)}>
            <Glyph.mute style={{ width: 15, height: 15 }} /> {muted ? '음소거됨' : '음소거'}
          </button>
          <button className={`btn btn-sm ${mon ? 'btn-gold' : 'btn-ghost'}`} style={{ flex: 1, justifyContent: 'center' }} onClick={() => setMon(m => !m)}>
            <Glyph.spk style={{ width: 15, height: 15 }} /> 모니터
          </button>
        </div>
      </div>

      <div className="hr-rule" />

      <div className="col gap10">
        <span className="smallcaps" style={{ fontSize: 9.5 }}>음질</span>
        <div className="seg" style={{ width: '100%' }}>
          {Object.keys(QUALITY).map(k => <button key={k} className={q === k ? 'on' : ''} style={{ flex: 1 }} onClick={() => setQ(k)}>{k}</button>)}
        </div>
        <p style={{ fontSize: 11.5, color: 'var(--ink-faint)', lineHeight: 1.5 }}>{QUALITY[q]}</p>
      </div>

      <div className="hr-rule" />

      {/* session info (aux) */}
      <div className="col gap12">
        <span className="smallcaps" style={{ fontSize: 9.5 }}>세션 정보</span>
        <AuxRow k="접속 인원" v={`${room.members.length + 1}명`} />
        <AuxRow k="평균 지연" v={<RttPill ms={room.rtt} />} />
        <AuxRow k="연주 시간" v={<SessionTimer />} />
        <AuxRow k="지역" v={room.region} />
      </div>
    </aside>
  );
}
function AuxRow({ k, v }) {
  return <div className="row between center"><span className="smallcaps" style={{ fontSize: 9.5, letterSpacing: '.1em' }}>{k}</span><span className="mono" style={{ fontSize: 13, color: 'var(--ink)' }}>{v}</span></div>;
}

/* =============== RIGHT: chat =============== */
const SEED_CHAT = [
  { who: '윤도현', t: '다들 모니터 레벨 괜찮아요?', mine: false, time: '21:03' },
  { who: '하루', t: '드럼 살짝 키울게요', mine: false, time: '21:03' },
  { who: '문라이트', t: '네 좋습니다, 인트로부터 가시죠', mine: true, time: '21:04' },
  { who: '서린', t: '하나 둘 셋 넷 🎵', mine: false, time: '21:04' },
  { who: 'KENTA', t: '베이스 조금 늦게 들어갈게요', mine: false, time: '21:05' },
];
function ChatPanel({ me, onClose }) {
  const [msgs, setMsgs] = useStateR(SEED_CHAT);
  const [val, setVal] = useStateR('');
  const endRef = useRefR(null);
  useEffectR(() => { if (endRef.current) endRef.current.scrollTop = endRef.current.scrollHeight; }, [msgs]);
  const send = () => {
    if (!val.trim()) return;
    const now = new Date();
    setMsgs(m => [...m, { who: me.name, t: val.trim(), mine: true, time: `${String(now.getHours()).padStart(2, '0')}:${String(now.getMinutes()).padStart(2, '0')}` }]);
    setVal('');
  };
  return (
    <aside style={{ width: 312, flex: '0 0 312px', borderLeft: '1px solid var(--line)', background: 'var(--bg-deep)', display: 'flex', flexDirection: 'column' }}>
      <div className="row between center" style={{ padding: '16px 18px', borderBottom: '1px solid var(--line)' }}>
        <div className="row gap8 center"><Glyph.chat style={{ width: 17, height: 17, color: 'var(--gold)' }} /><span className="display" style={{ fontSize: 20 }}>채팅</span></div>
        <button className="btn btn-icon btn-ghost btn-sm" style={{ padding: 6 }} onClick={onClose}><Glyph.close style={{ width: 16, height: 16 }} /></button>
      </div>
      <div ref={endRef} className="scroll" style={{ flex: 1, padding: '16px 16px', display: 'flex', flexDirection: 'column', gap: 14 }}>
        {msgs.map((m, i) => (
          <div key={i} className="col" style={{ alignItems: m.mine ? 'flex-end' : 'flex-start', gap: 4 }}>
            <div className="row gap6 center" style={{ flexDirection: m.mine ? 'row-reverse' : 'row' }}>
              <span style={{ fontSize: 11.5, fontWeight: 600, color: m.mine ? 'var(--gold)' : 'var(--ink-soft)' }}>{m.mine ? '나' : m.who}</span>
              <span className="mono" style={{ fontSize: 9.5, color: 'var(--ink-faint)' }}>{m.time}</span>
            </div>
            <div style={{ maxWidth: '85%', padding: '9px 12px', borderRadius: 10, fontSize: 13, lineHeight: 1.45,
              background: m.mine ? 'color-mix(in srgb, var(--gold) 16%, transparent)' : 'var(--panel)',
              border: `1px solid ${m.mine ? 'color-mix(in srgb, var(--gold) 40%, transparent)' : 'var(--line)'}`,
              borderTopRightRadius: m.mine ? 3 : 10, borderTopLeftRadius: m.mine ? 10 : 3 }}>{m.t}</div>
          </div>
        ))}
      </div>
      <div className="row gap8 center" style={{ padding: 14, borderTop: '1px solid var(--line)' }}>
        <input className="input" style={{ flex: 1 }} placeholder="메시지 입력" value={val} onChange={e => setVal(e.target.value)} onKeyDown={e => { if (e.key === 'Enter') send(); }} />
        <button className="btn btn-gold btn-icon" onClick={send}><Glyph.send style={{ width: 17, height: 17 }} /></button>
      </div>
    </aside>
  );
}

/* =============== CENTER A: participant LIST =============== */
function ListView({ peers }) {
  return (
    <div style={{ padding: '20px 26px 28px', display: 'flex', flexDirection: 'column', gap: 12 }}>
      <div className="row between center" style={{ padding: '0 4px 4px' }}>
        <span className="smallcaps" style={{ fontSize: 9.5 }}>참가자 {peers.length}명</span>
        <span className="smallcaps" style={{ fontSize: 9.5, color: 'var(--ink-faint)' }}>실시간 입력 · 지연</span>
      </div>
      {peers.map((p, i) => <ListRow key={i} p={p} seed={i + 3} />)}
    </div>
  );
}
function ListRow({ p, seed }) {
  const [muted, setMuted] = useStateR(false);
  return (
    <div className="card" style={{ padding: '14px 18px', display: 'flex', alignItems: 'center', gap: 18 }}>
      <Avatar name={p.name} inst={p.inst} size={44} />
      <div className="col" style={{ gap: 3, width: 124, flex: 'none' }}>
        <span style={{ fontSize: 15, fontWeight: 600, whiteSpace: 'nowrap' }}>{p.name}</span>
        <span className="smallcaps" style={{ fontSize: 9, letterSpacing: '.12em', color: 'var(--ink-faint)' }}>{p.inst}</span>
      </div>
      <div style={{ flex: 1, height: 40, minWidth: 60, color: muted ? 'var(--ink-faint)' : 'var(--steel)' }}>
        <Waveform n={48} seed={seed} active={!muted} />
      </div>
      <div style={{ width: 96, flex: 'none' }}><Meter active={!muted} base={0.42 + (seed % 3) * 0.12} /></div>
      <div className="col" style={{ alignItems: 'center', gap: 1, width: 52, flex: 'none' }}>
        <RttPill ms={p.rtt} />
        <span className="smallcaps" style={{ fontSize: 8, letterSpacing: '.08em', color: 'var(--ink-faint)' }}>{connWord(p.rtt)}</span>
      </div>
      <button className="btn btn-icon btn-sm" style={{ flex: 'none', padding: 7, ...(muted ? { background: 'var(--bad)', borderColor: 'var(--bad)', color: '#fff' } : { borderColor: 'var(--line)', color: 'var(--ink-soft)' }) }} onClick={() => setMuted(m => !m)}><Glyph.mute style={{ width: 15, height: 15 }} /></button>
    </div>
  );
}

/* =============== CENTER B: celestial orbit =============== */
function OrbitView({ peers, me, room }) {
  const [hover, setHover] = useStateR(null);
  const S = 520, C = S / 2;
  const rings = [{ ms: 15, r: 108 }, { ms: 30, r: 170 }, { ms: 50, r: 224 }];
  const scale = (rtt) => 70 + Math.min(rtt, 60) / 60 * 168;
  const planets = peers.map((p, i) => {
    const ang = (i / peers.length) * Math.PI * 2 - Math.PI / 2;
    const r = scale(p.rtt);
    return { p, x: C + Math.cos(ang) * r, y: C + Math.sin(ang) * r };
  });
  return (
    <div className="starfield" style={{ minHeight: '100%', display: 'flex', justifyContent: 'center', alignItems: 'center', padding: 24 }}>
      <div style={{ position: 'relative', width: S, height: S }}>
        <svg width={S} height={S} style={{ position: 'absolute', inset: 0 }}>
          {rings.map(rg => (
            <g key={rg.ms}>
              <circle cx={C} cy={C} r={rg.r} fill="none" stroke="var(--line)" strokeWidth="1" strokeDasharray="3 6" />
              <text x={C} y={C - rg.r - 6} textAnchor="middle" fontFamily="var(--font-mono)" fontSize="10" fill="var(--ink-faint)">{rg.ms}ms</text>
            </g>
          ))}
          {planets.map((pl, i) => (
            <line key={i} x1={C} y1={C} x2={pl.x} y2={pl.y} stroke={pl.p.rtt <= 30 ? 'var(--steel)' : 'var(--warn)'} strokeWidth="1.2" strokeDasharray="2 5" opacity=".55">
              <animate attributeName="stroke-dashoffset" from="0" to="-14" dur="0.7s" repeatCount="indefinite" />
            </line>
          ))}
        </svg>
        <div style={{ position: 'absolute', left: C, top: C, transform: 'translate(-50%,-50%)', width: 92, height: 92, borderRadius: '50%', background: 'radial-gradient(circle at 36% 30%, var(--raised), var(--panel-2))', border: '1px solid var(--gold)', boxShadow: '0 0 50px -8px var(--gold)', display: 'grid', placeItems: 'center' }}>
          <div className="col" style={{ alignItems: 'center', gap: 1 }}>
            <Glyph.crescent style={{ width: 20, height: 20, color: 'var(--gold)' }} />
            <span className="display" style={{ fontSize: 16 }}>{room.bpm}</span>
            <span className="smallcaps" style={{ fontSize: 7, letterSpacing: '.16em' }}>BPM</span>
          </div>
        </div>
        {planets.map((pl, i) => (
          <div key={i} onMouseEnter={() => setHover(i)} onMouseLeave={() => setHover(null)}
            style={{ position: 'absolute', left: pl.x, top: pl.y, transform: 'translate(-50%,-50%)', cursor: 'pointer', zIndex: hover === i ? 5 : 1 }}>
            <div className="col" style={{ alignItems: 'center', gap: 5 }}>
              <div style={{ position: 'relative' }}>
                <Avatar name={pl.p.name} inst={pl.p.inst} size={hover === i ? 56 : 46} />
                {hover === i && <div style={{ position: 'absolute', inset: -6, borderRadius: '50%', border: '1px solid var(--steel)', animation: 'pulse 1.2s infinite' }} />}
              </div>
              <div className="col" style={{ alignItems: 'center', gap: 0, background: 'var(--bg-deep)', padding: '3px 8px', borderRadius: 6, border: '1px solid var(--line)' }}>
                <span style={{ fontSize: 11, fontWeight: 600, whiteSpace: 'nowrap' }}>{pl.p.name}</span>
                <RttPill ms={pl.p.rtt} />
              </div>
            </div>
          </div>
        ))}
        <div style={{ position: 'absolute', left: 6, bottom: 6 }}>
          <div className="row gap8 center" style={{ background: 'var(--bg-deep)', border: '1px solid var(--gold)', borderRadius: 10, padding: '7px 12px 7px 7px' }}>
            <Avatar name={me.name} inst={me.inst} you size={38} />
            <div className="col" style={{ gap: 1 }}>
              <span style={{ fontSize: 12, fontWeight: 600, whiteSpace: 'nowrap' }}>{me.name}</span>
              <span className="mono" style={{ fontSize: 9.5, color: 'var(--gold)', whiteSpace: 'nowrap' }}>나 · {me.inst}</span>
            </div>
          </div>
        </div>
      </div>
      <div style={{ position: 'absolute', bottom: 18, left: 24 }} className="mono">
        <span style={{ fontSize: 10, color: 'var(--ink-faint)', lineHeight: 1.7 }}>중심에서 멀수록 지연이 큽니다<br />점선 = 상대와의 실시간 연결</span>
      </div>
    </div>
  );
}

/* =============== CENTER C: console / ledger =============== */
function ConsoleView({ peers }) {
  return (
    <div style={{ padding: '20px 26px 28px' }}>
      <div className="row" style={{ padding: '0 16px 10px', borderBottom: '1px solid var(--line-strong)' }}>
        {['CH', '연주자', '입력 신호', '레벨', '팬', '지연', '볼륨', ''].map((h, i) => (
          <span key={i} className="smallcaps" style={{ fontSize: 9, letterSpacing: '.14em', color: 'var(--ink-faint)', flex: [0.5, 2.2, 3.6, 1, 0.9, 1, 2, 0.8][i], textAlign: i >= 3 && i <= 5 ? 'center' : 'left' }}>{h}</span>
        ))}
      </div>
      {peers.map((p, i) => <ConsoleRow key={i} p={p} idx={i} />)}
    </div>
  );
}
function ConsoleRow({ p, idx }) {
  const [muted, setMuted] = useStateR(false);
  const [fader, setFader] = useStateR(72 - (idx % 3) * 6);
  return (
    <div className="row center" style={{ padding: '14px 16px', borderBottom: '1px solid var(--line)' }}>
      <span className="readout" style={{ flex: 0.5, fontSize: 13, color: 'var(--ink-faint)' }}>{String(idx + 1).padStart(2, '0')}</span>
      <div style={{ flex: 2.2 }} className="row gap10 center">
        <Avatar name={p.name} inst={p.inst} size={34} />
        <div className="col" style={{ gap: 1 }}>
          <span style={{ fontSize: 13, fontWeight: 600, whiteSpace: 'nowrap' }}>{p.name}</span>
          <span className="smallcaps" style={{ fontSize: 8.5, letterSpacing: '.12em', color: 'var(--ink-faint)' }}>{p.inst}</span>
        </div>
      </div>
      <div style={{ flex: 3.6, height: 32, color: muted ? 'var(--ink-faint)' : 'var(--steel)', paddingRight: 18 }}>
        <Waveform n={52} seed={idx + 5} active={!muted} />
      </div>
      <div style={{ flex: 1, paddingRight: 16 }}><Meter active={!muted} base={0.4 + (idx % 4) * 0.1} /></div>
      <div style={{ flex: 0.9, display: 'grid', placeItems: 'center' }}>
        <div className="knob" style={{ '--rot': `${-120 + (idx % 5) * 40}deg`, width: 30, height: 30 }}><span className="ind" /></div>
      </div>
      <div style={{ flex: 1, textAlign: 'center' }}><RttPill ms={p.rtt} /></div>
      <div style={{ flex: 2, padding: '0 14px' }}>
        <input type="range" min="0" max="100" value={fader} onChange={e => setFader(+e.target.value)} style={{ width: '100%', accentColor: 'var(--gold)' }} />
      </div>
      <div style={{ flex: 0.8, display: 'flex', justifyContent: 'flex-end' }}>
        <button className="btn btn-icon btn-sm" style={{ padding: 6, ...(muted ? { background: 'var(--bad)', borderColor: 'var(--bad)', color: '#fff' } : { borderColor: 'var(--line)', color: 'var(--ink-soft)' }) }} onClick={() => setMuted(m => !m)}><Glyph.mute style={{ width: 14, height: 14 }} /></button>
      </div>
    </div>
  );
}

/* ---- slim transport ---- */
function Transport({ room, onLeave, chatOn, onToggleChat }) {
  const [rec, setRec] = useStateR(false);
  return (
    <div style={{ flex: '0 0 auto', borderTop: '1px solid var(--line)', background: 'var(--bg-deep)', padding: '12px 26px', display: 'flex', alignItems: 'center', justifyContent: 'space-between' }}>
      <div className="row gap16 center">
        <span className="smallcaps" style={{ fontSize: 9, letterSpacing: '.16em' }}>메트로놈</span>
        <Metronome bpm={room.bpm} />
      </div>
      <div className="row gap10 center">
        <button className={`btn btn-sm ${chatOn ? 'btn-gold' : 'btn-ghost'}`} onClick={onToggleChat}><Glyph.chat style={{ width: 15, height: 15 }} /> 채팅</button>
        <button className={`btn btn-sm ${rec ? '' : 'btn-ghost'}`} style={rec ? { background: 'var(--bad)', borderColor: 'var(--bad)', color: '#fff' } : {}} onClick={() => setRec(r => !r)}>
          <span style={{ width: 9, height: 9, borderRadius: '50%', background: rec ? '#fff' : 'var(--bad)', animation: rec ? 'pulse 1.2s infinite' : 'none' }} /> {rec ? '녹음 중' : '녹음'}
        </button>
        <button className="btn btn-sm" style={{ borderColor: 'var(--bad)', color: 'var(--bad)' }} onClick={onLeave}><Glyph.leave style={{ width: 15, height: 15 }} /> 나가기</button>
      </div>
    </div>
  );
}

/* ================= ROOM ================= */
function RoomScreen({ room, me, view, onLeave }) {
  const [chat, setChat] = useStateR(true);
  const others = room.members;
  return (
    <div className="screen fade" style={{ minWidth: 0 }}>
      <header style={{ padding: '18px 28px 16px', borderBottom: '1px solid var(--line)' }}>
        <div className="row between center">
          <div className="row gap16 center">
            <Glyph.asterisk className="ast" style={{ width: 18, height: 18 }} />
            <div>
              <div className="eyebrow">합주 룸 · {room.live ? '연주 중' : '대기'}</div>
              <h1 className="display" style={{ fontSize: 30, marginTop: 3, whiteSpace: 'nowrap' }}>{room.name}</h1>
            </div>
          </div>
          <div className="row gap20 center">
            <div className="col" style={{ alignItems: 'flex-end', gap: 2 }}>
              <span className="smallcaps" style={{ fontSize: 8.5, letterSpacing: '.16em' }}>평균 지연</span>
              <RttPill ms={room.rtt} big />
            </div>
            <span className="badge">{others.length + 1}명 접속</span>
            <button className={`btn btn-sm ${chat ? 'btn-gold' : 'btn-ghost'}`} onClick={() => setChat(c => !c)}><Glyph.chat style={{ width: 15, height: 15 }} /> 채팅</button>
          </div>
        </div>
      </header>

      <div style={{ flex: 1, display: 'flex', minHeight: 0 }}>
        <MePanel me={me} room={room} />
        <div className="scroll" style={{ flex: 1, minWidth: 0 }}>
          {view === 'list' && <ListView peers={others} />}
          {view === 'orbit' && <OrbitView peers={others} me={me} room={room} />}
          {view === 'console' && <ConsoleView peers={others} />}
        </div>
        {chat && <ChatPanel me={me} onClose={() => setChat(false)} />}
      </div>

      <Transport room={room} onLeave={onLeave} chatOn={chat} onToggleChat={() => setChat(c => !c)} />
    </div>
  );
}

Object.assign(window, { RoomScreen });
