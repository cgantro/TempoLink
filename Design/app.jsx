/* app.jsx — data, router, tweaks, window scaler */
const { useState: useStateA, useEffect: useEffectA, useRef: useRefA } = React;

/* ---------------- mock data ---------------- */
const ME = { name: '문라이트', inst: '건반', rtt: 4, you: true, region: '서울 KR', quality: '저압축' };
const ROOMS = [
  { id: 11, name: 'Aurora 새벽 세션', host: '윤도현', live: true, region: '서울 KR', bpm: 92, rtt: 12,
    members: [{ name: '윤도현', inst: '기타', rtt: 9, region: '서울 KR' }, { name: '하루', inst: '드럼', rtt: 14, region: '부산 KR' }, { name: 'KENTA', inst: '베이스', rtt: 28, region: '도쿄 JP' }, { name: '서린', inst: '보컬', rtt: 11, region: '서울 KR' }] },
  { id: 23, name: 'Moonchild 합주실', host: 'AKIRA', live: true, region: '도쿄 JP', bpm: 124, rtt: 26,
    members: [{ name: 'AKIRA', inst: '건반', rtt: 24, region: '도쿄 JP' }, { name: 'MIYU', inst: '기타', rtt: 31, region: '오사카 JP' }, { name: '준', inst: '드럼', rtt: 22, region: '도쿄 JP' }] },
  { id: 31, name: '심야 재즈 트리오', host: '강이든', live: false, region: '서울 KR', bpm: 78, rtt: 8,
    members: [{ name: '강이든', inst: '베이스', rtt: 7, region: '서울 KR' }, { name: '노아', inst: '건반', rtt: 10, region: '서울 KR' }] },
  { id: 42, name: 'Eclipse 리허설', host: 'RINA', live: false, region: '오사카 JP', bpm: 140, rtt: 33,
    members: [{ name: 'RINA', inst: '보컬', rtt: 30, region: '오사카 JP' }, { name: 'SORA', inst: '기타', rtt: 35, region: '도쿄 JP' }, { name: '타쿠', inst: '드럼', rtt: 29, region: '도쿄 JP' }, { name: '미카', inst: '베이스', rtt: 38, region: '오사카 JP' }, { name: 'YUKI', inst: '건반', rtt: 27, region: '도쿄 JP' }] },
];

/* ---------------- palette / accent application ---------------- */
const ACCENTS = {
  '골드': ['#cba14c', '#a07e34'], '앰버': ['#d98a5a', '#b56a2c'],
  '로즈': ['#c98088', '#a85a62'], '스틸': ['#8fa9d6', '#4f6fa6'],
};
const DARK_TONE = {
  '진하게': { '--bg': '#070f25', '--bg-deep': '#050a1c', '--panel': '#0b1838', '--panel-2': '#0e2046', '--raised': '#122850' },
  '기본': { '--bg': '#0a1430', '--bg-deep': '#070f25', '--panel': '#0e1c40', '--panel-2': '#112450', '--raised': '#15295a' },
  '연하게': { '--bg': '#0e1d40', '--bg-deep': '#0a1633', '--panel': '#142a58', '--panel-2': '#173063', '--raised': '#1c3870' },
};
const LIGHT_TONE = {
  '진하게': { '--bg': '#e7d8b1', '--bg-deep': '#ddccA0', '--panel': '#efe2c1', '--panel-2': '#e6d6af', '--raised': '#f3e8ca' },
  '기본': { '--bg': '#efe6cd', '--bg-deep': '#e6dabb', '--panel': '#f6efd9', '--panel-2': '#efe5c8', '--raised': '#f9f3e0' },
  '연하게': { '--bg': '#f5eedb', '--bg-deep': '#ece2c8', '--panel': '#faf5e6', '--panel-2': '#f4ecd6', '--raised': '#fdf9ee' },
};

const TWEAK_DEFAULTS = /*EDITMODE-BEGIN*/{
  "theme": "dark",
  "paper": true,
  "displayFont": "세리프",
  "accent": "골드",
  "tone": "기본",
  "participantView": "파형 리스트"
}/*EDITMODE-END*/;

const VIEW_MAP = { '파형 리스트': 'list', '천체 궤도': 'orbit', '믹싱 콘솔': 'console' };

function App() {
  const [t, setTweak] = useTweaks(TWEAK_DEFAULTS);
  const [screen, setScreen] = useStateA('start');
  const [room, setRoom] = useStateA(ROOMS[0]);
  const [showCreate, setShowCreate] = useStateA(false);
  const winRef = useRefA(null);

  /* apply theme + palette */
  useEffectA(() => {
    const root = document.documentElement;
    root.classList.toggle('theme-light', t.theme === 'light');
    root.classList.toggle('theme-dark', t.theme !== 'light');
    document.getElementById('desktop').classList.toggle('paper-on', !!t.paper);
    root.style.setProperty('--display', t.displayFont === '산세리프' ? 'var(--font-display-sans)' : 'var(--font-display)');
    const [g, gd] = ACCENTS[t.accent] || ACCENTS['골드'];
    root.style.setProperty('--gold', g); root.style.setProperty('--gold-deep', gd);
    const tone = (t.theme === 'light' ? LIGHT_TONE : DARK_TONE)[t.tone] || {};
    Object.entries(tone).forEach(([k, v]) => root.style.setProperty(k, v));
  }, [t.theme, t.paper, t.displayFont, t.accent, t.tone]);

  /* scale window to fit viewport */
  useEffectA(() => {
    const fit = () => {
      const pad = 48;
      const s = Math.min((innerWidth - pad) / 1280, (innerHeight - pad) / 820, 1);
      if (winRef.current) winRef.current.style.transform = `scale(${s})`;
    };
    fit(); addEventListener('resize', fit);
    return () => removeEventListener('resize', fit);
  }, []);

  const join = (r) => { setRoom(r); setScreen('room'); };

  return (
    <div ref={winRef} style={{ transformOrigin: 'center center' }}>
      <MacChrome
        screen={screen}
        onNav={setScreen}
        showRail={screen !== 'start'}
        rttText={screen === 'room' ? `${room.region.split(' ')[0]} · 지연 ${room.rtt}ms` : '직접 연결 · 안정'}
      >
        {screen === 'start' && <StartScreen onEnter={() => setScreen('lobby')} />}
        {screen === 'lobby' && <LobbyScreen rooms={ROOMS} onJoin={join} onCreate={() => setShowCreate(true)} />}
        {screen === 'room' && <RoomScreen room={room} me={ME} view={VIEW_MAP[t.participantView]} onLeave={() => setScreen('lobby')} />}
        {screen === 'audio' && <AudioScreen me={ME} />}
        {screen === 'network' && <NetworkScreen room={room} />}
        {screen === 'settings' && <SettingsScreen me={ME} />}
        {showCreate && <CreateModal onClose={() => setShowCreate(false)} onCreate={() => { setShowCreate(false); setScreen('lobby'); }} />}
      </MacChrome>

      <TweaksPanel>
        <TweakSection label="테마 · THEME" />
        <TweakRadio label="모드" value={t.theme} options={['dark', 'light']} onChange={v => setTweak('theme', v)} />
        <TweakToggle label="종이 질감" value={t.paper} onChange={v => setTweak('paper', v)} />
        <TweakRadio label="팔레트 강도" value={t.tone} options={['진하게', '기본', '연하게']} onChange={v => setTweak('tone', v)} />
        <TweakColor label="골드 악센트" value={(ACCENTS[t.accent] || ACCENTS['골드'])[0]}
          options={Object.values(ACCENTS).map(a => a[0])}
          onChange={v => { const key = Object.keys(ACCENTS).find(k => ACCENTS[k][0] === v); setTweak('accent', key || '골드'); }} />
        <TweakSection label="타이포 · TYPE" />
        <TweakRadio label="디스플레이 폰트" value={t.displayFont} options={['세리프', '산세리프']} onChange={v => setTweak('displayFont', v)} />
        <TweakSection label="합주 룸 · LAYOUT" />
        <TweakRadio label="참가자 표현" value={t.participantView} options={['파형 리스트', '천체 궤도', '믹싱 콘솔']} onChange={v => setTweak('participantView', v)} />
        <TweakButton label="합주 룸으로 이동" onClick={() => setScreen('room')} />
      </TweaksPanel>
    </div>
  );
}

ReactDOM.createRoot(document.getElementById('root')).render(<App />);
