package com.tempolink.roomservice.dto;

import java.time.Instant;

public record NoticeResponse(
    String title,
    String body,
    Instant publishedAt
) {
}
