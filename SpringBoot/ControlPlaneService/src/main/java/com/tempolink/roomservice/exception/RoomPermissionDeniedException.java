package com.tempolink.roomservice.exception;

public class RoomPermissionDeniedException extends RuntimeException {
  public RoomPermissionDeniedException(String roomCode) {
    super("Permission denied for room: " + roomCode);
  }
}

