package com.tempolink.roomservice.api;

import com.tempolink.roomservice.dto.CreateRoomRequest;
import com.tempolink.roomservice.dto.JoinRoomRequest;
import com.tempolink.roomservice.dto.LeaveRoomRequest;
import com.tempolink.roomservice.dto.RoomResponse;
import com.tempolink.roomservice.service.RoomService;
import jakarta.validation.Valid;
import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;

import java.util.List;
import java.util.Map;

@RestController
@RequestMapping("/api/rooms")
public class RoomController {
  private final RoomService roomService;

  public RoomController(RoomService roomService) {
    this.roomService = roomService;
  }

  @PostMapping
  public ResponseEntity<RoomResponse> create(@Valid @RequestBody CreateRoomRequest request) {
    return ResponseEntity.ok(roomService.create(request));
  }

  @GetMapping
  public ResponseEntity<List<RoomResponse>> list() {
    return ResponseEntity.ok(roomService.list());
  }

  @GetMapping("/{roomCode}")
  public ResponseEntity<RoomResponse> get(@PathVariable String roomCode) {
    return ResponseEntity.ok(roomService.get(roomCode));
  }

  @PostMapping("/{roomCode}/join")
  public ResponseEntity<RoomResponse> join(@PathVariable String roomCode,
                                           @Valid @RequestBody JoinRoomRequest request) {
    return ResponseEntity.ok(roomService.join(roomCode, request.userId()));
  }

  @PostMapping("/{roomCode}/leave")
  public ResponseEntity<Map<String, Object>> leave(@PathVariable String roomCode,
                                                   @Valid @RequestBody LeaveRoomRequest request) {
    RoomResponse response = roomService.leave(roomCode, request.userId());
    return ResponseEntity.ok(Map.of(
        "room", response,
        "message", "left room"
    ));
  }
}

