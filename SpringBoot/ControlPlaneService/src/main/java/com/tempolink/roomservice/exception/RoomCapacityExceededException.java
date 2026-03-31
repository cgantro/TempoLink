package com.tempolink.roomservice.exception;

public class RoomCapacityExceededException extends RuntimeException {
  public RoomCapacityExceededException(String roomCode) {
    super("Room is full: " + roomCode);
  }
}

