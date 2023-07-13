#include <stdio.h>
#include <string.h>
#include <libwebsockets.h>

/*
Linux Reference:
https://github.com/iamscottmoyers/simple-libwebsockets-example/issues/2
https://stackoverflow.com/questions/11112318/linker-cannot-find-symbols-but-libraries-are-read-and-symbols-exist

Find Path:
pkg-config libwebsockets --libs --cflags

Linux Command:
gcc ws_client.c -o ws_client -I/usr/local/include -L/usr/local/lib -lwebsockets

OSX Command:
gcc ws_client.c -o ws_client -I/opt/homebrew/Cellar/libwebsockets/4.3.2_1/include -L/opt/homebrew/Cellar/libwebsockets/4.3.2_1/lib -lwebsockets -I/opt/homebrew/Cellar/openssl@3/3.1.1_1/include -L/opt/homebrew/Cellar/openssl@3/3.1.1_1/lib -lssl -lcrypto
*/

// Global Context
struct lws_context *context = NULL;
// WebSocket callback functions
static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);

// WebSocket protocols
static struct lws_protocols protocols[] = {
    {
        "my-protocol",           // Protocol name
        websocket_callback,      // Callback function
        0,                       // Per-session data size
        128,                    // Receive buffer size (if protocol has RX extension)
        0,                       // ID number (reserved)
        NULL,                    // User data
        0                        // Attach count
    },
    { NULL, NULL, 0, 0 }  // Terminate the list
};

// WebSocket callback functions
static int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    unsigned char txt[15 + LWS_PRE];
    memcpy(txt + LWS_PRE, "Hello, World!", 14);
    switch (reason)
    {
        case LWS_CALLBACK_CLIENT_ESTABLISHED:
            printf("Connected to WebSocket server\n");
            break;

        case LWS_CALLBACK_CLIENT_RECEIVE:
            printf("Received data: %s\n", (char *)in);
            lws_cancel_service(context);
            lws_close_reason(wsi, LWS_CLOSE_STATUS_NORMAL, NULL, 0);
            return -1;

        case LWS_CALLBACK_CLIENT_WRITEABLE:
            // Send a message to the server
            lws_write(wsi, txt + LWS_PRE, 14, LWS_WRITE_TEXT);
            break;

        case LWS_CALLBACK_CLOSED:
            printf("Disconnected from WebSocket server\n");
            break;

        default:
            break;
    }

    return 0;
}

int main(void)
{
    struct lws_context_creation_info info;
    struct lws_client_connect_info connect_info;
    struct lws *websocket;
    const char *full_address = "ws://localhost:8000";  // Change to your server's address

    // Set up libwebsockets context creation info
    memset(&info, 0, sizeof(info));
    info.port = CONTEXT_PORT_NO_LISTEN;
    info.protocols = protocols;
    info.gid = -1;
    info.uid = -1;

    // Create libwebsockets context
    context = lws_create_context(&info);
    if (!context)
    {
        printf("Failed to create libwebsockets context\n");
        return 1;
    }

    // Set up libwebsockets connection info
    memset(&connect_info, 0, sizeof(connect_info));
    connect_info.context = context;
    connect_info.address = "localhost";
    connect_info.port = 8000;
    connect_info.path = "/";
    // connect_info.host = lws_canonical_hostname(context);
    connect_info.host = full_address;
    connect_info.origin = full_address;
    connect_info.protocol = protocols[0].name;  // Use the first protocol defined
    connect_info.ietf_version_or_minus_one = -1;

    // Connect to the WebSocket server
    websocket = lws_client_connect_via_info(&connect_info);
    if (!websocket)
    {
        printf("Failed to connect to WebSocket server\n");
        lws_context_destroy(context);
        return 1;
    }

    // Event loop
    while (lws_service(context, 0) >= 0)
    {
        // No need to do anything here, the callback functions will handle events
    }

    lws_context_destroy(context);

    return 0;
}
