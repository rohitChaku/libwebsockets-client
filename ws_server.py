import asyncio
import websockets

# WebSocket server handler
async def websocket_handler(websocket, path):
    print("New WebSocket connection")

    try:
        # Receive messages from the client
        async for message in websocket:
            print(f"Received message: {message}")

            # Send a response back to the client
            response = f"Server received: {message}"
            await websocket.send(response)
            print(f"Sent response: {response}")
    finally:
        print("WebSocket connection closed")

# Run the WebSocket server
start_server = websockets.serve(websocket_handler, 'localhost', 8000)

# Start the event loop
asyncio.get_event_loop().run_until_complete(start_server)
asyncio.get_event_loop().run_forever()
