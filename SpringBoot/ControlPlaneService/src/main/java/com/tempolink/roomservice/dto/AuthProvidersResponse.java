package com.tempolink.roomservice.dto;

import java.util.List;

public record AuthProvidersResponse(
    List<AuthProviderInfoResponse> providers
) {
}

