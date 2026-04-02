package com.tempolink.roomservice.controller;

import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RestController;

import java.util.Map;

@RestController
@RequestMapping("/api/version")
public class VersionController {

    // 향후 DB나 application.yml에서 값을 읽어오도록 구성할 수 있습니다.
    private static final String LATEST_VERSION = "0.1.0";

    @GetMapping("/latest")
    public ResponseEntity<Map<String, Object>> getLatestVersion() {
        return ResponseEntity.ok(Map.of(
                "version", LATEST_VERSION,
                "forceUpdate", false // true일 경우 클라이언트에서 앱 실행을 막고 강제 업데이트 유도
        ));
    }
}