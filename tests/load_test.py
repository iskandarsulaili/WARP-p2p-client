#!/usr/bin/env python3
"""
P2P Load Testing Script

Tests P2P coordinator and WebRTC connections under load with multiple concurrent peers.

Usage:
    python3 load_test.py --peers 50 --duration 300 --coordinator ws://localhost:8001/api/signaling/ws
"""

import asyncio
import websockets
import json
import time
import argparse
import statistics
from datetime import datetime
from typing import List, Dict
import random
import string

class P2PPeer:
    """Simulated P2P peer for load testing"""
    
    def __init__(self, peer_id: str, coordinator_url: str):
        self.peer_id = peer_id
        self.coordinator_url = coordinator_url
        self.ws = None
        self.connected = False
        self.messages_sent = 0
        self.messages_received = 0
        self.latencies = []
        self.errors = 0
        
    async def connect(self):
        """Connect to P2P coordinator"""
        try:
            url = f"{self.coordinator_url}?peer_id={self.peer_id}"
            self.ws = await websockets.connect(url)
            self.connected = True
            print(f"[{self.peer_id}] Connected to coordinator")
            return True
        except Exception as e:
            print(f"[{self.peer_id}] Connection failed: {e}")
            self.errors += 1
            return False
    
    async def join_session(self, session_id: str):
        """Join a P2P session"""
        try:
            message = {
                "type": "join",
                "session_id": session_id
            }
            await self.ws.send(json.dumps(message))
            self.messages_sent += 1
            print(f"[{self.peer_id}] Joined session: {session_id}")
        except Exception as e:
            print(f"[{self.peer_id}] Join failed: {e}")
            self.errors += 1
    
    async def send_offer(self, target_peer: str):
        """Send WebRTC offer to peer"""
        try:
            start_time = time.time()
            
            message = {
                "type": "offer",
                "to": target_peer,
                "sdp": {
                    "type": "offer",
                    "sdp": "v=0\r\no=- 123456789 2 IN IP4 127.0.0.1\r\n..."
                }
            }
            await self.ws.send(json.dumps(message))
            self.messages_sent += 1
            
            latency = (time.time() - start_time) * 1000
            self.latencies.append(latency)
            
        except Exception as e:
            print(f"[{self.peer_id}] Send offer failed: {e}")
            self.errors += 1
    
    async def receive_messages(self):
        """Receive messages from coordinator"""
        try:
            async for message in self.ws:
                data = json.loads(message)
                self.messages_received += 1
                
                # Simulate processing different message types
                msg_type = data.get("type")
                if msg_type == "offer":
                    # Respond with answer
                    await self.send_answer(data.get("from"))
                elif msg_type == "ice-candidate":
                    # Process ICE candidate
                    pass
                    
        except Exception as e:
            print(f"[{self.peer_id}] Receive error: {e}")
            self.errors += 1
    
    async def send_answer(self, target_peer: str):
        """Send WebRTC answer to peer"""
        try:
            message = {
                "type": "answer",
                "to": target_peer,
                "sdp": {
                    "type": "answer",
                    "sdp": "v=0\r\no=- 987654321 2 IN IP4 127.0.0.1\r\n..."
                }
            }
            await self.ws.send(json.dumps(message))
            self.messages_sent += 1
        except Exception as e:
            print(f"[{self.peer_id}] Send answer failed: {e}")
            self.errors += 1
    
    async def disconnect(self):
        """Disconnect from coordinator"""
        if self.ws:
            await self.ws.close()
            self.connected = False
            print(f"[{self.peer_id}] Disconnected")
    
    def get_stats(self) -> Dict:
        """Get peer statistics"""
        return {
            "peer_id": self.peer_id,
            "messages_sent": self.messages_sent,
            "messages_received": self.messages_received,
            "avg_latency_ms": statistics.mean(self.latencies) if self.latencies else 0,
            "min_latency_ms": min(self.latencies) if self.latencies else 0,
            "max_latency_ms": max(self.latencies) if self.latencies else 0,
            "errors": self.errors
        }

async def run_load_test(num_peers: int, duration_seconds: int, coordinator_url: str):
    """Run load test with specified number of peers"""
    
    print(f"\n{'='*60}")
    print(f"P2P Load Test")
    print(f"{'='*60}")
    print(f"Peers: {num_peers}")
    print(f"Duration: {duration_seconds}s")
    print(f"Coordinator: {coordinator_url}")
    print(f"{'='*60}\n")
    
    # Create peers
    peers = []
    for i in range(num_peers):
        peer_id = f"load-test-peer-{i:04d}"
        peer = P2PPeer(peer_id, coordinator_url)
        peers.append(peer)
    
    # Connect all peers
    print(f"Connecting {num_peers} peers...")
    connect_tasks = [peer.connect() for peer in peers]
    results = await asyncio.gather(*connect_tasks)
    
    connected_count = sum(1 for r in results if r)
    print(f"Connected: {connected_count}/{num_peers} peers\n")
    
    if connected_count == 0:
        print("ERROR: No peers connected. Exiting.")
        return
    
    # Join session
    session_id = "load-test-session"
    print(f"Joining session: {session_id}")
    for peer in peers:
        if peer.connected:
            await peer.join_session(session_id)
    
    await asyncio.sleep(2)  # Wait for session join
    
    # Start receiving messages
    receive_tasks = [peer.receive_messages() for peer in peers if peer.connected]
    
    # Run test for specified duration
    print(f"\nRunning load test for {duration_seconds} seconds...")
    start_time = time.time()
    
    # Simulate peer-to-peer offers
    while time.time() - start_time < duration_seconds:
        # Random peer sends offer to another random peer
        if len(peers) >= 2:
            sender = random.choice([p for p in peers if p.connected])
            receiver = random.choice([p for p in peers if p.connected and p != sender])
            await sender.send_offer(receiver.peer_id)
        
        await asyncio.sleep(0.1)  # 10 messages/second per peer
    
    print(f"\nTest duration complete. Disconnecting peers...")
    
    # Disconnect all peers
    for peer in peers:
        if peer.connected:
            await peer.disconnect()
    
    # Print statistics
    print(f"\n{'='*60}")
    print(f"Load Test Results")
    print(f"{'='*60}\n")
    
    total_sent = sum(p.messages_sent for p in peers)
    total_received = sum(p.messages_received for p in peers)
    total_errors = sum(p.errors for p in peers)
    all_latencies = [lat for p in peers for lat in p.latencies]
    
    print(f"Total Messages Sent: {total_sent}")
    print(f"Total Messages Received: {total_received}")
    print(f"Total Errors: {total_errors}")
    print(f"Messages/Second: {total_sent / duration_seconds:.2f}")
    
    if all_latencies:
        print(f"\nLatency Statistics:")
        print(f"  Average: {statistics.mean(all_latencies):.2f} ms")
        print(f"  Median: {statistics.median(all_latencies):.2f} ms")
        print(f"  Min: {min(all_latencies):.2f} ms")
        print(f"  Max: {max(all_latencies):.2f} ms")
        print(f"  Std Dev: {statistics.stdev(all_latencies):.2f} ms" if len(all_latencies) > 1 else "  Std Dev: N/A")
    
    print(f"\n{'='*60}\n")

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="P2P Load Testing Script")
    parser.add_argument("--peers", type=int, default=10, help="Number of concurrent peers")
    parser.add_argument("--duration", type=int, default=60, help="Test duration in seconds")
    parser.add_argument("--coordinator", type=str, default="ws://localhost:8001/api/signaling/ws", help="Coordinator WebSocket URL")
    
    args = parser.parse_args()
    
    asyncio.run(run_load_test(args.peers, args.duration, args.coordinator))

