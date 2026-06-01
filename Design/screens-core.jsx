/* screens-core.jsx — Start / Login + Lobby */
const { useState: useStateC } = React;

/* ============== START / AUTH ============== */
function StartScreen({ onEnter }) {
  return (
    <div className="screen starfield fade" style={{ overflow: 'hidden' }}>
      <Glyph.asterisk className="corner-ast" style={{ width: 18, height: 18, top: 18, left: 22 }} />
      <Glyph.asterisk className="corner-ast" style={{ width: 18, height: 18, bottom: 18, right: 22 }} />

      <div style={{ display: 'grid', gridTemplateColumns: '1.15fr .85fr', height: '100%' }}>
        <StartHeroPanel />
        <AuthPanel onEnter={onEnter} />
      </div>
    </div>
  );
}

function StartHeroPanel() {
  return (
    <div className="start-hero">
      <div className="row between center">
        <div className="eyebrow">TempoLink</div>
      </div>
      <div className="start-wave">
        <Waveform n={48} seed={7} />
      </div>

      <div className="start-brand">
        <div className="start-moon">
          <div />
        </div>
        <h1 className="display">Tempo<span>Link</span></h1>
        <p>함께 연주할 준비가 끝났습니다.</p>
      </div>

      <div className="hr-double" />
    </div>
  );
}

function AuthPanel({ onEnter }) {
  const modes = ['로그인', '회원가입', 'Google'];
  const [mode, setMode] = useStateC(modes[0]);
  const [email, setEmail] = useStateC('');
  const [password, setPassword] = useStateC('');
  const [name, setName] = useStateC('');
  const [message, setMessage] = useStateC('');

  const submit = () => {
    if (mode === 'Google') {
      setMessage('Google 로그인 프로토타입입니다.');
      onEnter();
      return;
    }

    if (!email.trim() || !password.trim() || (mode === '회원가입' && !name.trim())) {
      setMessage('필수 정보를 입력해 주세요.');
      return;
    }

    setMessage(`${mode} 프로토타입이 완료되었습니다.`);
    onEnter();
  };

  return (
    <div className="auth-panel">
      <div>
        <div className="eyebrow">ACCOUNT · 시작하기</div>
        <h2 className="display">{mode === 'Google' ? 'Google로 계속하기' : mode}</h2>
      </div>

      <div className="seg auth-tabs">
        {modes.map(m => <button key={m} className={mode === m ? 'on' : ''} onClick={() => { setMode(m); setMessage(''); }}>{m}</button>)}
      </div>

      {mode === 'Google' ? (
        <div className="auth-google">
          <button className="btn btn-gold auth-submit" onClick={submit}>
            <Glyph.google style={{ width: 16, height: 16 }} /> Google로 계속하기
          </button>
          <p>브라우저 인증 연동 전 화면 흐름을 확인하는 프로토타입입니다.</p>
        </div>
      ) : (
        <div className="col gap20">
          {mode === '회원가입' && (
            <div className="field">
              <label>이름</label>
              <input className="input" placeholder="예) 문라이트" value={name} onChange={e => setName(e.target.value)} />
            </div>
          )}
          <div className="field">
            <label>이메일</label>
            <input className="input" type="email" placeholder="you@tempolink.app" value={email} onChange={e => setEmail(e.target.value)} />
          </div>
          <div className="field">
            <label>비밀번호</label>
            <input className="input" type="password" placeholder="비밀번호" value={password} onChange={e => setPassword(e.target.value)} />
          </div>
          <button className="btn btn-gold auth-submit" onClick={submit}>
            {mode === '회원가입' ? '계정 만들기' : '로그인'} <Glyph.crescent style={{ width: 15, height: 15 }} />
          </button>
        </div>
      )}

      <p className="mono auth-message">{message || ' '}</p>
    </div>
  );
}

/* ============== LOBBY ============== */
function LobbyScreen({ rooms, onJoin, onCreate }) {
  const [q, setQ] = useStateC('');
  const [filt, setFilt] = useStateC('전체');
  const filters = ['전체', '연주 중', '대기', 'KR', 'JP'];
  const list = rooms.filter(r => {
    if (q && !r.name.includes(q) && !r.host.includes(q)) return false;
    if (filt === '연주 중') return r.live;
    if (filt === '대기') return !r.live;
    if (filt === 'KR' || filt === 'JP') return r.region.includes(filt);
    return true;
  });
  return (
    <div className="screen fade">
      <header style={{ padding: '26px 34px 18px', borderBottom: '1px solid var(--line)' }}>
        <div className="row between center">
          <div>
            <div className="eyebrow">LOBBY — 스튜디오 목록</div>
            <h1 className="display" style={{ fontSize: 40, marginTop: 6, whiteSpace: 'nowrap' }}>오늘의 합주실</h1>
          </div>
          <button className="btn btn-gold" onClick={onCreate}><Glyph.plus style={{ width: 15, height: 15 }} /> 룸 만들기</button>
        </div>
        <div className="row between center" style={{ marginTop: 20, gap: 16 }}>
          <div className="row center" style={{ flex: 1, maxWidth: 380, position: 'relative' }}>
            <Glyph.search style={{ width: 16, height: 16, position: 'absolute', left: 13, color: 'var(--ink-faint)' }} />
            <input className="input" style={{ paddingLeft: 38 }} placeholder="룸 또는 호스트 검색" value={q} onChange={e => setQ(e.target.value)} />
          </div>
          <div className="seg">
            {filters.map(f => <button key={f} className={filt === f ? 'on' : ''} onClick={() => setFilt(f)}>{f}</button>)}
          </div>
        </div>
      </header>

      <div className="scroll" style={{ padding: '24px 34px 40px', flex: 1 }}>
        <div style={{ display: 'grid', gridTemplateColumns: 'repeat(2, 1fr)', gap: 18 }}>
          {list.map(r => <RoomCard key={r.id} r={r} onJoin={onJoin} />)}
        </div>
        {list.length === 0 && <div className="mono" style={{ color: 'var(--ink-faint)', textAlign: 'center', marginTop: 60 }}>조건에 맞는 합주실이 없습니다.</div>}
      </div>
    </div>
  );
}

function RoomCard({ r, onJoin }) {
  return (
    <div className="card" style={{ padding: 20, display: 'flex', flexDirection: 'column', gap: 14, cursor: 'pointer' }} onClick={() => onJoin(r)}>
      <Glyph.asterisk className="ast" style={{ width: 13, height: 13, position: 'absolute', top: 14, right: 14, opacity: .5 }} />
      <div className="row gap8 center">
        <span className={`badge ${r.live ? 'live' : 'idle'}`}><span className="dot" />{r.live ? '연주 중' : '대기'}</span>
        <span className="badge">{r.region}</span>
        <span className="badge">{r.bpm} BPM</span>
      </div>
      <div>
        <h3 className="display" style={{ fontSize: 27 }}>{r.name}</h3>
        <div className="mono" style={{ fontSize: 11, color: 'var(--ink-faint)', marginTop: 3 }}>HOST · {r.host}</div>
      </div>
      <div style={{ height: 34, color: r.live ? 'var(--steel)' : 'var(--ink-faint)' }}>
        <Waveform n={40} seed={r.id} active={r.live} />
      </div>
      <div className="row between center">
        <div className="row" style={{ marginLeft: 4 }}>
          {r.members.slice(0, 4).map((m, i) => (
            <div key={i} style={{ marginLeft: i ? -10 : 0 }}><Avatar name={m.name} inst={m.inst} size={32} /></div>
          ))}
          {r.members.length > 4 && <div className="ava" style={{ width: 32, height: 32, marginLeft: -10, fontSize: 11 }}>+{r.members.length - 4}</div>}
        </div>
        <div className="row gap10 center">
          <div className="col" style={{ alignItems: 'flex-end', gap: 1 }}>
            <span className="smallcaps" style={{ fontSize: 8.5, letterSpacing: '.16em' }}>평균 RTT</span>
            <RttPill ms={r.rtt} />
          </div>
          <button className="btn btn-sm btn-gold" onClick={(e) => { e.stopPropagation(); onJoin(r); }}>입장</button>
        </div>
      </div>
    </div>
  );
}

Object.assign(window, { StartScreen, LobbyScreen });
