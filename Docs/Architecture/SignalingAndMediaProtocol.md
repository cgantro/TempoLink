# Signaling And Media Protocol (MVP v1)

## Signaling (WebSocket)

Endpoint: `/ws/signaling?roomCode={roomCode}&userId={userId}`

Envelope:

```json
{
  "type": "signal.candidate",
  "roomCode": "AB12CD",
  "fromUserId": "u1",
  "toUserId": "u2",
  "payload": {
    "candidateType": "srflx",
    "ip": "203.0.113.10",
    "port": 51234
  }
}
```

Server-generated events:

- `room.joined`
- `peer.joined`
- `peer.left`
- `signal.error`

## ICE/TURN bootstrap

REST endpoint: `GET /api/network/ice`

Response:

```json
{
  "iceServers": [
    {
      "urls": ["stun:stun.l.google.com:19302"],
      "username": null,
      "credential": null
    },
    {
      "urls": ["turn:turn.example.com:3478?transport=udp"],
      "username": "temp-user",
      "credential": "temp-pass"
    }
  ],
  "credentialTtlSeconds": 600
}
```

## Media packet (UDP binary)

`PacketHeader` fields:

- `magic`, `version`, `type`
- `room_id`, `sender_id`, `sequence`
- `timestamp_us`
- `stream_id`
- `sample_rate_hz`
- `frame_samples`
- `channels`
- `payload_size`

Clock sync packet types:

- `kClockSync` (client -> server)
- `kClockSyncAck` (server -> client)

`kClockSyncAck` payload (24 bytes):

- 0..7: client send timestamp (us)
- 8..15: server receive timestamp (us)
- 16..23: server send timestamp (us)

