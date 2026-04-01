package com.tempolink.roomservice.controller;

import org.springframework.core.io.FileSystemResource;
import org.springframework.core.io.Resource;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpStatus;
import org.springframework.http.MediaType;
import org.springframework.http.ResponseEntity;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.GetMapping;
import org.springframework.web.bind.annotation.PathVariable;

import java.io.File;

@Controller
public class DownloadController {

    // 실제 빌드된 클라이언트 인스톨러 파일들이 저장될 서버 내부의 경로입니다.
    private static final String DOWNLOAD_DIR = "/app/downloads/";

    // 단일 메인 페이지 (다운로드 허브)
    @GetMapping("/")
    public String mainPage(Model model) {
        model.addAttribute("clientVersion", "0.1.0");
        return "download";
    }

    // 예전 주소(/download)로 접속하는 분들을 위한 리다이렉트
    @GetMapping("/download")
    public String oldDownloadPageRedirect() {
        return "redirect:/";
    }

    // 인스톨러 전용 다운로드 API
    @GetMapping("/api/download/installer/{os}")
    public ResponseEntity<Resource> downloadInstaller(@PathVariable("os") String os) {
        String fileName = switch (os.toLowerCase()) {
            case "windows" -> "TempoLink-Installer.exe";
            case "macos" -> "TempoLink-Installer.pkg";
            case "ubuntu" -> "TempoLink-Installer.deb"; // 사용자 환경 맞춤 설치형식
            default -> null;
        };

        if (fileName == null) {
            return ResponseEntity.badRequest().build();
        }

        File file = new File(DOWNLOAD_DIR + fileName);
        if (!file.exists()) {
            return ResponseEntity.status(HttpStatus.NOT_FOUND).build();
        }

        Resource resource = new FileSystemResource(file);
        return ResponseEntity.ok()
                .header(HttpHeaders.CONTENT_DISPOSITION, "attachment; filename=\"" + fileName + "\"")
                .contentType(MediaType.APPLICATION_OCTET_STREAM)
                .body(resource);
    }
}
