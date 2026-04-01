package com.tempolink.roomservice.common;

public final class ServiceConstants {
  private ServiceConstants() {}

  public static final String DEFAULT_RELAY_HOST = "127.0.0.1";
  public static final int DEFAULT_RELAY_PORT = 40000;
  public static final int DEFAULT_MAX_PARTICIPANTS = 6;
  public static final int MAX_ROOM_PARTICIPANTS = 6;

  public static final String TEST_ROOM_CODE = "TEST00";
  public static final String TEST_ROOM_HOST_USER_ID = "test-room-host";
  public static final int TEST_ROOM_MAX_PARTICIPANTS = 6;

  public static final String ERROR_ROOM_NOT_FOUND = "ROOM_NOT_FOUND";
  public static final String ERROR_ROOM_FULL = "ROOM_FULL";
  public static final String ERROR_ROOM_FORBIDDEN = "ROOM_FORBIDDEN";
  public static final String ERROR_INVALID_REQUEST = "INVALID_REQUEST";
}
