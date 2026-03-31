package com.tempolink.roomservice.exception;

public class RoomNotFoundException extends RuntimeException {
  public RoomNotFoundException(String roomCode) {
    super("Room not found: " + roomCode);
  }
}

