package com.tempolink.roomservice.dto;

public record AuthProviderInfoResponse(
    String id,
    boolean enabled,
    String startUrl
) {
}

