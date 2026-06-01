/* screens-settings.jsx — Audio, Connection status, User settings, Create modal */
const { useState: useStateS, useEffect: useEffectS } = React;

const QUALITY_S = {
  '고압축': '대역폭 절약 · 약간의 음질 손실',
  '저압축': '권장 · 음질과 지연의 균형',
  '무손실': '최고 음질 · 안정적인 회선에서',
};
function statusWord(rtt) { return rtt <= 15 ? '원활' : rtt <= 30 ? '양호' : '다소 지연'; }
function statusColor(rtt) { return rtt <= 15 ? 'var(--good)' : rtt <= 30 ? 'var(--gold)' : 'var(--warn)'; }

/* ============== AUDIO (simplified — engine self-manages buffers) ============== */
function AudioScreen({ me }) {
  const [q, setQ] = useStateS((me && me.quality) || '저압축');
  return (
    <div className="screen scroll fade">
      <header style={{ padding: '26px 34px 18px', borderBottom: '1px solid var(--line)' }}>
        <div className="eyebrow">AUDIO · 입력과 출력</div>
        <h1 className="display" style={{ fontSize: 38, marginTop: 6 }}>오디오 장치</h1>
      </header>
      <div style={{ padding: '26px 34px 40px', display: 'grid', gridTemplateColumns: '1fr 320px', gap: 26 }}>
        <div className="col gap24">
          <SettingCard title="입력 장치" sub="INPUT">
            <div className="field"><label>마이크 / 악기 입력</label>
              <select className="select" defaultValue="Focusrite Scarlett 2i2"><option>Focusrite Scarlett 2i2</option><option>Universal Audio Apollo Twin</option><option>내장 마이크</option></select>
            </div>
            <div className="field"><label>입력 레벨</label><div style={{ height: 8 }}><Meter base={0.55} /></div></div>
            <Slider label="입력 게인" val={64} unit="%" />
          </SettingCard>

          <SettingCard title="출력 장치" sub="OUTPUT">
            <div className="field"><label>헤드폰 / 스피커</label>
              <select className="select" defaultValue="Focusrite Scarlett 2i2"><option>Focusrite Scarlett 2i2</option><option>AirPods Pro (권장하지 않음)</option><option>내장 출력</option></select>
            </div>
            <Slider label="모니터 볼륨" val={78} unit="%" />
            <div className="row between center" style={{ marginTop: 4 }}>
              <div className="col" style={{ gap: 2 }}><span style={{ fontSize: 13 }}>직접 모니터링</span><span style={{ fontSize: 11, color: 'var(--ink-faint)' }}>내 소리를 지연 없이 바로 듣기</span></div><Toggle on />
            </div>
          </SettingCard>

          <SettingCard title="음질" sub="QUALITY">
            <div className="seg" style={{ width: '100%' }}>
              {Object.keys(QUALITY_S).map(k => <button key={k} className={q === k ? 'on' : ''} style={{ flex: 1 }} onClick={() => setQ(k)}>{k}</button>)}
            </div>
            <p style={{ fontSize: 12.5, color: 'var(--ink-soft)', lineHeight: 1.5 }}>{QUALITY_S[q]}</p>
          </SettingCard>
        </div>

        <div className="col gap20">
          <div className="card starfield" style={{ padding: 26, textAlign: 'center', position: 'sticky', top: 0 }}>
            <div className="eyebrow" style={{ color: 'var(--steel)' }}>예상 지연 시간</div>
            <div className="display" style={{ fontSize: 76, color: 'var(--gold)', margin: '14px 0 2px', lineHeight: 1 }}>6.6<span style={{ fontSize: 24, color: 'var(--ink-soft)' }}>ms</span></div>
            <p style={{ fontSize: 12, color: 'var(--ink-faint)', lineHeight: 1.5 }}>내 기기에서 발생하는 오디오 지연</p>
            <div className="hr-rule" style={{ margin: '20px 0' }} />
            <div className="row gap8 center" style={{ justifyContent: 'center', color: 'var(--good)' }}>
              <Glyph.check style={{ width: 16, height: 16 }} />
              <span style={{ fontSize: 12.5, color: 'var(--ink-soft)' }}>지연 시간은 자동으로 최적화됩니다</span>
            </div>
          </div>
          <div className="card" style={{ padding: 18 }}>
            <div className="row gap10 center">
              <Glyph.asterisk style={{ width: 16, height: 16, color: 'var(--gold)' }} />
              <span style={{ fontSize: 12, color: 'var(--ink-soft)', lineHeight: 1.5 }}>유선 인터넷과 오디오 인터페이스 사용을 권장합니다.</span>
            </div>
          </div>
        </div>
      </div>
    </div>
  );
}

/* ============== CONNECTION STATUS (de-jargoned) ============== */
function NetworkScreen({ room }) {
  const [hist, setHist] = useStateS(Array.from({ length: 40 }, () => 12 + Math.random() * 6));
  useEffectS(() => {
    const iv = setInterval(() => setHist(h => [...h.slice(1), 12 + Math.random() * 8]), 600);
    return () => clearInterval(iv);
  }, []);
  const peers = room ? room.members : [];
  const max = 40, W = 560, H = 120;
  const pts = hist.map((v, i) => `${(i / (hist.length - 1)) * W},${H - (v / max) * H}`).join(' ');
  return (
    <div className="screen scroll fade">
      <header style={{ padding: '26px 34px 18px', borderBottom: '1px solid var(--line)' }}>
        <div className="eyebrow">CONNECTION · 연결 상태</div>
        <h1 className="display" style={{ fontSize: 38, marginTop: 6 }}>연결 상태</h1>
      </header>
      <div style={{ padding: '26px 34px 40px', display: 'grid', gridTemplateColumns: '1.4fr 1fr', gap: 26 }}>
        <div className="col gap24">
          <div className="card" style={{ padding: 22 }}>
            <div className="row between center" style={{ marginBottom: 16 }}>
              <div className="smallcaps" style={{ fontSize: 9.5 }}>실시간 지연</div>
              <RttPill ms={Math.round(hist[hist.length - 1])} big />
            </div>
            <svg width="100%" viewBox={`0 0 ${W} ${H}`} style={{ overflow: 'visible' }}>
              {[15, 30].map(t => (
                <g key={t}><line x1="0" y1={H - (t / max) * H} x2={W} y2={H - (t / max) * H} stroke={t === 15 ? 'var(--good)' : 'var(--gold)'} strokeWidth="1" strokeDasharray="3 5" opacity=".6" />
                  <text x={W - 2} y={H - (t / max) * H - 4} textAnchor="end" fontFamily="var(--font-mono)" fontSize="9" fill={t === 15 ? 'var(--good)' : 'var(--gold)'}>{t}ms 목표</text></g>
              ))}
              <polyline points={pts} fill="none" stroke="var(--steel)" strokeWidth="2" strokeLinejoin="round" />
              <polygon points={`0,${H} ${pts} ${W},${H}`} fill="var(--steel)" opacity=".12" />
            </svg>
          </div>
          <div className="card" style={{ padding: 0 }}>
            <div className="row" style={{ padding: '14px 20px', borderBottom: '1px solid var(--line-strong)' }}>
              {['연주자', '지역', '지연', '상태'].map((h, i) => <span key={i} className="smallcaps" style={{ fontSize: 9, letterSpacing: '.12em', flex: [2.4, 1.6, 1, 1.2][i] }}>{h}</span>)}
            </div>
            {peers.map((p, i) => (
              <div key={i} className="row center" style={{ padding: '13px 20px', borderBottom: i < peers.length - 1 ? '1px solid var(--line)' : 0 }}>
                <div className="row gap10 center" style={{ flex: 2.4 }}><Avatar name={p.name} inst={p.inst} size={28} /><span style={{ fontSize: 13, fontWeight: 600, whiteSpace: 'nowrap' }}>{p.name}</span></div>
                <span className="mono" style={{ flex: 1.6, fontSize: 12, color: 'var(--ink-soft)' }}>{p.region || '도쿄 JP'}</span>
                <div style={{ flex: 1 }}><RttPill ms={p.rtt} /></div>
                <div style={{ flex: 1.2 }}><span className="badge" style={{ color: statusColor(p.rtt), borderColor: 'color-mix(in srgb, currentColor 45%, transparent)' }}><span className="dot" style={{ background: statusColor(p.rtt) }} />{statusWord(p.rtt)}</span></div>
              </div>
            ))}
          </div>
        </div>
        <div className="col gap24">
          <div className="card starfield" style={{ padding: 24, textAlign: 'center' }}>
            <div className="eyebrow" style={{ color: 'var(--steel)' }}>연결 방식</div>
            <div style={{ position: 'relative', height: 150, margin: '8px 0' }}>
              {[0, 1, 2].map(i => <div key={i} style={{ position: 'absolute', left: '50%', top: '50%', width: 60 + i * 44, height: 60 + i * 44, transform: 'translate(-50%,-50%)', borderRadius: '50%', border: '1px solid var(--steel)', opacity: .5 - i * .12, animation: `ping 2.4s ${i * 0.4}s infinite ease-out` }} />)}
              <div style={{ position: 'absolute', left: '50%', top: '50%', transform: 'translate(-50%,-50%)' }}><Glyph.network style={{ width: 40, height: 40, color: 'var(--gold)' }} /></div>
            </div>
            <div className="display" style={{ fontSize: 26, color: 'var(--good)' }}>직접 연결됨</div>
            <p style={{ fontSize: 12, color: 'var(--ink-faint)', marginTop: 6, lineHeight: 1.5 }}>상대와 직접 연결되어 가장 빠른 경로로 소리를 주고받습니다</p>
          </div>
          <div className="card" style={{ padding: 22 }}>
            <div className="smallcaps" style={{ fontSize: 9.5, marginBottom: 14 }}>회선 요약</div>
            <div className="col gap12">
              {[['연결 방식', '직접 연결'], ['연결 지역', '서울'], ['회선 상태', '양호'], ['재연결', '0회']].map(([k, v]) => (
                <div key={k} className="row between"><span className="smallcaps" style={{ fontSize: 9.5, letterSpacing: '.1em' }}>{k}</span><span className="mono" style={{ fontSize: 12.5, color: v === '양호' ? 'var(--good)' : 'var(--ink)' }}>{v}</span></div>
              ))}
            </div>
          </div>
        </div>
      </div>
      <style>{`@keyframes ping{0%{opacity:.6;transform:translate(-50%,-50%) scale(.5)}100%{opacity:0;transform:translate(-50%,-50%) scale(1)}}`}</style>
    </div>
  );
}

/* ============== USER SETTINGS ============== */
const INSTS = ['건반', '기타', '베이스', '드럼', '보컬'];
function SettingsScreen({ me }) {
  const [q, setQ] = useStateS((me && me.quality) || '저압축');
  return (
    <div className="screen scroll fade">
      <header style={{ padding: '26px 34px 18px', borderBottom: '1px solid var(--line)' }}>
        <div className="eyebrow">SETTINGS · 프로필과 환경</div>
        <h1 className="display" style={{ fontSize: 38, marginTop: 6 }}>설정</h1>
      </header>
      <div style={{ padding: '26px 34px 40px', display: 'grid', gridTemplateColumns: '1fr 1fr', gap: 26 }}>
        <div className="col gap24">
          <SettingCard title="프로필" sub="PROFILE">
            <div className="row gap16 center">
              <div style={{ position: 'relative' }}>
                <Avatar name={me.name} inst={me.inst} you size={64} />
              </div>
              <div className="col gap8" style={{ flex: 1 }}>
                <button className="btn btn-sm btn-ghost" style={{ alignSelf: 'flex-start' }}>사진 변경</button>
                <span style={{ fontSize: 11.5, color: 'var(--ink-faint)' }}>다른 연주자에게 보이는 프로필입니다</span>
              </div>
            </div>
            <div className="field"><label>이름</label><input className="input" defaultValue={me.name} /></div>
            <div className="field"><label>한 줄 소개</label><input className="input" placeholder="예) 새벽 감성 건반" /></div>
            <div className="row gap16">
              <div className="field" style={{ flex: 1 }}><label>기본 악기</label><select className="select" defaultValue={me.inst}>{INSTS.map(i => <option key={i}>{i}</option>)}</select></div>
              <div className="field" style={{ flex: 1 }}><label>기본 지역</label><select className="select" defaultValue="서울"><option>서울</option><option>부산</option><option>도쿄</option><option>오사카</option></select></div>
            </div>
          </SettingCard>

          <SettingCard title="기본 음질" sub="QUALITY">
            <div className="seg" style={{ width: '100%' }}>
              {Object.keys(QUALITY_S).map(k => <button key={k} className={q === k ? 'on' : ''} style={{ flex: 1 }} onClick={() => setQ(k)}>{k}</button>)}
            </div>
            <p style={{ fontSize: 12.5, color: 'var(--ink-soft)', lineHeight: 1.5 }}>{QUALITY_S[q]}</p>
          </SettingCard>
        </div>

        <div className="col gap24">
          <SettingCard title="환경 설정" sub="PREFERENCES">
            <PrefRow k="입장 시 자동 음소거" d="합주실에 들어갈 때 내 소리를 끈 상태로 시작" on={false} />
            <div className="hr-rule" />
            <PrefRow k="입장 알림" d="다른 연주자가 들어오면 알림" on={true} />
            <div className="hr-rule" />
            <PrefRow k="메시지 알림" d="채팅 메시지가 오면 알림" on={true} />
            <div className="hr-rule" />
            <PrefRow k="마지막 합주실 자동 입장" d="앱을 켜면 직전 합주실로 바로 입장" on={false} />
          </SettingCard>

          <SettingCard title="계정" sub="ACCOUNT">
            <div className="row between center">
              <div className="row gap12 center"><Avatar name={me.name} inst={me.inst} you size={40} /><div className="col" style={{ gap: 2 }}><span style={{ fontSize: 13, fontWeight: 600 }}>{me.name}</span><span className="mono" style={{ fontSize: 11, color: 'var(--ink-faint)' }}>moonlight@tempolink.app</span></div></div>
              <button className="btn btn-sm" style={{ borderColor: 'var(--bad)', color: 'var(--bad)' }}>로그아웃</button>
            </div>
          </SettingCard>
        </div>
      </div>
    </div>
  );
}
function PrefRow({ k, d, on }) {
  return (
    <div className="row between center">
      <div className="col" style={{ gap: 2, paddingRight: 16 }}><span style={{ fontSize: 13.5 }}>{k}</span><span style={{ fontSize: 11.5, color: 'var(--ink-faint)', lineHeight: 1.4 }}>{d}</span></div>
      <Toggle on={on} />
    </div>
  );
}

/* ---- helpers ---- */
function SettingCard({ title, sub, children }) {
  return (
    <div className="card" style={{ padding: 22 }}>
      <div className="row between center" style={{ marginBottom: 16 }}>
        <h3 className="display" style={{ fontSize: 22, whiteSpace: 'nowrap' }}>{title}</h3>
        <span className="smallcaps" style={{ fontSize: 9.5, color: 'var(--ink-faint)' }}>{sub}</span>
      </div>
      <div className="col gap16">{children}</div>
    </div>
  );
}
function Slider({ label, val, unit }) {
  const [v, setV] = useStateS(val);
  return (
    <div className="field"><div className="row between"><label>{label}</label><span className="mono" style={{ fontSize: 12, color: 'var(--gold)' }}>{v}{unit}</span></div>
      <input type="range" min="0" max="100" value={v} onChange={e => setV(+e.target.value)} style={{ width: '100%', accentColor: 'var(--gold)' }} /></div>
  );
}
function Toggle({ on: init }) {
  const [on, setOn] = useStateS(!!init);
  return <button onClick={() => setOn(o => !o)} style={{ width: 44, height: 24, flex: 'none', borderRadius: 999, border: '1px solid var(--line-strong)', background: on ? 'var(--gold)' : 'var(--bg-deep)', position: 'relative', cursor: 'pointer', transition: 'all .16s' }}>
    <span style={{ position: 'absolute', top: 2, left: on ? 22 : 2, width: 18, height: 18, borderRadius: '50%', background: on ? '#1a1505' : 'var(--ink-soft)', transition: 'all .16s' }} /></button>;
}

/* ============== CREATE ROOM MODAL ============== */
function CreateModal({ onClose, onCreate }) {
  const [name, setName] = useStateS('');
  const [copied, setCopied] = useStateS(false);
  const link = 'tempolink://join/aurora-7f3a2';
  return (
    <div style={{ position: 'absolute', inset: 0, zIndex: 50, background: 'color-mix(in srgb, var(--bg-deep) 78%, transparent)', backdropFilter: 'blur(3px)', display: 'grid', placeItems: 'center', padding: 30 }} onClick={onClose}>
      <div className="card starfield fade" style={{ width: 560, padding: 0, overflow: 'hidden' }} onClick={e => e.stopPropagation()}>
        <div style={{ padding: '22px 28px', borderBottom: '1px solid var(--line)' }} className="row between center">
          <div><div className="eyebrow">NEW ROOM · 합주실 생성</div><h2 className="display" style={{ fontSize: 28, marginTop: 4, whiteSpace: 'nowrap' }}>새 합주실 만들기</h2></div>
          <Glyph.crescent style={{ width: 26, height: 26, color: 'var(--gold)' }} />
        </div>
        <div style={{ padding: '24px 28px' }} className="col gap20">
          <div className="field"><label>합주실 이름</label><input className="input" placeholder="예) Aurora 새벽 세션" value={name} onChange={e => setName(e.target.value)} /></div>
          <div className="row gap16">
            <div className="field" style={{ flex: 1 }}><label>지역</label><select className="select"><option>서울</option><option>부산</option><option>도쿄</option><option>오사카</option></select></div>
            <div className="field" style={{ flex: 1 }}><label>내 파트</label><select className="select">{INSTS.map(i => <option key={i}>{i}</option>)}</select></div>
          </div>
          <div className="row gap16">
            <div className="field" style={{ flex: 1 }}><label>BPM</label><input className="input" defaultValue="92" /></div>
            <div className="field" style={{ flex: 1 }}><label>최대 인원</label><input className="input" defaultValue="6" /></div>
          </div>
          <div className="field"><label>초대 링크</label>
            <div className="row gap8">
              <input className="input mono" style={{ fontSize: 13, color: 'var(--steel)' }} readOnly value={link} />
              <button className="btn btn-ghost" onClick={() => { setCopied(true); setTimeout(() => setCopied(false), 1500); }}>{copied ? <Glyph.check style={{ width: 16, height: 16, color: 'var(--good)' }} /> : <Glyph.copy style={{ width: 16, height: 16 }} />}</button>
            </div>
          </div>
        </div>
        <div style={{ padding: '18px 28px', borderTop: '1px solid var(--line)' }} className="row between center">
          <span style={{ fontSize: 11.5, color: 'var(--ink-faint)' }}>링크를 받은 연주자가 바로 입장합니다</span>
          <div className="row gap10"><button className="btn btn-ghost" onClick={onClose}>취소</button><button className="btn btn-gold" onClick={() => onCreate(name || 'Aurora 새벽 세션')}>합주실 생성</button></div>
        </div>
      </div>
    </div>
  );
}

Object.assign(window, { AudioScreen, NetworkScreen, SettingsScreen, CreateModal });
