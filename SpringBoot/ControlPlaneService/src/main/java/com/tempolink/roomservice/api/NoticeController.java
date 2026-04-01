package com.tempolink.roomservice.api;

import com.tempolink.roomservice.dto.NoticeResponse;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.time.Instant;
import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/notices")
public class NoticeController {
  @GetMapping
  public ResponseEntity<Map<String, List<NoticeResponse>>> list() {
    return ResponseEntity.ok(Map.of("notices", List.of(
        new NoticeResponse(
            "TempoLink TestRoom 안내",
            "TestRoom(TEST00)은 음질/네트워크 점검용으로 항상 제공됩니다.",
            Instant.now().minusSeconds(3600)),
        new NoticeResponse(
            "저지연 권장 설정",
            "가급적 유선 네트워크, ASIO/CoreAudio, 64~128 샘플 버퍼를 권장합니다.",
            Instant.now().minusSeconds(7200)),
        new NoticeResponse(
            "P2P 연결 실패 처리",
            "현재 빌드는 P2P 우선 정책이며 direct 연결 실패 시 세션 참여가 실패할 수 있습니다.",
            Instant.now().minusSeconds(10800))
    )));
  }
}
