package com.tempolink.roomservice.api;

import com.tempolink.roomservice.common.ServiceConstants;
import com.tempolink.roomservice.exception.RoomCapacityExceededException;
import com.tempolink.roomservice.exception.RoomNotFoundException;
import com.tempolink.roomservice.exception.RoomPermissionDeniedException;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.ExceptionHandler;
import org.springframework.web.bind.annotation.RestControllerAdvice;

import java.time.Instant;
import java.util.Map;

@RestControllerAdvice
public class ApiExceptionHandler {
  @ExceptionHandler(RoomNotFoundException.class)
  public ResponseEntity<Map<String, Object>> handleNotFound(RoomNotFoundException ex) {
    return ResponseEntity.status(HttpStatus.NOT_FOUND).body(Map.of(
        "timestamp", Instant.now().toString(),
        "error", ServiceConstants.ERROR_ROOM_NOT_FOUND,
        "message", ex.getMessage()
    ));
  }

  @ExceptionHandler(RoomCapacityExceededException.class)
  public ResponseEntity<Map<String, Object>> handleRoomFull(RoomCapacityExceededException ex) {
    return ResponseEntity.status(HttpStatus.CONFLICT).body(Map.of(
        "timestamp", Instant.now().toString(),
        "error", ServiceConstants.ERROR_ROOM_FULL,
        "message", ex.getMessage()
    ));
  }

  @ExceptionHandler(RoomPermissionDeniedException.class)
  public ResponseEntity<Map<String, Object>> handleForbidden(RoomPermissionDeniedException ex) {
    return ResponseEntity.status(HttpStatus.FORBIDDEN).body(Map.of(
        "timestamp", Instant.now().toString(),
        "error", ServiceConstants.ERROR_ROOM_FORBIDDEN,
        "message", ex.getMessage()
    ));
  }

  @ExceptionHandler(IllegalArgumentException.class)
  public ResponseEntity<Map<String, Object>> handleBadRequest(IllegalArgumentException ex) {
    return ResponseEntity.status(HttpStatus.BAD_REQUEST).body(Map.of(
        "timestamp", Instant.now().toString(),
        "error", ServiceConstants.ERROR_INVALID_REQUEST,
        "message", ex.getMessage()
    ));
  }
}
