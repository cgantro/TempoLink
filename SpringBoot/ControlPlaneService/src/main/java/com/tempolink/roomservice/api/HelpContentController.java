package com.tempolink.roomservice.api;

import com.tempolink.roomservice.dto.DocumentResponse;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

@RestController
public class HelpContentController {
  @GetMapping("/api/manual")
  public ResponseEntity<DocumentResponse> manual() {
    return ResponseEntity.ok(new DocumentResponse("""
        # TempoLink Manual
        - 오디오 장치: 입력/출력 장치를 먼저 선택하세요.
        - 권장 환경: 유선 네트워크 + 저지연 장치(ASIO/CoreAudio).
        - 룸 입장 전: 파트/장치/입력레벨을 확인하세요.
        - 세션 중: 참가자별 지연/손실/연결상태를 확인하세요.
        - 연결 실패: 재접속 버튼으로 다시 시도하세요.
        """));
  }

  @GetMapping("/api/faq")
  public ResponseEntity<DocumentResponse> faq() {
    return ResponseEntity.ok(new DocumentResponse("""
        # TempoLink Q&A
        Q. 왜 지연이 커지나요?
        A. 무선 환경, 높은 버퍼, 피크 시간대 회선 혼잡이 주요 원인입니다.

        Q. 장치가 안 잡히면?
        A. Settings에서 입력/출력 장치를 다시 선택하고 Apply를 누르세요.

        Q. 방은 몇 명까지 가능한가요?
        A. 현재 최대 6명입니다.
        """));
  }
}
